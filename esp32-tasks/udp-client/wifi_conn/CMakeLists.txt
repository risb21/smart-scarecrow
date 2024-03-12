set(srcs
    "wifi_connection.c"
    "wifi_connection.h"
    "wifi_config.h"
)

idf_components_register(
    SRCS ${srcs}
    REQUIRES esp_wifi nvs_flash
    INCLUDE_DIRS "."
)