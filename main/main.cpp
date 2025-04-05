#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// If your pedestrian_detect code is also in C++, #include it here
// #include "some_pedestrian_header.h"

extern "C" {
    #include "servo_control.h"
}

extern "C" void app_main(void)
{
    printf("Starting 2-axis smooth tracking in C++...\n");

    // 1) Initialize the servo driver
    servo_control_init();

    // 2) Optionally init your pedestrian detection stuff here
    // init_pedestrian_detect();

    // 3) Main loop
    while (true) {
        // Update servo logic
        servo_control_update();

        // Optional: run pedestrian detection, etc.
        // run_pedestrian_detect();

        // 10 ms delay so we don't spin too tight
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
