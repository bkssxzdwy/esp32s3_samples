#ifndef __LCD_H_
#define __LCD_H_

#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "mycolor.h"
#include "xl9555.h"

#define LCD_CS GPIO_NUM_1
#define LCD_DC GPIO_NUM_2
#define LCD_WR GPIO_NUM_42
#define LCD_RD GPIO_NUM_41
#define LCD_RST GPIO_NUM_NC

#define GPIO_LCD_D0 GPIO_NUM_40
#define GPIO_LCD_D1 GPIO_NUM_39
#define GPIO_LCD_D2 GPIO_NUM_38
#define GPIO_LCD_D3 GPIO_NUM_12
#define GPIO_LCD_D4 GPIO_NUM_11
#define GPIO_LCD_D5 GPIO_NUM_10
#define GPIO_LCD_D6 GPIO_NUM_9
#define GPIO_LCD_D7 GPIO_NUM_46

/* LCD信息结构体 */
typedef struct _lcd_device_t
{
    uint16_t width;    /* 宽度 */
    uint16_t height;   /* 高度 */
    uint16_t pwidth;   /* 宽度 */
    uint16_t pheight;  /* 高度 */
    uint8_t direction; /* 横屏还是竖屏控制：0，竖屏；1，横屏。 */
    uint16_t wramcmd;  /* 开始写gram指令 */
    uint16_t setxcmd;  /* 设置x坐标指令 */
    uint16_t setycmd;  /* 设置y坐标指令 */
    uint16_t wr;       /* 命令/数据IO */
    uint16_t cs;       /* 片选IO */
    uint16_t dc;       /* dc */
    uint16_t rd;       /* rd */
} lcd_device_t;

/* lcd配置结构体 */
typedef struct _lcd_config_t
{
    void *user_ctx;                                            /* 回调函数传入参数 */
    esp_lcd_panel_io_color_trans_done_cb_t notify_flush_ready; /* 刷新回调函数 */
} lcd_cfg_t;

void lcd_init(lcd_cfg_t lcd_config);
void lcd_clear(uint16_t color);
void lcd_display_direction(uint8_t direction);
void lcd_backlight(uint8_t on);
                                                          
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);  
void lcd_show_char(uint16_t x, uint16_t y, char chr, uint8_t size, uint8_t mode, uint16_t color);
void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p, uint16_t color);
#endif