#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "led.h"
#include "lcd.h"
#include "ppbuffer.h"
#include "usb_stream.h"
#include "jpeg_decoder.h"
#include "esp_timer.h"

static const char *TAG = "uvc_camera_lcd_demo";
#define DEMO_UVC_XFER_BUFFER_SIZE (88 * 1024) /* 双缓冲 */
#define BIT0_FRAME_START (0x01 << 0)
static EventGroupHandle_t s_evt_handle;

typedef struct
{
    uvc_frame_size_t *camera_frame_list;
    size_t camera_frame_list_num;
    size_t camera_currect_frame_index;
} camera_resolution_info_t;

static camera_resolution_info_t camera_resolution_info = {0};
static uint8_t *jpg_frame_buf1 = NULL;
static uint8_t *jpg_frame_buf2 = NULL;
static uint8_t *xfer_buffer_a = NULL;
static uint8_t *xfer_buffer_b = NULL;
static uint8_t *frame_buffer = NULL;
static PingPongBuffer_t *ppbuffer_handle = NULL;
static uint16_t current_width = 0;
static uint16_t current_height = 0;
static bool if_ppbuffer_init = false;

typedef enum
{
    DISPLAY_MODE_DIRECT,         /* 源分辨率与 LCD 一致 */
    DISPLAY_MODE_JPEG_SCALED,    /* JPEG 解码阶段缩放到 LCD 尺寸 */
    DISPLAY_MODE_SOFTWARE_SCALE, /* 全尺寸解码 + 软件最近邻缩放 */
} display_mode_t;

static display_mode_t s_display_mode = DISPLAY_MODE_DIRECT;
static esp_jpeg_image_scale_t s_jpeg_scale = JPEG_IMAGE_SCALE_0;
static uint16_t s_decode_width = 0;
static uint16_t s_decode_height = 0;

/**
 * @brief       根据源分辨率与 LCD 尺寸，选择最优显示缩放策略
 */
static void update_display_scale_config(uint16_t src_w, uint16_t src_h)
{
    const uint16_t dst_w = lcd_dev.width;
    const uint16_t dst_h = lcd_dev.height;

    if (src_w == dst_w && src_h == dst_h)
    {
        s_display_mode = DISPLAY_MODE_DIRECT;
        s_jpeg_scale = JPEG_IMAGE_SCALE_0;
        s_decode_width = src_w;
        s_decode_height = src_h;
        ESP_LOGI(TAG, "Display: direct %ux%u", src_w, src_h);
        return;
    }

    if (src_w % dst_w == 0 && src_h % dst_h == 0)
    {
        const uint16_t ratio_w = src_w / dst_w;
        const uint16_t ratio_h = src_h / dst_h;

        if (ratio_w == ratio_h)
        {
            esp_jpeg_image_scale_t scale = JPEG_IMAGE_SCALE_0;
            switch (ratio_w)
            {
            case 2:
                scale = JPEG_IMAGE_SCALE_1_2;
                break;
            case 4:
                scale = JPEG_IMAGE_SCALE_1_4;
                break;
            case 8:
                scale = JPEG_IMAGE_SCALE_1_8;
                break;
            default:
                break;
            }

            if (scale != JPEG_IMAGE_SCALE_0)
            {
                s_display_mode = DISPLAY_MODE_JPEG_SCALED;
                s_jpeg_scale = scale;
                s_decode_width = dst_w;
                s_decode_height = dst_h;
                ESP_LOGI(TAG, "Display: JPEG 1/%u scale %ux%u -> %ux%u",
                         ratio_w, src_w, src_h, dst_w, dst_h);
                return;
            }
        }
    }

    s_display_mode = DISPLAY_MODE_SOFTWARE_SCALE;
    s_jpeg_scale = JPEG_IMAGE_SCALE_0;
    s_decode_width = src_w;
    s_decode_height = src_h;
    ESP_LOGI(TAG, "Display: software scale %ux%u -> %ux%u", src_w, src_h, dst_w, dst_h);
}

/**
 * @brief       定点累加器最近邻缩放（RGB565）
 */
static void scale_nearest_rgb565_fixed_point(const uint16_t *src, uint16_t *dst,
                                             uint16_t src_w, uint16_t src_h,
                                             uint16_t dst_w, uint16_t dst_h)
{
    const uint32_t y_step = ((uint32_t)src_h << 16) / dst_h;
    const uint32_t x_step = ((uint32_t)src_w << 16) / dst_w;
    uint32_t src_y_acc = 0;

    for (int y = 0; y < dst_h; y++, src_y_acc += y_step)
    {
        const uint16_t *src_row = src + (src_y_acc >> 16) * src_w;
        uint16_t *dst_row = dst + y * dst_w;
        uint32_t src_x_acc = 0;

        for (int x = 0; x < dst_w; x++, src_x_acc += x_step)
        {
            dst_row[x] = src_row[src_x_acc >> 16];
        }
    }
}

/**
 * @brief       Jpeg解码器一张图片
 * @param       input_buf       :输入数据
 * @param       len             :大小
 * @param       output_buf      :输出数据
 * @retval      无
 */
static int esp_jpeg_decoder_one_picture(uint8_t *input_buf, size_t len, uint8_t *output_buf)
{
    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata = (uint8_t *)input_buf,
        .indata_size = len,
        .outbuf = (uint8_t *)(output_buf),
        .outbuf_size = s_decode_width * s_decode_height * sizeof(uint16_t),
        .out_format = JPEG_IMAGE_FORMAT_RGB565,
        .out_scale = s_jpeg_scale,
        .flags = {
            .swap_color_bytes = 0,
        }};

    esp_jpeg_image_output_t outimg;
    esp_err_t ret = esp_jpeg_decode(&jpeg_cfg, &outimg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "JPEG decode failed: %s", esp_err_to_name(ret));
    }

    return ret;
}

/**
 * @brief       自适应JPG帧缓冲器
 * @param       length       :大小
 * @retval      无
 */
static void adaptive_jpg_frame_buffer(size_t length)
{
    if (jpg_frame_buf1 != NULL)
    {
        free(jpg_frame_buf1);
    }

    if (jpg_frame_buf2 != NULL)
    {
        free(jpg_frame_buf2);
    }
    /* 申请内存 */
    jpg_frame_buf1 = (uint8_t *)heap_caps_aligned_alloc(16, length, MALLOC_CAP_SPIRAM);
    assert(jpg_frame_buf1 != NULL);
    jpg_frame_buf2 = (uint8_t *)heap_caps_aligned_alloc(16, length, MALLOC_CAP_SPIRAM);
    assert(jpg_frame_buf2 != NULL);
    /* 申请ppbuffer存储区域 */
    ESP_ERROR_CHECK(ppbuffer_create(ppbuffer_handle, jpg_frame_buf2, jpg_frame_buf1));
    if_ppbuffer_init = true;
}

/**
 * @brief       摄像头回调函数
 * @param       frame       :从UVC设备接收到的图像帧
 * @param       ptr         :转入参数（未使用）
 * @retval      无
 */
static void camera_frame_cb(uvc_frame_t *frame, void *ptr)
{
    if (current_width != frame->width || current_height != frame->height)
    {
        current_width = frame->width;
        current_height = frame->height;
        update_display_scale_config(current_width, current_height);
        adaptive_jpg_frame_buffer(s_decode_width * s_decode_height * 2);
    }

    static void *jpeg_buffer = NULL;
    /* 获取可写缓冲区 */
    ppbuffer_get_write_buf(ppbuffer_handle, &jpeg_buffer);
    assert(jpeg_buffer != NULL);
    /* JPEG解码 */
    esp_jpeg_decoder_one_picture((uint8_t *)frame->data, frame->data_bytes, jpeg_buffer);
    /* 通知缓冲区写完成 */
    ppbuffer_set_write_done(ppbuffer_handle);

    vTaskDelay(pdMS_TO_TICKS(1));
}

/**
 * @brief       usb摄像头任务函数
 * @param       arg     :未使用
 * @retval      无
 */
static void usb_display_task(void *arg)
{
    uint16_t *scaled_buffer = malloc(lcd_dev.width * lcd_dev.height * sizeof(uint16_t));
    assert(scaled_buffer != NULL);
    uint16_t *lcd_buffer = NULL;
    int64_t count_start_time = 0;
    int frame_count = 0;
    int fps = 0;

    while (!if_ppbuffer_init)
    {
        vTaskDelay(1);
    }

    while (1)
    {
        /* 获取可读缓冲区 */
        if (ppbuffer_get_read_buf(ppbuffer_handle, (void *)&lcd_buffer) == ESP_OK)
        {
            if (s_display_mode == DISPLAY_MODE_SOFTWARE_SCALE)
            {
                scale_nearest_rgb565_fixed_point(lcd_buffer, scaled_buffer,
                                                 s_decode_width, s_decode_height,
                                                 lcd_dev.width, lcd_dev.height);
                esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, lcd_dev.width, lcd_dev.height, scaled_buffer);
            }
            else
            {
                esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, lcd_dev.width, lcd_dev.height, lcd_buffer);
            }

            /* 通知缓冲区读完成 */
            ppbuffer_set_read_done(ppbuffer_handle);

            if (count_start_time == 0)
            {
                count_start_time = esp_timer_get_time();
            }

            if (++frame_count == 20)
            {
                frame_count = 0;
                fps = 20 * 1000000 / (esp_timer_get_time() - count_start_time);
                count_start_time = esp_timer_get_time();
                ESP_LOGI(TAG, "camera fps: %d src=%d*%d mode=%d", fps, current_width, current_height, s_display_mode);
            }
        }

        vTaskDelay(1);
    }
}

/**
 * @brief       USB数据流初始化
 * @param       无
 * @retval      ESP_OK：成功初始化；其他表示初始化失败
 */
static esp_err_t usb_stream_init(void)
{
    uvc_config_t uvc_config = {
        .frame_interval = FRAME_INTERVAL_FPS_30,
        .xfer_buffer_size = DEMO_UVC_XFER_BUFFER_SIZE,
        .xfer_buffer_a = xfer_buffer_a,
        .xfer_buffer_b = xfer_buffer_b,
        .frame_buffer_size = DEMO_UVC_XFER_BUFFER_SIZE,
        .frame_buffer = frame_buffer,
        .frame_cb = &camera_frame_cb,
        .frame_cb_arg = NULL,
        .frame_width = FRAME_RESOLUTION_ANY,
        .frame_height = FRAME_RESOLUTION_ANY,
        .flags = FLAG_UVC_SUSPEND_AFTER_START,
    };

    esp_err_t ret = uvc_streaming_config(&uvc_config);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "uvc streaming config failed");
    }
    return ret;
}

/**
 * @brief       从UVC支持的分辨率列表中，选取与LCD相同或最接近的分辨率
 * @param       frame_list  : UVC分辨率列表
 * @param       list_num    : 列表长度
 * @retval      最佳匹配项索引；列表为空时返回 (size_t)-1
 */
static size_t usb_camera_find_lcd_matched_index(const uvc_frame_size_t *frame_list, size_t list_num)
{
    if (frame_list == NULL || list_num == 0)
    {
        return (size_t)-1;
    }

    const uint16_t target_w = lcd_dev.width;
    const uint16_t target_h = lcd_dev.height;

    for (size_t i = 0; i < list_num; i++)
    {
        if (frame_list[i].width == target_w && frame_list[i].height == target_h)
        {
            ESP_LOGI(TAG, "Exact LCD match: %ux%u", frame_list[i].width, frame_list[i].height);
            return i;
        }
    }

    size_t best_index = 0;
    uint32_t best_score = UINT32_MAX;
    for (size_t i = 0; i < list_num; i++)
    {
        int dw = (int)frame_list[i].width - (int)target_w;
        int dh = (int)frame_list[i].height - (int)target_h;
        uint32_t score = (uint32_t)(dw * dw + dh * dh);
        if (score < best_score)
        {
            best_score = score;
            best_index = i;
        }
    }

    ESP_LOGI(TAG, "Closest match to LCD %ux%u: %ux%u (index %u)",
             target_w, target_h,
             frame_list[best_index].width, frame_list[best_index].height,
             (unsigned)best_index);
    return best_index;
}

/**
 * @brief       usb数据流回调函数
 * @param       event   : 事件
 * @param       arg     : 参数（未使用）
 * @retval      无
 */
static void usb_stream_state_changed_cd(usb_stream_state_t event, void *arg)
{
    switch (event)
    {
    /* 连接状态 */
    case STREAM_CONNECTED:
        uvc_frame_size_list_get(NULL, &camera_resolution_info.camera_frame_list_num, NULL);

        if (camera_resolution_info.camera_frame_list_num)
        {
            ESP_LOGI(TAG, "UVC: get frame list size = %u", camera_resolution_info.camera_frame_list_num);
            uvc_frame_size_t *_frame_list = (uvc_frame_size_t *)malloc(camera_resolution_info.camera_frame_list_num * sizeof(uvc_frame_size_t));

            camera_resolution_info.camera_frame_list = (uvc_frame_size_t *)realloc(camera_resolution_info.camera_frame_list, camera_resolution_info.camera_frame_list_num * sizeof(uvc_frame_size_t));

            if (NULL == camera_resolution_info.camera_frame_list)
            {
                ESP_LOGE(TAG, "camera_resolution_info.camera_frame_list");
            }

            uvc_frame_size_list_get(_frame_list, NULL, NULL);

            ESP_LOGI(TAG, "\tlcd_width=%d,lcd_height=%d", lcd_dev.width, lcd_dev.height);
            for (size_t i = 0; i < camera_resolution_info.camera_frame_list_num; i++)
            {
                camera_resolution_info.camera_frame_list[i] = _frame_list[i];
                ESP_LOGI(TAG, "\tframe[%u] = %ux%u", i, _frame_list[i].width, _frame_list[i].height);
            }

            camera_resolution_info.camera_currect_frame_index = usb_camera_find_lcd_matched_index(
                camera_resolution_info.camera_frame_list, camera_resolution_info.camera_frame_list_num);

            if (-1 == camera_resolution_info.camera_currect_frame_index)
            {
                ESP_LOGE(TAG, "find lcd matched resolution fail");
                break;
            }
            ESP_ERROR_CHECK(uvc_frame_size_reset(camera_resolution_info.camera_frame_list[camera_resolution_info.camera_currect_frame_index].width,
                                                 camera_resolution_info.camera_frame_list[camera_resolution_info.camera_currect_frame_index].height, FPS2INTERVAL(30)));

            if (_frame_list != NULL)
            {
                free(_frame_list);
            }
            /* 等待USB摄像头连接 */
            usb_streaming_control(STREAM_UVC, CTRL_RESUME, NULL);
            xEventGroupSetBits(s_evt_handle, BIT0_FRAME_START);
        }
        else
        {
            ESP_LOGW(TAG, "UVC: get frame list size = %u", camera_resolution_info.camera_frame_list_num);
        }
        /* 设备连接成功 */
        ESP_LOGI(TAG, "Device connected");
        break;
    /* 关闭连接 */
    case STREAM_DISCONNECTED:
        xEventGroupClearBits(s_evt_handle, BIT0_FRAME_START);
        /* 设备断开 */
        ESP_LOGI(TAG, "Device disconnected");
        break;
    default:
        ESP_LOGE(TAG, "Unknown event");
        break;
    }
}

void app_main(void)
{
    lcd_cfg_t lcd_config_info = {0};
    lcd_config_info.notify_flush_ready = NULL;
    lcd_config_info.user_ctx = NULL;

    led_init();                /* 初始化LED */
    myiic_init();              /* IIC初始化 */
    xl9555_init();             /* 初始化按键 */
    lcd_init(lcd_config_info); /* 初始化LCD */

    /* 创建事件组 */
    s_evt_handle = xEventGroupCreate();

    if (s_evt_handle == NULL)
    {
        ESP_LOGE(TAG, "line-%u event group create failed", __LINE__);
        assert(0);
    }

    /* 申请USB双缓冲 */
    xfer_buffer_a = (uint8_t *)malloc(DEMO_UVC_XFER_BUFFER_SIZE);
    assert(xfer_buffer_a != NULL);
    xfer_buffer_b = (uint8_t *)malloc(DEMO_UVC_XFER_BUFFER_SIZE);
    assert(xfer_buffer_b != NULL);

    /* mjpeg一帧缓冲 */
    frame_buffer = (uint8_t *)malloc(DEMO_UVC_XFER_BUFFER_SIZE);
    assert(frame_buffer != NULL);

    /* 为ppbuffer_handle句柄申请缓冲 */
    ppbuffer_handle = (PingPongBuffer_t *)malloc(sizeof(PingPongBuffer_t));
    assert(ppbuffer_handle != NULL);

    /* 显示摄像头图形 */
    xTaskCreate(usb_display_task, "usb_display_task", 4 * 1024, NULL, 5, NULL);

    /* USB数据流初始化 */
    ESP_ERROR_CHECK(usb_stream_init());

    /* 注册回调函数 */
    ESP_ERROR_CHECK(usb_streaming_state_register(&usb_stream_state_changed_cd, NULL));

    /* 开启USB数据流转输  */
    ESP_ERROR_CHECK(usb_streaming_start());
    /* 等待连接  */
    ESP_ERROR_CHECK(usb_streaming_connect_wait(portMAX_DELAY));
}