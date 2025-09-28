#include "ultrasonic_sensor.h"
#include "esp_log.h"

static const char *TAG = "ultrasonic";

void ultrasonic_init(void)
{
    // Cấu hình GPIO cho TRIG (output)
    gpio_config_t trig_config = {
        .pin_bit_mask = (1ULL << TRIG_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&trig_config);
    
    // Cấu hình GPIO cho ECHO (input)
    gpio_config_t echo_config = {
        .pin_bit_mask = (1ULL << ECHO_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&echo_config);
    
    // Khởi tạo TRIG ở mức thấp
    gpio_set_level(TRIG_PIN, 0);
    
    ESP_LOGI(TAG, "Ultrasonic sensor initialized");
}

float read_ultrasonic_distance(void)
{
    uint32_t start_time, end_time;
    float distance;
    
    // Gửi xung TRIG
    gpio_set_level(TRIG_PIN, 1);
    esp_rom_delay_us(10);  // Delay 10 micro giây
    gpio_set_level(TRIG_PIN, 0);
    
    // Đợi ECHO lên cao
    start_time = esp_timer_get_time();
    while (gpio_get_level(ECHO_PIN) == 0) {
        if ((esp_timer_get_time() - start_time) > 10000) {  // Timeout 10ms
            return -1;  // Lỗi timeout
        }
    }
    start_time = esp_timer_get_time();
    
    // Đợi ECHO xuống thấp
    while (gpio_get_level(ECHO_PIN) == 1) {
        if ((esp_timer_get_time() - start_time) > 10000) {  // Timeout 10ms
            return -1;  // Lỗi timeout
        }
    }
    end_time = esp_timer_get_time();
    
    // Tính khoảng cách: (thời gian * tốc độ âm thanh) / 2
    // Tốc độ âm thanh = 340 m/s = 0.034 cm/micro giây
    distance = ((end_time - start_time) * 0.034) / 2;
    
    return distance;
}
