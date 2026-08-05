#ifndef __TEXT_H
#define __TEXT_H

#include "font_lib.h"
#include "esp_partition.h"
#include "spi_flash_mmap.h"
#include "esp_log.h"
#include "ff.h"
#include "lcd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void text_show_char(uint16_t x, uint16_t y, uint8_t *gbk_code, uint8_t size, uint8_t mode, uint32_t color, uint32_t g_back_color);                            /* 显示汉字 */
void text_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *str, uint8_t size, uint8_t mode, uint32_t color, uint32_t g_back_color); /* 显示汉字字符串(UTF-8输入) */

#endif