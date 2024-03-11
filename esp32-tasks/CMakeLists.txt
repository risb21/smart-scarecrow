# get IDF version for comparison
set(idf_version "${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}")

# set conversion sources
set(srcs
  **/lib/esp32-camera/conversions/yuv.c
  **/lib/esp32-camera/conversions/to_jpg.cpp
  **/lib/esp32-camera/conversions/to_bmp.c
  **/lib/esp32-camera/conversions/jpge.cpp
  **/lib/esp32-camera/conversions/esp_jpg_decode.c
  )

set(priv_include_dirs
**/lib/esp32-camera/conversions/private_include
  )

set(include_dirs
  **/lib/esp32-camera/driver/include
  **/lib/esp32-camera/conversions/include
  )

set(COMPONENT_REQUIRES driver)

# set driver sources only for supported platforms
if(IDF_TARGET STREQUAL "esp32" OR IDF_TARGET STREQUAL "esp32s2" OR IDF_TARGET STREQUAL "esp32s3")
  list(APPEND srcs
    **/lib/esp32-camera/driver/esp_camera.c
    **/lib/esp32-camera/driver/cam_hal.c
    **/lib/esp32-camera/driver/sccb.c
    **/lib/esp32-camera/driver/sensor.c
    **/lib/esp32-camera/sensors/ov2640.c
    **/lib/esp32-camera/sensors/ov3660.c
    **/lib/esp32-camera/sensors/ov5640.c
    **/lib/esp32-camera/sensors/ov7725.c
    **/lib/esp32-camera/sensors/ov7670.c
    **/lib/esp32-camera/sensors/nt99141.c
    **/lib/esp32-camera/sensors/gc0308.c
    **/lib/esp32-camera/sensors/gc2145.c
    **/lib/esp32-camera/sensors/gc032a.c
    **/lib/esp32-camera/sensors/bf3005.c
    **/lib/esp32-camera/sensors/bf20a6.c
    **/lib/esp32-camera/sensors/sc101iot.c
    **/lib/esp32-camera/sensors/sc030iot.c
    **/lib/esp32-camera/sensors/sc031gs.c
    )

  list(APPEND priv_include_dirs
    **/lib/esp32-camera/driver/private_include
    **/lib/esp32-camera/sensors/private_include
    **/lib/esp32-camera/target/private_include
    )

  if(IDF_TARGET STREQUAL "esp32")
    list(APPEND srcs
      **/lib/esp32-camera/target/xclk.c
      **/lib/esp32-camera/target/esp32/ll_cam.c
      )
  endif()

  if(IDF_TARGET STREQUAL "esp32s2")
    list(APPEND srcs
      **/lib/esp32-camera/target/xclk.c
      **/lib/esp32-camera/target/esp32s2/ll_cam.c
      **/lib/esp32-camera/target/tjpgd.c
      )

    list(APPEND priv_include_dirs
      **/lib/esp32-camera/target/esp32s2/private_include
      )
  endif()

  if(IDF_TARGET STREQUAL "esp32s3")
    list(APPEND srcs
      **/lib/esp32-camera/target/esp32s3/ll_cam.c
      )
  endif()

  set(priv_requires freertos nvs_flash)

  set(min_version_for_esp_timer "4.2")
  if (idf_version VERSION_GREATER_EQUAL min_version_for_esp_timer)
    list(APPEND priv_requires esp_timer)
  endif()

endif()

# CONFIG_ESP_ROM_HAS_JPEG_DECODE is available from IDF v4.4 but
# previous IDF supported chips already support JPEG decoder, hence okay to use this
if(idf_version VERSION_GREATER_EQUAL "4.4" AND NOT CONFIG_ESP_ROM_HAS_JPEG_DECODE)
  list(APPEND srcs
    **/lib/esp32-camera/target/tjpgd.c
  )
  list(APPEND priv_include_dirs
    **/lib/esp32-camera/target/jpeg_include/
  )
endif()

idf_component_register(
  SRCS ${srcs}
  INCLUDE_DIRS ${include_dirs}
  PRIV_INCLUDE_DIRS ${priv_include_dirs}
  REQUIRES driver  # due to include of driver/gpio.h in esp_camera.h
  PRIV_REQUIRES ${priv_requires}
)