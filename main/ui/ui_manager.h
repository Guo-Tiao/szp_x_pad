#pragma once

#include "esp_err.h"
#include "lvgl.h"

//LVGL初始化
void szp_lvgl_init(void);
//LVGL定时处理
uint32_t szp_lvgl_timer_handler(void);
//获取屏幕
lv_obj_t *szp_lvgl_get_scr_act(void);
//主界面ui显示
void szp_ui_main_setup(void);




/****************UI-蓝牙GATTS****************/
#include "bluetooth/szp_ble_gatts.h"
//UI更新蓝牙状态
void szp_ui_update_ble_gatts_evnet(SzpBleGattsEvent ev);
/****************UI-蓝牙GATTS****************/

/****************UI-Network****************/
#include "network/network_manager.h"
void szp_ui_update_network_wifi_evnet(SzpWifiStateEvent ev);
/****************UI-Network****************/