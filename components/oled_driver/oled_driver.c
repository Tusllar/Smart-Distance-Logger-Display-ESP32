/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: CC0-1.0
 */

#include "oled_driver.h"
#include "esp_log.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ssd1306.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdio.h>
 
 static const char *TAG = "oled_driver";
 
 // OLED Driver structure
 struct oled_driver_t {
     i2c_master_bus_handle_t i2c_bus;
     esp_lcd_panel_io_handle_t io_handle;
     esp_lcd_panel_handle_t panel_handle;
     uint8_t *buffer;
     int width;
     int height;
     int rotation;
     int inverted;
 };
 
 // Simple font data (5x7 pixels)
 static const uint8_t font5x7[96][5] = {
     {0x00, 0x00, 0x00, 0x00, 0x00}, // space
     {0x00, 0x00, 0x5f, 0x00, 0x00}, // !
     {0x00, 0x07, 0x00, 0x07, 0x00}, // "
     {0x14, 0x7f, 0x14, 0x7f, 0x14}, // #
     {0x24, 0x2a, 0x7f, 0x2a, 0x12}, // $
     {0x23, 0x13, 0x08, 0x64, 0x62}, // %
     {0x36, 0x49, 0x55, 0x22, 0x50}, // &
     {0x00, 0x05, 0x03, 0x00, 0x00}, // '
     {0x00, 0x1c, 0x22, 0x41, 0x00}, // (
     {0x00, 0x41, 0x22, 0x1c, 0x00}, // )
     {0x14, 0x08, 0x3e, 0x08, 0x14}, // *
     {0x08, 0x08, 0x3e, 0x08, 0x08}, // +
     {0x00, 0x00, 0xa0, 0x60, 0x00}, // ,
     {0x08, 0x08, 0x08, 0x08, 0x08}, // -
     {0x00, 0x60, 0x60, 0x00, 0x00}, // .
     {0x20, 0x10, 0x08, 0x04, 0x02}, // /
     {0x3e, 0x51, 0x49, 0x45, 0x3e}, // 0
     {0x00, 0x42, 0x7f, 0x40, 0x00}, // 1
     {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
     {0x21, 0x41, 0x45, 0x4b, 0x31}, // 3
     {0x18, 0x14, 0x12, 0x7f, 0x10}, // 4
     {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
     {0x3c, 0x4a, 0x49, 0x49, 0x30}, // 6
     {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
     {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
     {0x06, 0x49, 0x49, 0x29, 0x1e}, // 9
     {0x00, 0x36, 0x36, 0x00, 0x00}, // :
     {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
     {0x08, 0x14, 0x22, 0x41, 0x00}, // <
     {0x14, 0x14, 0x14, 0x14, 0x14}, // =
     {0x00, 0x41, 0x22, 0x14, 0x08}, // >
     {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
     {0x32, 0x49, 0x79, 0x41, 0x3e}, // @
     {0x7e, 0x11, 0x11, 0x11, 0x7e}, // A
     {0x7f, 0x49, 0x49, 0x49, 0x36}, // B
     {0x3e, 0x41, 0x41, 0x41, 0x22}, // C
     {0x7f, 0x41, 0x41, 0x22, 0x1c}, // D
     {0x7f, 0x49, 0x49, 0x49, 0x41}, // E
     {0x7f, 0x09, 0x09, 0x09, 0x01}, // F
     {0x3e, 0x41, 0x49, 0x49, 0x7a}, // G
     {0x7f, 0x08, 0x08, 0x08, 0x7f}, // H
     {0x00, 0x41, 0x7f, 0x41, 0x00}, // I
     {0x20, 0x40, 0x41, 0x3f, 0x01}, // J
     {0x7f, 0x08, 0x14, 0x22, 0x41}, // K
     {0x7f, 0x40, 0x40, 0x40, 0x40}, // L
     {0x7f, 0x02, 0x0c, 0x02, 0x7f}, // M
     {0x7f, 0x04, 0x08, 0x10, 0x7f}, // N
     {0x3e, 0x41, 0x41, 0x41, 0x3e}, // O
     {0x7f, 0x09, 0x09, 0x09, 0x06}, // P
     {0x3e, 0x41, 0x51, 0x21, 0x5e}, // Q
     {0x7f, 0x09, 0x19, 0x29, 0x46}, // R
     {0x46, 0x49, 0x49, 0x49, 0x31}, // S
     {0x01, 0x01, 0x7f, 0x01, 0x01}, // T
     {0x3f, 0x40, 0x40, 0x40, 0x3f}, // U
     {0x1f, 0x20, 0x40, 0x20, 0x1f}, // V
     {0x3f, 0x40, 0x38, 0x40, 0x3f}, // W
     {0x63, 0x14, 0x08, 0x14, 0x63}, // X
     {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
     {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
     {0x00, 0x7f, 0x41, 0x41, 0x00}, // [
     {0x02, 0x04, 0x08, 0x10, 0x20}, // backslash
     {0x00, 0x41, 0x41, 0x7f, 0x00}, // ]
     {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
     {0x40, 0x40, 0x40, 0x40, 0x40}, // _
     {0x00, 0x01, 0x02, 0x04, 0x00}, // `
     {0x20, 0x54, 0x54, 0x54, 0x78}, // a
     {0x7f, 0x48, 0x44, 0x44, 0x38}, // b
     {0x38, 0x44, 0x44, 0x44, 0x20}, // c
     {0x38, 0x44, 0x44, 0x48, 0x7f}, // d
     {0x38, 0x54, 0x54, 0x54, 0x18}, // e
     {0x08, 0x7e, 0x09, 0x01, 0x02}, // f
     {0x18, 0xa4, 0xa4, 0xa4, 0x7c}, // g
     {0x7f, 0x08, 0x04, 0x04, 0x78}, // h
     {0x00, 0x44, 0x7d, 0x40, 0x00}, // i
     {0x40, 0x80, 0x84, 0x7d, 0x00}, // j
     {0x7f, 0x10, 0x28, 0x44, 0x00}, // k
     {0x00, 0x41, 0x7f, 0x40, 0x00}, // l
     {0x7c, 0x04, 0x18, 0x04, 0x78}, // m
     {0x7c, 0x08, 0x04, 0x04, 0x78}, // n
     {0x38, 0x44, 0x44, 0x44, 0x38}, // o
     {0xfc, 0x24, 0x24, 0x24, 0x18}, // p
     {0x18, 0x24, 0x24, 0x18, 0xfc}, // q
     {0x7c, 0x08, 0x04, 0x04, 0x08}, // r
     {0x48, 0x54, 0x54, 0x54, 0x20}, // s
     {0x04, 0x3f, 0x44, 0x40, 0x20}, // t
     {0x3c, 0x40, 0x40, 0x20, 0x7c}, // u
     {0x1c, 0x20, 0x40, 0x20, 0x1c}, // v
     {0x3c, 0x40, 0x30, 0x40, 0x3c}, // w
     {0x44, 0x28, 0x10, 0x28, 0x44}, // x
     {0x1c, 0xa0, 0xa0, 0xa0, 0x7c}, // y
     {0x44, 0x64, 0x54, 0x4c, 0x44}, // z
     {0x00, 0x08, 0x36, 0x41, 0x00}, // {
     {0x00, 0x00, 0x7f, 0x00, 0x00}, // |
     {0x00, 0x41, 0x36, 0x08, 0x00}, // }
     {0x10, 0x08, 0x08, 0x10, 0x08}, // ~
     {0x78, 0x46, 0x41, 0x46, 0x78}  // DEL
 };
 
 // Helper function to get font width
 static int get_font_width(oled_font_t font) {
     switch (font) {
         case OLED_FONT_SMALL: return 5;
         case OLED_FONT_MEDIUM: return 8;
         case OLED_FONT_LARGE: return 12;
         default: return 5;
     }
 }
 
 // Helper function to get font height
 static int get_font_height(oled_font_t font) {
     switch (font) {
         case OLED_FONT_SMALL: return 7;
         case OLED_FONT_MEDIUM: return 10;
         case OLED_FONT_LARGE: return 16;
         default: return 7;
     }
 }
 
 // Helper function to draw a character
 static void draw_char(oled_driver_t *driver, char c, int x, int y, oled_font_t font) {
     if (c < 32 || c > 126) return; // Only printable characters
     
     int font_width = get_font_width(font);
     int font_height = get_font_height(font);
     
     if (font == OLED_FONT_SMALL) {
         // Use 5x7 font
         const uint8_t *char_data = font5x7[c - 32];
         
         for (int i = 0; i < 5; i++) {
             for (int j = 0; j < 7; j++) {
                 if (x + i < driver->width && y + j < driver->height) {
                     if (char_data[i] & (1 << j)) {
                         oled_draw_pixel(driver, x + i, y + j, 1);
                     }
                 }
             }
         }
     } else {
         // For larger fonts, we'll scale the small font
         int scale = (font == OLED_FONT_MEDIUM) ? 2 : 3;
         const uint8_t *char_data = font5x7[c - 32];
         
         for (int i = 0; i < 5; i++) {
             for (int j = 0; j < 7; j++) {
                 if (char_data[i] & (1 << j)) {
                     // Draw scaled pixel
                     for (int si = 0; si < scale; si++) {
                         for (int sj = 0; sj < scale; sj++) {
                             int px = x + i * scale + si;
                             int py = y + j * scale + sj;
                             if (px < driver->width && py < driver->height) {
                                 oled_draw_pixel(driver, px, py, 1);
                             }
                         }
                     }
                 }
             }
         }
     }
 }
 
 esp_err_t oled_init(const oled_config_t *config, oled_driver_t **driver) {
     if (!config || !driver) {
         return ESP_ERR_INVALID_ARG;
     }
 
     // Allocate driver structure
     oled_driver_t *drv = calloc(1, sizeof(oled_driver_t));
     if (!drv) {
         return ESP_ERR_NO_MEM;
     }
 
     drv->width = config->width;
     drv->height = config->height;
     drv->rotation = 0;
     drv->inverted = 0;
 
     // Allocate display buffer
     drv->buffer = calloc(1, config->width * config->height / 8);
     if (!drv->buffer) {
         free(drv);
         return ESP_ERR_NO_MEM;
     }
 
     // Initialize I2C bus
     i2c_master_bus_config_t bus_config = {
         .clk_source = I2C_CLK_SRC_DEFAULT,
         .glitch_ignore_cnt = 7,
         .i2c_port = 0,
         .sda_io_num = config->sda_pin,
         .scl_io_num = config->scl_pin,
         .flags.enable_internal_pullup = true,
     };
     ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &drv->i2c_bus));
 
     // Install panel IO
     esp_lcd_panel_io_i2c_config_t io_config = {
         .dev_addr = config->i2c_addr,
         .scl_speed_hz = config->pixel_clock,
         .control_phase_bytes = 1,
         .lcd_cmd_bits = 8,
         .lcd_param_bits = 8,
         .dc_bit_offset = 6,
     };
     ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(drv->i2c_bus, &io_config, &drv->io_handle));
 
     // Install panel driver
     esp_lcd_panel_dev_config_t panel_config = {
         .bits_per_pixel = 1,
         .reset_gpio_num = config->rst_pin,
     };
     
     esp_lcd_panel_ssd1306_config_t ssd1306_config = {
         .height = config->height,
     };
     panel_config.vendor_config = &ssd1306_config;
     ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(drv->io_handle, &panel_config, &drv->panel_handle));
 
     // Initialize panel
     ESP_ERROR_CHECK(esp_lcd_panel_reset(drv->panel_handle));
     ESP_ERROR_CHECK(esp_lcd_panel_init(drv->panel_handle));
     ESP_ERROR_CHECK(esp_lcd_panel_mirror(drv->panel_handle, true, true));
     ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(drv->panel_handle, true));
 
     *driver = drv;
     ESP_LOGI(TAG, "OLED driver initialized successfully");
     return ESP_OK;
 }
 
 esp_err_t oled_deinit(oled_driver_t *driver) {
     if (!driver) {
         return ESP_ERR_INVALID_ARG;
     }
 
     if (driver->panel_handle) {
         esp_lcd_panel_del(driver->panel_handle);
     }
     if (driver->io_handle) {
         esp_lcd_panel_io_del(driver->io_handle);
     }
     if (driver->i2c_bus) {
         i2c_del_master_bus(driver->i2c_bus);
     }
     if (driver->buffer) {
         free(driver->buffer);
     }
     free(driver);
 
     ESP_LOGI(TAG, "OLED driver deinitialized");
     return ESP_OK;
 }
 
 esp_err_t oled_clear(oled_driver_t *driver) {
     if (!driver) {
         return ESP_ERR_INVALID_ARG;
     }
 
     memset(driver->buffer, 0, driver->width * driver->height / 8);
     return oled_update(driver);
 }
 
 esp_err_t oled_display_text(oled_driver_t *driver, const char *text, int x, int y, 
                            oled_font_t font, oled_align_t align) {
     if (!driver || !text) {
         return ESP_ERR_INVALID_ARG;
     }
 
     int font_width = get_font_width(font);
     int font_height = get_font_height(font);
     int text_width = strlen(text) * font_width;
     
     // Adjust x position based on alignment
     switch (align) {
         case OLED_ALIGN_CENTER:
             x = x - text_width / 2;
             break;
         case OLED_ALIGN_RIGHT:
             x = x - text_width;
             break;
         case OLED_ALIGN_LEFT:
         default:
             break;
     }
 
     // Draw each character
     int char_x = x;
     for (int i = 0; text[i] != '\0'; i++) {
         if (char_x >= 0 && char_x + font_width <= driver->width && 
             y >= 0 && y + font_height <= driver->height) {
             draw_char(driver, text[i], char_x, y, font);
         }
         char_x += font_width;
     }
 
     return oled_update(driver);
 }
 
 esp_err_t oled_display_text_wrap(oled_driver_t *driver, const char *text, int x, int y, 
                                 int max_width, oled_font_t font) {
     if (!driver || !text) {
         return ESP_ERR_INVALID_ARG;
     }
 
     int font_width = get_font_width(font);
     int font_height = get_font_height(font);
     int current_x = x;
     int current_y = y;
     int line_width = 0;
     int max_chars_per_line = max_width / font_width;
 
     for (int i = 0; text[i] != '\0'; i++) {
         if (text[i] == '\n' || line_width >= max_chars_per_line) {
             current_x = x;
             current_y += font_height;
             line_width = 0;
             if (text[i] == '\n') continue;
         }
 
         if (current_x >= 0 && current_x + font_width <= driver->width && 
             current_y >= 0 && current_y + font_height <= driver->height) {
             draw_char(driver, text[i], current_x, current_y, font);
         }
         current_x += font_width;
         line_width++;
     }
 
     return oled_update(driver);
 }
 
 esp_err_t oled_draw_pixel(oled_driver_t *driver, int x, int y, int color) {
     if (!driver || x < 0 || x >= driver->width || y < 0 || y >= driver->height) {
         return ESP_ERR_INVALID_ARG;
     }
 
     int byte_index = (y / 8) * driver->width + x;
     int bit_index = y % 8;
 
     if (color) {
         driver->buffer[byte_index] |= (1 << bit_index);
     } else {
         driver->buffer[byte_index] &= ~(1 << bit_index);
     }
 
     return ESP_OK;
 }
 
 esp_err_t oled_draw_line(oled_driver_t *driver, int x1, int y1, int x2, int y2, int color) {
     if (!driver) {
         return ESP_ERR_INVALID_ARG;
     }
 
     int dx = abs(x2 - x1);
     int dy = abs(y2 - y1);
     int sx = (x1 < x2) ? 1 : -1;
     int sy = (y1 < y2) ? 1 : -1;
     int err = dx - dy;
 
     while (1) {
         oled_draw_pixel(driver, x1, y1, color);
         if (x1 == x2 && y1 == y2) break;
         
         int e2 = 2 * err;
         if (e2 > -dy) {
             err -= dy;
             x1 += sx;
         }
         if (e2 < dx) {
             err += dx;
             y1 += sy;
         }
     }
 
     return ESP_OK;
 }
 
 esp_err_t oled_draw_rectangle(oled_driver_t *driver, int x, int y, int width, int height, 
                              int filled, int color) {
     if (!driver) {
         return ESP_ERR_INVALID_ARG;
     }
 
     if (filled) {
         for (int i = 0; i < height; i++) {
             for (int j = 0; j < width; j++) {
                 oled_draw_pixel(driver, x + j, y + i, color);
             }
         }
     } else {
         // Draw outline
         oled_draw_line(driver, x, y, x + width - 1, y, color);
         oled_draw_line(driver, x, y, x, y + height - 1, color);
         oled_draw_line(driver, x + width - 1, y, x + width - 1, y + height - 1, color);
         oled_draw_line(driver, x, y + height - 1, x + width - 1, y + height - 1, color);
     }
 
     return ESP_OK;
 }
 
 esp_err_t oled_update(oled_driver_t *driver) {
     if (!driver) {
         return ESP_ERR_INVALID_ARG;
     }
 
     return esp_lcd_panel_draw_bitmap(driver->panel_handle, 0, 0, driver->width, driver->height, driver->buffer);
 }
 
 esp_err_t oled_set_brightness(oled_driver_t *driver, uint8_t brightness) {
     if (!driver) {
         return ESP_ERR_INVALID_ARG;
     }
 
     // SSD1306 doesn't have brightness control, but we can simulate it
     // by inverting the display
     return ESP_OK;
 }
 
 esp_err_t oled_invert(oled_driver_t *driver, int invert) {
     if (!driver) {
         return ESP_ERR_INVALID_ARG;
     }
 
     driver->inverted = invert;
     return esp_lcd_panel_invert_color(driver->panel_handle, invert);
 }
 
 esp_err_t oled_rotate(oled_driver_t *driver, int rotation) {
     if (!driver) {
         return ESP_ERR_INVALID_ARG;
     }
 
     driver->rotation = rotation;
     return esp_lcd_panel_swap_xy(driver->panel_handle, rotation == 90 || rotation == 270);
 }
 