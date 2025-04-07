#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
//  Public API for controlling 2-axis servo system
// ---------------------------------------------------------------------------

/**
 * @brief Initialize the servo system (LEDC driver, default angles).
 */
void servo_control_init(void);

/**
 * @brief Sets an (x,y) coordinate in [0..1600, 0..1200].
 *        This internally computes angles and sets them as targets.
 * @param x coordinate [0..CAMERA_MAX_X]
 * @param y coordinate [0..CAMERA_MAX_Y]
 */
void servo_control_setXY(int x, int y);

/**
 * @brief Must be called periodically (e.g., every 20 ms).
 *        Smoothly updates the servo angles from current to target.
 */
void servo_control_update(void);

#ifdef __cplusplus
}
#endif
