#pragma once
#include "esp_err.h"
#include "stdbool.h"
/******* 触摸屏驱动 *******/
//触摸屏初始化
esp_err_t szp_touch_init(void);
//获取触摸坐标
bool szp_touch_get_coord(uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);