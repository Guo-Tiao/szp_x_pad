#pragma once
#include "esp_err.h"
#include "sdkconfig.h"

//屏幕参数
#define SZP_LCD_H_RES                         320            //屏幕水平大小
#define SZP_LCD_V_RES                          240           //屏幕垂直大小


/******* LCD显示驱动 *******/
//LCD刷新回调
typedef void (*szp_lcd_on_flush_ready_cb)(void);

//显示屏初始化
esp_err_t szp_lcd_init(void);
//回调注册
void szp_lcd_flush_ready_cb_register(szp_lcd_on_flush_ready_cb cb);
//绘制图像
void szp_lcd_draw_bitmap(const void *bitmap,int x_start, int y_start, int x_end, int y_end);
//设置背光量(0-100)
void szp_lcd_set_brightness(uint16_t val);