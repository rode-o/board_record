#include "servo_demo.h"
#include "servo_control.h"
#include "servo_config.h"

#include <cstdio>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "servo_demo";

/**
 * Demonstration: horizontal, vertical, diagonal sweeps
 * using servo_control_setXY() + servo_control_update().
 */
void servo_control_test_sweeps(void)
{
    ESP_LOGI(TAG, "Starting servo_control_test_sweeps()...");

    // 1) Horizontal sweep at y=600
    int fixedY = CAMERA_MAX_Y / 2; // ~600
    for (int x = 0; x <= CAMERA_MAX_X; x += 100) {
        servo_control_setXY(x, fixedY);

        // Wait ~300 ms, calling update in small increments
        for (int ms = 0; ms < 300; ms += 20) {
            servo_control_update();
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    // 2) Vertical sweep at x=800
    int fixedX = CAMERA_MAX_X / 2; // ~800
    for (int y = 0; y <= CAMERA_MAX_Y; y += 100) {
        servo_control_setXY(fixedX, y);
        for (int ms = 0; ms < 300; ms += 20) {
            servo_control_update();
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    // 3) Diagonal sweep top-left -> bottom-right
    int stepCount = 16;
    for (int i = 0; i <= stepCount; i++) {
        int x = (CAMERA_MAX_X * i) / stepCount; // 0..1600
        int y = (CAMERA_MAX_Y * i) / stepCount; // 0..1200
        servo_control_setXY(x, y);
        for (int ms = 0; ms < 300; ms += 20) {
            servo_control_update();
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    ESP_LOGI(TAG, "servo_control_test_sweeps() completed!");
}
