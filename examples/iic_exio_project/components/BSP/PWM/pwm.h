#ifndef __SW_PWM_H_
#define __SW_PWM_H_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/ledc.h"
#include "driver/gpio.h"

#define LEDC_PWM_TIMER LEDC_TIMER_1
#define LEDC_PWM_CH0_GPIO GPIO_NUM_4
#define LEDC_PWM_CH0_CHANNEL LEDC_CHANNEL_1

#define LEDC_PWM_FREQ 5000
#define LEDC_PWM_DUTY_RES LEDC_TIMER_13_BIT         // 13位分辨率
#define LEDC_PWM_DUTY_MAX ((1 << 13) - 1) // 8191

void pwm_init();
void pwm_set_duty(uint16_t duty);
void pwm_set_duty_fade(uint16_t duty, uint32_t fade_time_ms);

#endif