
set(srcs
    servo_controller.h
    servo_controller.c
)

set(include_dirs
    "."
)

idf_components_register(
    SRCS ${srcs}
    REQUIRES esp_wifi nvs_flash driver
    INCLUDE_DIRS ${include_dirs}
)

