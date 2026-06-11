#ifndef __SERVO_H_
#define __SERVO_H_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "SERVO";

#define SERVO_PWM_TIMER LEDC_TIMER_1
#define SERVO_PWM_CH0_GPIO GPIO_NUM_5
#define SERVO_CHANNEL LEDC_CHANNEL_1

#define SERVO_PWM_FREQ 50
#define SERVO_PWM_DUTY_RES LEDC_TIMER_13_BIT         // 13位分辨率
#define SERVO_PWM_DUTY_MAX ((1 << 13) - 1) // 8191

// 角度对应的脉冲宽度 (单位: ms)
#define PULSE_0_DEG            0.5f   // 0度
#define PULSE_45_DEG           1.0f   // 45度
#define PULSE_90_DEG           1.5f   // 90度
#define PULSE_135_DEG          2.0f   // 135度
#define PULSE_180_DEG          2.5f   // 180度

// 计算占空比: 占空比 = (脉冲宽度 / 周期) * 分辨率最大值
#define CALC_DUTY(pulse_ms)    (uint32_t)((pulse_ms / 20.0f) * ((1 << 13) - 1))

// 各角度对应的占空比
#define DUTY_0_DEG             CALC_DUTY(PULSE_0_DEG)
#define DUTY_45_DEG            CALC_DUTY(PULSE_45_DEG)
#define DUTY_90_DEG            CALC_DUTY(PULSE_90_DEG)
#define DUTY_135_DEG           CALC_DUTY(PULSE_135_DEG)
#define DUTY_180_DEG           CALC_DUTY(PULSE_180_DEG)

void servo_init();
void servo_set_angle(uint8_t angle);
// 缓慢移动到目标角度 (平滑运动)
void servo_move_smooth(uint8_t target_angle, uint32_t move_time_ms);
#endif