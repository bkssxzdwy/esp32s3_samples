#include <stdio.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led.h"
#include "uart.h"

const char *TAG_UART = "uart";

void app_main(void)
{
    esp_err_t ret;
    uint8_t len = 0;
    uint8_t read_bytes_len = 0;
    uint16_t times = 0;
    unsigned char data[RX_BUF_SIZE] = {0};

    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    uart_init(115200); /*初始UART驱动 */
    while (1)
    {
        uart_get_buffered_data_len(USART_UX, (size_t *)&len); /* 获取环形缓冲区数据长度 */
        if (len > 0)
        {
            memset(data, 0, RX_BUF_SIZE);
            read_bytes_len = uart_read_bytes(USART_UX, data, len, 100);
            ESP_LOGI(TAG_UART, "uart_read_bytes=%d", len);
            if (read_bytes_len > 0)
            {
                ESP_LOGI(TAG_UART, "Message received:%s", data);
                uart_write_bytes(USART_UX, (const char *)data, read_bytes_len);
            }
            else
            {
                ESP_LOGI(TAG_UART, "Message read error（%d)", read_bytes_len);
            }
        }
        else
        {
            times++;
            if (times % 200 == 0)
            {
                ESP_LOGI(TAG_UART, "Awaiting message");
            }

            if (times % 30 == 0)
            {
                led_toggle();
            }
        }
        vTaskDelay(10);
    }
}
