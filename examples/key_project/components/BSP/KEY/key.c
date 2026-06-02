#include "key.h"

void key_init(void)
{
    gpio_config_t gpio_init_struct;
    /*禁用引脚中断*/
    gpio_init_struct.intr_type = GPIO_INTR_DISABLE;
    /*引脚为输入模式 */
    gpio_init_struct.mode = GPIO_MODE_INPUT;
    gpio_init_struct.pull_up_en = GPIO_PULLUP_ENABLE;      /* 使能上拉 */
    gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE; /* 失能下拉 */
    gpio_init_struct.pin_bit_mask = 1ull << BOOT_GPIO_PIN; /* BOOT按键引脚 */
    gpio_config(&gpio_init_struct);
}

/**
 * @brief       按键扫描函数
 * @retval      键值, 定义如下:
 *              BOOT_PRES, 1, BOOT按下
 */
uint8_t key_scan(void)
{
    uint8_t keyval = 0;

    /*读取按键是否按下 */
    int boot_gpio_level = gpio_get_level(BOOT_GPIO_PIN);
    if (!boot_gpio_level)
    {
        /*表示按键按下了 */
        keyval = BOOT_PRES;
        /* 去抖动 */
        vTaskDelay(10);   
    }

    return keyval;
}