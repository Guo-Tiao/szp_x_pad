#pragma once
#include "esp_err.h"
#include "sdkconfig.h"

//屏幕参数
#if CONFIG_SZP_LVGL_VER_DISP
#define SZP_LCD_H_RES                         240            //屏幕水平大小
#define SZP_LCD_V_RES                          320            //屏幕垂直大小
#else
#define SZP_LCD_H_RES                         320            //屏幕水平大小
#define SZP_LCD_V_RES                           240           //屏幕垂直大小
#endif

#define SZP_LCD_PIXEL_CLOCK_HZ     (20 * 1000 * 1000)   //屏幕像素时钟频率
#define SZP_LCD_CMD_BITS                    8           //屏幕命令位
#define SZP_LCD_PARAM_BITS                8           //屏幕参数位

/******* LCD显示驱动 *******/
//LCD刷新回调
typedef void (*szp_lcd_on_flush_ready_cb)(void);

//显示屏初始化
esp_err_t szp_lcd_init(void);
//回调注册
void szp_lcd_flush_ready_cb_register(szp_lcd_on_flush_ready_cb cb);
//绘制图像
void szp_lcd_draw_bitmap(const void *bitmap,int x_start, int y_start, int x_end, int y_end);