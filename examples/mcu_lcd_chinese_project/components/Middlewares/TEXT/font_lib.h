#ifndef __FONT_LIB_H_
#define __FONT_LIB_H_

#include "esp_partition.h"
#include "spi_flash_mmap.h"
#include "esp_log.h"
#include "ff.h"
#include "lcd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern uint32_t FONT_INFO_ADDR;

/* 字库信息结构体定义
 * 用来保存字库基本信息，地址，大小等
 */
typedef struct
{
    uint8_t fontok;             /* 字库存在标志，0XAA，字库正常；其他，字库不存在 */
    uint32_t ugbkaddr;          /* unigbk的地址 */
    uint32_t ugbksize;          /* unigbk的大小 */
    uint32_t f12addr;           /* gbk12地址 */
    uint32_t gbk12size;         /* gbk12的大小 */
    uint32_t f16addr;           /* gbk16地址 */
    uint32_t gbk16size;         /* gbk16的大小 */
    uint32_t f24addr;           /* gbk24地址 */
    uint32_t gbk24size;         /* gbk24的大小 */
} _font_info;


/* 字库信息结构体 */
extern _font_info ftinfo;

uint8_t font_lib_init(void);  
esp_err_t font_lib_partition_read(void * buffer, uint32_t offset, uint32_t length);    
esp_err_t font_lib_partition_write(void *buffer,uint32_t offset,uint32_t length);
esp_err_t font_lib_partition_erase_sector(uint32_t offset);
uint8_t font_lib_update(uint8_t *src);

uint8_t font_lib_erase();

#endif