#include "uart.h"

QueueHandle_t uart_queue;

void uart_init(uint32_t baudrate)
{
    uart_config_t uart_config = {
        .baud_rate = baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,    
        .rx_flow_ctrl_thresh = 0,              //当.flow_ctrl = UART_HW_FLOWCTRL_DISABLE时 流控禁用时建议设为 0
        .source_clk = UART_SCLK_DEFAULT,       // 显式指定时钟源
    };

    /* 配置uart参数 */
    ESP_ERROR_CHECK(uart_param_config(USART_UX, &uart_config));

    /* 配置uart引脚 */
    ESP_ERROR_CHECK(uart_set_pin(USART_UX, USART_TX_GPIO_PIN, USART_RX_GPIO_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    /* 安装串口驱动 */
    uart_driver_install(USART_UX, RX_BUF_SIZE * 2, RX_BUF_SIZE * 2, 10, &uart_queue, 0);

}