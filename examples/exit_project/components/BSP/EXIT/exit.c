#include "exit.h"
#include "led.h"
#include "esp_log.h"

static void IRAM_ATTR exit_gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    if (gpio_num == EXIT_GPIO_PIN)
    {
        led_toggle();
    }
}

void exit_init(void)
{
    gpio_config_t gpio_init_struct;
    /*引脚为输入模式 */
    gpio_init_struct.mode = GPIO_MODE_INPUT;
    /*使能上拉 */
    gpio_init_struct.pull_up_en = GPIO_PULLUP_ENABLE;
    /*失能下拉 */
    gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE;
    /*使用边沿触发方式*/
    gpio_init_struct.intr_type = GPIO_INTR_NEGEDGE;
    /* BOOT按键引脚 */
    gpio_init_struct.pin_bit_mask = 1ull << EXIT_GPIO_PIN;

    gpio_config(&gpio_init_struct);

    /*注册中断函数
    当传入 0 时，等价于 ESP_INTR_FLAG_DEFAULT，即：
    由系统自动选择合适的中断级别（通常为 Level 1）
    不强制要求 IRAM 驻留
    不要求独占或共享
    适用于绝大多数普通 GPIO 中断场景
    */
    gpio_install_isr_service(0);

    /*分配中断函数*/
    gpio_isr_handler_add(EXIT_GPIO_PIN, exit_gpio_isr_handler, (void *)EXIT_GPIO_PIN);
}