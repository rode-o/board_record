#include "servo_control.h"
#include "servo_config.h"     // pin definitions, freq, etc.

#include <cmath>             // fabsf, etc.
#include <cstdio>            // printf (if needed)
#include "driver/ledc.h"
#include "esp_timer.h"       // esp_timer_get_time()
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "servo_control";

// ------------------ Internal State ------------------

// We store angles for the horizontal servo (servo1) and vertical servo (servo2)
static float currentServo1Angle = 90.0f;
static float targetServo1Angle  = 90.0f;

static float currentServo2Angle = 90.0f;
static float targetServo2Angle  = 90.0f;

static uint64_t previousUpdateMillis = 0ULL;

// ------------------ Utility Functions ------------------

// Convert servo angle (0..180°) -> microseconds pulse
static uint32_t angleToPulseUs(float angle)
{
    float alpha = angle / (float)SERVO_MAX_ANGLE; // 0..1
    float span  = (float)(SERVO_MAX_US - SERVO_MIN_US);
    return (uint32_t)(SERVO_MIN_US + alpha * span);
}

// Convert microseconds -> LEDC duty
static uint32_t pulseUsToDuty(uint32_t us)
{
    const uint32_t period_us = 20000; // 50 Hz => 20000 µs
    uint32_t max_duty = (1 << SERVO_TIMER_BITS) - 1;  // e.g. 2^14 - 1

    // duty = (us / period_us) * max_duty
    uint64_t duty = ((uint64_t)us * max_duty) / period_us;
    return (uint32_t)duty;
}

// Return current time in milliseconds
static uint64_t millis()
{
    return esp_timer_get_time() / 1000ULL;
}

// Write an angle to a specific servo channel
static void setServoAngle(ledc_channel_t channel, float angle)
{
    uint32_t pulse_us = angleToPulseUs(angle);
    uint32_t duty     = pulseUsToDuty(pulse_us);

    ledc_set_duty(SERVO_SPEED_MODE, channel, duty);
    ledc_update_duty(SERVO_SPEED_MODE, channel);
}

// ------------------ Public API Implementations ------------------

void servo_control_init(void)
{
    ESP_LOGI(TAG, "Initializing servo control on pins %d (H), %d (V)",
             SERVO1_PIN, SERVO2_PIN);

    // Configure the LEDC timer
    ledc_timer_config_t timer_conf = {};
    timer_conf.speed_mode      = SERVO_SPEED_MODE;
    timer_conf.timer_num       = SERVO_TIMER_NUM; 
    timer_conf.duty_resolution = SERVO_TIMER_BITS;
    timer_conf.freq_hz         = SERVO_FREQUENCY;
    timer_conf.clk_cfg         = LEDC_AUTO_CLK;
    ledc_timer_config(&timer_conf);

    // Configure channels
    ledc_channel_config_t ch1 = {};
    ch1.speed_mode     = SERVO_SPEED_MODE;
    ch1.channel        = SERVO1_CHANNEL;
    ch1.timer_sel      = SERVO_TIMER_NUM;
    ch1.intr_type      = LEDC_INTR_DISABLE;
    ch1.gpio_num       = SERVO1_PIN;
    ch1.duty           = 0;
    ch1.hpoint         = 0;
    ch1.flags.output_invert = false;
    ledc_channel_config(&ch1);

    ledc_channel_config_t ch2 = {};
    ch2.speed_mode     = SERVO_SPEED_MODE;
    ch2.channel        = SERVO2_CHANNEL;
    ch2.timer_sel      = SERVO_TIMER_NUM;
    ch2.intr_type      = LEDC_INTR_DISABLE;
    ch2.gpio_num       = SERVO2_PIN;
    ch2.duty           = 0;
    ch2.hpoint         = 0;
    ch2.flags.output_invert = false;
    ledc_channel_config(&ch2);

    // Start at 90° each
    setServoAngle(SERVO1_CHANNEL, currentServo1Angle);
    setServoAngle(SERVO2_CHANNEL, currentServo2Angle);

    ESP_LOGI(TAG, "Servo control init done. Starting angles: H=%.2f°, V=%.2f°",
             currentServo1Angle, currentServo2Angle);
}

void servo_control_setXY(int x, int y)
{
    // Bound inputs
    if (x < 0) x = 0;
    if (x > CAMERA_MAX_X) x = CAMERA_MAX_X;
    if (y < 0) y = 0;
    if (y > CAMERA_MAX_Y) y = CAMERA_MAX_Y;

    // Convert x to horizontal angle: x=0 => 180°, x=1600 => 0°
    float angleH = 180.0f - ((float)x * 180.0f / (float)CAMERA_MAX_X);

    // Convert y to vertical angle: y=0 => 180°, y=1200 => 0°
    float angleV = 180.0f - ((float)y * 180.0f / (float)CAMERA_MAX_Y);

    targetServo1Angle = angleH;
    targetServo2Angle = angleV;

    ESP_LOGI(TAG, "servo_control_setXY(%d, %d) => angles (H=%.2f°, V=%.2f°)",
             x, y, angleH, angleV);
}

void servo_control_update(void)
{
    uint64_t now = millis();
    if ((now - previousUpdateMillis) < SERVO_UPDATE_MS) {
        // not yet time
        return;
    }
    previousUpdateMillis = now;

    // Smoothly approach target for servo1
    float diffH = targetServo1Angle - currentServo1Angle;
    if (fabsf(diffH) < 0.1f) {
        currentServo1Angle = targetServo1Angle;
    } else {
        currentServo1Angle += diffH * SERVO_SMOOTH_FACTOR;
    }
    setServoAngle(SERVO1_CHANNEL, currentServo1Angle);

    // Smoothly approach target for servo2
    float diffV = targetServo2Angle - currentServo2Angle;
    if (fabsf(diffV) < 0.1f) {
        currentServo2Angle = targetServo2Angle;
    } else {
        currentServo2Angle += diffV * SERVO_SMOOTH_FACTOR;
    }
    setServoAngle(SERVO2_CHANNEL, currentServo2Angle);
}
