#pragma once

// ----------------------------------------------------------------------------
// SERVO CONFIGURATION CONSTANTS
// ----------------------------------------------------------------------------

// Coordinate space [0..CAMERA_MAX_X, 0..CAMERA_MAX_Y]
#define CAMERA_MAX_X         1600
#define CAMERA_MAX_Y         1200

// LEDC (PWM) frequency & resolution
#define SERVO_FREQUENCY      50               // 50 Hz
#define SERVO_TIMER_BITS     LEDC_TIMER_14_BIT // 14-bit resolution
#define SERVO_MIN_US         500
#define SERVO_MAX_US         2500
#define SERVO_MAX_ANGLE      180

// Pin numbers for your hardware (ESP32-S3 example)
#define SERVO1_PIN           8   // horizontal
#define SERVO2_PIN           43  // vertical

// LEDC channel assignments
#define SERVO1_CHANNEL       LEDC_CHANNEL_0
#define SERVO2_CHANNEL       LEDC_CHANNEL_1
#define SERVO_SPEED_MODE     LEDC_LOW_SPEED_MODE
#define SERVO_TIMER_NUM      LEDC_TIMER_0

// Smoothing update settings
#define SERVO_UPDATE_MS      20ULL
#define SERVO_SMOOTH_FACTOR  0.02f
