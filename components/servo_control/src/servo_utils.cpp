#include "servo_utils.h"
#include "servo_control.h"
#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * Parks both servos at 0°, i.e. x=1600 => horizontal=0°, y=1200 => vertical=0°.
 * Then does a 2 second loop calling servo_control_update() so it can finish movement.
 * After that, it returns, leaving the servos at 0°.
 */
void servo_control_park(void)
{
    // x=1600 => horizontal servo angle=0°, y=1200 => vertical servo angle=0°
    int xForZeroAngle = 1600;
    int yForZeroAngle = 1200;

    printf("Parking servos at 0°...\n");

    // Send the command
    servo_control_setXY(xForZeroAngle, yForZeroAngle);

    // Let them move smoothly for 2 seconds
    for (int ms = 0; ms < 2000; ms += 20) {
        servo_control_update();
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    printf("Servo park complete. Servos at 0°.\n");
}
