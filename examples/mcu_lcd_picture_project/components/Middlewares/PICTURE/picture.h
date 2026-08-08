#ifndef __PICTURE_H
#define __PICTURE_H

#include "lcd.h"
#include <unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "ff.h"
#include "fstools.h"
#include "bmp.h"
#include "image.h"
#include "gif.h"
#include "jpeg.h"
#include "png.h"


void picture_init();

uint8_t picture_show(char *file_name, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

#endif