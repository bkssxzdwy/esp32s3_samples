#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "myiic.h"
#include "spi_sd.h"
#include "font_lib.h"
#include "text.h"
#include "key.h"
#include "xl9555.h"
#include "fstools.h"
#include "picture.h"

const char *TAG_MAIN = "LCD";

void app_main(void)
{
    esp_err_t ret;
    uint8_t res;

    FF_DIR picdir;
    uint16_t pic_file_count;
    FILINFO *picfileinfo;
    char *pname;
    uint32_t *picoffsettbl;
    uint16_t curindex = 0;
    uint16_t temp_dptr;

    uint8_t key = 0;
    uint8_t key0 = 0;
    uint8_t pause = 0; /* 暂停标记 */
    uint8_t t = 0;

    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
    myiic_init(); /* 初始化IIC */

    xl9555_init();

    spi_init();

    lcd_cfg_t lcd_cfg = {0};
    lcd_cfg.notify_flush_ready = NULL;

    lcd_init(lcd_cfg); /*初始化LCD*/
    lcd_clear(WHITE);

    while (spi_sd_init())
    {
        lcd_show_string(10, 10, 320, 16, 16, "SD Card Error, Please Check...", RED);
        vTaskDelay(pdMS_TO_TICKS(1000));
        lcd_clear(WHITE);
    }

    while (font_lib_init())
    {
        lcd_show_string(10, 10, 320, 16, 16, "Font Init Error, Please Check...", RED);
        res = font_lib_update((uint8_t *)"0:");
        if (res)
        {
            lcd_show_string(10, 30, 320, 16, 16, "Font Update Failed!", RED);
        }
        else
        {
            lcd_show_string(10, 30, 320, 16, 16, "Font Update Success!", GREEN);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
        lcd_clear(WHITE);
    }

    res = fstools_init();

    while (res)
    {
        text_show_string(10, 62, 320, 24, "fstools init error.", 24, 0, RED, WHITE);
        vTaskDelay(pdMS_TO_TICKS(1000));
        lcd_clear(WHITE);
    }

    while (f_opendir(&picdir, "0:/PICTURE"))
    {
        ESP_LOGE(TAG_MAIN, "Open PICTURE dir failed.");
        text_show_string(10, 10, 320, 16, "读取PICTURE文件夹错误，请检查。", 16, 0, RED, WHITE);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    pic_file_count = fstools_get_file_count("0:/PICTURE");

    text_show_string(10, 10, 320, 16, "PICTURE文件夹文件数量：", 16, 0, RED, WHITE);
    lcd_show_num(10, 30, pic_file_count, 2, 16, RED);

    picfileinfo = (FILINFO *)malloc(sizeof(FILINFO)); /* 申请内存 */
    pname = malloc(255 * 2 + 1);                      /* 为带路径的文件名分配内存 */
    picoffsettbl = malloc(4 * pic_file_count);        /* 申请4*pic_file_count个字节的内存,用于存放图片索引 */

    while (!picfileinfo || !pname || !picoffsettbl)
    {
        text_show_string(30, 150, 240, 16, "内存分配失败!", 16, 0, RED, WHITE);
        vTaskDelay(pdMS_TO_TICKS(200));
        lcd_fill(30, 150, 240, 186, WHITE); /* 清除显示 */
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    /* 记录索引 */
    ret = f_opendir(&picdir, "0:/PICTURE"); /* 打开目录 */

    if (ret == FR_OK)
    {
        curindex = 0; /* 当前索引为0 */

        while (1) /* 全部查询一遍 */
        {
            temp_dptr = picdir.dptr;               /* 记录当前dptr偏移 */
            ret = f_readdir(&picdir, picfileinfo); /* 读取目录下的一个文件 */
            if (ret != FR_OK || picfileinfo->fname[0] == 0)
                break; /* 错误了/到末尾了,退出 */

            ret = fstools_file_type(picfileinfo->fname);

            if ((ret & 0X0F) != 0X00) /* 取高四位,看看是不是图片文件 */
            {
                picoffsettbl[curindex] = temp_dptr; /* 记录索引 */
                curindex++;
            }
        }
    }

    text_show_string(30, 150, 240, 16, "图片开始显示...", 16, 0, RED, WHITE);
    vTaskDelay(pdMS_TO_TICKS(1000));
    picture_init();
    curindex = 0;

    ret = f_opendir(&picdir, "0:/PICTURE");
    while (1)
    {
        lcd_clear(BLACK);
        if (curindex >= pic_file_count)
        {
            curindex = 0;
        }
        fstools_dir_sdi(&picdir, picoffsettbl[curindex]);
        ret = f_readdir(&picdir, picfileinfo);
        if (ret != FR_OK || picfileinfo->fname[0] == 0)
        {
            text_show_string(30, 150, 240, 16, "读取图片异常", 16, 0, RED, BLACK);
        }
        else
        {
            strcpy((char *)pname, "0:/PICTURE/");
            strcat((char *)pname, (const char *)picfileinfo->fname);
            ret = picture_show(pname, 0, 0, lcd_device.width, lcd_device.height);
            if (ret == PIC_FORMAT_ERR)
            {
                text_show_string(30, 120, 240, 16, "图片格式不支持", 16, 0, RED, BLACK);
            }
            text_show_string(2, 2, lcd_device.width, 16, (char *)pname, 16, 0, RED, BLACK); /* 显示图片名字 */
        }

        char index_str[16];

        snprintf(index_str, sizeof(index_str), "%u/%u", (unsigned)(curindex + 1), (unsigned)pic_file_count);
        text_show_string(2, lcd_device.height - 18, lcd_device.width, 16, index_str, 16, 0, RED, BLACK); /* 左下角显示当前位置 */

        t = 0;
        while (1)
        {
            key = xl9555_key_scan(0); /* 扫描K1,K2按键 */
            key0 = key_scan();        /* 扫描K0按键 */

            if (t > 250)
            {
                /*表示自动播放*/
                curindex++;
                t = 0;
                break;
            }

            if (key == KEY1_PRES) /* 上一张 */
            {
                curindex--;
                break;
            }
            else if (key == KEY0_PRES) /* 下一张 */
            {
                curindex++;
                break;
            }
            else if (key0 == BOOT)
            {
                pause = !pause;
                t = 0;

                ESP_LOGI(TAG_MAIN, "key0 pressed.pause=%d", pause);
            }

            if (pause == 0)
            {
                t++;
            }

            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    free(picfileinfo);  /* 释放内存 */
    free(pname);        /* 释放内存 */
    free(picoffsettbl); /* 释放内存 */

    while (1)
    {
        vTaskDelay(1);
    }
}