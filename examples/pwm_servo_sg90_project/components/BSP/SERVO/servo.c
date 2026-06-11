#include "servo.h"

void servo_init()
{
    ledc_timer_config_t timer_config = {
        .duty_resolution = SERVO_PWM_DUTY_RES,
        .freq_hz = SERVO_PWM_FREQ,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = SERVO_PWM_TIMER,
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .gpio_num = SERVO_PWM_CH0_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = SERVO_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = SERVO_PWM_TIMER,
        .duty = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}

void servo_set_angle(uint8_t angle)
{

    uint32_t duty;

    // 线性插值计算占空比
    if (angle <= 0)
    {
        duty = DUTY_0_DEG;
    }
    else if (angle >= 180)
    {
        duty = DUTY_180_DEG;
    }
    else
    {
        // 公式: duty = DUTY_0_DEG + (angle / 180.0) * (DUTY_180_DEG - DUTY_0_DEG)
        duty = DUTY_0_DEG + (uint32_t)((angle / 180.0f) * (DUTY_180_DEG - DUTY_0_DEG));
    }

    ledc_set_duty(LEDC_LOW_SPEED_MODE, SERVO_CHANNEL, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, SERVO_CHANNEL);

    //ESP_LOGI(TAG, "Set angle: %d°, duty: %lu", angle, duty);
}

void servo_move_smooth(uint8_t target_angle, uint32_t move_time_ms)
{
    ESP_LOGI(TAG, "Move smooth target angle: %d°, move_time_ms: %lu", target_angle, move_time_ms);
    static uint8_t current_angle = 0; // 默认初始位置0度
    uint8_t step = (target_angle > current_angle) ? 1 : -1;
    uint32_t step_delay_ms = move_time_ms / abs(target_angle - current_angle);

    for (uint8_t angle = current_angle; angle != target_angle; angle += step)
    {
        servo_set_angle(angle);
        vTaskDelay(pdMS_TO_TICKS(step_delay_ms));
    }
    servo_set_angle(target_angle);
    current_angle = target_angle;
}