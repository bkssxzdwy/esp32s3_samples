#include "lcd.h"

static const char *TAG = "LCD";
esp_lcd_panel_handle_t panel_handle = NULL; /* LCD句柄 */
lcd_device_t lcd_device;

void lcd_init(lcd_cfg_t lcd_config)
{
    lcd_device.pwidth = 320;
    lcd_device.pheight = 240;
    lcd_device.rd = LCD_RD;
    lcd_device.dc = LCD_DC;
    lcd_device.wr = LCD_WR;
    lcd_device.cs = LCD_CS;

    esp_lcd_panel_io_handle_t io_handle = NULL;
    /*
    为什么 RD 要初始化为高电平？
    在 8080 并行总线中，RD（读） 是低电平有效信号。
    当主控需要从 LCD 读取数据时，会拉低 RD 引脚，同时配合 CS、DC 等控制信号进行读操作。
    因此，在空闲状态（不读取）时，RD 必须保持高电平，否则 LCD 会误认为收到读命令，可能导致总线冲突或数据错乱。
    */
    gpio_config_t gpio_cfg = {0};

    gpio_cfg.intr_type = GPIO_INTR_DISABLE;
    gpio_cfg.mode = GPIO_MODE_INPUT_OUTPUT;
    gpio_cfg.pin_bit_mask = 1ull << lcd_device.rd;
    gpio_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_cfg.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&gpio_cfg);

    gpio_set_level(lcd_device.rd, 1);

    esp_lcd_i80_bus_handle_t i80_bus_handle = NULL;
    esp_lcd_i80_bus_config_t i80_bus_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .dc_gpio_num = lcd_device.dc,
        .wr_gpio_num = lcd_device.wr,
        .data_gpio_nums = {
            GPIO_LCD_D0,
            GPIO_LCD_D1,
            GPIO_LCD_D2,
            GPIO_LCD_D3,
            GPIO_LCD_D4,
            GPIO_LCD_D5,
            GPIO_LCD_D6,
            GPIO_LCD_D7,
        },
        .bus_width = 8,
        .max_transfer_bytes = lcd_device.pwidth * lcd_device.pheight * sizeof(uint16_t),
        .psram_trans_align = 64,
        .sram_trans_align = 4,
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&i80_bus_config, &i80_bus_handle));

    esp_lcd_panel_io_i80_config_t io_i80_config = {
        .cs_gpio_num = lcd_device.cs,
        .pclk_hz = (20 * 1000 * 1000),
        .trans_queue_depth = 8,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .flags = {
            .swap_color_bytes = 1,
        },
        .on_color_trans_done = lcd_config.notify_flush_ready,
        .user_ctx = lcd_config.user_ctx,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus_handle, &io_i80_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_dev_config = {
        .reset_gpio_num = LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_dev_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);                                /*复位屏幕*/
    esp_lcd_panel_init(panel_handle);                                 /*初始化屏幕*/
    esp_lcd_panel_invert_color(panel_handle, true);                   /*开启颜色反转*/
    esp_lcd_panel_set_gap(panel_handle, 0, 0);                        /*设置XY偏移*/
    esp_lcd_panel_io_tx_param(io_handle, 0x36, (uint8_t[]){0}, 1);    /*控制ST7789的显存读写方向,0x00表示默认的扫描方向（从上到下，从左到右），无镜像或旋转*/
    esp_lcd_panel_io_tx_param(io_handle, 0x3A, (uint8_t[]){0x65}, 1); /*接口像素格式命令,16位色（65K色）设置是0x55*/
    lcd_display_direction(1);                                         /*设置屏幕方向*/

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true)); /*启动屏幕*/
    lcd_clear(WHITE);                                               /*屏幕填充白色*/
    lcd_backlight(1);                                               /*打开屏幕背光*/
}

void lcd_clear(uint16_t color)
{
    uint16_t *buffer = heap_caps_malloc(lcd_device.width * sizeof(uint16_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (NULL == buffer)
    {
        ESP_LOGE(TAG, "malloc memory for clear is not enough.");
    }
    else
    {
        for (uint16_t i = 0; i < lcd_device.width; i++)
        {
            buffer[i] = color;
        }
        for (uint16_t y = 0; y < lcd_device.height; y++)
        {
            esp_lcd_panel_draw_bitmap(panel_handle, 0, y, lcd_device.width, y + 1, buffer);
        }
        heap_caps_free(buffer);
    }
}

void lcd_display_direction(uint8_t direction)
{
    lcd_device.direction = direction;
    if (lcd_device.direction == 0)
    {
        /*竖屏（宽240，高320）*/
        lcd_device.width = lcd_device.pheight;
        lcd_device.height = lcd_device.pwidth;
        esp_lcd_panel_swap_xy(panel_handle, false);
        esp_lcd_panel_mirror(panel_handle, false, false);
    }
    else if (lcd_device.direction == 1)
    {
        /*横屏（宽320，高240）*/
        lcd_device.width = lcd_device.pwidth;
        lcd_device.height = lcd_device.pheight;
        esp_lcd_panel_swap_xy(panel_handle, true);
        esp_lcd_panel_mirror(panel_handle, true, false);
    }
}

void lcd_backlight(uint8_t on)
{
    on ? xl9555_pin_write(LCD_BL_IO, 1) : xl9555_pin_write(LCD_BL_IO, 0);
}