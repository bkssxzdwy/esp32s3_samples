#include <stdio.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "exit.h"

void app_main(void)
{
    esp_err_t ret;
    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    led_init(); /* 初始化LED */
    exit_init(); /* 中断初始化 */
    /*熄灭LED灯 */
    led_off();
    while (1)
    {
        vTaskDelay(10);
    }
}
