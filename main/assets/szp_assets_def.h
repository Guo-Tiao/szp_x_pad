#pragma once
/*
ICON资源定义
 */

#include "lvgl.h"


/**************************** ICON ****************************/
//标题栏图标
LV_FONT_DECLARE(icon_szp_title_set);
#define SZP_SYMBOL_WIFI_CONNECT              "\xEE\x9D\xAC"            //WIFI连接图标:0xe76c
#define SZP_SYMBOL_WIFI_DISCONNECT        "\xEE\x99\x93"            //WIFI断开图标:0xe653

#define SZP_SYMBOL_BLE_GATTS_START          "\xEE\xB1\x8A"           //蓝牙开启图标:0xec4a
#define SZP_SYMBOL_BLE_GATTS_STOP          "\xEE\xA8\xAF"             //蓝牙关闭图标:0xea2f


/**************************** GIF ****************************/
//主界面动图
LV_IMG_DECLARE(gif_szp_duckyo);