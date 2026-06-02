#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/ledc.h"
#include "esp_err.h"

#define SERVO_GPIO GPIO_NUM_5

#define SERVO_FREQ_HZ 50
#define SERVO_PERIOD_US 20000

#define LEDC_TIMER_NUM LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL_NUM LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_14_BIT

/*
 * 360°连续旋转舵机:
 *
 * 1500us -> 停止
 * >1500us -> 顺时针
 * <1500us -> 逆时针
 */

static void servo_set_pulse_us(uint32_t pulse_us)
{
    uint32_t max_duty = (1 << 14);

    uint32_t duty =
        (pulse_us * max_duty) /
        SERVO_PERIOD_US;

    ESP_ERROR_CHECK(
        ledc_set_duty(
            LEDC_MODE,
            LEDC_CHANNEL_NUM,
            duty));

    ESP_ERROR_CHECK(
        ledc_update_duty(
            LEDC_MODE,
            LEDC_CHANNEL_NUM));
}

static void servo_stop(void)
{
    servo_set_pulse_us(1500);
}

static void servo_cw(uint32_t speed)
{
    /*
     * speed:
     * 0~500
     *
     * 1500~2000us
     */

    if (speed > 500)
        speed = 500;

    servo_set_pulse_us(1500 + speed);
}

static void servo_ccw(uint32_t speed)
{
    /*
     * speed:
     * 0~500
     *
     * 1500~1000us
     */

    if (speed > 500)
        speed = 500;

    servo_set_pulse_us(1500 - speed);
}

static void servo_init(void)
{
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER_NUM,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = SERVO_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    ledc_channel_config_t channel_conf = {
        .gpio_num = SERVO_GPIO,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL_NUM,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_NUM,
        .duty = 0,
        .hpoint = 0,
    };

    ESP_ERROR_CHECK(ledc_channel_config(&channel_conf));

    servo_stop();
}

void app_main(void)
{
    servo_init();

    while (1)
    {
        printf("CW FAST\n");

        servo_cw(200);

        vTaskDelay(pdMS_TO_TICKS(3000));

        printf("STOP\n");

        servo_stop();

        vTaskDelay(pdMS_TO_TICKS(1000));

        printf("CCW FAST\n");

        servo_ccw(200);

        vTaskDelay(pdMS_TO_TICKS(3000));

        printf("STOP\n");

        servo_stop();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}