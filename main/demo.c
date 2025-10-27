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
#define CONFIG_VIDEO_ENABLE_LVGL_SUPPORT 1
#if CONFIG_VIDEO_ENABLE_LVGL_SUPPORT
#include "lvgl_driver_video.h"
#endif

static const char *TAG = "DEMO";

#if CONFIG_VIDEO_ENABLE_LVGL_SUPPORT
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

static lv_obj_t *meter;
static void meter_set_value(void *indic, int32_t v) {
  lv_meter_set_indicator_value(meter, indic, v);
}

void demo_meter(lv_obj_t *scr) {
  meter = lv_meter_create(scr);
  lv_obj_center(meter);
  lv_obj_set_size(meter, 200, 200);

  /*Add a scale first*/
  lv_meter_scale_t *scale = lv_meter_add_scale(meter);
  lv_meter_set_scale_ticks(meter, scale, 41, 2, 10,
                           lv_palette_main(LV_PALETTE_GREY));
  lv_meter_set_scale_major_ticks(meter, scale, 8, 4, 15, lv_color_black(), 10);

  lv_meter_indicator_t *indic;

  /*Add a blue arc to the start*/
  indic =
      lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_BLUE), 0);
  lv_meter_set_indicator_start_value(meter, indic, 0);
  lv_meter_set_indicator_end_value(meter, indic, 20);

  /*Make the tick lines blue at the start of the scale*/
  indic =
      lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_BLUE),
                               lv_palette_main(LV_PALETTE_BLUE), false, 0);
  lv_meter_set_indicator_start_value(meter, indic, 0);
  lv_meter_set_indicator_end_value(meter, indic, 20);

  /*Add a red arc to the end*/
  indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_RED), 0);
  lv_meter_set_indicator_start_value(meter, indic, 80);
  lv_meter_set_indicator_end_value(meter, indic, 100);

  /*Make the tick lines red at the end of the scale*/
  indic =
      lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_RED),
                               lv_palette_main(LV_PALETTE_RED), false, 0);
  lv_meter_set_indicator_start_value(meter, indic, 80);
  lv_meter_set_indicator_end_value(meter, indic, 100);

  /*Add a needle line indicator*/
  indic = lv_meter_add_needle_line(meter, scale, 4,
                                   lv_palette_main(LV_PALETTE_GREY), -10);

  /*Create an animation to set the value*/
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_exec_cb(&a, meter_set_value);
  lv_anim_set_var(&a, indic);
  lv_anim_set_values(&a, 0, 100);
  lv_anim_set_time(&a, 2000);
  lv_anim_set_repeat_delay(&a, 100);
  lv_anim_set_playback_time(&a, 500);
  lv_anim_set_playback_delay(&a, 100);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_start(&a);
}

void create_label(lv_style_t *style, lv_obj_t *cont, const char *text,
                  uint8_t row, uint8_t col) {
  lv_obj_t *obj = lv_label_create(cont);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, col, 1,
                       LV_GRID_ALIGN_STRETCH, row, 1);

  lv_label_set_text(obj, text);
  lv_obj_add_style(obj, style, 0);
}

const char *get_chip_name(esp_chip_model_t model) {
  switch (model) {
  case CHIP_ESP32:
    return "ESP32";
    break;

  case CHIP_ESP32S2:
    return "ESP32-S2";
    break;

  case CHIP_ESP32S3:
    return "ESP32-S3";
    break;

  case CHIP_ESP32C3:
    return "ESP32-C3";
    break;

  case CHIP_ESP32H2:
    return "ESP32-H2";
    break;

  default:
    return "UNKNOWN";
    break;
  }
}

void demo_system_info(lv_obj_t *scr) {
  lv_obj_t *label1 = lv_label_create(scr);
  lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 0);
  lv_label_set_text(label1, "System Information");

  static lv_coord_t col_dsc[] = {160, 80, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {20, 20, 20, 20, 20, LV_GRID_TEMPLATE_LAST};

  lv_obj_t *cont = lv_obj_create(scr);
  lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
  lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
  lv_obj_set_size(cont, 300, 170);
  lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_set_layout(cont, LV_LAYOUT_GRID);

  static lv_style_t style;
  lv_style_init(&style);
  lv_style_set_radius(&style, 5);
  lv_style_set_border_width(&style, 1);
  lv_style_set_bg_opa(&style, LV_OPA_COVER);
  lv_style_set_bg_color(&style, lv_palette_lighten(LV_PALETTE_BLUE, 1));
  lv_style_set_border_color(&style, lv_color_black());
  lv_style_set_pad_left(&style, 5);
  lv_style_set_border_side(&style,
                           LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_RIGHT);

  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  char value[32];
  const char *const labels[] = {"Chip", "Chip cores", "Bluetooth",
                                "Silicon rev", "FLASH"};
  for (int row = 0; row < 5; row++) {
    create_label(&style, cont, labels[row], row, 0);

    switch (row) {
    case 0:
      snprintf(value, sizeof(value), "%s", get_chip_name(chip_info.model));
      break;

    case 1:
      snprintf(value, sizeof(value), "%d", chip_info.cores);
      break;

    case 2:
      snprintf(value, sizeof(value), "%s %s",
               (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
               (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "");
      break;

    case 3:
      snprintf(value, sizeof(value), "%d", chip_info.revision);
      break;

    case 4:
      snprintf(value, sizeof(value), "%d MiB",
               spi_flash_get_chip_size() / (1024 * 1024));
      break;
    }

    create_label(&style, cont, value, row, 1);
  }
}

#define MAX_DEMO_COUNT 4
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
  demo_system_info(screens[index++]);
  demo_meter(screens[index++]);
  demo_rhino(screens[index++]);

  assert(index <= MAX_DEMO_COUNT);

  lv_disp_load_scr(screens[screen_index++]);
  const uint32_t timeout_ms = 10000;
  lv_timer_create(swap_screen_timer_callback, timeout_ms, NULL);
}

void run_demo_single_slide(const GRAPHICS_MODE mode) {
  ESP_LOGI(TAG, "Single Screen Demo");
  const int delta = 20;

  lv_video_disp_init(mode);

  lv_obj_t *scr = lv_obj_create(NULL);
  demo_pm5544(scr);
  lv_scr_load(scr);

#if CONFIG_VIDEO_DIAG_ENABLE_INTERRUPT_STATS
  int n = 0;
#endif

  while (1) {
    lv_task_handler();

    vTaskDelay(delta / portTICK_PERIOD_MS);

#if CONFIG_VIDEO_DIAG_ENABLE_INTERRUPT_STATS
    if (n++ > 1000 / delta) {
      video_show_stats();
      n = 0;
    }
#endif
  }
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

#if CONFIG_VIDEO_DIAG_ENABLE_INTERRUPT_STATS
  int n = 0;
#endif
  while (1) {
    lv_task_handler();

    vTaskDelay(20 / portTICK_PERIOD_MS);

#if CONFIG_VIDEO_DIAG_ENABLE_INTERRUPT_STATS
    if (n++ > 50) {
      video_show_stats();
      n = 0;
    }
#endif
  }
}

void run_demo_lvgl(void)
{
#if LV_COLOR_DEPTH < 16
  const size_t pixel_count = 320 * 200;
  lv_color_t *lvgl_pixel_buffer =
      heap_caps_malloc(sizeof(lv_color_t) * pixel_count, MALLOC_CAP_8BIT);
  assert(lvgl_pixel_buffer);

  lv_video_disp_init_buf(NTSC_320x200, lvgl_pixel_buffer, pixel_count);
#else
  // no memory for buffers
  ESP_LOGI(TAG, "Using direct framebuffer access.");
  lv_video_disp_init(NTSC_320x200);
#endif
  ESP_LOGI(TAG, "Custom Rectangles Demo");

  // 创建屏幕
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_scr_load(scr);

  // 设置屏幕背景色为深灰色
  lv_obj_set_style_bg_color(scr, lv_color_make(40, 40, 40), LV_STATE_DEFAULT);

  // 创建标题
  lv_obj_t* title = lv_label_create(scr);
  lv_label_set_text(title, "Rectangles Demo");
  lv_obj_set_style_text_color(title, lv_color_white(), LV_STATE_DEFAULT);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);

  // 矩形1: 填充的红色矩形
  lv_obj_t* rect1 = lv_obj_create(scr);
  lv_obj_set_size(rect1, 60, 40);
  lv_obj_set_pos(rect1, 20, 30);
  lv_obj_set_style_bg_color(rect1, lv_palette_main(LV_PALETTE_RED), LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(rect1, 2, LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(rect1, lv_color_white(), LV_STATE_DEFAULT);

  // 矩形2: 不填充的蓝色边框矩形
  lv_obj_t* rect2 = lv_obj_create(scr);
  lv_obj_set_size(rect2, 60, 40);
  lv_obj_set_pos(rect2, 90, 30);
  lv_obj_set_style_bg_opa(rect2, LV_OPA_TRANSP, LV_STATE_DEFAULT);  // 透明背景
  lv_obj_set_style_border_width(rect2, 3, LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(rect2, lv_palette_main(LV_PALETTE_BLUE), LV_STATE_DEFAULT);

  // 矩形3: 填充的绿色矩形
  lv_obj_t* rect3 = lv_obj_create(scr);
  lv_obj_set_size(rect3, 60, 40);
  lv_obj_set_pos(rect3, 160, 30);
  lv_obj_set_style_bg_color(rect3, lv_palette_main(LV_PALETTE_GREEN), LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(rect3, 2, LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(rect3, lv_color_black(), LV_STATE_DEFAULT);
  lv_obj_set_style_radius(rect3, 10, LV_STATE_DEFAULT);  // 圆角

  // 矩形4: 不填充的黄色边框矩形
  lv_obj_t* rect4 = lv_obj_create(scr);
  lv_obj_set_size(rect4, 60, 40);
  lv_obj_set_pos(rect4, 230, 30);
  lv_obj_set_style_bg_opa(rect4, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(rect4, 4, LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(rect4, lv_palette_main(LV_PALETTE_YELLOW), LV_STATE_DEFAULT);

  // 矩形5: 填充的紫色矩形（大尺寸）
  lv_obj_t* rect5 = lv_obj_create(scr);
  lv_obj_set_size(rect5, 80, 60);
  lv_obj_set_pos(rect5, 20, 85);
  lv_obj_set_style_bg_color(rect5, lv_palette_main(LV_PALETTE_PURPLE), LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(rect5, 2, LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(rect5, lv_palette_lighten(LV_PALETTE_PURPLE, 2), LV_STATE_DEFAULT);

  // 矩形6: 不填充的青色边框矩形（大尺寸）
  lv_obj_t* rect6 = lv_obj_create(scr);
  lv_obj_set_size(rect6, 80, 60);
  lv_obj_set_pos(rect6, 110, 85);
  lv_obj_set_style_bg_opa(rect6, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(rect6, 5, LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(rect6, lv_palette_main(LV_PALETTE_CYAN), LV_STATE_DEFAULT);
  lv_obj_set_style_radius(rect6, 15, LV_STATE_DEFAULT);

  // 矩形7: 半透明填充的橙色矩形
  lv_obj_t* rect7 = lv_obj_create(scr);
  lv_obj_set_size(rect7, 80, 60);
  lv_obj_set_pos(rect7, 200, 85);
  lv_obj_set_style_bg_color(rect7, lv_palette_main(LV_PALETTE_ORANGE), LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(rect7, LV_OPA_70, LV_STATE_DEFAULT);  // 70% 不透明度
  lv_obj_set_style_border_width(rect7, 3, LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(rect7, lv_color_white(), LV_STATE_DEFAULT);

  // 矩形8: 不填充的白色虚线边框矩形
  lv_obj_t* rect8 = lv_obj_create(scr);
  lv_obj_set_size(rect8, 100, 30);
  lv_obj_set_pos(rect8, 110, 160);
  lv_obj_set_style_bg_opa(rect8, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(rect8, 2, LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(rect8, lv_color_white(), LV_STATE_DEFAULT);

  // 添加说明文字
  lv_obj_t* info = lv_label_create(scr);
  lv_label_set_text(info, "Filled & Outlined Rectangles");
  lv_obj_set_style_text_color(info, lv_palette_lighten(LV_PALETTE_GREY, 3), LV_STATE_DEFAULT);
  lv_obj_align(info, LV_ALIGN_BOTTOM_MID, 0, -5);

#if CONFIG_VIDEO_DIAG_ENABLE_INTERRUPT_STATS
  int n = 0;
#endif

  // 主循环
  while (1) {
    lv_task_handler();
    vTaskDelay(20 / portTICK_PERIOD_MS);

#if CONFIG_VIDEO_DIAG_ENABLE_INTERRUPT_STATS
    if (n++ > 50) {
      video_show_stats();
      n = 0;
    }
#endif
  }
}

// ==================== 贪吃蛇游戏 ====================
#define SNAKE_GRID_WIDTH 32
#define SNAKE_GRID_HEIGHT 20
#define SNAKE_CELL_SIZE 10
#define SNAKE_MAX_LENGTH (SNAKE_GRID_WIDTH * SNAKE_GRID_HEIGHT)

typedef enum {
  DIR_UP,
  DIR_DOWN,
  DIR_LEFT,
  DIR_RIGHT
} snake_direction_t;

typedef struct {
  int16_t x;
  int16_t y;
} snake_point_t;

typedef struct {
  snake_point_t body[SNAKE_MAX_LENGTH];
  uint16_t length;
  snake_direction_t direction;
  snake_point_t food;
  uint32_t score;
  bool game_over;
  lv_obj_t* canvas;
  lv_color_t* canvas_buf;
} snake_game_t;

static snake_game_t snake_game;

// 生成随机食物位置
void snake_generate_food(void) {
  bool valid = false;
  while (!valid) {
    snake_game.food.x = esp_random() % SNAKE_GRID_WIDTH;
    snake_game.food.y = esp_random() % SNAKE_GRID_HEIGHT;
    
    // 检查食物是否在蛇身上
    valid = true;
    for (uint16_t i = 0; i < snake_game.length; i++) {
      if (snake_game.body[i].x == snake_game.food.x && 
          snake_game.body[i].y == snake_game.food.y) {
        valid = false;
        break;
      }
    }
  }
}

// 初始化贪吃蛇游戏
void snake_init(void) {
  snake_game.length = 3;
  snake_game.direction = DIR_RIGHT;
  snake_game.score = 0;
  snake_game.game_over = false;
  
  // 初始化蛇的位置（屏幕中央）
  snake_game.body[0].x = SNAKE_GRID_WIDTH / 2;
  snake_game.body[0].y = SNAKE_GRID_HEIGHT / 2;
  snake_game.body[1].x = snake_game.body[0].x - 1;
  snake_game.body[1].y = snake_game.body[0].y;
  snake_game.body[2].x = snake_game.body[0].x - 2;
  snake_game.body[2].y = snake_game.body[0].y;
  
  snake_generate_food();
}

// 简单的AI：使用曼哈顿距离寻找食物
void snake_ai_move(void) {
  snake_point_t head = snake_game.body[0];
  int16_t dx = snake_game.food.x - head.x;
  int16_t dy = snake_game.food.y - head.y;
  
  snake_direction_t new_dir = snake_game.direction;
  
  // 优先处理距离更远的轴
  if (abs(dx) > abs(dy)) {
    // 水平方向优先
    if (dx > 0 && snake_game.direction != DIR_LEFT) {
      new_dir = DIR_RIGHT;
    } else if (dx < 0 && snake_game.direction != DIR_RIGHT) {
      new_dir = DIR_LEFT;
    } else if (dy > 0 && snake_game.direction != DIR_UP) {
      new_dir = DIR_DOWN;
    } else if (dy < 0 && snake_game.direction != DIR_DOWN) {
      new_dir = DIR_UP;
    }
  } else {
    // 垂直方向优先
    if (dy > 0 && snake_game.direction != DIR_UP) {
      new_dir = DIR_DOWN;
    } else if (dy < 0 && snake_game.direction != DIR_DOWN) {
      new_dir = DIR_UP;
    } else if (dx > 0 && snake_game.direction != DIR_LEFT) {
      new_dir = DIR_RIGHT;
    } else if (dx < 0 && snake_game.direction != DIR_RIGHT) {
      new_dir = DIR_LEFT;
    }
  }
  
  // 检查新方向是否会立即撞到自己的身体
  snake_point_t next_head = head;
  switch (new_dir) {
    case DIR_UP:    next_head.y--; break;
    case DIR_DOWN:  next_head.y++; break;
    case DIR_LEFT:  next_head.x--; break;
    case DIR_RIGHT: next_head.x++; break;
  }
  
  // 如果新方向会撞到身体，尝试其他方向
  bool will_hit_body = false;
  for (uint16_t i = 1; i < snake_game.length; i++) {
    if (snake_game.body[i].x == next_head.x && snake_game.body[i].y == next_head.y) {
      will_hit_body = true;
      break;
    }
  }
  
  if (will_hit_body) {
    // 尝试其他可行方向
    snake_direction_t try_dirs[] = {DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};
    for (int i = 0; i < 4; i++) {
      if (try_dirs[i] == snake_game.direction) continue;
      if ((try_dirs[i] == DIR_UP && snake_game.direction == DIR_DOWN) ||
          (try_dirs[i] == DIR_DOWN && snake_game.direction == DIR_UP) ||
          (try_dirs[i] == DIR_LEFT && snake_game.direction == DIR_RIGHT) ||
          (try_dirs[i] == DIR_RIGHT && snake_game.direction == DIR_LEFT)) {
        continue; // 不能反向
      }
      
      next_head = head;
      switch (try_dirs[i]) {
        case DIR_UP:    next_head.y--; break;
        case DIR_DOWN:  next_head.y++; break;
        case DIR_LEFT:  next_head.x--; break;
        case DIR_RIGHT: next_head.x++; break;
      }
      
      bool safe = true;
      for (uint16_t j = 1; j < snake_game.length; j++) {
        if (snake_game.body[j].x == next_head.x && snake_game.body[j].y == next_head.y) {
          safe = false;
          break;
        }
      }
      
      if (safe) {
        new_dir = try_dirs[i];
        break;
      }
    }
  }
  
  snake_game.direction = new_dir;
}

// 更新贪吃蛇游戏逻辑
void snake_update(void) {
  if (snake_game.game_over) return;
  
  // AI 决定移动方向
  snake_ai_move();
  
  // 计算新的头部位置
  snake_point_t new_head = snake_game.body[0];
  switch (snake_game.direction) {
    case DIR_UP:
      new_head.y--;
      break;
    case DIR_DOWN:
      new_head.y++;
      break;
    case DIR_LEFT:
      new_head.x--;
      break;
    case DIR_RIGHT:
      new_head.x++;
      break;
  }
  
  // 检查边界碰撞
  if (new_head.x < 0 || new_head.x >= SNAKE_GRID_WIDTH ||
      new_head.y < 0 || new_head.y >= SNAKE_GRID_HEIGHT) {
    snake_game.game_over = true;
    ESP_LOGI(TAG, "Game Over! Score: %d", snake_game.score);
    return;
  }
  
  // 检查自身碰撞
  for (uint16_t i = 0; i < snake_game.length; i++) {
    if (snake_game.body[i].x == new_head.x && snake_game.body[i].y == new_head.y) {
      snake_game.game_over = true;
      ESP_LOGI(TAG, "Game Over! Score: %d", snake_game.score);
      return;
    }
  }
  
  // 移动蛇身
  for (int16_t i = snake_game.length - 1; i > 0; i--) {
    snake_game.body[i] = snake_game.body[i - 1];
  }
  snake_game.body[0] = new_head;
  
  // 检查是否吃到食物
  if (new_head.x == snake_game.food.x && new_head.y == snake_game.food.y) {
    if (snake_game.length < SNAKE_MAX_LENGTH) {
      snake_game.length++;
      snake_game.score += 10;
      ESP_LOGI(TAG, "Score: %d, Length: %d", snake_game.score, snake_game.length);
    }
    snake_generate_food();
  }
}

// 绘制贪吃蛇游戏
void snake_draw(void) {
  // 清空画布（黑色背景）
  lv_canvas_fill_bg(snake_game.canvas, lv_color_black(), LV_OPA_COVER);
  
  // 绘制网格线（可选，浅灰色）
  lv_draw_line_dsc_t line_dsc;
  lv_draw_line_dsc_init(&line_dsc);
  line_dsc.color = lv_color_make(30, 30, 30);
  line_dsc.width = 1;
  
  for (int x = 0; x <= SNAKE_GRID_WIDTH; x++) {
    lv_point_t points[2];
    points[0].x = x * SNAKE_CELL_SIZE;
    points[0].y = 0;
    points[1].x = x * SNAKE_CELL_SIZE;
    points[1].y = SNAKE_GRID_HEIGHT * SNAKE_CELL_SIZE;
    lv_canvas_draw_line(snake_game.canvas, points, 2, &line_dsc);
  }
  for (int y = 0; y <= SNAKE_GRID_HEIGHT; y++) {
    lv_point_t points[2];
    points[0].x = 0;
    points[0].y = y * SNAKE_CELL_SIZE;
    points[1].x = SNAKE_GRID_WIDTH * SNAKE_CELL_SIZE;
    points[1].y = y * SNAKE_CELL_SIZE;
    lv_canvas_draw_line(snake_game.canvas, points, 2, &line_dsc);
  }
  
  // 绘制食物（红色）
  lv_draw_rect_dsc_t food_dsc;
  lv_draw_rect_dsc_init(&food_dsc);
  food_dsc.bg_color = lv_palette_main(LV_PALETTE_RED);
  food_dsc.bg_opa = LV_OPA_COVER;
  food_dsc.radius = 2;
  
  lv_coord_t food_x = snake_game.food.x * SNAKE_CELL_SIZE + 1;
  lv_coord_t food_y = snake_game.food.y * SNAKE_CELL_SIZE + 1;
  lv_coord_t food_w = SNAKE_CELL_SIZE - 2;
  lv_coord_t food_h = SNAKE_CELL_SIZE - 2;
  lv_canvas_draw_rect(snake_game.canvas, food_x, food_y, food_w, food_h, &food_dsc);
  
  // 绘制蛇（绿色头部，浅绿色身体）
  lv_draw_rect_dsc_t snake_dsc;
  lv_draw_rect_dsc_init(&snake_dsc);
  snake_dsc.bg_opa = LV_OPA_COVER;
  snake_dsc.radius = 1;
  
  for (uint16_t i = 0; i < snake_game.length; i++) {
    if (i == 0) {
      // 头部 - 亮绿色
      snake_dsc.bg_color = lv_palette_main(LV_PALETTE_LIGHT_GREEN);
    } else {
      // 身体 - 绿色
      snake_dsc.bg_color = lv_palette_main(LV_PALETTE_GREEN);
    }
    
    lv_coord_t body_x = snake_game.body[i].x * SNAKE_CELL_SIZE + 1;
    lv_coord_t body_y = snake_game.body[i].y * SNAKE_CELL_SIZE + 1;
    lv_coord_t body_w = SNAKE_CELL_SIZE - 2;
    lv_coord_t body_h = SNAKE_CELL_SIZE - 2;
    lv_canvas_draw_rect(snake_game.canvas, body_x, body_y, body_w, body_h, &snake_dsc);
  }
}

// 游戏定时器回调
void snake_timer_callback(lv_timer_t* timer) {
  if (!snake_game.game_over) {
    snake_update();
    snake_draw();
  } else {
    // 游戏结束，重新开始
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    snake_init();
    ESP_LOGI(TAG, "Restarting Snake Game...");
  }
}

void run_demo_snake(void) {
#if LV_COLOR_DEPTH < 16
  const size_t pixel_count = 320 * 200;
  lv_color_t *lvgl_pixel_buffer =
      heap_caps_malloc(sizeof(lv_color_t) * pixel_count, MALLOC_CAP_8BIT);
  assert(lvgl_pixel_buffer);

  lv_video_disp_init_buf(NTSC_320x200, lvgl_pixel_buffer, pixel_count);
#else
  ESP_LOGI(TAG, "Using direct framebuffer access.");
  lv_video_disp_init(NTSC_320x200);
#endif
  ESP_LOGI(TAG, "Snake Game Demo - Auto Play");

  // 创建屏幕
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_scr_load(scr);
  lv_obj_set_style_bg_color(scr, lv_color_black(), LV_STATE_DEFAULT);

  // 创建标题
  lv_obj_t* title = lv_label_create(scr);
  lv_label_set_text(title, "Snake Game - AI");
  lv_obj_set_style_text_color(title, lv_color_white(), LV_STATE_DEFAULT);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 2);

  // 创建画布
  const uint16_t canvas_width = SNAKE_GRID_WIDTH * SNAKE_CELL_SIZE;
  const uint16_t canvas_height = SNAKE_GRID_HEIGHT * SNAKE_CELL_SIZE;
  
  snake_game.canvas_buf = heap_caps_malloc(
      LV_CANVAS_BUF_SIZE_TRUE_COLOR(canvas_width, canvas_height),
      MALLOC_CAP_8BIT);
  assert(snake_game.canvas_buf);
  
  snake_game.canvas = lv_canvas_create(scr);
  lv_canvas_set_buffer(snake_game.canvas, snake_game.canvas_buf,
                       canvas_width, canvas_height, LV_IMG_CF_TRUE_COLOR);
  lv_obj_align(snake_game.canvas, LV_ALIGN_CENTER, 0, 0);

  // 初始化游戏
  snake_init();
  snake_draw();

  // 创建游戏定时器（150ms 更新一次）
  lv_timer_create(snake_timer_callback, 150, NULL);

#if CONFIG_VIDEO_DIAG_ENABLE_INTERRUPT_STATS
  int n = 0;
#endif

  // 主循环
  while (1) {
    lv_task_handler();
    vTaskDelay(20 / portTICK_PERIOD_MS);

#if CONFIG_VIDEO_DIAG_ENABLE_INTERRUPT_STATS
    if (n++ > 50) {
      video_show_stats();
      n = 0;
    }
#endif
  }
}
#endif

void app_main(void) {
  ESP_ERROR_CHECK(uart_set_baudrate(UART_NUM_0, CONFIG_ESPTOOLPY_MONITOR_BAUD));

  //	esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("*", ESP_LOG_DEBUG);

  ESP_LOGI(TAG, "Application start...");
  ESP_LOGD(TAG, "DEBUG Output enabled");

  ESP_LOGI(TAG, "CPU Speed %d MHz", CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ);
  assert(240 == CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ);

#if VIDEO_DIAG_ENABLE_INTERRUPT_STATS
  ESP_LOGI(TAG, "Interrupt timing stats enabled");
#endif

#if CONFIG_VIDEO_ENABLE_LVGL_SUPPORT
  // 贪吃蛇游戏 - AI 自动玩
  run_demo_snake();

  // 或者使用其他演示（注释掉上面一行，取消注释下面的）
  // run_demo_lvgl();  // 矩形演示
  // run_demo_slides();  // 幻灯片演示
  // run_demo_single_slide(NTSC_320x200);  // 单个界面演示
#else
  // 简单测试模式（无 LVGL）
  video_test_ntsc(VIDEO_TEST_CHECKERS);
  // video_test_ntsc(VIDEO_TEST_PM5544);
  // video_test_pal(VIDEO_TEST_PM5544);
#endif
}