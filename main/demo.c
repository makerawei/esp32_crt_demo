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
#include <driver/dac.h>
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <soc/rtc.h>
#include <stdio.h>
#include "lvgl_driver_video.h"

static const char *TAG = "DEMO";

void demo_pm5544(lv_obj_t *scr) {
  char mode[14];
  video_get_mode_description(mode, sizeof(mode));

  LV_IMG_DECLARE(pm5544);
  lv_obj_t *img1 = lv_img_create(scr);
  lv_img_set_src(img1, &pm5544);
  lv_obj_set_align(img1, LV_ALIGN_CENTER);
  lv_obj_set_size(img1, pm5544.header.w, pm5544.header.h);
  lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

  lv_obj_t *top_label = lv_label_create(scr);
  lv_obj_set_width(top_label, 55);
  lv_obj_set_style_text_color(top_label, lv_color_white(), LV_STATE_DEFAULT);
  lv_obj_set_style_text_align(top_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(top_label, LV_ALIGN_CENTER, 0, -83);

  lv_obj_t *mode_text = lv_label_create(scr);
  lv_obj_set_width(mode_text, 75);
  lv_obj_set_style_text_color(mode_text, lv_color_white(), LV_STATE_DEFAULT);
  lv_obj_align(mode_text, LV_ALIGN_CENTER, 0, 66);
  lv_obj_set_style_text_align(mode_text, LV_TEXT_ALIGN_CENTER, 0);

  char *t = strtok(mode, " ");
  lv_label_set_text(top_label, t);
  t = strtok(NULL, " ");
  lv_label_set_text(mode_text, t);
}

void demo_rhino(lv_obj_t *scr) {
  LV_IMG_DECLARE(rhino);
  lv_obj_t *img1 = lv_img_create(scr);
  lv_img_set_src(img1, &rhino);
  lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_size(img1, rhino.header.w, rhino.header.h);
}

#define MAX_DEMO_COUNT 2
lv_obj_t *screens[MAX_DEMO_COUNT];
static int screen_index = 0;

void swap_screen_timer_callback(lv_timer_t *timer) {
  lv_scr_load_anim(screens[screen_index], LV_SCR_LOAD_ANIM_MOVE_LEFT, 1000,
                   1000, false);
  screen_index++;
  if (screen_index >= MAX_DEMO_COUNT) {
    screen_index = 0;
  }
}

void slides_demo(void) {
  for (size_t i = 0; i < MAX_DEMO_COUNT; i++) {
    screens[i] = lv_obj_create(NULL);
  }

  size_t index = 0;
  demo_pm5544(screens[index++]);
  demo_rhino(screens[index++]);

  assert(index <= MAX_DEMO_COUNT);

  lv_disp_load_scr(screens[screen_index++]);
  const uint32_t timeout_ms = 10000;
  lv_timer_create(swap_screen_timer_callback, timeout_ms, NULL);
}

void run_demo_slides(void) {
#if LV_COLOR_DEPTH < 16
  const size_t pixel_count = 320 * 200;
  lv_color_t *lvgl_pixel_buffer =
      heap_caps_malloc(sizeof(lv_color_t) * pixel_count, MALLOC_CAP_8BIT);
  assert(lvgl_pixel_buffer);

  lv_video_disp_init_buf(PAL_320x200, lvgl_pixel_buffer, pixel_count);
#else
  // no memory for buffers
  ESP_LOGI(
      TAG,
      "Using direct framebuffer access. Expect tearing effect for animations.");
  lv_video_disp_init(PAL_320x200);
#endif
  ESP_LOGI(TAG, "Slides Demo");

  slides_demo();

  while (1) {
    lv_task_handler();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void app_main(void) {
  ESP_ERROR_CHECK(uart_set_baudrate(UART_NUM_0, CONFIG_ESPTOOLPY_MONITOR_BAUD));

  esp_log_level_set("*", ESP_LOG_DEBUG);

  ESP_LOGI(TAG, "Application start...");
  ESP_LOGD(TAG, "DEBUG Output enabled");

  ESP_LOGI(TAG, "CPU Speed %d MHz", CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ);
  assert(240 == CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ);

  run_demo_slides();
}
