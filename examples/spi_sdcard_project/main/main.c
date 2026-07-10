#include <stdio.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "spi_sd.h"

const char *TAG_MAIN = "SPI SDCARD";

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

    spi_init();

    while (spi_sd_init())
    {
        ESP_LOGI(TAG_MAIN, "SD Card Error, Please Check...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGI(TAG_MAIN, "SD Card OK!");
    uint64_t bytes_total = 0, bytes_free = 0;
    sd_get_fatfs_usage(&bytes_total, &bytes_free);

    // 注意：除以 1024 * 1024ULL，加上 ULL 后缀确保常量也是 64 位运算
    ESP_LOGI(TAG_MAIN, "Total: %llu MB", bytes_total / (1024ULL * 1024ULL));
    ESP_LOGI(TAG_MAIN, "Free: %llu MB", bytes_free / (1024ULL * 1024ULL));

    FILE *f = fopen(MOUNT_POINT "/data.txt", "w");
    if (f != NULL)
    {
        fprintf(f, "Hello SDCard on ESP32-S3 ！\n");
        fprintf(f, "这是一条写入SD卡文件的测试数据。\n");
        fclose(f);
    }
    else
    {
        ESP_LOGE(TAG_MAIN, "open file for write failed.");
    }

    f = fopen(MOUNT_POINT "/data.txt", "r");
    if (f != NULL)
    {
        ESP_LOGI(TAG_MAIN, "read from file.");
        char buf[128];
        while (fgets(buf, sizeof(buf), f) != NULL)
        {
            ESP_LOGI(TAG_MAIN, "%s", buf);
        }
        fclose(f);
    }
    else
    {
        ESP_LOGE(TAG_MAIN, "open file for read failed.");
    }

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}