#include "at24c02.h"

i2c_master_dev_handle_t eeprom_handle = NULL;

/**
 * @brief       初始化AT24C02
 * @param       无
 * @retval      ESP_OK:初始化成功
 */
esp_err_t at24c02_init(void)
{
    i2c_device_config_t eeprom_i2c_device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = AT_ADDR,
        .scl_speed_hz = IIC_SPEED_CLK,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handler, &eeprom_i2c_device_config, &eeprom_handle));

    return ESP_OK;
}

/**
 * @brief       在AT24C02指定地址读出一个数据
 * @param       addr: 开始读数的地址
 * @retval      读到的数据
 */
uint8_t at24c02_read_one_byte(uint8_t addr)
{
    uint8_t data = 0;

    ESP_ERROR_CHECK(i2c_master_transmit_receive(eeprom_handle, &addr, 1, &data, 1, -1));

    return data;
}

/**
 * @brief       在AT24C02指定地址写入一个数据
 * @param       addr: 写入数据的目的地址
 * @param       data: 要写入的数据
 * @retval      无
 */
void at24c02_write_one_byte(uint8_t addr, uint8_t data)
{
    uint8_t send_buf[2] = {0};

    send_buf[0] = addr % 256;
    send_buf[1] = data;

    ESP_ERROR_CHECK(i2c_master_transmit(eeprom_handle, send_buf, 2, -1));

    /* 写周期等待，AT24C存储芯片在收到I2C停止信号后，会进入内部编程周期，此期间芯片不响应任何请求 需延迟10ms左右时间*/
    esp_rom_delay_us(10000);
}

/**
 * @brief       在AT24C02里面的指定地址开始读出指定个数的数据
 * @param       addr    : 开始读出的地址 对24c02为0~255
 * @param       pbuf    : 数据数组首地址
 * @param       datalen : 要读出数据的个数
 * @retval      无
 */
void at24c02_read(uint8_t addr, uint8_t *pbuf, uint8_t datalen)
{
    while (datalen--)
    {
        *pbuf++ = at24c02_read_one_byte(addr++);
        if (addr >= 255)
        {
            break;
        }
    }
}

/**
 * @brief       在AT24C02里面的指定地址开始写入指定个数的数据
 * @param       addr    : 开始写入的地址 对24c02为0~255
 * @param       pbuf    : 数据数组首地址
 * @param       datalen : 要写入数据的个数
 * @retval      无
 */
void at24c02_write(uint8_t addr, uint8_t *pbuf, uint8_t datalen)
{
    while (datalen--)
    {
        at24c02_write_one_byte(addr, *pbuf);
        addr++;
        pbuf++;
        if (addr >= 255)
        {
            break;
        }
    }
}