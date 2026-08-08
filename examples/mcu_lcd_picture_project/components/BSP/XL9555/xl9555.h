#ifndef __XL9555_H_
#define __XL9555_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "myiic.h"
#include "string.h"
#include "esp_log.h"

#define KEY0                        xl9555_pin_read(KEY0_IO)        /* 读取K1引脚 */
#define KEY1                        xl9555_pin_read(KEY1_IO)        /* 读取K2引脚 */

#define KEY0_PRES                   2                               /* K1按下 */
#define KEY1_PRES                   3                               /* K2按下 */

#define XL9555_INT_IO               GPIO_NUM_3                      /* XL9555_INT 中断引脚 */

/* XL9555寄存器 */
#define XL9555_INPUT_PORT0_REG 0     /* 输入寄存器0地址 */
#define XL9555_INPUT_PORT1_REG 1     /* 输入寄存器1地址 */
#define XL9555_OUTPUT_PORT0_REG 2    /* 输出寄存器0地址 */
#define XL9555_OUTPUT_PORT1_REG 3    /* 输出寄存器1地址 */
#define XL9555_INVERSION_PORT0_REG 4 /* 极性反转寄存器0地址 */
#define XL9555_INVERSION_PORT1_REG 5 /* 极性反转寄存器1地址 */
#define XL9555_CONFIG_PORT0_REG 6    /* 方向配置寄存器0地址 */
#define XL9555_CONFIG_PORT1_REG 7    /* 方向配置寄存器1地址 */

/* XL9555 IIC地址*/
#define XL9555_ADDR 0X20 /* XL9555地址(左移了一位) */

/* XL9555各个IO的功能 */
#define AP_INT_IO 0x0001
#define QMA_INT_IO 0x0002
#define BEEP_IO 0x0004
#define KEY1_IO 0x0008
#define KEY0_IO 0x0010
#define SPK_CTRL_IO 0x0020
#define CTP_RST_IO 0x0040
#define LCD_BL_IO 0x0080
#define LEDR_IO 0x0100
#define CTP_INT_IO 0x0200
#define IO1_2 0x0400
#define IO1_3 0x0800
#define IO1_4 0x1000
#define IO1_5 0x2000
#define IO1_6 0x4000
#define IO1_7 0x8000

esp_err_t xl9555_init(void);

void xl9555_ioconfig();

int xl9555_pin_read(uint16_t pin);

uint16_t xl9555_pin_write(uint16_t pin, int val);

esp_err_t xl9555_register_read(uint8_t reg_addr, uint8_t *data, size_t len);                  

esp_err_t xl9555_register_write(uint8_t reg_addr, uint8_t *data, size_t len); 

uint8_t xl9555_key_scan(uint8_t mode);                                  /* 扫描扩展按键 */

#endif