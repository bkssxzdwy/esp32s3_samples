#ifndef __LED_H_
#define __LED_H_
#include "driver/gpio.h"

/*前面提到LED灯是接在GPIO4这个引脚上*/
#define LED_GPIO_PIN GPIO_NUM_4

/*GPIO4引脚的输出电平状态*/
enum GPIO_OUTPUT_STATE
{
    PIN_RESET, // 低电平
    PIN_SET    // 高电平
};

/*初始LED配置*/
void led_init(void);
/*LED灯亮 */
void led_on(void);
/*LED灯熄灭*/
void led_off(void);
#endif