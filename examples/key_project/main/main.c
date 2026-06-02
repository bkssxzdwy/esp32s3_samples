#include <stdio.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "key.h"

void app_main(void)
{
    uint8_t key;
    esp_err_t ret;
    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    led_init(); /* 初始化LED */
    key_init(); /* KEY初始化 */
    while (1)
    {
        key = key_scan(); /* 获取键值 */
        switch (key)
        {
        case BOOT_PRES: /* BO被按下 */
        {
            led_on(); /* LED灯亮起 */
            break;
        }
        default:
        {
            led_off(); /* LED灯熄灭 */
            break;
        }
        }
        vTaskDelay(10);
    }
}
