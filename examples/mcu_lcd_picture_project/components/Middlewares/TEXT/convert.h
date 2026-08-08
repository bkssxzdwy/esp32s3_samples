#ifndef __CONVERT_H
#define __CONVERT_H

#include <stdint.h>

/**
 * @brief  UTF-8 字符串转 Unicode(UTF-16LE) 字节流
 * @param  utf8     : 输入 UTF-8 字符串(以 '\\0' 结尾)
 * @param  unicode  : 输出缓冲区, 小端 UTF-16, 末尾写 0x0000
 * @retval >=0 输出的 Unicode 字节数(不含末尾 0); -1 非法 UTF-8
 */
int convert_utf8_to_unicode(const char *utf8, uint8_t *unicode);

/**
 * @brief  Unicode / GBK 互转(查 Flash 中 UNIGBK.BIN)
 * @param  src : 待转换码
 * @param  dir : 0=Unicode→GBK, 1=GBK→Unicode
 * @retval 转换结果; ASCII(<0x80)原样返回; 失败返回 0
 */
uint16_t convert_code(uint16_t src, uint32_t dir);

/**
 * @brief  Unicode(UTF-16LE) 字符串转 GBK 字符串
 * @param  unicode : 输入, 小端 UTF-16, 以 0x0000 结尾
 * @param  gbk     : 输出 GBK 字符串(以 '\\0' 结尾); 汉字高字节在前
 */
void convert_unicode_to_gbk(const uint8_t *unicode, uint8_t *gbk);

/**
 * @brief  UTF-8 字符串直接转 GBK 字符串
 * @param  utf8 : 输入 UTF-8
 * @param  gbk  : 输出 GBK
 * @retval 0 成功; -1 UTF-8 非法或转换失败
 */
int convert_utf8_to_gbk(const char *utf8, uint8_t *gbk);

/**
 * @brief  判断是否为当前转换器可接受的合法 UTF-8
 * @param  str : 输入字符串
 * @retval 1 合法; 0 非法或空指针
 */
int convert_is_valid_utf8(const char *str);

/**
 * @brief  自动识别 UTF-8 / GBK 并统一输出为 GBK
 * @note   合法 UTF-8(含纯 ASCII)走 UTF-8→GBK; 否则按 GBK 原样拷贝
 * @param  str      : 输入(UTF-8 或 GBK)
 * @param  gbk      : 输出 GBK 缓冲
 * @param  gbk_size : 输出缓冲大小(字节)
 * @retval 0 成功; -1 参数非法
 */
int convert_auto_to_gbk(const char *str, uint8_t *gbk, uint16_t gbk_size);

#endif
