#include <stdio.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "myiic.h"
#include "spi_sd.h"
#include "font_lib.h"
#include "text.h"
#include "key.h"
#include "xl9555.h"

const char *TAG_MAIN = "LCD";

void app_main(void)
{
    esp_err_t ret;
    uint8_t res;
    uint8_t key = 0;

    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
    myiic_init(); /* 初始化IIC */

    xl9555_init();

    spi_init();

    lcd_cfg_t lcd_cfg = {0};
    lcd_cfg.notify_flush_ready = NULL;

    lcd_init(lcd_cfg); /*初始化LCD*/
    lcd_clear(WHITE);
    

    while (spi_sd_init())
    {
        lcd_show_string(10, 10, 320, 16, 16, "SD Card Error, Please Check...", RED);
        vTaskDelay(pdMS_TO_TICKS(1000));
        lcd_clear(WHITE);
    }

    while (font_lib_init())
    {
        lcd_show_string(10, 10, 320, 16, 16, "Font Init Error, Please Check...", RED);
    UPD:
        res = font_lib_update((uint8_t *)"0:");
        if (res)
        {
            lcd_show_string(10, 30, 320, 16, 16, "Font Update Failed!", RED);
        }
        else
        {
            lcd_show_string(10, 30, 320, 16, 16, "Font Update Success!", GREEN);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
        lcd_clear(WHITE);
    }

    text_show_string(10, 10, 320, 12, "你好，欢迎来到中文世界！[12]", 12, 0, RED, WHITE);
    text_show_string(10, 24, 320, 16, "你好，欢迎来到中文世界！[16]", 16, 0, RED, WHITE);
    text_show_string(10, 42, 320, 24, "欢迎来到中文世界！[24]", 24, 0, RED, WHITE);

    while (1)
    {
        vTaskDelay(1);
        key = key_scan();
        if (key == 1)
        {
            goto UPD; /* 跳转到UPD位置（强制更新字库） */
        }
    }
}