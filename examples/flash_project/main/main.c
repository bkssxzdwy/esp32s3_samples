#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "esp_flash.h"
#include "esp_partition.h"

#include "nvs_flash.h"
#include "nvs.h"

#include "esp_littlefs.h"
#include "esp_vfs.h"

const char *TAG_MAIN = "FLASH";

void flash_raw_demo(void)
{
    /*1.查找自定义数据分区*/
    const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, 0x40, "user_data");
    if (partition == NULL)
    {
        ESP_LOGE(TAG_MAIN, "未找到user_data分区");
        return;
    }
    ESP_LOGI(TAG_MAIN, "分区大小：%lu bytes", partition->size);

    /*2. 准备要写入的数据*/
    const char *write_data = "Hello,ESP32-S3 Flash!";
    size_t data_len = strlen(write_data) + 1;

    /*3. 先擦除要写入区域，擦除一个扇区（4KB）*/
    ESP_ERROR_CHECK(esp_partition_erase_range(partition, 0, 4096));

    /*4. 写入数据*/
    ESP_ERROR_CHECK(esp_partition_write(partition, 0, write_data, data_len));
    ESP_LOGI(TAG_MAIN, "数据写入成功");

    /*5. 读取数据验证*/
    char read_buf[64] = {0};
    ESP_ERROR_CHECK(esp_partition_read(partition, 0, read_buf, data_len));
    ESP_LOGI(TAG_MAIN, "读取到数据：%s", read_buf);
}

void flash_nvs_demo(void)
{
    /*1. 初始化NVS Flash(如果NVS分区为空，会自动格式化)*/
    esp_err_t ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /*2. 打开NVS句柄，"demo"是命名空间，用于隔离不同模块的键*/
    /*命名空间，长度不超过15个字符，
    只能包含字母（A-Z, a-z）、数字（0-9）和下划线（_），不能以数字开头
    区分大小写*/
    nvs_handle_t my_handle;
    ret = nvs_open("demo", NVS_READWRITE, &my_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_MAIN, "打开NVS失败（%s）", esp_err_to_name(ret));
        return;
    }

    /*3.写入数据，支持多种类型数据*/
    /*键（Key）长度不能超过15个字符
    键名必须是 ASCII 字符串，建议只使用字母（a-z, A-Z）、数字（0-9）和下划线（_）
    非空，区分大小写
    同一个命名空间内唯一，
    数据类型固定，一旦一个键被写入，其数据类型就被固定了。如果后续尝试用不同类型的数据（例如，先存了整数，后存字符串）去写入同一个键，操作会返回错误。*/
    int32_t boot_count = 0;
    nvs_get_i32(my_handle, "boot_cnt", &boot_count);
    if (boot_count == 0)
    {
        char device_name[32] = "Device-0001";
        nvs_set_str(my_handle, "dev_name", device_name);
    }
    boot_count++;
    nvs_set_i32(my_handle, "boot_cnt", boot_count);
    ESP_LOGI(TAG_MAIN, "boot count=%d", boot_count);

    /*4.提交写入，NVS的写操作有缓存，必须commit才真正写入Flash*/
    nvs_commit(my_handle);

    /*5.读取数据*/
    char read_dev_name[32];
    size_t read_dev_name_len = sizeof(read_dev_name);
    ret = nvs_get_str(my_handle, "dev_name", read_dev_name, &read_dev_name_len);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG_MAIN, "设备名称：%s", read_dev_name);
    }

    /*6.关闭句柄*/
    nvs_close(my_handle);
}

void flash_littlefs_demo(void)
{
    /*1.配置LittleFS挂载参数*/
    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/littlefs",       // 挂载点，挂载后通过/littlefs/xxx访问文件
        .partition_label = "storage",   // 分区表中对应的分区名
        .format_if_mount_failed = true, // 挂载失败时自动格式化
        .dont_mount = false,
    };

    /*2.挂载LittleFS*/
    esp_err_t err = esp_vfs_littlefs_register(&conf);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_MAIN, "LittleFS挂载失败：%s", esp_err_to_name(err));
        return;
    }

    /*获取文件系统信息，总容量，已使用容量*/
    size_t total = 0, used = 0;
    esp_littlefs_info(conf.partition_label, &total, &used);
    ESP_LOGI(TAG_MAIN, "分区总大小：%d bytes,已使用：%d bytes", total, used);

    /*3.写入文件，使用标准C函数*/
    FILE *f = fopen("/littlefs/hello.txt", "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG_MAIN, "打开文件失败");
        return;
    }
    fprintf(f, "Hello,LittleFS on ESP32-S3!\n");
    fprintf(f, "这是一条写入文件系统的测试数据。\n");
    fclose(f);
    ESP_LOGI(TAG_MAIN, "文件写入成功。");

    /*4.读取文件*/
    f = fopen("/littlefs/hello.txt", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG_MAIN, "打开文件失败。");
        return;
    }
    char buf[128];
    while (fgets(buf, sizeof(buf), f) != NULL)
    {
        ESP_LOGI(TAG_MAIN, "读取：%s", buf);
    }
    fclose(f);

    /*5. 列出目录中的文件*/
    ESP_LOGI(TAG_MAIN, "列出/littlefs目录下文件：");
    DIR *dir = opendir("/littlefs");
    if (dir)
    {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
        {
            ESP_LOGI(TAG_MAIN, " %s", entry->d_name);
        }
        closedir(dir);
    }
}

void app_main(void)
{

    // flash_raw_demo();

    //flash_nvs_demo();

    flash_littlefs_demo();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}