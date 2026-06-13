#include "led.h"

void led_init(void)
{
    gpio_config_t io_conf = {0};

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << LED_GPIO_PIN;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    gpio_config(&io_conf);
}

void led_toggle(void)
{
    gpio_set_level(LED_GPIO_PIN, !gpio_get_level(LED_GPIO_PIN));
}