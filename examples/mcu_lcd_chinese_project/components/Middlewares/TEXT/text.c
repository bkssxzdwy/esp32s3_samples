#include "text.h"
#include "convert.h"

static uint8_t text_is_gbk_char(unsigned char *gbk_code)
{
    unsigned char qh, ql;
    qh = gbk_code[0];
    ql = gbk_code[1];
    if (qh < 0x81 || qh == 0xFF)
    {
        return 1;
    }
    if (ql < 0x40)
    {
        return 1;
    }
    if (ql == 0x7F)
    {
        return 2;
    }
    if (ql > 0x7E && ql < 0x80)
    {
        return 3;
    }
    if (ql == 0xFF)
    {
        return 4;
    }
    return 0;
}

/**
 * @brief       获取汉字点阵数据
 * @param       code            : 当前汉字编码(GBK码)
 * @param       mat             : 当前汉字点阵数据存放地址
 * @param       mat_size        : 当前汉字点阵数据的大小
 * @param       size            : 字体大小,值：12,16,24
 * @retval      无
 */
static void text_get_gbk_mat(unsigned char *gbk_code, unsigned char *mat, uint8_t mat_size, uint8_t size)
{
    unsigned char qh, ql;
    uint8_t res = 0;
    uint16_t i;
    unsigned long foffset;

    qh = gbk_code[0];
    ql = gbk_code[1];

    res = text_is_gbk_char(gbk_code);
    if (res)
    {
        for (i = 0; i < mat_size; i++)
        {
            *mat++ = 0x00;
        }
        return;
    }
    if (ql < 0x7F)
    {
        ql -= 0x40;
    }
    else
    {
        ql -= 0x41;
    }

    qh -= 0x81;
    foffset = ((unsigned long)190 * qh + ql) * mat_size; /* 得到字库中的字节偏移量 */

    switch (size)
    {
    case 12:
    {
        font_lib_partition_read(mat, foffset + ftinfo.f12addr, mat_size);
        break;
    }
    case 16:
    {
        font_lib_partition_read(mat, foffset + ftinfo.f16addr, mat_size);
        break;
    }
    case 24:
    {
        font_lib_partition_read(mat, foffset + ftinfo.f24addr, mat_size);
        break;
    }
    }
}

/**
 * @brief       显示一个指定大小的汉字
 * @param       x,y          : 汉字的坐标
 * @param       gbk_code     : 汉字GBK码
 * @param       size         : 字体大小,值：12,16,24
 * @param       mode         : 显示模式
 *   @note                        0, 正常显示(不需要显示的点,用LCD背景色填充,即g_back_color)
 *   @note                        1, 叠加显示(仅显示需要显示的点, 不需要显示的点, 不做处理)
 * @param       color        : 字体颜色
 * @retval      无
 */
void text_show_char(uint16_t x, uint16_t y, uint8_t *gbk_code, uint8_t size, uint8_t mode, uint32_t color, uint32_t g_back_color)
{
    uint8_t csize;
    uint8_t font_size = size;
    uint8_t *c_mat;
    uint8_t i, j, temp;
    uint16_t y0 = y;

    if (font_size != 12 && font_size != 16 && font_size != 24)
    {
        return;
    }
    csize = (font_size / 8 + ((font_size % 8) ? 1 : 0)) * font_size;
    c_mat = (uint8_t *)malloc(csize);
    if (c_mat == NULL)
    {
        return;
    }
    text_get_gbk_mat(gbk_code, c_mat, csize, font_size);
    for (i = 0; i < csize; i++)
    {
        temp = c_mat[i];
        for (j = 0; j < 8; j++)
        {
            if (temp & 0x80)
            {
                lcd_draw_point(x, y, color);
            }
            else if (mode == 0)
            {
                lcd_draw_point(x, y, g_back_color);
            }
            temp <<= 1;
            y++;
            if ((y - y0) == font_size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
    free(c_mat);
}

/**
 * @brief       在指定位置开始显示一个字符串
 * @note        该函数支持自动换行; str 为 UTF-8 编码, 内部转 GBK 后显示
 * @param       x,y   : 起始坐标
 * @param       width : 显示区域宽度
 * @param       height: 显示区域高度
 * @param       str   : UTF-8 字符串
 * @param       size  : 字体大小,值：12,16,24
 * @param       mode  : 显示模式
 * @note                0, 正常显示(不需要显示的点,用LCD背景色填充,即g_back_color)
 * @note                1, 叠加显示(仅显示需要显示的点, 不需要显示的点, 不做处理)
 * @param       color : 字体颜色
 * @retval      无
 */
void text_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *str, uint8_t size, uint8_t mode, uint32_t color, uint32_t g_back_color)
{
    uint16_t x0 = x;
    uint16_t y0 = y;
    uint8_t is_gbk = 0; /* 字符或者中文 */
    uint8_t gbk_buf[256];
    uint8_t *pstr;

    if (str == NULL)
    {
        return;
    }

    /* UTF-8 → GBK, 再按 GBK 规则描点 */
    if (convert_utf8_to_gbk(str, gbk_buf) != 0)
    {
        return;
    }
    pstr = gbk_buf;

    while ((*pstr != 0))
    {
        if (is_gbk)
        {
            /* 当前为 GBK 中文字符 */
            if (x > (x0 + width - size)) /* 换行 */
            {
                y += size;
                x = x0;
            }
            if (y > (y0 + height - size))
            {
                break;
            }

            text_show_char(x, y, pstr, size, mode, color, g_back_color);
            pstr += 2;
            x += size;
            is_gbk = 0;
        }
        else
        {
            if (*pstr > 0x80)
            {
                is_gbk = 1;
            }
            else
            {
                /* ASCII */
                if (x > (x0 + width - size / 2)) /* 换行 */
                {
                    y += size;
                    x = x0;
                }

                if (y > (y0 + height - size)) /* 越界 */
                {
                    break;
                }

                if (*pstr == 13) /* 换行符号 */
                {
                    y += size;
                    x = x0;
                    pstr++;
                }
                else
                {
                    lcd_show_char(x, y, *pstr, size, mode, color);
                }

                pstr++;
                x += size / 2; /* 英文字符宽度为汉字一半 */
            }
        }
    }
}