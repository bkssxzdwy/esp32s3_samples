#include "picture.h"

_pic_phy pic_phy;
_pic_info pic_info;

/**
 * @brief       水平多点填充
 * @param       x, y          : 起始坐标
 * @param       size          : 水平填充点数
 * @param       color         : 颜色数组
 * @retval      无
 */
static void picture_multi_color(uint16_t x, uint16_t y, uint16_t size,
                                uint16_t *colors) {
  esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + size - 1, y + 1, colors);
}

void picture_init() {
  pic_phy.draw_point = lcd_draw_point; /* 画点函数实现,仅GIF需要 */
  pic_phy.fill = lcd_fill;             /* 填充函数实现,仅GIF需要 */
  pic_phy.draw_hline = lcd_draw_hline; /* 画水平线函数实现,仅GIF需要 */
  pic_phy.multicolor = picture_multi_color;

  pic_info.lcd_width = lcd_device.width;
  pic_info.lcd_height = lcd_device.height;
}

/**
 * @brief       显示图片
 *   @note      图片仅在x,y和width, height限定的区域内显示.
 *
 * @param       filename      : 包含路径的文件名(.bmp/.jpg/.jpeg/.gif等)
 * @param       x, y          : 起始坐标
 * @param       width, height : 显示区域
 * @retval      无
 */
uint8_t picture_show(char *file_name, uint16_t x, uint16_t y, uint16_t width,
                     uint16_t height) {
  uint8_t file_type;
  uint8_t res = 0;

  if (width == 0 || height == 0) {
    return PIC_WINDOW_ERR;
  }

  if (((x + width) > pic_info.lcd_width) ||
      ((y + height) > pic_info.lcd_height)) {
    return PIC_WINDOW_ERR;
  }

  file_type = fstools_file_type(file_name);
  ESP_LOGI("PICTURE", "File type:%#x", file_type);

  switch (file_type) {
  case T_BMP:
    ESP_LOGI("PICTURE", "File is bmp.");
    res = bmp_decode(file_name, width, height);
    break;
  case T_GIF:
    res = gif_decode(file_name, x, y, width, height);
    break;
  case T_JPG:
  case T_JPEG:
    res = jpeg_decode(file_name, width, height); /* 解码JPG/JPEG */
    break;
  case T_PNG:
    res = png_decode(file_name, width, height); /* 解码PNG */
    break;

  default:
    res = PIC_FORMAT_ERR;
    break;
  }
  return res;
}