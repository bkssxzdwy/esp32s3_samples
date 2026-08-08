#ifndef __SPISD_H_
#define __SPISD_H_

#include <unistd.h>
#include "esp_err.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"

#define TF_CS_PIN GPIO_NUM_17

#define MOUNT_POINT "/0:"

#define SPI_SCLK_PIN GPIO_NUM_7
#define SPI_MISO_PIN GPIO_NUM_15
#define SPI_MOSI_PIN GPIO_NUM_16

#define SD_SPI_HOST SPI2_HOST

extern spi_device_handle_t spi_sd_handle;	/* SD卡句柄 */

esp_err_t spi_sd_init(void); /* SPI SD卡初始化*/

void sd_get_fatfs_usage(uint64_t *out_total_bytes, uint64_t *out_free_bytes);

esp_err_t spi_init(void); /* SPI初始化*/

#endif