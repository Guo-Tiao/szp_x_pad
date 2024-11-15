#include "ui_szp_app_page.h"
#include "assets/szp_assets_def.h"
#include "ui_common_def.h"

/********************************LV对象成员********************************/
//app页面成员
static lv_style_t lv_app_page_style;//app页面样式
static lv_obj_t * lv_app_page_flex;//app视图对象

//APP按键
static lv_style_t lv_app_obj_style; //app对象样式
static lv_style_t lv_app_btn_style; //btn对象样式
lv_obj_t *lv_app_uart_btn;//串口按钮
lv_obj_t *lv_app_compass_btn;//指南针(地磁传感器)按钮
lv_obj_t *lv_app_gyro_btn;//陀螺仪按钮
lv_obj_t *lv_app_th_btn;//温湿度按钮
lv_obj_t *lv_app_setting_btn;//设置按钮

/********************************LV对象成员********************************/

//按键公共回调
static void ui_app_btn_common_event_cb(lv_event_t* event)
{
    lv_obj_t *current_btn = event->current_target;
    if (event->code == LV_EVENT_PRESSED)
    {
       lv_obj_t* img = lv_obj_get_child(current_btn, 0);
        if (lv_obj_is_valid(img))
        {
            lv_img_set_zoom(img, (uint16_t)(lv_obj_get_width(current_btn) * 0.7 / 64 * LV_IMG_ZOOM_NONE));
        }
    }
    else if(event->code==LV_EVENT_RELEASED)
    {
        lv_obj_t* img = lv_obj_get_child(current_btn, 0);
        if (lv_obj_is_valid(img))
        {
            lv_img_set_zoom(img, (uint16_t)(lv_obj_get_width(current_btn) * 1 / 64 * LV_IMG_ZOOM_NONE));
        }
    }
}

lv_obj_t* ui_app_create_btn(const void * btn_img,const char* btn_text)
{
    lv_obj_t *obj = lv_obj_create(lv_app_page_flex);
    lv_obj_add_style(obj, &lv_app_obj_style, 0);

    lv_obj_t *btn = lv_btn_create(obj);
    lv_obj_add_style(btn, &lv_app_btn_style, 0);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 7);

    lv_obj_t *  img = lv_img_create(btn);
    lv_img_set_src(img, btn_img);
    lv_obj_center(img);

    lv_obj_t *text = lv_label_create(obj);
    lv_obj_set_style_text_font(text, &font_szp_Harmony_16, 0);
    lv_label_set_text(text, btn_text);
    lv_obj_align(text, LV_ALIGN_BOTTOM_MID, 0, -7);
    lv_obj_add_event_cb(btn, ui_app_btn_common_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(btn, ui_app_btn_common_event_cb, LV_EVENT_RELEASED, NULL);
    return btn;
}



static void ui_app_page_init(lv_obj_t *parent)
{
    // 初始化视图样式和对象
    lv_style_init(&lv_app_page_style);
    lv_style_set_radius(&lv_app_page_style, 0);
    lv_style_set_bg_opa(&lv_app_page_style, LV_OPA_TRANSP);
    lv_style_set_text_color(&lv_app_page_style, lv_color_hex(0xffffff));
    lv_style_set_border_width(&lv_app_page_style, 0);
    lv_style_set_pad_all(&lv_app_page_style, 0);
    lv_style_set_width(&lv_app_page_style, SZP_LV_UI_HOR-80);
    lv_style_set_height(&lv_app_page_style, SZP_LV_UI_VER - lv_obj_get_height(szp_ui_get_sys_title()));

    lv_app_page_flex = lv_obj_create(parent);
    lv_obj_add_style(lv_app_page_flex, &lv_app_page_style, 0);
    lv_obj_set_scrollbar_mode(lv_app_page_flex, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(lv_app_page_flex, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(lv_app_page_flex, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_snap_y(lv_app_page_flex, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_style_bg_opa(lv_app_page_flex, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(lv_app_page_flex, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(lv_app_page_flex, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(lv_app_page_flex, 10, LV_PART_MAIN);   //图标之间的间隙
    lv_obj_align(lv_app_page_flex, LV_ALIGN_BOTTOM_MID, 0, 0);


    // 设置应用图标style
    lv_style_init(&lv_app_obj_style);
    lv_style_set_radius(&lv_app_obj_style, 20);
    lv_style_set_bg_opa(&lv_app_obj_style, LV_OPA_TRANSP);
    lv_style_set_text_color(&lv_app_obj_style, lv_color_hex(0xffffff)); 
    lv_style_set_border_width(&lv_app_obj_style, 0);
    lv_style_set_shadow_width(&lv_app_obj_style, 0);
    lv_style_set_pad_all(&lv_app_obj_style, 1);
    lv_style_set_width(&lv_app_obj_style, 100);
    lv_style_set_height(&lv_app_obj_style, 120);

    lv_style_init(&lv_app_btn_style);
    lv_style_set_radius(&lv_app_btn_style, 20);  
    lv_style_set_bg_opa(&lv_app_btn_style, LV_OPA_TRANSP);
    lv_style_set_border_width(&lv_app_btn_style, 0);
    lv_style_set_shadow_width(&lv_app_btn_style, 0);
    lv_style_set_pad_all(&lv_app_btn_style, 1);
    lv_style_set_width(&lv_app_btn_style, 80);
    lv_style_set_height(&lv_app_btn_style, 80);

    //串口按键
    lv_app_uart_btn = ui_app_create_btn(&img_szp_uart, "串口");
    lv_app_compass_btn = ui_app_create_btn(&img_szp_compass, "指南针");
    lv_app_gyro_btn = ui_app_create_btn(&img_szp_gyro, "陀螺仪");
    lv_app_th_btn = ui_app_create_btn(&img_szp_th, "温湿度");
    lv_app_setting_btn = ui_app_create_btn(&img_szp_setting, "设置");
}


void ui_app_page_setup(lv_obj_t *parent)
{
   ui_app_page_init(parent);
}