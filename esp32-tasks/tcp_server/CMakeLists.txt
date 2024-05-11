set(srcs
    "tcp_servo_control.c"
    "tcp_servo_control.h",
    "../servo_control/servo_controller.c"
    "../servo_control/servo_controller.h"
  )

idf_component_register(
  SRCS ${srcs}
  REQUIRES driver nvs_flash
  INCLUDE_DIRS "."
)
