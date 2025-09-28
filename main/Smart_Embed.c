/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "oled_driver.h"
#include "ultrasonic_sensor.h"
#include "driver/gpio.h"
#include "http_server_app.h"
#include "esp32c3_wifi.h"
#include "freertos/queue.h"
#include "sd_card_spi.h"

static const char *TAG = "smart_embed";
// Queue for LED control
QueueHandle_t led_queue = NULL;
QueueHandle_t distance_queue = NULL;

// Global variables
static oled_driver_t *g_oled = NULL;
static float g_distance = 0.0f;
static bool g_distance_valid = false;
volatile int g_led_status = 0; // 0: off, 1: on

// LED configuration
#define LED_PIN 2
#define DISTANCE_THRESHOLD 10.0f  // 10cm threshold

// Task handles
static TaskHandle_t display_task_handle = NULL;
static TaskHandle_t sensor_task_handle = NULL;
static TaskHandle_t led_task_handle = NULL;
static TaskHandle_t http_server_task_handle = NULL;
static TaskHandle_t sdcard_task_handle = NULL;
// Task functions
static void sdcard_task(void *pvParameters)
{
    ESP_LOGI(TAG, "SD Card task started");
    while (1) {
        float distance = 0.0f;
        // Nhận dữ liệu từ queue (block tối đa 1 giây)
        if (xQueuePeek(distance_queue, &distance, pdMS_TO_TICKS(1000)) == pdTRUE) {
            long long timestamp = esp_timer_get_time() / 1000; // ms
            if (!sdcard_save_sensor_data(distance, timestamp)) {
                ESP_LOGE(TAG, "Failed to save sensor data to SD card");
            } else {
                ESP_LOGI(TAG, "Saved: %.2f cm, %lld ms", distance, timestamp);
            }
        }
        // Có thể delay nhẹ nếu muốn
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
static void sensor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Sensor task started");
    
    while (1) {
        // Read distance from ultrasonic sensor
        float distance = read_ultrasonic_distance();
        
        // Update global variables
        if (distance > 0 && distance < 400) {  // Valid distance range (2cm - 4m)
            g_distance = distance;
            g_distance_valid = true;
            xQueueSend(distance_queue, &distance, 0); // Gửi dữ liệu vào queue
            ESP_LOGI(TAG, "Distance: %.1f cm", distance);
        } else {
            g_distance = 0.0f;
            g_distance_valid = false;
            ESP_LOGW(TAG, "Distance reading error or out of range");
        }
        
        // Wait 500ms before next reading
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void display_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Display task started");
    
    // Display initial title
    oled_clear(g_oled);
    oled_display_text(g_oled, "Distance Logger", 64, 5, OLED_FONT_SMALL, OLED_ALIGN_CENTER);
    oled_display_text(g_oled, "Smart Embed", 64, 20, OLED_FONT_SMALL, OLED_ALIGN_CENTER);
    oled_draw_line(g_oled, 0, 30, 127, 30, 1);
    oled_update(g_oled);
    
    while (1) {
        // Clear the distance display area
        oled_draw_rectangle(g_oled, 5, 35, 118, 25, 1, 0);  // Clear area with black rectangle
        
        if (g_distance_valid) {
            float distance_q = 0.0f;
            xQueuePeek(distance_queue, &distance_q, portMAX_DELAY);
            // Display distance
            char distance_str[20];
            sprintf(distance_str, "Distance: %.1f cm", distance_q);
            oled_display_text(g_oled, distance_str, 64, 40, OLED_FONT_SMALL, OLED_ALIGN_CENTER);
            
            // Display status
            oled_display_text(g_oled, "Status: OK", 64, 50, OLED_FONT_SMALL, OLED_ALIGN_CENTER);
        } else {
            // Display error
            oled_display_text(g_oled, "Distance: ERROR", 64, 40, OLED_FONT_SMALL, OLED_ALIGN_CENTER);
            oled_display_text(g_oled, "Status: OUT OF RANGE", 64, 55, OLED_FONT_SMALL, OLED_ALIGN_CENTER);
        }
        
        // Update display
        oled_update(g_oled);
        
        // Wait 200ms before next display update
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

static void led_task(void *pvParameters)
{
    ESP_LOGI(TAG, "LED task started");
    
    // Configure LED GPIO
    gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&led_config);
    
    // Initialize LED off
    gpio_set_level(LED_PIN, 0);
    
    while (1) {
        // Check if distance is valid and below threshold
        if (g_distance_valid && g_distance < DISTANCE_THRESHOLD) {
            // Turn LED on
            gpio_set_level(LED_PIN, 1);
            g_led_status = 1;
            ESP_LOGI(TAG, "LED ON - Distance: %.1f cm < %.1f cm", g_distance, DISTANCE_THRESHOLD);
        } else {
            // Turn LED off
            gpio_set_level(LED_PIN, 0);
            g_led_status = 0;
        }
        
        // Check every 100ms for responsive LED control
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void http_server_task(void *pvParameters)
{
    ESP_LOGI(TAG, "HTTP Server task started");
    
    // Wait a bit for WiFi to be ready
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    // Start HTTP server
    start_webserver();
    ESP_LOGI(TAG, "HTTP Server started on http://192.168.4.1");
    
    // Keep task running
    while (1) {
        // Monitor HTTP server status
        vTaskDelay(pdMS_TO_TICKS(30000));  // Check every 30 seconds
        ESP_LOGI(TAG, "HTTP Server running - Distance: %.1f cm, LED: %s", 
                 g_distance, (g_distance_valid && g_distance < DISTANCE_THRESHOLD) ? "ON" : "OFF");
    }
}


void app_main(void)
{
    ESP_LOGI(TAG, "Starting Smart Distance Logger & Display");
    // Create queue for LED control
    led_queue = xQueueCreate(4, sizeof(int)); // Tạo queue cho LED
    distance_queue = xQueueCreate(8, sizeof(float)); // Tạo queue cho dữ liệu khoảng cách
    
    // SD card initialization
    if (!sdcard_init()) {
        ESP_LOGE(TAG, "SD card init failed!");
        // Có thể return hoặc chỉ log lỗi
    }
    // Initialize WiFi AP using component
    wifi_init_sta();

    // Configure OLED
    oled_config_t config = {
        .sda_pin = 3,
        .scl_pin = 0,
        .rst_pin = -1,  // No reset pin
        .i2c_addr = 0x3C,
        .pixel_clock = 400000,
        .width = 128,
        .height = 64
    };

    // Initialize OLED driver
    esp_err_t ret = oled_init(&config, &g_oled);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize OLED: %s", esp_err_to_name(ret));
        return;
    }

    // Initialize ultrasonic sensor
    ultrasonic_init();
    ESP_LOGI(TAG, "Ultrasonic sensor initialized");

    ESP_LOGI(TAG, "System initialized successfully");

    // Create sensor task (higher priority)
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 3, &sensor_task_handle);
    if (sensor_task_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create sensor task");
        return;
    }

    // Create display task (lower priority)
    xTaskCreate(display_task, "display_task", 4096, NULL, 2, &display_task_handle);
    if (display_task_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create display task");
        return;
    }

    // Create LED task (medium priority)
    xTaskCreate(led_task, "led_task", 2048, NULL, 2, &led_task_handle);
    if (led_task_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create LED task");
        return;
    }

    // Create HTTP server task (low priority)
    xTaskCreate(http_server_task, "http_server_task", 8192, NULL, 1, &http_server_task_handle);
    if (http_server_task_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create HTTP server task");
        return;
    }
    // Create SD card task (medium priority)
    xTaskCreate(sdcard_task, "sdcard_task", 4096, NULL, 2, &sdcard_task_handle);
    if (sdcard_task_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create SD card task");
        return;
    }

    ESP_LOGI(TAG, "All tasks created successfully");

    // Main task just waits (or can be used for other purposes)
    while (1) {
        // Monitor system health or handle other tasks
        vTaskDelay(pdMS_TO_TICKS(10000));  // Check every 10 seconds
        
        bool led_status = (g_distance_valid && g_distance < DISTANCE_THRESHOLD);
        ESP_LOGI(TAG, "System running - Distance: %.1f cm, Valid: %s, LED: %s", 
                 g_distance, g_distance_valid ? "Yes" : "No", led_status ? "ON" : "OFF");
    }
}