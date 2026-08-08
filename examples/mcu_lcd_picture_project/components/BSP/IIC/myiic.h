#ifndef __MYIIC_H_
#define __MYIIC_H_

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"

#define IIC_NUM_PORT I2C_NUM_0       /*IIC0 */
#define IIC_SPEED_CLK 400000         /*速率400K*/
#define IIC_SDA_GPIO_PIN GPIO_NUM_48 /*IIC0_SDA引脚*/
#define IIC_SCL_GPIO_PIN GPIO_NUM_45 /*IIC0_SCL引脚*/

extern i2c_master_bus_handle_t bus_handler; /*IIC总线句柄*/

esp_err_t myiic_init(void);

#endif