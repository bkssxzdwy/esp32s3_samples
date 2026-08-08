#ifndef __IMAGE_H
#define __IMAGE_H

#define PIC_FORMAT_ERR 0x27 /* 格式错误 */
#define PIC_SIZE_ERR 0x28   /* 图片尺寸错误 */
#define PIC_WINDOW_ERR 0x29 /* 窗口设定错误 */
#define PIC_MEM_ERR 0x11    /* 内存错误 */

#define rgb565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

/* 图片显示物理层接口 */
/* 在移植的时候,必须由用户自己实现这几个函数 */
typedef struct
{
    /* void draw_point(uint16_t x,uint16_t y,uint32_t color) 画点函数 */
    void (*draw_point)(uint16_t, uint16_t, uint16_t);

    /* void fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint32_t color) 单色填充函数 */
    void (*fill)(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);

    /* void draw_hline(uint16_t x0,uint16_t y0,uint16_t len,uint16_t color) 画水平线函数 */
    void (*draw_hline)(uint16_t, uint16_t, uint16_t, uint16_t);

    /* void piclib_multi_color(uint16_t x, uint16_t y, uint16_t width, uint16_t *color) 多点填充 */
    void (*multicolor)(uint16_t, uint16_t, uint16_t, uint16_t *);
} _pic_phy;

extern _pic_phy pic_phy;

typedef struct
{
    uint16_t lcd_width;
    uint16_t lcd_height;
} _pic_info;

extern _pic_info pic_info;


#endif