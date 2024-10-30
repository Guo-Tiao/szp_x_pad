#include "ui_szp_main.h"
#include "lvgl.h"
#include "ui_common_def.h"
#include "common/common_macro.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "time.h"

//主界面
static lv_style_t lv_main_style;//主样式
lv_obj_t * lv_main_obj;//主界面本体

//标题栏
static lv_style_t lv_main_title_style;//主样式
lv_obj_t *lv_main_title;//主界面标题
lv_obj_t *lv_time_label;//主界面标题栏时间控件
lv_obj_t *lv_ble_gatts_label;//主界面标题栏蓝牙信息
lv_obj_t *lv_wifi_label;//主界面标题栏wifi信息

//初始化主界面
static void ui_mian_init()
{
    //修改背景
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0); 
    //设置主界面样式
    lv_style_init(&lv_main_style);
    lv_style_set_radius(&lv_main_style, 0); //不设置圆角
    lv_style_set_bg_opa( &lv_main_style, LV_OPA_COVER );
    lv_style_set_bg_color(&lv_main_style, lv_color_hex(0xBEEBFA));
    lv_style_set_bg_grad_color( &lv_main_style, lv_color_hex(0xFFD2E6) );
    lv_style_set_bg_grad_dir( &lv_main_style, LV_GRAD_DIR_VER );
    lv_style_set_border_width(&lv_main_style, 0);
    lv_style_set_pad_all(&lv_main_style, 0);
    lv_style_set_width(&lv_main_style, SZP_LV_UI_HOR);  
    lv_style_set_height(&lv_main_style, SZP_LV_UI_VER);

  // 创建主界面基本对象
    lv_main_obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(lv_main_obj, &lv_main_style, 0);
}

//更新系统标题栏
static void task_update_title_info(void *arg)
{
    for (;;)
    {
       //刷新时间
        time_t now = 0;
        struct tm time_info = { 0 };
        time(&now);
        localtime_r(&now, &time_info);
        char buf[20];
        strftime(buf, sizeof(buf), "%H:%M:%S", &time_info);
        lv_label_set_text(lv_time_label, buf);

        //todo 延时操作
        vTaskDelay(SZP_MS_TO_TICK(500));
    }
}
void ui_mian_update_ble_gatts_evnet(SzpBleGattsEvent ev)
{
    if(lv_ble_gatts_label)
    {
        (ev == EV_SZP_BLE_GATTS_START) ? lv_obj_clear_flag(lv_ble_gatts_label, LV_OBJ_FLAG_HIDDEN) : lv_obj_add_flag(lv_ble_gatts_label, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_mian_update_network_wifi_evnet(SzpWifiStateEvent ev)
{
    if(lv_wifi_label)
    {
        if(ev == EV_SZP_WIFI_CONNECT_SUCCESS)
        {
            lv_obj_clear_flag(lv_wifi_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_align_to(lv_ble_gatts_label, lv_wifi_label, LV_ALIGN_OUT_LEFT_MID, -10, 0);
        }
        else
        {
            lv_obj_add_flag(lv_wifi_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_align_to(lv_ble_gatts_label, lv_main_title, LV_ALIGN_RIGHT_MID, 5, 0);
        }

    }
}

//标题栏初始化
static void ui_title_init()
{
    //设置标题栏样式
    lv_style_init(&lv_main_title_style);
    lv_style_set_radius(&lv_main_title_style, 0);
    lv_style_set_bg_opa( &lv_main_title_style, LV_OPA_20);
    lv_style_set_text_color(&lv_main_title_style, lv_color_hex(0xA0A0A0));
    lv_style_set_text_font(&lv_main_title_style, &lv_font_montserrat_16);
    lv_style_set_border_width(&lv_main_title_style, 0);
    lv_style_set_width(&lv_main_title_style, SZP_LV_UI_HOR);  
    lv_style_set_height(&lv_main_title_style, SZP_LV_UI_VER/8+5);

    //创建标题
    lv_main_title = lv_obj_create(lv_main_obj);
    lv_obj_add_style(lv_main_title, &lv_main_title_style, 0);
    lv_obj_clear_flag(lv_main_title, LV_OBJ_FLAG_SCROLLABLE);
    //创建左上角时间显示
    lv_time_label= lv_label_create(lv_main_title);
    lv_obj_set_width(lv_time_label, SZP_LV_UI_HOR/4);
    lv_label_set_text(lv_time_label, "00:00:00");
    lv_obj_align(lv_time_label, LV_ALIGN_LEFT_MID, 5, 0);

    //创建WIFI信息栏
    lv_wifi_label= lv_label_create(lv_main_title);
    lv_label_set_text(lv_wifi_label, LV_SYMBOL_WIFI);
    lv_obj_align_to(lv_wifi_label, lv_main_title, LV_ALIGN_RIGHT_MID, 5, 0);


    // 创建蓝牙gatts信息栏
    lv_ble_gatts_label = lv_label_create(lv_main_title);
    lv_label_set_text(lv_ble_gatts_label,  (szp_ble_gatts_get_current_event() == EV_SZP_BLE_GATTS_START) ? LV_SYMBOL_BLUETOOTH : "");
    (szp_ble_gatts_get_current_event() == EV_SZP_BLE_GATTS_START) ?  lv_obj_clear_flag(lv_ble_gatts_label, LV_OBJ_FLAG_HIDDEN) :  lv_obj_add_flag(lv_ble_gatts_label, LV_OBJ_FLAG_HIDDEN);

    SzpWifiStateEvent wifi_ev = network_wifi_current_state();
    if (wifi_ev == EV_SZP_WIFI_CONNECT_SUCCESS)
    {
        lv_obj_clear_flag(lv_wifi_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align_to(lv_ble_gatts_label, lv_wifi_label, LV_ALIGN_OUT_LEFT_MID, -10, 0);
    }
    else
    {
        lv_obj_add_flag(lv_wifi_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align_to(lv_ble_gatts_label, lv_main_title, LV_ALIGN_RIGHT_MID, 5, 0);
    }

    //todo:开启线程
    xTaskCreate(task_update_title_info, "task_update_title_info", 2048, NULL, 5, NULL);
}
void ui_main_setup(void)
{
    ui_mian_init();
    ui_title_init();
}


