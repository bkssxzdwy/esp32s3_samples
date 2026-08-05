#include "spi_sd.h"

/* SD卡设备句柄 */
spi_device_handle_t spi_sd_handle = NULL;
sdmmc_card_t *sdmmc_card;
const char mount_point[] = MOUNT_POINT;
esp_err_t mount_ret = ESP_FAIL;

esp_err_t spi_sd_init(void)
{
    esp_err_t ret = ESP_OK;

    if (spi_sd_handle != NULL)
    {
        if (mount_ret == ESP_OK)
        {
            esp_vfs_fat_sdcard_unmount(mount_point, sdmmc_card);
            mount_ret = ESP_FAIL;
        }
    }
    else if (spi_sd_handle == NULL)
    {
        spi_init();
    }

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 4 * 1024 * sizeof(uint8_t),
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.host_id = SD_SPI_HOST;
    slot_config.gpio_cs = TF_CS_PIN;
    slot_config.gpio_cd = GPIO_NUM_NC;
    slot_config.gpio_wp = GPIO_NUM_NC;
    slot_config.gpio_int = GPIO_NUM_NC;

    mount_ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &sdmmc_card);
    ret |= mount_ret;
    if (mount_ret != ESP_OK)
    {
        ESP_LOGE("SD", "Mount failed (0x%x)", ret);
    }
    else
    {
        ESP_LOGI("SD", "Mount success");
    }

    vTaskDelay(pdMS_TO_TICKS(10));
    return ret;
}

#include <stdint.h>
#include "ff.h"

void sd_get_fatfs_usage(uint64_t *out_total_bytes, uint64_t *out_free_bytes)
{
    FATFS *fs;
    DWORD free_clusters;

    // 获取空闲簇，如果失败直接返回0，防止系统崩溃
    FRESULT res = f_getfree("0:", &free_clusters, &fs);
    if (res != FR_OK || fs == NULL)
    {
        if (out_total_bytes != NULL)
            *out_total_bytes = 0;
        if (out_free_bytes != NULL)
            *out_free_bytes = 0;
        return;
    }

    // 强制转换为 uint64_t 进行乘法运算，彻底防止 32位系统下的整数溢出
    uint64_t total_bytes = (uint64_t)(fs->n_fatent - 2) * fs->csize * fs->ssize;
    uint64_t free_bytes = (uint64_t)free_clusters * fs->csize * fs->ssize;

    if (out_total_bytes != NULL)
    {
        *out_total_bytes = total_bytes;
    }

    if (out_free_bytes != NULL)
    {
        *out_free_bytes = free_bytes;
    }
}

esp_err_t spi_init(void)
{
    spi_bus_config_t buscfg = {
        .sclk_io_num = SPI_SCLK_PIN, /* 时钟引脚 */
        .mosi_io_num = SPI_MOSI_PIN, /* 主机输出从机输入引脚 */
        .miso_io_num = SPI_MISO_PIN, /* 主机输入从机输出引脚 */
        .quadwp_io_num = -1,         /* 用于Quad模式的WP引脚,未使用时设置为-1 */
        .quadhd_io_num = -1,         /* 用于Quad模式的HD引脚,未使用时设置为-1 */
        .max_transfer_sz = 320 * 240 * sizeof(uint16_t),
    };
    /* 初始化SPI总线 */
    ESP_ERROR_CHECK(spi_bus_initialize(SD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    /* SPI驱动接口配置,SPI SD卡时钟是20-25MHz */
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 20 * 1000 * 1000, /* SPI时钟 */
        .mode = 0,                          /* SPI模式0 */
        .spics_io_num = TF_CS_PIN,          /* 片选引脚 */
        .queue_size = 7,                    /* 事务队列尺寸 7个 */
    };

    /* 添加SPI总线设备 */
    ESP_ERROR_CHECK(spi_bus_add_device(SD_SPI_HOST, &devcfg, &spi_sd_handle));

    return ESP_OK;
}