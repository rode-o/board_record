#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parks both servos at 0° (using x=1600, y=1200 in your coordinate system),
 *        smoothly moves them over ~2 seconds. 
 * 
 * By default, it then stops. If you want to hold position in an infinite loop,
 * you can do so either inside this function or in main.cpp after calling it.
 */
void servo_control_park(void);

#ifdef __cplusplus
}
#endif
