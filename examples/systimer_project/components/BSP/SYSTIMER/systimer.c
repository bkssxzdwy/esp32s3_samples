#include "systimer.h"
#include "led.h"

void systimer_callback(void *arg)
{
    led_toggle();
}

void systimer_init(uint64_t tps)
{
    esp_timer_handle_t esp_timer_handle;

    esp_timer_create_args_t esp_timer_create_args = {
        .callback = &systimer_callback,
        .arg = NULL,
    };

    esp_timer_create(&esp_timer_create_args, &esp_timer_handle);

    esp_timer_start_periodic(esp_timer_handle, tps);
}