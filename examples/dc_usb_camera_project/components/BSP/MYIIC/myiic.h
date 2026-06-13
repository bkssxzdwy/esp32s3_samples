/**
 ****************************************************************************************************
 * @file        iic.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2024-06-25
 * @brief       MYIIC驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 ESP32S3 BOX 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#ifndef __IIC_H
#define __IIC_H

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"


/* 引脚与相关参数定义 */
#define IIC_NUM_PORT       I2C_NUM_0        /* IIC0 */
#define IIC_SPEED_CLK      400000           /* 速率400K */
#define IIC_SDA_GPIO_PIN   GPIO_NUM_48      /* IIC0_SDA引脚 */
#define IIC_SCL_GPIO_PIN   GPIO_NUM_45      /* IIC0_SCL引脚 */

extern i2c_master_bus_handle_t bus_handle;  /* 总线句柄 */

/* 函数声明 */
esp_err_t myiic_init(void);                 /* 初始化MYIIC */

#endif
