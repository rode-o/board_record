#include "servo_control.h"
#include <cmath>         // for fabs, round
#include <cstdio>        // for printf (if needed)
#include "driver/ledc.h"
#include "esp_timer.h"   // for esp_timer_get_time()
#include "esp_random.h"  // for esp_random()
#include "esp_log.h"

static const char *TAG = "servo_control";

// -- Servo Config --
constexpr int SERVO_FREQUENCY   = 50;                // 50 Hz
constexpr auto SERVO_TIMER_BITS = LEDC_TIMER_14_BIT; // Up to 14 bits on ESP32-S3

// Typical servo pulse range
constexpr int SERVO_MIN_US      = 500;
constexpr int SERVO_MAX_US      = 2500;
constexpr int SERVO_MAX_ANGLE   = 180;

// Pin numbers: adapt to actual GPIO for your hardware
constexpr int SERVO1_PIN        = 7;
constexpr int SERVO2_PIN        = 44;

// LEDC settings
constexpr auto SERVO1_CHANNEL   = LEDC_CHANNEL_0;
constexpr auto SERVO2_CHANNEL   = LEDC_CHANNEL_1;
constexpr auto SERVO_SPEED_MODE = LEDC_LOW_SPEED_MODE; // S3 only has low-speed mode
constexpr auto SERVO_TIMER_NUM  = LEDC_TIMER_0;

// Smoothing / timing
static float currentServo1Angle = 90.0f;
static float targetServo1Angle  = 90.0f;
static float currentServo2Angle = 90.0f;
static float targetServo2Angle  = 90.0f;

static uint64_t previousMillis  = 0ULL;
constexpr uint64_t updateInterval = 20ULL; // 20 ms
constexpr float smoothingFactor    = 0.02f;

static uint64_t lastTargetChange = 0ULL;
constexpr uint64_t trackingDelay = 5000ULL; // 5 seconds

// Convert angle -> microseconds pulse
static uint32_t angleToPulseUs(float angle) {
    float alpha = angle / static_cast<float>(SERVO_MAX_ANGLE); // 0..1
    float span  = static_cast<float>(SERVO_MAX_US - SERVO_MIN_US);
    return static_cast<uint32_t>(SERVO_MIN_US + alpha * span);
}

// Convert microseconds -> LEDC duty
static uint32_t pulseUsToDuty(uint32_t us) {
    const uint32_t period_us = 20000;  // 1 / 50 Hz = 20 ms
    // For 14-bit resolution: 2^14 - 1 = 16383
    uint32_t max_duty = (1 << SERVO_TIMER_BITS) - 1;

    // duty = (us / period_us) * max_duty
    uint64_t duty = static_cast<uint64_t>(us) * max_duty / period_us;
    return static_cast<uint32_t>(duty);
}

// Return current time in ms (like Arduino's millis())
static uint64_t millis() {
    return esp_timer_get_time() / 1000ULL;
}

// A random function from [low..high)
static int randomRange(int low, int high) {
    // esp_random() is a 32-bit random
    return static_cast<int>(esp_random() % (high - low)) + low;
}

// Helper to set servo angle on a given channel
static void setServoAngle(ledc_channel_t channel, float angle) {
    uint32_t pulse_us = angleToPulseUs(angle);
    uint32_t duty     = pulseUsToDuty(pulse_us);

    ledc_set_duty(SERVO_SPEED_MODE, channel, duty);
    ledc_update_duty(SERVO_SPEED_MODE, channel);
}

void servo_control_init() {
    ESP_LOGI(TAG, "Initializing servo control for pins %d and %d", SERVO1_PIN, SERVO2_PIN);

    // 1) Configure the LEDC timer
    ledc_timer_config_t timer_conf = {}; // zero-init
    timer_conf.speed_mode      = SERVO_SPEED_MODE;
    timer_conf.timer_num       = SERVO_TIMER_NUM; 
    timer_conf.duty_resolution = SERVO_TIMER_BITS;
    timer_conf.freq_hz         = SERVO_FREQUENCY;
    timer_conf.clk_cfg         = LEDC_AUTO_CLK;
    // If your IDF has "bool deconfigure", set it:
    // timer_conf.deconfigure = false;

    ledc_timer_config(&timer_conf);

    // 2) Configure channels
    ledc_channel_config_t ch1 = {};
    ch1.speed_mode     = SERVO_SPEED_MODE;
    ch1.channel        = SERVO1_CHANNEL;
    ch1.timer_sel      = SERVO_TIMER_NUM; // or .timer_num
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

    // Set initial angles
    setServoAngle(SERVO1_CHANNEL, currentServo1Angle);
    setServoAngle(SERVO2_CHANNEL, currentServo2Angle);

    ESP_LOGI(TAG, "Servo Control init done. Starting angles at (%.2f, %.2f)",
             currentServo1Angle, currentServo2Angle);
}


void servo_control_update() {
    uint64_t now = millis();

    // Generate new "camera inputs" every 5 seconds
    if ((now - lastTargetChange) >= trackingDelay) {
        lastTargetChange = now;

        // Horizontal: 0..1600 => invert 0->180
        int cameraX = randomRange(0, 1601);
        float angle1 = 180.0f - (cameraX * 180.0f / 1600.0f);
        targetServo1Angle = angle1;
        ESP_LOGI(TAG, "Horizontal camera input: %d -> Target angle: %.2f", cameraX, angle1);

        // Vertical: 0..1200 => invert 0->180
        int cameraY = randomRange(0, 1201);
        float angle2 = 180.0f - (cameraY * 180.0f / 1200.0f);
        targetServo2Angle = angle2;
        ESP_LOGI(TAG, "Vertical camera input: %d -> Target angle: %.2f", cameraY, angle2);
    }

    // Smoothly update servos every 20 ms
    if ((now - previousMillis) >= updateInterval) {
        previousMillis = now;

        // Horizontal servo
        float diff1 = targetServo1Angle - currentServo1Angle;
        if (std::fabs(diff1) < 0.1f) {
            currentServo1Angle = targetServo1Angle;
        } else {
            currentServo1Angle += diff1 * smoothingFactor;
        }
        setServoAngle(SERVO1_CHANNEL, currentServo1Angle);

        // Vertical servo
        float diff2 = targetServo2Angle - currentServo2Angle;
        if (std::fabs(diff2) < 0.1f) {
            currentServo2Angle = targetServo2Angle;
        } else {
            currentServo2Angle += diff2 * smoothingFactor;
        }
        setServoAngle(SERVO2_CHANNEL, currentServo2Angle);
    }
}
