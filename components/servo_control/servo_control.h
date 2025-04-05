#pragma once

// If you want to use ESP_LOGI, include esp_log here
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the servo driver
void servo_control_init();

// Update logic (smooth angle transitions, random targets, etc.)
void servo_control_update();

#ifdef __cplusplus
}
#endif
