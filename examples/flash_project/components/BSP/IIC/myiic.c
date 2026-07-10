#include "myiic.h"

i2c_master_bus_handle_t bus_handler; /* IIC总线句柄 */

esp_err_t myiic_init(void)
{
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = IIC_NUM_PORT,
        .scl_io_num = IIC_SCL_GPIO_PIN,
        .sda_io_num = IIC_SDA_GPIO_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handler));

    return ESP_OK;
}