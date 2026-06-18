#include "pwm.h"

void pwm_init()
{
    ledc_timer_config_t timer_config = {
        .duty_resolution = LEDC_PWM_DUTY_RES,
        .freq_hz = LEDC_PWM_FREQ,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_PWM_TIMER,
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .gpio_num = LEDC_PWM_CH0_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_PWM_CH0_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_PWM_TIMER,
        .duty = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));

     ledc_fade_func_install(0);
}

void pwm_set_duty(uint16_t duty)
{
    uint16_t real_duty = LEDC_PWM_DUTY_MAX - duty;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_PWM_CH0_CHANNEL, real_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_PWM_CH0_CHANNEL);
}

void pwm_set_duty_fade(uint16_t duty, uint32_t fade_time_ms)
{
    uint16_t real_duty = LEDC_PWM_DUTY_MAX - duty;
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_PWM_CH0_CHANNEL, real_duty, fade_time_ms);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_PWM_CH0_CHANNEL, LEDC_FADE_NO_WAIT);
}