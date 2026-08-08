#include "xl9555.h"

const char *XL9555_TAG = "xl9555";
i2c_master_dev_handle_t xl9555_handle = NULL;

esp_err_t xl9555_init(void)
{
    uint8_t r_data[2];

    i2c_device_config_t xl9555_i2c_device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .scl_speed_hz = IIC_SPEED_CLK,
        .device_address = XL9555_ADDR,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handler, &xl9555_i2c_device_config, &xl9555_handle));

    /* 上电先读取一次，清除中断标志 */
    ESP_ERROR_CHECK(xl9555_register_read(XL9555_INPUT_PORT0_REG, r_data, 2));

    /* 配置XL9555芯片的引脚的输入输出模式*/
    xl9555_ioconfig();

    /* 关闭蜂鸣器 */
    xl9555_pin_write(BEEP_IO, 1);

    /* 关闭喇叭 */
    xl9555_pin_write(SPK_CTRL_IO, 1);
    
    /* 点亮红色LED灯 */
    xl9555_pin_write(LEDR_IO, 0);

    return ESP_OK;
}

uint16_t xl9555_pin_write(uint16_t pin, int val)
{
    uint8_t w_data[2];
    uint16_t temp = 0x0000;

    xl9555_register_read(XL9555_INPUT_PORT0_REG, w_data, 2);

    if (pin <= LCD_BL_IO)
    {
        if (val)
        {
            w_data[0] |= (uint8_t)(0xFF & pin);
        }
        else
        {
            w_data[0] &= ~(uint8_t)(0xFF & pin);
        }
    }
    else
    {
        if (val)
        {
            w_data[1] |= (uint8_t)(0xFF & (pin >> 8));
        }
        else
        {
            w_data[1] &= ~(uint8_t)(0xFF & (pin >> 8));
        }
    }

    temp = ((uint16_t)w_data[1] << 8) | w_data[0];

    xl9555_register_write(XL9555_OUTPUT_PORT0_REG, w_data, 2);

    return temp;
}

int xl9555_pin_read(uint16_t pin)
{
    uint16_t ret;
    uint8_t r_data[2];

    xl9555_register_read(XL9555_INPUT_PORT0_REG, r_data, 2);

    ret = r_data[1] << 8 | r_data[0];

    return (ret & pin) ? 1 : 0;
}

void xl9555_ioconfig()
{
    /* 0001 1011
    P00(AP INT 输入IO 值：1)
    P01(QMA INT 输入IO 值：1)
    P02(BEEP 输出IO 值：0)
    P03(K2 输入IO 值：1)
    P04(K1 输入IO 值：1)
    P05(SPK CTRL 输出IO 值：0)
    P06(CTP RST 输出IO 值：0)
    P07(LCD BL 输出IO 值：1)
    */
    uint8_t p0x = 0x1B;
    /* 1111 1110
    P10(LEDR 输出IO 值：0)
    P11(CTP INT 输入IO 值：1)
    P12(IO1_2 输入IO 值：1)
    P13(IO1_3 输入IO 值：1)
    P14(IO1_4 输入IO 值：1)
    P15(IO1_5 输入IO 值：1)
    P16(IO1_6 输入IO 值：1)
    P17(IO1_7 输入IO 值：1)
    */
    uint8_t p1x = 0xFE;

    uint8_t data[2];
    data[0] = p0x;
    data[1] = p1x;

    esp_err_t err;

    do
    {
        err = xl9555_register_write(XL9555_CONFIG_PORT0_REG, data, 2);
    } while (err != ESP_OK);
}

esp_err_t xl9555_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(xl9555_handle, &reg_addr, 1, data, len, -1);
}

esp_err_t xl9555_register_write(uint8_t reg_addr, uint8_t *data, size_t len)
{
    esp_err_t ret;

    uint8_t *buf = malloc(1 + len);
    if (buf == NULL)
    {
        ESP_LOGE(XL9555_TAG, "%s memory failed", __func__);
        return ESP_ERR_NO_MEM; /* 分配内存失败 */
    }

    buf[0] = reg_addr;          /* 0号元素为寄存器数值 */
    memcpy(buf + 1, data, len); /* 拷贝数据至存储区中 */

    ret = i2c_master_transmit(xl9555_handle, buf, len + 1, -1);

    free(buf); /* 发送完成释放内存 */

    return ret;
}

 /**
  * @brief       按键扫描函数
  * @param       mode:0->不连续;1->连续
  * @retval      键值, 定义如下:
  *              KEY0_PRES, 2, K1按下
  *              KEY1_PRES, 3, K2按下
  */
 uint8_t xl9555_key_scan(uint8_t mode)
 {
     uint8_t keyval = 0;
     static uint8_t key_up = 1;                                          /* 按键按松开标志 */
 
     if (mode)
     {
         key_up = 1;                                                     /* 支持连按 */
     }
     
     if (key_up && (KEY0 == 0 || KEY1 == 0))                             /* 按键松开标志为1, 且有任意一个按键按下了 */
     {
         vTaskDelay(10);                                                 /* 去抖动 */
         key_up = 0;
 
         if (KEY0 == 0)
         {
             keyval = KEY0_PRES;
         }
 
         if (KEY1 == 0)
         {
             keyval = KEY1_PRES;
         }
     }
     else if (KEY0 == 1 && KEY1 == 1)                                    /* 没有任何按键按下, 标记按键松开 */
     {
         key_up = 1;
     }
 
     return keyval;                                                      /* 返回键值 */
 }