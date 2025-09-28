#pragma once

#include <stdbool.h>

bool sdcard_init(void);
bool sdcard_save_sensor_data(float distance, long long timestamp);