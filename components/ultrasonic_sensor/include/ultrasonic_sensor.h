#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

// Định nghĩa GPIO cho cảm biến siêu âm
#define TRIG_PIN GPIO_NUM_8  // GPIO 6 cho TRIG
#define ECHO_PIN GPIO_NUM_7  // GPIO 7 cho ECHO

// Khởi tạo cảm biến siêu âm
void ultrasonic_init(void);

// Đọc khoảng cách (cm)
float read_ultrasonic_distance(void);

#endif
