idf_component_register(
    SRCS "demo.c" "rhino.c" "pm5544.c" lv_font_msyh_20.c
    INCLUDE_DIRS ""
    REQUIRES esp32_composite_video_lib lvgl nvs_flash driver esp_wifi
)
