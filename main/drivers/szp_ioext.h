#pragma once
#include "esp_err.h"

//IO扩展芯片初始化
void szp_ioext_init(void);
// 设置PCA9557芯片的某个IO引脚输出高低电平
esp_err_t szp_ioext_set_output(uint8_t gpio_bit, uint8_t level);
// 控制 PCA9557_LCD_CS 引脚输出高低电平 参数0输出低电平 参数1输出高电平
void szp_ioext_lcd_cs(uint8_t level);
// 控制 PCA9557_PA_EN 引脚输出高低电平 参数0输出低电平 参数1输出高电平
void szp_ioext_pa_en(uint8_t level);
// 控制 PCA9557_DVP_PWDN 引脚输出高低电平 参数0输出低电平 参数1输出高电平
void szp_ioext_dvp_pwdn(uint8_t level);