#include "mygptimer.h"
#include "esp_attr.h"  
#include "led.h"

bool IRAM_ATTR mygptimer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    led_toggle();

    return false;
}

void mygptimer_init(void)
{    /* 1.创建GPTimer句柄并配置基本参数 */
    gptimer_handle_t gptimer_handle = NULL;
    gptimer_config_t gptimer_config = {
        .clk_src = GPTIMER_CLK_SRC_XTAL,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1 * 1000 , // 期望的计数频率 = 1 kHz（即 1 ms/计数)
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&gptimer_config, &gptimer_handle));

    /* 2.设置报警及自动重装载*/
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = 500,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true};
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer_handle, &alarm_config));

    /* 3.注册中断回调 */
    gptimer_event_callbacks_t event_callback={
        .on_alarm=mygptimer_callback,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer_handle,&event_callback,NULL));

    /* 4.使能定时器*/
    ESP_ERROR_CHECK(gptimer_enable(gptimer_handle));

    /* 5.启动定时器*/
    ESP_ERROR_CHECK(gptimer_start(gptimer_handle));
}