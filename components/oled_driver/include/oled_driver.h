/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: CC0-1.0
 */

 #pragma once

 #include "esp_err.h"
 #include "esp_lcd_panel_io.h"
 #include "esp_lcd_panel_ops.h"
 #include "driver/i2c_master.h"
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 // OLED Driver Configuration
 typedef struct {
     int sda_pin;           // SDA pin number
     int scl_pin;           // SCL pin number
     int rst_pin;           // Reset pin number (-1 if not used)
     uint8_t i2c_addr;      // I2C address (default: 0x3C)
     uint32_t pixel_clock;  // Pixel clock frequency (default: 400kHz)
     int width;             // Display width
     int height;            // Display height
 } oled_config_t;
 
 // Text alignment options
 typedef enum {
     OLED_ALIGN_LEFT,
     OLED_ALIGN_CENTER,
     OLED_ALIGN_RIGHT
 } oled_align_t;
 
 // Font size options
 typedef enum {
     OLED_FONT_SMALL,
     OLED_FONT_MEDIUM,
     OLED_FONT_LARGE
 } oled_font_t;
 
 // OLED Driver Handle
 typedef struct oled_driver_t oled_driver_t;
 
 /**
  * @brief Initialize OLED driver
  * 
  * @param config Configuration structure
  * @param driver Pointer to store driver handle
  * @return esp_err_t ESP_OK on success
  */
 esp_err_t oled_init(const oled_config_t *config, oled_driver_t **driver);
 
 /**
  * @brief Deinitialize OLED driver
  * 
  * @param driver Driver handle
  * @return esp_err_t ESP_OK on success
  */
 esp_err_t oled_deinit(oled_driver_t *driver);
 
 /**
  * @brief Clear the entire display
  * 
  * @param driver Driver handle
  * @return esp_err_t ESP_OK on success
  */
 esp_err_t oled_clear(oled_driver_t *driver);
 
 /**
  * @brief Display text at specified position
  * 
  * @param driver Driver handle
  * @param text Text to display
  * @param x X position (0 = leftmost)
  * @param y Y position (0 = topmost)
  * @param font Font size
  * @param align Text alignment
  * @return esp_err_t ESP_OK on success
  */
 esp_err_t oled_display_text(oled_driver_t *driver, const char *text, int x, int y, 
                            oled_font_t font, oled_align_t align);
 
 /**
  * @brief Display text with auto line wrapping
  * 
  * @param driver Driver handle
  * @param text Text to display
  * @param x X position
  * @param y Y position
  * @param max_width Maximum width for text wrapping
  * @param font Font size
  * @return esp_err_t ESP_OK on success
  */
 esp_err_t oled_display_text_wrap(oled_driver_t *driver, const char *text, int x, int y, 
                                 int max_width, oled_font_t font);
 
 /**
  * @brief Draw a pixel
  * 
  * @param driver Driver handle
  * @param x X position
  * @param y Y position
  * @param color 1 for white, 0 for black
  * @return esp_err_t ESP_OK on success
  */
 esp_err_t oled_draw_pixel(oled_driver_t *driver, int x, int y, int color);
 
 /**
  * @brief Draw a line
  * 
  * @param driver Driver handle
  * @param x1 Start X position
  * @param y1 Start Y position
  * @param x2 End X position
  * @param y2 End Y position
  * @param color 1 for white, 0 for black
  * @return esp_err_t ESP_OK on success
  */
 esp_err_t oled_draw_line(oled_driver_t *driver, int x1, int y1, int x2, int y2, int color);
 
 /**
  * @brief Draw a rectangle
  * 
  * @param driver Driver handle
  * @param x X position
  * @param y Y position
  * @param width Rectangle width
  * @param height Rectangle height
  * @param filled 1 for filled, 0 for outline only
  * @param color 1 for white, 0 for black
  * @return esp_err_t ESP_OK on success
  */
 esp_err_t oled_draw_rectangle(oled_driver_t *driver, int x, int y, int width, int height, 
                              int filled, int color);
 
 /**
  * @brief Update display (send buffer to screen)
  * 
  * @param driver Driver handle
  * @return esp_err_t ESP_OK on success
  */
 esp_err_t oled_update(oled_driver_t *driver);
 
 /**
  * @brief Set display brightness
  * 
  * @param driver Driver handle
  * @param brightness Brightness level (0-255)
  * @return esp_err_t ESP_OK on success
  */
 esp_err_t oled_set_brightness(oled_driver_t *driver, uint8_t brightness);
 
 /**
  * @brief Invert display colors
  * 
  * @param driver Driver handle
  * @param invert 1 to invert, 0 for normal
  * @return esp_err_t ESP_OK on success
  */
 esp_err_t oled_invert(oled_driver_t *driver, int invert);
 
 /**
  * @brief Rotate display
  * 
  * @param driver Driver handle
  * @param rotation 0, 90, 180, or 270 degrees
  * @return esp_err_t ESP_OK on success
  */
 esp_err_t oled_rotate(oled_driver_t *driver, int rotation);
 
 #ifdef __cplusplus
 }
 #endif
 