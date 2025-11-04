#include "driver/i2s.h"
#include "driver/rtc_io.h"
#include "esp32/rom/lldesc.h"
#include "esp_efuse.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/queue.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "video.h"
#include "lvgl_driver_video.h"
#include <driver/dac.h>
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <soc/rtc.h>
#include <stdio.h>

static const char *TAG = "DEMO";

// 声明外部图片数据
extern const lv_img_dsc_t rhino;

void demo_display_image(const GRAPHICS_MODE mode) {
  ESP_LOGI(TAG, "Single Screen Demo");
  lv_video_disp_init(mode);

  lv_obj_t *scr = lv_obj_create(NULL);

  lv_scr_load(scr);

  // 创建图片对象
  lv_obj_t *img = lv_img_create(scr);
  
  // 设置图片源为 rhino
  lv_img_set_src(img, &rhino);
  
  // 将图片居中显示
  lv_obj_center(img);

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void app_main(void) {
  ESP_ERROR_CHECK(uart_set_baudrate(UART_NUM_0, CONFIG_ESPTOOLPY_MONITOR_BAUD));
  assert(240 == CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ);
  demo_display_image(NTSC_320x200);  // 单个界面演示
}
