#pragma once

#include "esp_err.h"
#include "lvgl.h"

//LVGL初始化
void szp_lvgl_init(void);
//LVGL定时处理
uint32_t szp_lvgl_timer_handler(void);
//获取屏幕
lv_obj_t *szp_lvgl_get_scr_act(void);
