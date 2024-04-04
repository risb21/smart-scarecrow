

set(srcs
    "udp_socket.c"
    "udp_socket.h"
    "server_config.h"
    # "wifi_config.h"
)

idf_components_register(
    SRCS ${srcs}
    REQUIRES esp_wifi nvs_flash
    INCLUDE_DIRS "."
)

