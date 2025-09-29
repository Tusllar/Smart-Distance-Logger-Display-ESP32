#include "http_server_app.h"

/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

// #include <string.h>
// #include <stdlib.h>
// #include <unistd.h>
#include <esp_log.h>
#include <stdbool.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "ultrasonic_sensor.h"
#define EXAMPLE_HTTP_QUERY_KEY_MAX_LEN  (64)

#define MOUNT_POINT "/sdcard"

/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */

static const char *TAG = "example";
static httpd_handle_t server = NULL;
#define LED_PIN 2
extern volatile int g_led_status;
extern QueueHandle_t distance_queue;
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");
// Read-only access to latest distance without consuming queue
extern float g_distance;
extern bool g_distance_valid;


static esp_err_t sensor_history_handler(httpd_req_t *req)
{
    char path[64];
    snprintf(path, sizeof(path), "%s/sensor.csv", MOUNT_POINT);
    FILE *f = fopen(path, "r");
    if (!f) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr_chunk(req, "[");   // mở mảng JSON

    char line[128];
    bool first = true;
    while (fgets(line, sizeof(line), f)) {
        float distance;
        long long timestamp;
        if (sscanf(line, "%f,%lld", &distance, &timestamp) == 2) {
            if (!first) httpd_resp_sendstr_chunk(req, ",");
            char item[64];
            snprintf(item, sizeof(item),
                     "{\"distance\":%.2f,\"timestamp\":%lld}",
                     distance, timestamp);
            httpd_resp_sendstr_chunk(req, item);
            first = false;
        }
    }
    httpd_resp_sendstr_chunk(req, "]");   // đóng mảng
    httpd_resp_sendstr_chunk(req, NULL);  // kết thúc chunked

    fclose(f);
    return ESP_OK;
}


static const httpd_uri_t sensor_history = {
    .uri       = "/sensor/history",
    .method    = HTTP_GET,
    .handler   = sensor_history_handler,
    .user_ctx  = NULL
};
/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    /* Send response with custom headers and body set as the
     * string passed in user context*/
    // const char* html_content = "<html><body><h1>ESP32 LED Control</h1><button>ON</button><button>OFF</button></body></html>";
    // httpd_resp_set_type(req, "text/html");
    // httpd_resp_send(req, html_content, strlen(html_content));
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
    // const char* resp_str = (const char*) req->user_ctx;
    // httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

// Serve index.html at root path
static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}

static const httpd_uri_t root = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_get_handler,
    .user_ctx  = NULL
};

// Handler phục vụ file tĩnh (CSS, JS, ảnh)
esp_err_t static_file_handler(httpd_req_t *req)
{
    const char *filepath = NULL;
    if (strcmp(req->uri, "/styles.css") == 0) {
        filepath = "/spiffs/styles.css"; // hoặc "/main/styles.css" nếu dùng VFS
    }
    // Thêm các file khác nếu cần

    if (filepath) {
        FILE *f = fopen(filepath, "r");
        if (!f) {
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        httpd_resp_set_type(req, "text/css");
        char buf[512];
        size_t read_bytes;
        while ((read_bytes = fread(buf, 1, sizeof(buf), f)) > 0) {
            httpd_resp_send_chunk(req, buf, read_bytes);
        }
        fclose(f);
        httpd_resp_send_chunk(req, NULL, 0);
        return ESP_OK;
    }
    httpd_resp_send_404(req);
    return ESP_FAIL;
}

// Đăng ký URI cho file tĩnh
void register_static_files(httpd_handle_t server)
{
    httpd_uri_t css_uri = {
        .uri      = "/styles.css",
        .method   = HTTP_GET,
        .handler  = static_file_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &css_uri);
}

/* An HTTP POST handler */
static esp_err_t echo_post_handler(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        /* Log data received */
        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        ESP_LOGI(TAG, "====================================");
    }

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t echo = {
    .uri       = "/echo",
    .method    = HTTP_POST,
    .handler   = echo_post_handler,
    .user_ctx  = NULL
};

/* An HTTP_ANY handler */
static esp_err_t any_handler(httpd_req_t *req)
{
    /* Send response with body set as the
     * string passed in user context*/
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t any = {
    .uri       = "/any",
    .method    = HTTP_ANY,
    .handler   = any_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "Hello World!"
};

/* LED control handler */
static esp_err_t led_control_handler(httpd_req_t *req)
{
    char query[100];
    char resp[100];
    
    // Get query string
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        char param[32];
        if (httpd_query_key_value(query, "state", param, sizeof(param)) == ESP_OK) {
            ESP_LOGI(TAG, "LED control request: %s", param);
            
            if (strcmp(param, "on") == 0) {
                // Turn on LED (GPIO 1)
                gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
                gpio_set_level(LED_PIN, 1);
                snprintf(resp, sizeof(resp), "{\"status\":\"success\",\"led\":\"on\"}");
            } else if (strcmp(param, "off") == 0) {
                // Turn off LED (GPIO 1)
                gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
                gpio_set_level(LED_PIN, 0);
                snprintf(resp, sizeof(resp), "{\"status\":\"success\",\"led\":\"off\"}");
            } else {
                snprintf(resp, sizeof(resp), "{\"status\":\"error\",\"message\":\"Invalid state\"}");
            }
        } else {
            snprintf(resp, sizeof(resp), "{\"status\":\"error\",\"message\":\"Missing state parameter\"}");
        }
    } else {
        snprintf(resp, sizeof(resp), "{\"status\":\"error\",\"message\":\"Invalid request\"}");
    }
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, strlen(resp));
    
    return ESP_OK;
}

static const httpd_uri_t led_control = {
    .uri       = "/led",
    .method    = HTTP_GET,
    .handler   = led_control_handler,
    .user_ctx  = NULL
};

/* LED status handler - để kiểm tra trạng thái LED */
static esp_err_t led_status_handler(httpd_req_t *req)
{
    // char resp[100];
    
    // // Đọc trạng thái LED từ GPIO 1 (theo main code)
    // int led_level = gpio_get_level(LED_PIN);
    
    // ESP_LOGI(TAG, "LED status check - GPIO level: %d", led_level);
    
    // if (led_level == 1) {
    //     snprintf(resp, sizeof(resp), "{\"status\":\"success\",\"led\":\"on\"}");
    // } else {
    //     snprintf(resp, sizeof(resp), "{\"status\":\"success\",\"led\":\"off\"}");
    // }
    char resp[100];
    
    if (g_led_status == 1) {
        snprintf(resp, sizeof(resp), "{\"status\":\"success\",\"led\":\"on\"}");
    } else {
        snprintf(resp, sizeof(resp), "{\"status\":\"success\",\"led\":\"off\"}");
    }
    
    ESP_LOGI(TAG, "LED status response: %s", resp);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, strlen(resp));
    
    return ESP_OK;
}

static const httpd_uri_t led_status = {
    .uri       = "/led/status",
    .method    = HTTP_GET,
    .handler   = led_status_handler,
    .user_ctx  = NULL
};

/* Cảm biến siêu âm handler */
static esp_err_t ultrasonic_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Ultrasonic API called");
    float distance = g_distance;
    // Không tiêu thụ queue để tránh mất dữ liệu lịch sử
    float peek_value = 0.0f;
    if (xQueueReceive(distance_queue, &peek_value, 0) == pdTRUE) {
        distance = peek_value;
    }
    // Đọc khoảng cách từ cảm biến siêu âm
    // float distance = read_ultrasonic_distance();
    
    ESP_LOGI(TAG, "Distance read: %.2f cm", distance);
    
    char resp[128];
    const char *led_str = (g_led_status == 1) ? "on" : "off";
    snprintf(resp, sizeof(resp), "{\"distance\":%.2f,\"timestamp\":%lld,\"led\":\"%s\"}",
             distance, esp_timer_get_time() / 1000, led_str);
    
    ESP_LOGI(TAG, "Sending response: %s", resp);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, strlen(resp));
    
    return ESP_OK;
}

static const httpd_uri_t ultrasonic = {
    .uri       = "/ultrasonic",
    .method    = HTTP_GET,
    .handler   = ultrasonic_handler,
    .user_ctx  = NULL
};

/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/hello", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    } else if (strcmp("/echo", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

static esp_err_t ctrl_put_handler(httpd_req_t *req)
{
    char buf;
    int ret;

    if ((ret = httpd_req_recv(req, &buf, 1)) <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    if (buf == '0') {
        /* URI handlers can be unregistered using the uri string */
        ESP_LOGI(TAG, "Unregistering /hello and /echo URIs");
        httpd_unregister_uri(req->handle, "/hello");
        httpd_unregister_uri(req->handle, "/echo");
        /* Register the custom error handler */
        httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, http_404_error_handler);
    }
    else {
        ESP_LOGI(TAG, "Registering /hello and /echo URIs");
        httpd_register_uri_handler(req->handle, &hello);
        httpd_register_uri_handler(req->handle, &echo);
        /* Unregister custom error handler */
        httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, NULL);
    }

    /* Respond with empty body */
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t ctrl = {
    .uri       = "/ctrl",
    .method    = HTTP_PUT,
    .handler   = ctrl_put_handler,
    .user_ctx  = NULL
};

void start_webserver(void)
{
    // Use the global server handle
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &echo);
        httpd_register_uri_handler(server, &ctrl);
        httpd_register_uri_handler(server, &any);
        httpd_register_uri_handler(server, &led_control);
        httpd_register_uri_handler(server, &led_status);  // Thêm endpoint LED status
        httpd_register_uri_handler(server, &ultrasonic);  // Thêm endpoint mới
        httpd_register_uri_handler(server, &sensor_history);
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);

    }else{
        ESP_LOGI(TAG, "Error starting server!");
    }



}

void stop_webserver(void)
{
    // Stop the httpd server
    httpd_stop(server);
}


