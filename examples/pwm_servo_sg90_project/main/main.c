#include <stdio.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "servo.h"

const char *TAG_MAIN = "PWM";

void app_main(void)
{
    esp_err_t ret;

    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    servo_init();

    // 等待舵机稳定
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (1)
    {
        ESP_LOGI(TAG, "=== Servo Test Start ===");

        // // 测试1: 各角度测试
        // ESP_LOGI(TAG, "Test 1: Fixed angles");
        // servo_set_angle(0);      // 0度
        // vTaskDelay(pdMS_TO_TICKS(2000));
        // servo_set_angle(45);     // 45度
        // vTaskDelay(pdMS_TO_TICKS(2000));
        // servo_set_angle(90);     // 90度
        // vTaskDelay(pdMS_TO_TICKS(2000));
        // servo_set_angle(135);    // 135度
        // vTaskDelay(pdMS_TO_TICKS(2000));
        // servo_set_angle(180);    // 180度
        // vTaskDelay(pdMS_TO_TICKS(2000));

        // // 测试2: 连续摆动
        // ESP_LOGI(TAG, "Test 2: Continuous swing");
        // for (int i = 0; i < 5; i++) {
        //     servo_set_angle(0);
        //     vTaskDelay(pdMS_TO_TICKS(1000));
        //     servo_set_angle(180);
        //     vTaskDelay(pdMS_TO_TICKS(1000));
        // }

        // 测试3: 平滑运动
        // ESP_LOGI(TAG, "Test 3: Smooth movement");
        // servo_move_smooth(90, 1000);
        // vTaskDelay(pdMS_TO_TICKS(1000));

        // servo_move_smooth(45, 1000);
        // vTaskDelay(pdMS_TO_TICKS(1000));

        // servo_move_smooth(0, 1000);
        // vTaskDelay(pdMS_TO_TICKS(1000));

        // servo_move_smooth(45, 1000);
        // vTaskDelay(pdMS_TO_TICKS(1000));

        // servo_move_smooth(90, 1000);
        // vTaskDelay(pdMS_TO_TICKS(1000));

        // servo_move_smooth(135, 1000);
        // vTaskDelay(pdMS_TO_TICKS(1000));

        // servo_move_smooth(180, 1000);
        // vTaskDelay(pdMS_TO_TICKS(1000));
        //测试3: 平滑运动-------------------------------------

        // servo_move_smooth(90, 1000); // 2秒转到90度
        // vTaskDelay(pdMS_TO_TICKS(1000));

        // servo_move_smooth(180, 1500); // 1.5秒转到180度
        // vTaskDelay(pdMS_TO_TICKS(1000));
        // servo_move_smooth(90, 2000); // 2秒转到90度
        // vTaskDelay(pdMS_TO_TICKS(1000));
        // servo_move_smooth(0, 1000); // 1秒回到0度
        // vTaskDelay(pdMS_TO_TICKS(1000));

        // // 测试4: 正弦摆动效果
        // ESP_LOGI(TAG, "Test 4: Sinusoidal sweep");
        // for (int angle = 0; angle <= 180; angle += 10) {
        //     servo_set_angle(angle);
        //     vTaskDelay(pdMS_TO_TICKS(100));
        // }
        // for (int angle = 180; angle >= 0; angle -= 10) {
        //     servo_set_angle(angle);
        //     vTaskDelay(pdMS_TO_TICKS(100));
        // }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}