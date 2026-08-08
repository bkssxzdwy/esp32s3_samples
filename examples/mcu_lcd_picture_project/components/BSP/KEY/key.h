#ifndef __KEY_H_
#define __KEY_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

/*IO操作*/
#define BOOT            gpio_get_level(BOOT_GPIO_PIN)

/* 引脚定义 丝印为K0按键（BOOT）另一端连接在ATK-MWS3S模组的IO0上*/
#define BOOT_GPIO_PIN   GPIO_NUM_0

/* 按键按下定义 */
#define BOOT_PRES       1       /* BOOT按键按下 */

void key_init(void);
uint8_t key_scan(void);

#endif