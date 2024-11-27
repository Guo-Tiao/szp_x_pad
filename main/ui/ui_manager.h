#pragma once

#include "esp_err.h"
#include "lvgl.h"

/*************LVGL接口*************/
//LVGL初始化
void szp_lvgl_init(void);
//LVGL定时处理
uint32_t szp_lvgl_timer_handler(void);
//获取屏幕
lv_obj_t *szp_lvgl_get_scr_act(void);

/*************LVGL接口*************/



/*************UI接口*************/
//安装UI系统
void szp_ui_sys_setup(void);
//获取UI系统对象
lv_obj_t *szp_ui_get_ui_sys_obj(void);
//获取系统标题栏
lv_obj_t *szp_ui_get_sys_title(void);

/**UI-蓝牙GATTS**/
#include "bluetooth/szp_ble_gatts.h"
//UI更新蓝牙状态
void szp_ui_update_ble_gatts_evnet(SzpBleGattsEvent ev);
/**UI-蓝牙GATTS**/

/**UI-Network**/
#include "network/network_manager.h"
void szp_ui_update_network_wifi_event(SzpWifiStateEvent ev);
//授时完成回调
void szp_ui_sntp_complete_event();
/**UI-Network**/
/*************UI接口*************/


