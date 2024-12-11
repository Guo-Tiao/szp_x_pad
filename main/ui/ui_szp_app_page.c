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
lv_obj_t *lv_app_gyro_btn;//陀螺仪按钮
lv_obj_t *lv_app_th_btn;//温湿度按钮
lv_obj_t *lv_app_setting_btn;//设置按钮

//APP子页面
lv_obj_t *lv_app_common_obj;//APP公共对象
lv_obj_t *lv_app_common_bar;//APP公共侧边栏
lv_obj_t *lv_app_back_btn;//APP返回按钮
lv_obj_t *lv_app_common_parent_page;//APP公共父界面

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

//按钮返回回调
static void ui_app_page_back_cb(lv_event_t * e)
{
    if(lv_app_common_obj!= NULL)
    {
        lv_obj_del(lv_app_common_obj);
        lv_app_common_obj = NULL;
    }
}
//创建APP页面
static void ui_app_create_app_page()
{
    //创建APP公共对象
    static lv_style_t lv_app_common_style;
    lv_style_init(&lv_app_common_style);
    lv_style_set_radius(&lv_app_common_style, 0); 
    lv_style_set_bg_opa( &lv_app_common_style, LV_OPA_COVER );
    lv_style_set_bg_color(&lv_app_common_style, lv_color_hex(0x78ECBE));
    lv_style_set_bg_grad_color( &lv_app_common_style, lv_color_hex(0x87CEEB ));
    lv_style_set_bg_grad_dir( &lv_app_common_style, LV_GRAD_DIR_VER );
    lv_style_set_border_width(&lv_app_common_style, 0);
    lv_style_set_pad_all(&lv_app_common_style, 0);
    lv_style_set_width(&lv_app_common_style, SZP_LV_UI_HOR);  
    lv_style_set_height(&lv_app_common_style, SZP_LV_UI_VER); 
    lv_app_common_obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(lv_app_common_obj, &lv_app_common_style, 0);

    //创建APP通用侧边栏
    static lv_style_t lv_app_bar_style;
    lv_style_init(&lv_app_bar_style);
    lv_style_set_radius(&lv_app_bar_style, 0);
    lv_style_set_bg_opa( &lv_app_bar_style, LV_OPA_20);
    lv_style_set_text_color(&lv_app_bar_style, lv_color_hex(0xA0A0A0));
    lv_style_set_text_font(&lv_app_bar_style, &lv_font_montserrat_16);
    lv_style_set_border_width(&lv_app_bar_style, 0);
    lv_style_set_width(&lv_app_bar_style, SZP_LV_UI_HOR/8+10);
    lv_style_set_height(&lv_app_bar_style, SZP_LV_UI_VER);
    lv_app_common_bar = lv_obj_create(lv_app_common_obj);
    lv_obj_add_style(lv_app_common_bar, &lv_app_bar_style, 0);
    lv_obj_clear_flag(lv_app_common_bar, LV_OBJ_FLAG_SCROLLABLE);

    //创建APP返回按钮
    static lv_style_t lv_app_back_btn_style;
    lv_style_init(&lv_app_back_btn_style);
    lv_style_set_radius(&lv_app_back_btn_style, 5);
    lv_style_set_bg_opa(&lv_app_back_btn_style, LV_OPA_30);
    lv_style_set_text_color(&lv_app_back_btn_style, lv_color_hex(0xffffff)); 
    lv_style_set_border_width(&lv_app_back_btn_style, 0);
    lv_style_set_shadow_width(&lv_app_back_btn_style, 1);
    lv_style_set_pad_all(&lv_app_back_btn_style, 1);
    lv_style_set_width(&lv_app_back_btn_style, SZP_LV_UI_HOR/8);
    lv_style_set_height(&lv_app_back_btn_style, SZP_LV_UI_VER/2);
    lv_app_back_btn = lv_btn_create(lv_app_common_bar);
    lv_obj_add_style(lv_app_back_btn, &lv_app_back_btn_style, 0);
    lv_obj_align(lv_app_back_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(lv_app_back_btn, ui_app_page_back_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *  img = lv_img_create(lv_app_back_btn);
    lv_img_set_src(img, &img_szp_back);
    lv_obj_center(img);

    //创建APP通用父界面
    static lv_style_t lv_app_parent_style;
    lv_style_init(&lv_app_parent_style);
    lv_style_set_radius(&lv_app_parent_style, 0); 
    lv_style_set_bg_opa( &lv_app_parent_style, LV_OPA_TRANSP );
    lv_style_set_border_width(&lv_app_parent_style, 0);
    lv_style_set_pad_all(&lv_app_parent_style, 0);
    lv_style_set_width(&lv_app_parent_style, SZP_LV_UI_HOR-SZP_LV_UI_HOR/8-10);
    lv_style_set_height(&lv_app_parent_style, SZP_LV_UI_VER); 
    lv_app_common_parent_page = lv_obj_create(lv_app_common_obj);
    lv_obj_add_style(lv_app_common_parent_page, &lv_app_parent_style, 0);
    lv_obj_align_to(lv_app_common_parent_page, lv_app_common_bar, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
}


//创建app按钮
static lv_obj_t* ui_app_create_btn(const void * btn_img,const char* btn_text,lv_event_cb_t clicked_cb)
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
    lv_obj_add_event_cb(btn, clicked_cb, LV_EVENT_CLICKED, NULL);
    return btn;
}



//串口APP回调
extern void ui_app_uart_setup(lv_obj_t* parent);
static void ui_app_uart_cb(lv_event_t * e)
{
    ui_app_create_app_page();
    ui_app_uart_setup(lv_app_common_parent_page);
}
//指南针APP回调
extern void ui_app_compass_setup(lv_obj_t* parent);
static void ui_app_compass_cb(lv_event_t * e)
{
    ui_app_create_app_page();
    ui_app_compass_setup(lv_app_common_parent_page);
}
//陀螺仪APP回调
extern void ui_app_gyro_setup(lv_obj_t* parent);
static void ui_app_gyro_cb(lv_event_t * e)
{
    ui_app_create_app_page();
    ui_app_gyro_setup(lv_app_common_parent_page);
}
//温湿度APP回调
extern void ui_app_th_setup(lv_obj_t* parent);
static void ui_app_th_cb(lv_event_t * e)
{
    ui_app_create_app_page();
    ui_app_th_setup(lv_app_common_parent_page);
}
//设置APP回调
extern void ui_app_setting_setup(lv_obj_t* parent);
static void ui_app_setting_cb(lv_event_t * e)
{
    ui_app_create_app_page();
    ui_app_setting_setup(lv_app_common_parent_page);
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
    lv_style_set_width(&lv_app_page_style, SZP_LV_UI_HOR-20);
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

    //创建APP
    lv_app_uart_btn = ui_app_create_btn(&img_szp_uart, "串口",ui_app_uart_cb); 
    lv_app_gyro_btn = ui_app_create_btn(&img_szp_gyro, "陀螺仪",ui_app_gyro_cb); 
    lv_app_th_btn = ui_app_create_btn(&img_szp_th, "温湿度",ui_app_th_cb);
    lv_app_setting_btn = ui_app_create_btn(&img_szp_setting, "设置",ui_app_setting_cb);
}


void ui_app_page_setup(lv_obj_t *parent)
{
   ui_app_page_init(parent);
}