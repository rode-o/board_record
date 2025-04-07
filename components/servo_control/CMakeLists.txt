idf_component_register(
    SRCS
        "src/servo_core.cpp"
        "src/servo_demo.cpp"
        "src/servo_utils.cpp"   # <-- newly added
    INCLUDE_DIRS
        "include"
    REQUIRES
        driver
        freertos
        esp_timer
        log
)
