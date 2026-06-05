#ifndef __EXIT_H_
#define __EXIT_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

/* 引脚定义 丝印为K0按键（BOOT）另一端连接在ATK-MWS3S模组的IO0上*/
#define EXIT_GPIO_PIN   GPIO_NUM_0

void exit_init(void);

#endif