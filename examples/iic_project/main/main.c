#include <stdio.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "pwm.h"

const char *TAG_MAIN = "PWM";

void app_main(void)
{
    esp_err_t ret;

    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    pwm_init();

    while (1)
    {
        // 软件方式
        // // 渐亮
        // for (int duty = 0; duty <= LEDC_PWM_DUTY_MAX; duty += 100)
        // {
        //     pwm_set_duty(duty);
        //     vTaskDelay(pdMS_TO_TICKS(20));
        // }
        // // 渐灭
        // for (int duty = LEDC_PWM_DUTY_MAX; duty >= 0; duty -= 100)
        // {
        //     pwm_set_duty(duty);
        //     vTaskDelay(pdMS_TO_TICKS(20));
        // }

        
        // 硬件方式
        // 渐亮：2秒内从灭到最亮
        pwm_set_duty_fade(LEDC_PWM_DUTY_MAX, 2000);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // 渐灭：2秒内从最亮到灭
        pwm_set_duty_fade(0, 2000);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}