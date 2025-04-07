#include "servo_control.h"
#include <cmath>         // for fabs, round
#include <cstdio>        // for printf (if needed)
#include "driver/ledc.h"
#include "esp_timer.h"   // for esp_timer_get_time()
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "servo_control";

// ----------------- Configurable Constants -----------------

// For coordinate space (0..1600 x 0..1200)
constexpr int CAMERA_MAX_X    = 1600;
constexpr int CAMERA_MAX_Y    = 1200;

// Map coordinate -> servo angle in range 0..180°, inverted
// x=0 => angle=180, x=1600 => angle=0
// y=0 => angle=180, y=1200 => angle=0

// For servo signals
constexpr int SERVO_FREQUENCY   = 50;                // 50 Hz
constexpr auto SERVO_TIMER_BITS = LEDC_TIMER_14_BIT; // Up to 14 bits on ESP32-S3
constexpr int SERVO_MIN_US      = 500;
constexpr int SERVO_MAX_US      = 2500;
constexpr int SERVO_MAX_ANGLE   = 180;

// Pin numbers for your hardware
constexpr int SERVO1_PIN        = 8;   // Horizontal servo
constexpr int SERVO2_PIN        = 43;  // Vertical servo

// LEDC settings
constexpr auto SERVO1_CHANNEL   = LEDC_CHANNEL_0;
constexpr auto SERVO2_CHANNEL   = LEDC_CHANNEL_1;
constexpr auto SERVO_SPEED_MODE = LEDC_LOW_SPEED_MODE;
constexpr auto SERVO_TIMER_NUM  = LEDC_TIMER_0;

// Smoothing update settings
// - We'll update every 20 ms
// - We'll slowly approach the target angle
constexpr uint64_t updateIntervalMS = 20ULL; 
constexpr float     smoothingFactor = 0.02f; 

// ----------------- Internal State -----------------

static float currentServo1Angle = 90.0f;  // horizontal servo
static float targetServo1Angle  = 90.0f;

static float currentServo2Angle = 90.0f;  // vertical servo
static float targetServo2Angle  = 90.0f;

static uint64_t previousUpdateMillis = 0ULL;

// ----------------- Utility Functions -----------------

// Convert servo angle (0..180°) -> microseconds pulse
static uint32_t angleToPulseUs(float angle)
{
    float alpha = angle / (float)SERVO_MAX_ANGLE; // range 0..1
    float span  = (float)(SERVO_MAX_US - SERVO_MIN_US);
    return (uint32_t)(SERVO_MIN_US + alpha * span);
}

// Convert microseconds -> LEDC duty
static uint32_t pulseUsToDuty(uint32_t us)
{
    // 50 Hz => 1/50 => 20000 us
    const uint32_t period_us = 20000;
    uint32_t max_duty = (1 << SERVO_TIMER_BITS) - 1;  // e.g. 2^14 - 1 = 16383

    // duty = (us / period_us) * max_duty
    uint64_t duty = ( (uint64_t) us * max_duty ) / period_us;
    return (uint32_t) duty;
}

// Return current time in milliseconds
static uint64_t millis()
{
    return esp_timer_get_time() / 1000ULL;
}

// Helper to write an angle to a servo channel
static void setServoAngle(ledc_channel_t channel, float angle)
{
    uint32_t pulse_us = angleToPulseUs(angle);
    uint32_t duty     = pulseUsToDuty(pulse_us);

    ledc_set_duty(SERVO_SPEED_MODE, channel, duty);
    ledc_update_duty(SERVO_SPEED_MODE, channel);
}

// ----------------- Public Functions -----------------

void servo_control_init(void)
{
    ESP_LOGI(TAG, "Initializing servo control on pins %d (H), %d (V)",
             SERVO1_PIN, SERVO2_PIN);

    // 1) Configure the LEDC timer
    ledc_timer_config_t timer_conf = {};
    timer_conf.speed_mode      = SERVO_SPEED_MODE;
    timer_conf.timer_num       = SERVO_TIMER_NUM; 
    timer_conf.duty_resolution = SERVO_TIMER_BITS;
    timer_conf.freq_hz         = SERVO_FREQUENCY;
    timer_conf.clk_cfg         = LEDC_AUTO_CLK;

    ledc_timer_config(&timer_conf);

    // 2) Configure channels
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

    ESP_LOGI(TAG, "Servo Control init done. Starting angles at (%.2f°, %.2f°)",
             currentServo1Angle, currentServo2Angle);
}

void servo_control_setXY(int x, int y)
{
    // Bound the inputs to [0..CAMERA_MAX_X], [0..CAMERA_MAX_Y]
    if (x < 0) x = 0;
    if (x > CAMERA_MAX_X) x = CAMERA_MAX_X;
    if (y < 0) y = 0;
    if (y > CAMERA_MAX_Y) y = CAMERA_MAX_Y;

    // Map x=0 => angle=180, x=CAMERA_MAX_X => angle=0
    float angleH = 180.0f - ( (float)x * 180.0f / (float)CAMERA_MAX_X );
    // Map y=0 => angle=180, y=CAMERA_MAX_Y => angle=0
    float angleV = 180.0f - ( (float)y * 180.0f / (float)CAMERA_MAX_Y );

    targetServo1Angle = angleH;
    targetServo2Angle = angleV;

    ESP_LOGI(TAG, "servo_control_setXY(%d,%d) => angles (H=%.2f°, V=%.2f°)",
             x, y, angleH, angleV);
}

void servo_control_update(void)
{
    uint64_t now = millis();
    if ((now - previousUpdateMillis) < updateIntervalMS) {
        // Not time to update yet
        return;
    }
    previousUpdateMillis = now;

    // Smoothly approach target angles
    float diff1 = targetServo1Angle - currentServo1Angle;
    if (fabsf(diff1) < 0.1f) {
        currentServo1Angle = targetServo1Angle;
    } else {
        currentServo1Angle += diff1 * smoothingFactor;
    }
    setServoAngle(SERVO1_CHANNEL, currentServo1Angle);

    float diff2 = targetServo2Angle - currentServo2Angle;
    if (fabsf(diff2) < 0.1f) {
        currentServo2Angle = targetServo2Angle;
    } else {
        currentServo2Angle += diff2 * smoothingFactor;
    }
    setServoAngle(SERVO2_CHANNEL, currentServo2Angle);
}

// Optional demonstration: move horizontally, vertically, diagonal
void servo_control_test_sweeps(void)
{
    ESP_LOGI(TAG, "Starting servo_control_test_sweeps()...");

    // 1) Horizontal sweep from left (x=0) to right (x=1600) at a fixed y=600
    int fixedY = CAMERA_MAX_Y / 2; // 600
    for (int x = 0; x <= CAMERA_MAX_X; x += 100) {
        servo_control_setXY(x, fixedY);
        // Wait ~300ms, but keep calling servo_control_update in small steps
        // so it smooths gradually
        for (int ms = 0; ms < 300; ms += 20) {
            servo_control_update();
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    // 2) Vertical sweep from top (y=0) to bottom (y=1200) at x=800
    int fixedX = CAMERA_MAX_X / 2; // 800
    for (int y = 0; y <= CAMERA_MAX_Y; y += 100) {
        servo_control_setXY(fixedX, y);
        for (int ms = 0; ms < 300; ms += 20) {
            servo_control_update();
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    // 3) Diagonal: from top-left (0,0) to bottom-right (1600,1200)
    int stepCount = 16; // step in bigger increments to see a diagonal
    for (int i = 0; i <= stepCount; i++) {
        int x = (CAMERA_MAX_X * i) / stepCount;   // 0..1600
        int y = (CAMERA_MAX_Y * i) / stepCount;   // 0..1200
        servo_control_setXY(x, y);
        for (int ms = 0; ms < 300; ms += 20) {
            servo_control_update();
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    ESP_LOGI(TAG, "servo_control_test_sweeps() completed!");
}
