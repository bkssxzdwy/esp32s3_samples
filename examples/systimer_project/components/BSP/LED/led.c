#include "led.h"
#include "esp_log.h"

void led_init(void)
{
    gpio_config_t io_conf = {0};
    /*禁用引脚中断*/
    io_conf.intr_type = GPIO_INTR_DISABLE;
    /*设置引脚为输入输出模式*/
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << LED_GPIO_PIN;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}
void led_on(void)
{
    int gpio_level = gpio_get_level(LED_GPIO_PIN);
    if (gpio_level)
    {
        gpio_set_level(LED_GPIO_PIN, 0);
    }
}
void led_off(void)
{
    int gpio_level = gpio_get_level(LED_GPIO_PIN);
    if (!gpio_level)
    {
        gpio_set_level(LED_GPIO_PIN, 1);
    }
}
void led_toggle(void)
{
    int gpio_level = gpio_get_level(LED_GPIO_PIN);
    /*0表示亮起，1表示熄灭*/
    if (gpio_level)
    {
        led_on();
    }
    else
    {
        led_off();
    }
}