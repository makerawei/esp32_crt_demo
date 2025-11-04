idf_component_register(
    SRCS "demo.c" "rhino.c"
    INCLUDE_DIRS ""
    REQUIRES esp32_composite_video_lib lvgl nvs_flash driver esp_wifi
)
