#pragma once
#include "ui_manager.h"
//主界面UI安装
void ui_main_setup(void);

//蓝牙状态更新
void ui_mian_update_ble_gatts_evnet(SzpBleGattsEvent ev);
//wifi状态更新
void ui_mian_update_network_wifi_evnet(SzpWifiStateEvent ev);