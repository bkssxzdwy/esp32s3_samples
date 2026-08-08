#ifndef __FSTOOLS_H
#define __FSTOOLS_H

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "ff.h"

/* fstools_file_type返回的类型定义
 * 根据表FILE_TYPE_TBL获得.在fstools.c里面定义
 */
#define T_BIN 0x00  /* BIN文件 */
#define T_LRC 0x10  /* LRC文件 */
#define T_NES 0x20  /* NES文件 */
#define T_SMS 0x21  /* SMS文件 */
#define T_TEXT 0x30 /* TXT文件 */
#define T_C 0x31    /* C文件 */
#define T_H 0x32    /* H文件 */
#define T_WAV 0x40  /* WAV文件 */
#define T_MP3 0x41  /* MP3文件 */
#define T_APE 0x42  /* APE文件 */
#define T_FLAC 0x43 /* FLAC文件 */
#define T_BMP 0x51  /* BMP文件 */
#define T_JPG 0x52  /* JPG文件 */
#define T_JPEG 0x53 /* JPEG文件 */
#define T_GIF 0x54  /* GIF文件 */
#define T_PNG 0x55  /* GIF文件 */
#define T_AVI 0x60  /* AVI文件 */

uint8_t fstools_init(void);
uint8_t fstools_file_type(char *fname);      /*识别文件类型*/
uint16_t fstools_get_file_count(char *path); /*获取Path路径下，文件总个数*/

/*按偏移 seek 目录项的函数*/
/*把已打开目录的读指针直接跳到指定字节偏移 ofs，下次 f_readdir 就读到那条目录项。*/
/*对应 FatFs 源码里的内部函数 dir_sdi（directory seek）*/
FRESULT fstools_dir_sdi(FF_DIR *dp,DWORD ofs);

#endif