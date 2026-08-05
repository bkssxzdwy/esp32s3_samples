#include "font_lib.h"
#include "spi_sd.h"

#define SD_MOUNT_POINT "0:"

/* 每次操作限制在 4K 之内 */
#define SECTOR_SIZE 0X1000

/* 字库区域占用的总扇区数大小(3个字库+unigbk表+字库信息=3238700 字节,约占791个扇区,一个扇区4K字节) */
#define FONT_SECSIZE 791

/* 每次操作限制在 4K（4096） 之内 */
#define SECTOR_SIZE 0X1000

static const char *font_lib_tag = "Font_LIB";
#define FONT_INFO_ADDR 0
const esp_partition_t *storage_partition;
_font_info ftinfo;

/* 字库存放在TF卡中的路径 */
const char *FONT_GBK_PATH[4] =
    {
        "/SYSTEM/FONT/UNIGBK.BIN", /* UNIGBK.BIN的存放位置 */
        "/SYSTEM/FONT/GBK12.FON",  /* GBK12的存放位置 */
        "/SYSTEM/FONT/GBK16.FON",  /* GBK16的存放位置 */
        "/SYSTEM/FONT/GBK24.FON",  /* GBK24的存放位置 */
};

/* 更新时的提示信息 */
const char *FONT_UPDATE_REMIND_TBL[4] =
    {
        "Updating UNIGBK.BIN", /* 提示正在更新UNIGBK.bin */
        "Updating GBK12.FON ", /* 提示正在更新GBK12 */
        "Updating GBK16.FON ", /* 提示正在更新GBK16 */
        "Updating GBK24.FON ", /* 提示正在更新GBK24 */
};
/**
 * @brief       显示当前字体更新进度
 * @param       x, y    : 坐标
 * @param       size    : 字体大小
 * @param       totsize : 整个文件大小
 * @param       pos     : 当前文件指针位置
 * @param       color   : 字体颜色
 * @retval      无
 */
static void font_lib_show_progress(uint16_t x, uint16_t y, uint8_t size, uint32_t totsize, uint32_t pos, uint32_t color)
{
    float prog;
    uint8_t t = 0XFF;

    prog = (float)pos / totsize;
    prog *= 100;

    if (t != prog)
    {
        lcd_show_string(x + 3 * size / 2, y, 240, 320, size, "%", color);
        t = prog;

        if (t > 100)
            t = 100;

        lcd_show_num(x, y, t, 3, size, color);
    }
}

uint8_t font_lib_init(void)
{
    uint8_t t = 0;
    storage_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");

    if (storage_partition == NULL)
    {
        ESP_LOGE(font_lib_tag, "Flash storage partition not found.");
        return 1;
    }

    while (t < 10)
    {
        t++;
        font_lib_partition_read((uint8_t *)&ftinfo, FONT_INFO_ADDR, sizeof(ftinfo)); /*读出_font_info结构体数据，用于判断字库是否存在*/

        if (ftinfo.fontok == 0xAA)
        {
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (ftinfo.fontok != 0xAA)
    {
        return 1;
    }

    return ESP_OK;
}

/**
 * @brief       分区表读取数据
 * @param       buffer    : 读取数据的存储区
 * @param       offset    : 读取数据的起始地址
 * @param       length    : 读取大小
 * @retval      ESP_OK:表示成功;其他:表示失败
 */
esp_err_t font_lib_partition_read(void *buffer, uint32_t offset, uint32_t length)
{
    esp_err_t err;

    if (buffer == NULL || (length > SECTOR_SIZE))
    {
        ESP_LOGE(font_lib_tag, "ESP_ERR_INVALID_ARG");
        return ESP_ERR_INVALID_ARG;
    }

    err = esp_partition_read(storage_partition, offset, buffer, length);

    if (err != ESP_OK)
    {
        ESP_LOGE(font_lib_tag, "Flash read failed.");
        return err;
    }

    return err;
}

/**
 * @brief       分区表写入数据
 * @param       buffer    : 写入数据的存储区
 * @param       offset    : 写入数据的起始地址
 * @param       length    : 写入大小
 * @retval      ESP_OK:表示成功;其他:表示失败
 */
esp_err_t font_lib_partition_write(void *buffer, uint32_t offset, uint32_t length)
{
    esp_err_t err;

    if (buffer == NULL || length > SECTOR_SIZE)
    {
        ESP_LOGE(font_lib_tag, "ESP_ERR_INVALID_ARG");
        return ESP_ERR_INVALID_ARG;
    }

    err = esp_partition_write(storage_partition, offset, buffer, length);

    if (err != ESP_OK)
    {
        ESP_LOGE(font_lib_tag, "Flash write failed.");
    }

    return err;
}

esp_err_t font_lib_partition_erase_sector(uint32_t offset)
{
    esp_err_t err;
    err = esp_partition_erase_range(storage_partition, offset, SECTOR_SIZE);
    if (err != ESP_OK)
    {
        ESP_LOGE(font_lib_tag, "Flash erase failed.");
    }
    return err;
}

static uint8_t font_lib_update_font(uint8_t *src, uint8_t font_bgk_path_index)
{
    esp_err_t rval = ESP_OK;
    uint8_t res;
    FIL *fftemp;
    uint8_t *font_fpath;
    uint32_t flash_addr = 0;
    uint8_t *temp_buf;
    uint16_t bread;
    uint32_t offx = 0;

    fftemp = (FIL *)malloc(sizeof(FIL));
    if (fftemp == NULL)
    {
        rval = 1;
        return rval;
    }
    temp_buf = malloc(SECTOR_SIZE);
    font_fpath = malloc(100);
    strcpy((char *)font_fpath, (char *)src);
    strcat((char *)font_fpath, (char *)FONT_GBK_PATH[font_bgk_path_index]);
    res = f_open(fftemp, (const TCHAR *)font_fpath, FA_READ);
    if (res)
    {
        rval = 2;
    }

    lcd_clear(WHITE);
    lcd_show_string(30, 50, 320, 240, 16, (char *)FONT_UPDATE_REMIND_TBL[font_bgk_path_index], RED);
    if (rval == 0)
    {
        switch (font_bgk_path_index)
        {
        case 0:                                                /* 更新 UNIGBK.BIN */
            ftinfo.ugbkaddr = FONT_INFO_ADDR + sizeof(ftinfo); /* 信息头之后，紧跟UNIGBK转换码表 */
            ftinfo.ugbksize = fftemp->obj.objsize;             /* UNIGBK大小 */
            flash_addr = ftinfo.ugbkaddr;
            break;
        case 1:                                                 /* 更新 GBK12.BIN */
            ftinfo.f12addr = ftinfo.ugbkaddr + ftinfo.ugbksize; /* UNIGBK之后，紧跟GBK12字库 */
            ftinfo.gbk12size = fftemp->obj.objsize;             /* GBK12字库大小 */
            flash_addr = ftinfo.f12addr;                        /* GBK12的起始地址 */
            break;

        case 2:                                                 /* 更新 GBK16.BIN */
            ftinfo.f16addr = ftinfo.f12addr + ftinfo.gbk12size; /* GBK12之后，紧跟GBK16字库 */
            ftinfo.gbk16size = fftemp->obj.objsize;             /* GBK16字库大小 */
            flash_addr = ftinfo.f16addr;                        /* GBK16的起始地址 */
            break;

        case 3:                                                 /* 更新 GBK24.BIN */
            ftinfo.f24addr = ftinfo.f16addr + ftinfo.gbk16size; /* GBK16之后，紧跟GBK24字库 */
            ftinfo.gbk24size = fftemp->obj.objsize;             /* GBK24字库大小 */
            flash_addr = ftinfo.f24addr;                        /* GBK24的起始地址 */
            break;
        }

        while (res == FR_OK)
        {
            res = f_read(fftemp, temp_buf, SECTOR_SIZE, (UINT *)&bread);
            if (res != FR_OK)
                break;

            font_lib_partition_write(temp_buf, offx + flash_addr, bread);
            offx += bread;

            font_lib_show_progress(30, 80, 24, fftemp->obj.objsize, offx, RED);

            if (bread != SECTOR_SIZE)
                break;
        }

        f_close(fftemp);
    }
    free(fftemp);
    free(font_fpath);

    return rval;
}

uint8_t font_lib_update(uint8_t *src)
{
    uint8_t rval = ESP_OK;
    uint8_t res = 0;
    uint16_t i;
    uint8_t *fpath;
    uint32_t *buf;
    FIL *fftemp;
    ftinfo.fontok = 0xFF;

    fpath = malloc(100);
    buf = malloc(SECTOR_SIZE);
    fftemp = (FIL *)malloc(sizeof(FIL));
    if (buf == NULL || fpath == NULL || fftemp == NULL)
    {
        free(fftemp);
        free(fpath);
        free(buf);
        return 1;
    }

    for (i = 0; i < 4; i++)
    {
        strcpy((char *)fpath, (char *)src);
        strcat((char *)fpath, (char *)FONT_GBK_PATH[i]);
        res = f_open(fftemp, (const TCHAR *)fpath, FA_READ);
        if (res)
        {
            ESP_LOGE(font_lib_tag, "Font BGK path invalid,Path:%s", fpath);
            rval |= 1 << 7;
            break;
        }
    }
    free(fftemp);

    if (rval == 0)
    {
        font_lib_erase();
        for (i = 0; i < 4; i++)
        {
            res = font_lib_update_font(src, i);

            if (res)
            {
                free(buf);
                free(fpath);
                return i + 2;
            }
        }
        ftinfo.fontok = 0xAA;
        font_lib_partition_write((uint8_t *)&ftinfo, FONT_INFO_ADDR, sizeof(ftinfo));
    }

    free(fpath);
    free(buf);

    return rval;
}

/**
 * 先擦除字库区域,提高写入速度
 */
uint8_t font_lib_erase(void)
{
    lcd_clear(WHITE);
    lcd_show_string(30, 50, 320, 240, 16, "Eraseing sectors...", RED);
    uint8_t rval = ESP_OK;

    int i, j;
    uint32_t *buf;

    buf = malloc(SECTOR_SIZE);
    for (i = 0; i < FONT_SECSIZE; i++)
    {
        font_lib_show_progress(30, 80, 24, FONT_SECSIZE, i, RED);
        uint32_t sector_addr = FONT_INFO_ADDR + i * SECTOR_SIZE;
        font_lib_partition_read((uint8_t *)buf, sector_addr, SECTOR_SIZE);
        int need_erase = 0;
        for (j = 0; j < 1024; j++)
        {
            if (buf[j] != 0xFFFFFFFF)
            {
                need_erase = 1;
                break;
            }
        }
        if (need_erase)
        {
            font_lib_partition_erase_sector(sector_addr);
        }
    }

    free(buf);

    return rval;
}