
#ifndef __LED_H_
#define __LED_H_

#include "driver/gpio.h"

#define LED_GPIO_PIN GPIO_NUM_4

enum GPIO_OUTPUT_STATE
{
    PIN_RESET,
    PIN_SET
};

void led_init(void);

void led_toggle(void);

#endif