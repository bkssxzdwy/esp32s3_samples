#include "convert.h"
#include "font_lib.h"

/* 高低字节交换: GBK 码写入字符串时高字节在前 */
#define CONVERT_SWAP16(x) ((uint16_t)((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8)))

/**
 * @brief  UTF-8 → Unicode(UTF-16LE)
 * @note   使用无符号运算避免符号扩展问题
 */
int convert_utf8_to_unicode(const char *utf8, uint8_t *unicode)
{
    const uint8_t *p_in = (const uint8_t *)utf8;
    uint8_t *p_out = unicode;
    int output_size = 0;
    uint16_t code;

    if (utf8 == NULL || unicode == NULL)
    {
        return -1;
    }

    while (*p_in)
    {
        if (*p_in <= 0x7F)
        {
            /* 1 字节 UTF-8 → ASCII */
            code = *p_in;
        }
        else if ((*p_in & 0xE0) == 0xC0)
        {
            /* 2 字节 UTF-8 */
            uint8_t high = p_in[0];
            uint8_t low = p_in[1];

            if (low == 0 || (low & 0xC0) != 0x80)
            {
                return -1;
            }
            code = (uint16_t)(((high & 0x1F) << 6) | (low & 0x3F));
            p_in++;
        }
        else if ((*p_in & 0xF0) == 0xE0)
        {
            /* 3 字节 UTF-8(常用汉字) */
            uint8_t high = p_in[0];
            uint8_t middle = p_in[1];
            uint8_t low = p_in[2];

            if (middle == 0 || low == 0 ||
                (middle & 0xC0) != 0x80 || (low & 0xC0) != 0x80)
            {
                return -1;
            }
            code = (uint16_t)(((high & 0x0F) << 12) |
                              ((middle & 0x3F) << 6) |
                              (low & 0x3F));
            p_in += 2;
        }
        else
        {
            /* 不支持 4 字节及以上 UTF-8 */
            return -1;
        }

        /* 小端写入 UTF-16 */
        *p_out++ = (uint8_t)(code & 0xFF);
        *p_out++ = (uint8_t)((code >> 8) & 0xFF);
        output_size += 2;
        p_in++;
    }

    /* Unicode 串结束符 0x0000 */
    *p_out++ = 0;
    *p_out = 0;
    return output_size;
}

/**
 * @brief  查 UNIGBK.BIN 做 Unicode↔GBK
 */
uint16_t convert_code(uint16_t src, uint32_t dir)
{
    uint16_t t[2];
    uint16_t c;
    uint32_t i, li, hi;
    uint16_t n;
    uint32_t gbk2uni_offset = 0;

    if (src < 0x80)
    {
        return src; /* ASCII 无需转换 */
    }

    if (ftinfo.ugbksize < 8)
    {
        return 0;
    }

    if (dir)
    {
        /* GBK → Unicode: 使用后半表 */
        gbk2uni_offset = ftinfo.ugbksize / 2;
    }
    else
    {
        /* Unicode → GBK: 使用前半表 */
        gbk2uni_offset = 0;
    }

    /* 半表条目数 = (ugbksize/2)/4, 下标上限再减 1 */
    hi = ftinfo.ugbksize / 2;
    hi = hi / 4 - 1;
    li = 0;

    for (n = 16; n; n--)
    {
        i = li + (hi - li) / 2;
        if (font_lib_partition_read(t, ftinfo.ugbkaddr + i * 4 + gbk2uni_offset, 4) != ESP_OK)
        {
            return 0;
        }
        if (src == t[0])
        {
            break;
        }
        if (src > t[0])
        {
            li = i;
        }
        else
        {
            hi = i;
        }
    }

    c = n ? t[1] : 0;
    return c;
}

/**
 * @brief  Unicode(UTF-16LE) → GBK
 */
void convert_unicode_to_gbk(const uint8_t *unicode, uint8_t *gbk)
{
    uint16_t temp;
    uint8_t buf[2];
    uint8_t *p_out = gbk;

    if (unicode == NULL || gbk == NULL)
    {
        return;
    }

    while (unicode[0] || unicode[1])
    {
        buf[0] = *unicode++;
        buf[1] = *unicode++;
        temp = convert_code((uint16_t)buf[0] | ((uint16_t)buf[1] << 8), 0);

        if (temp < 0x80)
        {
            *p_out++ = (uint8_t)temp;
        }
        else
        {
            /* 高字节在前, 供 text_show_string 按 GBK 解析 */
            temp = CONVERT_SWAP16(temp);
            *p_out++ = (uint8_t)(temp & 0xFF);
            *p_out++ = (uint8_t)((temp >> 8) & 0xFF);
        }
    }

    *p_out = 0;
}

/**
 * @brief  UTF-8 → GBK(便捷接口)
 */
int convert_utf8_to_gbk(const char *utf8, uint8_t *gbk)
{
    /* 临时 Unicode 缓冲: 按输入长度 *2 + 2 估算, 栈上不宜过大时由调用方保证
     * 这里用固定上限, 足够短句显示; 超长请分两次转换
     */
    uint8_t unicode_buf[512];
    int ret;

    if (utf8 == NULL || gbk == NULL)
    {
        return -1;
    }

    ret = convert_utf8_to_unicode(utf8, unicode_buf);
    if (ret < 0)
    {
        return -1;
    }

    convert_unicode_to_gbk(unicode_buf, gbk);
    return 0;
}
