#include <stdio.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "xl9555.h"
#include "myiic.h"
#include "lcd.h"

const char *TAG_MAIN = "LCD";

static SemaphoreHandle_t s_gpio_sem = NULL;

const uint16_t b_colors[] = {
    WHITE, BLACK, RED, GREEN,
    BLUE, MAGENTA, YELLOW, CYAN,
    BROWN, BRRED, GRAY, DARKBLUE,
    LIGHTBLUE, GRAYBLUE, LIGHTGREEN, LGRAY,
    LGRAYBLUE, LBBLUE};
const uint16_t COLOR_COUNT = 18;

int color_index = 0;

void gpio_task(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(s_gpio_sem, portMAX_DELAY) == pdTRUE)
        {
            esp_rom_delay_us(20000);
            if (gpio_get_level(XL9555_INT_IO) == 0)
            {
                int key0 = xl9555_pin_read(KEY0_IO);
                int key1 = xl9555_pin_read(KEY1_IO);
                ESP_LOGI(TAG_MAIN, "key0=%d,key1=%d", key0, key1);
                if (key0 == 0)
                {
                    xl9555_pin_write(LEDR_IO, 1);
                    // xl9555_pin_write(BEEP_IO, 1);
                    color_index--;
                    if (color_index < 0)
                    {
                        color_index = COLOR_COUNT - 1;
                    }
                    lcd_clear(b_colors[color_index]);
                }
                else if (key1 == 0)
                {
                    xl9555_pin_write(LEDR_IO, 0);
                    // xl9555_pin_write(BEEP_IO, 0);
                    color_index++;
                    if (color_index >= COLOR_COUNT)
                    {
                        color_index = 0;
                    }
                    lcd_clear(b_colors[color_index]);
                }
            }
        }
    }
}

static void IRAM_ATTR xl9555_exit_gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    if (gpio_num == XL9555_INT_IO)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(s_gpio_sem, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

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
    myiic_init();  /* 初始化IIC */
    xl9555_init(); /* 初始化XL9555 */

    gpio_config_t gpio_init_struct;

    /* 配置XL9555器件的INT中断引脚 */
    gpio_init_struct.mode = GPIO_MODE_INPUT;               /* 选择为输入模式 */
    gpio_init_struct.pull_up_en = GPIO_PULLUP_ENABLE;      /* 上拉使能 */
    gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE; /* 下拉失能 */
    gpio_init_struct.intr_type = GPIO_INTR_NEGEDGE;        /* 下降沿触发 */
    gpio_init_struct.pin_bit_mask = 1ull << XL9555_INT_IO; /* 设置的引脚的位掩码 */
    gpio_config(&gpio_init_struct);                        /* 配置使能 */

    /* 注册中断服务 */
    gpio_install_isr_service(0);

    /* 设置GPIO的中断回调函数 */
    gpio_isr_handler_add(XL9555_INT_IO, xl9555_exit_gpio_isr_handler, (void *)XL9555_INT_IO);

    s_gpio_sem = xSemaphoreCreateBinary();
    xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 5, NULL);

    lcd_cfg_t lcd_cfg = {0};
    lcd_cfg.notify_flush_ready = NULL;

    lcd_init(lcd_cfg); /*初始化LCD*/

    color_index = 0;
    lcd_clear(b_colors[color_index]);

    lcd_show_string(10, 10, 240, 32, 32, "Hello World", RED);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}