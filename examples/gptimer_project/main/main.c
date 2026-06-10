#include <stdio.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led.h"
#include "mygptimer.h"

const char *TAG = "MYGTPTIMER";


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

    led_init();

    led_off();

    mygptimer_init();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}