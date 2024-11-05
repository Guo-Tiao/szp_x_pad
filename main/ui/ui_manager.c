#include "ui_manager.h"
#include "lvgl.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "common/common_macro.h"

#include "drivers/szp_lcd.h"
#include "drivers/szp_touch.h"

#include "ui_common_def.h"
#include "ui_szp_main.h"
#include "assets/szp_assets_def.h"

#include "time.h"

#define SZP_LVGL_TICK_PERIOD_MS    2

static lv_disp_draw_buf_t               szp_lvgl_disp_buf;           //显示缓存
static lv_disp_drv_t                         szp_lvgl_disp_drv;           //显示驱动 
static lv_disp_t                                *szp_lvgl_disp=NULL;           //显示屏幕 

/********************************LV对象成员********************************/
//UI系统对象
static lv_style_t lv_ui_sys_style;//系统样式
static lv_obj_t * lv_ui_sys_obj;//UI系统本体

//系统标题栏
static lv_style_t lv_sys_title_style;//标题栏样式
lv_obj_t *lv_sys_title;//系统标题栏
//标题栏控件
lv_obj_t *lv_st_time_lb;//系统标题栏-时间控件
lv_obj_t *lv_st_heap_lb;//系统标题栏-内存控件
lv_obj_t *lv_st_ble_gatts_lb;//系统标题栏-蓝牙信息
lv_obj_t *lv_st_wifi_lb;//系统标题栏-wifi信息
/********************************LV对象成员********************************/

/*************LVGL接口*************/
//刷新准备
static void szp_lvgl_flush_ready()
{
    lv_disp_flush_ready(&szp_lvgl_disp_drv);
}
//显示绘制回调
static void szp_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    szp_lcd_draw_bitmap(color_map, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1);
}
//触摸回调
static void szp_lvgl_touch_cb(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
     uint16_t touchpad_x[1] = {0};
     uint16_t touchpad_y[1] = {0};
     uint8_t touchpad_cnt = 0;
     uint16_t strength = 0;
     bool touchpad_pressed = szp_touch_get_coord(touchpad_x, touchpad_y, &strength, &touchpad_cnt, 1);

     if (touchpad_pressed && touchpad_cnt > 0)
     {
         data->point.x = touchpad_x[0];
         data->point.y = touchpad_y[0];
         data->state = LV_INDEV_STATE_PRESSED;
    } 
    else 
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
//lvgl自增tick
static void szp_lvgl_increase_tick(void *arg)
{
    lv_tick_inc(SZP_LVGL_TICK_PERIOD_MS);
}
//lvgl定时
uint32_t szp_lvgl_timer_handler(void)
{
    return lv_timer_handler();
}
//获取屏幕
lv_obj_t *szp_lvgl_get_scr_act(void)
{
    return  lv_disp_get_scr_act(szp_lvgl_disp);
}
//lvgl初始化
void szp_lvgl_init(void)
{
    //驱动初始化         
    //初始化LCD
    szp_lcd_init();
    szp_lcd_flush_ready_cb_register(szp_lvgl_flush_ready);
    //初始化触摸屏
    szp_touch_init();
    //LVGL初始化
    lv_init();
    //缓存初始化
    lv_color_t *buf1 = heap_caps_malloc(SZP_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1);
    lv_color_t *buf2 = heap_caps_malloc(SZP_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);
    lv_disp_draw_buf_init(&szp_lvgl_disp_buf, buf1, buf2, SZP_LCD_H_RES * 20);
   
    //显示驱动初始化
    lv_disp_drv_init(&szp_lvgl_disp_drv);
    szp_lvgl_disp_drv.hor_res = SZP_LCD_H_RES;
    szp_lvgl_disp_drv.ver_res = SZP_LCD_V_RES;
    szp_lvgl_disp_drv.flush_cb = szp_lvgl_flush_cb;
    szp_lvgl_disp_drv.draw_buf = &szp_lvgl_disp_buf;
    szp_lvgl_disp= lv_disp_drv_register(&szp_lvgl_disp_drv);

    //开启定时
    const esp_timer_create_args_t lvgl_tick_timer_args = 
    {
        .callback = &szp_lvgl_increase_tick,
        .name = "szp_lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, SZP_LVGL_TICK_PERIOD_MS * 1000);

    //输入(触摸)驱动初始化
    static lv_indev_drv_t indev_drv;    // Input device driver (Touch)
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.disp = szp_lvgl_disp;
    indev_drv.read_cb = szp_lvgl_touch_cb;
    lv_indev_drv_register(&indev_drv);
    //设置为默认屏幕
    lv_disp_set_default(szp_lvgl_disp);
    return;
}
/*************LVGL接口*************/


/*************UI接口*************/
//主界面OBJ初始化
static void szp_ui_sys_obj_init(void)
{
    //修改背景
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0); 
    //设置主界面样式
    lv_style_init(&lv_ui_sys_style);
    lv_style_set_radius(&lv_ui_sys_style, 0); //不设置圆角
    lv_style_set_bg_opa( &lv_ui_sys_style, LV_OPA_COVER );
    lv_style_set_bg_color(&lv_ui_sys_style, lv_color_hex(0xBEEBFA));
    //lv_style_set_bg_grad_color( &lv_ui_sys_style, lv_color_hex(0xFFD2E6) );
    // lv_style_set_bg_grad_dir( &lv_ui_sys_style, LV_GRAD_DIR_VER );
    lv_style_set_border_width(&lv_ui_sys_style, 0);
    lv_style_set_pad_all(&lv_ui_sys_style, 0);
    lv_style_set_width(&lv_ui_sys_style, SZP_LV_UI_HOR);  
    lv_style_set_height(&lv_ui_sys_style, SZP_LV_UI_VER);

   // 创建主界面基本对象
    lv_ui_sys_obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(lv_ui_sys_obj, &lv_ui_sys_style, 0);
}

//更新系统标题栏
static void task_update_sys_title_info(void *arg)
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
        lv_label_set_text(lv_st_time_lb, buf);
        //刷新内存
        sprintf(buf, "%.2fkb", (esp_get_free_heap_size() / 1000.00));
        lv_label_set_text(lv_st_heap_lb, buf);
        //延时500MS
        vTaskDelay(SZP_MS_TO_TICK(500));
    }
}

void szp_ui_update_ble_gatts_evnet(SzpBleGattsEvent ev)
{
    if(lv_st_ble_gatts_lb)
    {
        lv_label_set_text(lv_st_ble_gatts_lb, (ev == EV_SZP_BLE_GATTS_START) ? SZP_SYMBOL_BLE_GATTS_START : SZP_SYMBOL_BLE_GATTS_STOP);
    }
}

void szp_ui_update_network_wifi_evnet(SzpWifiStateEvent ev)
{
   if(lv_st_wifi_lb)
    {
        lv_label_set_text(lv_st_wifi_lb, (ev==EV_SZP_WIFI_CONNECT_SUCCESS)?SZP_SYMBOL_WIFI_CONNECT:SZP_SYMBOL_WIFI_DISCONNECT);
    }
}

//标题栏初始化
static void szp_ui_sys_title_init()
{
    //设置标题栏样式
    lv_style_init(&lv_sys_title_style);
    lv_style_set_radius(&lv_sys_title_style, 0);
    lv_style_set_bg_opa( &lv_sys_title_style, LV_OPA_20);
    lv_style_set_text_color(&lv_sys_title_style, lv_color_hex(0xA0A0A0));
    lv_style_set_text_font(&lv_sys_title_style, &lv_font_montserrat_16);
    lv_style_set_border_width(&lv_sys_title_style, 0);
    lv_style_set_width(&lv_sys_title_style, SZP_LV_UI_HOR);  
    lv_style_set_height(&lv_sys_title_style, SZP_LV_UI_VER/8+5);

    //创建标题
    lv_sys_title = lv_obj_create(lv_ui_sys_obj);
    lv_obj_add_style(lv_sys_title, &lv_sys_title_style, 0);
    lv_obj_clear_flag(lv_sys_title, LV_OBJ_FLAG_SCROLLABLE);
    //创建左上角时间显示
    lv_st_time_lb= lv_label_create(lv_sys_title);
    lv_obj_set_width(lv_st_time_lb, SZP_LV_UI_HOR/4);
    lv_label_set_text(lv_st_time_lb, "00:00:00");
    lv_obj_align(lv_st_time_lb, LV_ALIGN_LEFT_MID, 5, 0);
    //创建左上角内存显示
    lv_st_heap_lb = lv_label_create(lv_sys_title);
    lv_obj_set_width(lv_st_heap_lb, SZP_LV_UI_HOR / 4);
    lv_label_set_text(lv_st_heap_lb, "0kb");
    lv_obj_align_to(lv_st_heap_lb, lv_st_time_lb,LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    //创建WIFI信息栏
    lv_st_wifi_lb= lv_label_create(lv_sys_title);
    lv_obj_set_style_text_font(lv_st_wifi_lb, &icon_szp_title_set, 0);
    lv_label_set_text(lv_st_wifi_lb, (network_wifi_current_state()==EV_SZP_WIFI_CONNECT_SUCCESS)?SZP_SYMBOL_WIFI_CONNECT:SZP_SYMBOL_WIFI_DISCONNECT);
    lv_obj_align_to(lv_st_wifi_lb, lv_sys_title, LV_ALIGN_RIGHT_MID, 5, 0);

    // 创建蓝牙gatts信息栏
    lv_st_ble_gatts_lb = lv_label_create(lv_sys_title);
    lv_obj_set_style_text_font(lv_st_ble_gatts_lb, &icon_szp_title_set, 0);
    lv_label_set_text(lv_st_ble_gatts_lb,  (szp_ble_gatts_get_current_event() == EV_SZP_BLE_GATTS_START) ? SZP_SYMBOL_BLE_GATTS_START : SZP_SYMBOL_BLE_GATTS_STOP);
    lv_obj_align_to(lv_st_ble_gatts_lb, lv_st_wifi_lb, LV_ALIGN_OUT_LEFT_MID, -10, 0);

    //开启系统标题栏刷新线程
    xTaskCreate(task_update_sys_title_info, "task_update_sys_title_info", 2048, NULL, 5, NULL);
}

//UI系统安装
void szp_ui_sys_setup(void)
{
    //先初始化UI系统对象
    szp_ui_sys_obj_init();
    //初始化系统标题栏
    szp_ui_sys_title_init();

    //安装主页UI
    ui_main_setup();
}
//获取UI对象
lv_obj_t *szp_ui_get_ui_sys_obj(void)
{
    return lv_ui_sys_obj;
}
//获取系统标题栏
lv_obj_t *szp_ui_get_sys_title(void)
{
    return lv_sys_title;
}

/*************UI接口*************/

