#include "lvgl.h"
#include "assets/szp_assets_def.h"
#include "core/work_controller.h"
#include "bluetooth/szp_ble_gatts.h"
#include "network/network_manager.h"
#include "drivers/szp_lcd.h"

#define LIST_SCROLL_RIGHT       -1
#define LIST_SCROLL_LEFT           1
#define LIST_SCROLL_DIR LIST_SCROLL_LEFT

lv_obj_t *lv_setting_list;//设置列表
//设置列表
lv_obj_t *lv_st_ble_panel;//蓝牙设置板
lv_obj_t *lv_st_ble_sw;//蓝牙开关

lv_obj_t *lv_st_wifi_panel;//wifi设置板
lv_obj_t *lv_st_wifi_sw;//蓝牙开关


lv_obj_t *lv_st_weather_panel;//天气设置板
lv_obj_t *lv_st_weather_drop;//天气下来

lv_obj_t *lv_st_brightness_panel;//背光设置板
lv_obj_t *lv_st_brightness_slider;//背光条
lv_obj_t *lv_st_brightness_lb;//背光值

lv_obj_t *lv_st_about_panel;//关于设置板

//控件滚动事件
void ui_app_setting_list_scroll_cb(lv_event_t *e)
{
      lv_obj_t *list = lv_event_get_target(e);
      lv_area_t list_area;
      lv_obj_get_coords(list, &list_area);
      //获取中心Y坐标
      lv_coord_t list_y_center = list_area.y1 + lv_area_get_height(&list_area) / 2;
      //旋转比例
      lv_coord_t r = lv_obj_get_height(list) * 7 / 10;
      uint32_t i;
      uint32_t child_cnt = lv_obj_get_child_cnt(list);
      //遍历子元素
      for (i = 0; i < child_cnt; i++)
      {
            lv_obj_t *child = lv_obj_get_child(list, i);
            lv_area_t child_a;
            lv_obj_get_coords(child, &child_a);
            //获取子元素中心Y坐标
            lv_coord_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;

            lv_coord_t diff_y = child_y_center - list_y_center;
            diff_y = LV_ABS(diff_y);

            lv_coord_t x;
            if (diff_y >= r)
            {
                  x = r;
            }
            else
            {
                  uint32_t x_sqr = r * r - diff_y * diff_y;
                  lv_sqrt_res_t res;
                  lv_sqrt(x_sqr, &res, 0x8000); 
                  x = r - res.i;
            }
            //位移X坐标
            lv_obj_set_style_translate_x(child, LIST_SCROLL_DIR*x , 0);
      }
}

//父界面删除事件
static void ui_app_setting_parent_del(lv_event_t *ev)
{
  
}

//蓝牙设置
static void ui_app_st_ble_sw_cb(lv_event_t *ev)
{
    bool check = lv_obj_has_state(lv_st_ble_sw, LV_STATE_CHECKED);
    if(check)
    {
        szp_work_ble_start();
    }
    else
    {
        szp_work_ble_stop();
    }
}
//WIFI设置
static void ui_app_st_wifi_sw_cb(lv_event_t *ev)
{
    bool check = lv_obj_has_state(lv_st_wifi_sw, LV_STATE_CHECKED);
    if(check)
    {
        network_wifi_connect();
    }
    else
    {
        network_wifi_disconnect();
    }
}
//weater设置
static void ui_app_st_weather_drop_cb(lv_event_t *ev)
{
    uint16_t idx = lv_dropdown_get_selected(lv_st_weather_drop);
    uint16_t min = 10;
    switch (idx)
    {
    case 0:
        min = 5;
        break;
    case 1:
        min = 10;
        break;
    case 2:
        min = 30;
        break;
    case 3:
        min = 60;
        break;
    default:
        break;
    }
    network_weather_set_update_time(min);
}
//背光设置
static void ui_app_st_bn_slider_cb(lv_event_t *ev)
{
    int val = lv_slider_get_value(lv_st_brightness_slider);
    lv_label_set_text_fmt(lv_st_brightness_lb, "%d", val);
    szp_lcd_set_brightness(val);
}

//设置列表子元素样式
static void ui_app_setting_set_list_item_style(lv_obj_t* item)
{
    lv_obj_set_style_text_align(item, LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_style_radius(item, 0, 0);
    lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(item, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_opa(item, LV_OPA_COVER,0);
    lv_obj_set_style_border_width(item, 1, 0);
    lv_obj_set_style_border_side(item, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_pad_left(item, 0, 0);
    lv_obj_set_style_pad_right(item, 0, 0);
    lv_obj_set_style_pad_top(item, 0, 0);
    lv_obj_set_style_pad_bottom(item, 0, 0);
    lv_obj_set_style_text_color(item, lv_color_hex(0xffffff),0);
}
//创建列表控件
static lv_obj_t* ui_app_setting_create_list_item_panel(lv_obj_t *list,lv_coord_t width,lv_coord_t height)
{
    lv_obj_t *item = lv_obj_create(list);
    lv_obj_set_width(item, width);
    lv_obj_set_height(item, height);
    lv_obj_set_align(item, LV_ALIGN_CENTER);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);
    ui_app_setting_set_list_item_style(item);
    return item;
}
//创建列表子元素标题和图标
static lv_obj_t* ui_app_setting_create_list_item_title_icon(lv_obj_t *panel,const char* title,const void *icon)
{
    //创建图标
    lv_obj_t *icon_img = lv_img_create(panel);
    lv_img_set_src(icon_img, icon);
    lv_obj_set_width(icon_img, LV_SIZE_CONTENT);
    lv_obj_set_height(icon_img, LV_SIZE_CONTENT);
    lv_obj_align(icon_img, LV_ALIGN_LEFT_MID, 0, 0);

    //创建标题
    lv_obj_t *title_lb = lv_label_create(panel);
    lv_obj_set_width(title_lb, LV_SIZE_CONTENT);  
    lv_obj_set_height(title_lb, LV_SIZE_CONTENT);
    lv_obj_align(title_lb, LV_ALIGN_TOP_MID, 0, 0);
    lv_label_set_text(title_lb, title);
    return title_lb;
}

void ui_app_setting_setup(lv_obj_t* parent)
{
    lv_obj_add_event_cb(parent, ui_app_setting_parent_del, LV_EVENT_DELETE, NULL);

    //初始化设置列表
    //设置样式
    static lv_style_t list_style;
    lv_style_init(&list_style);
    lv_style_set_width(&list_style, lv_obj_get_width(parent));
    lv_style_set_height(&list_style, lv_obj_get_height(parent));
    lv_style_set_radius(&list_style, 0);
    lv_style_set_bg_opa(&list_style, 0);
    lv_style_set_border_width(&list_style, 0);
    lv_style_set_pad_left(&list_style, 0);
    lv_style_set_pad_right(&list_style, 0);
    lv_style_set_pad_top(&list_style, 50);
    lv_style_set_pad_bottom(&list_style, 50);
    lv_style_set_text_color(&list_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&list_style, &font_szp_Harmony_14);
    //设置列表
    lv_setting_list = lv_obj_create(parent);
    lv_obj_add_style(lv_setting_list, &list_style, 0);
    lv_obj_align(lv_setting_list, LV_ALIGN_CENTER,0,0);
    lv_obj_set_flex_flow(lv_setting_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(lv_setting_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(lv_setting_list, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(lv_setting_list, LV_DIR_VER);
    lv_obj_add_event_cb(lv_setting_list, ui_app_setting_list_scroll_cb, LV_EVENT_SCROLL, NULL);

    //设置标题栏
    lv_obj_t *lv_title_lb = lv_label_create(lv_setting_list);
    lv_obj_set_width(lv_title_lb, 100);
    lv_obj_set_height(lv_title_lb, LV_SIZE_CONTENT);
    lv_obj_align(lv_title_lb, LV_ALIGN_CENTER,0,20);
    lv_obj_set_style_text_font(lv_title_lb, &font_szp_Harmony_16,0);
    lv_label_set_text(lv_title_lb, "设置");
    ui_app_setting_set_list_item_style(lv_title_lb);

    //创建蓝牙开关
    lv_st_ble_panel=ui_app_setting_create_list_item_panel(lv_setting_list,180,64);
    ui_app_setting_create_list_item_title_icon(lv_st_ble_panel, "蓝牙开关", &img_szp_ble);
    lv_st_ble_sw = lv_switch_create(lv_st_ble_panel);
    lv_obj_align(lv_st_ble_sw, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_width(lv_st_ble_sw, 50);
    lv_obj_set_height(lv_st_ble_sw, 25);
    ((szp_ble_gatts_get_current_event() == EV_SZP_BLE_GATTS_START) ? lv_obj_add_state(lv_st_ble_sw, LV_STATE_CHECKED) : lv_obj_clear_state(lv_st_ble_sw, LV_STATE_CHECKED));
    lv_obj_add_event_cb(lv_st_ble_sw, ui_app_st_ble_sw_cb, LV_EVENT_CLICKED, NULL);

    //创建WIFI开关
    lv_st_wifi_panel=ui_app_setting_create_list_item_panel(lv_setting_list,180,64);
    ui_app_setting_create_list_item_title_icon(lv_st_wifi_panel, "WIFI开关", &img_szp_wifi);
    lv_st_wifi_sw = lv_switch_create(lv_st_wifi_panel);
    lv_obj_align(lv_st_wifi_sw, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_width(lv_st_wifi_sw, 50);
    lv_obj_set_height(lv_st_wifi_sw, 25);
    ((network_wifi_current_state() == EV_SZP_WIFI_CONNECT_SUCCESS) ? lv_obj_add_state(lv_st_wifi_sw, LV_STATE_CHECKED) : lv_obj_clear_state(lv_st_wifi_sw, LV_STATE_CHECKED));
    lv_obj_add_event_cb(lv_st_wifi_sw, ui_app_st_wifi_sw_cb, LV_EVENT_CLICKED, NULL);

    // 创建天气设置
    lv_st_weather_panel  = ui_app_setting_create_list_item_panel(lv_setting_list, 200, 75);
    lv_obj_t *weather_lb = ui_app_setting_create_list_item_title_icon(lv_st_weather_panel, "天气更新时间(分钟)", &img_szp_weather);
    lv_obj_align(weather_lb, LV_ALIGN_TOP_MID, 15, 0);

    lv_st_weather_drop = lv_dropdown_create(lv_st_weather_panel);
    lv_obj_set_style_text_font(lv_st_weather_drop, &lv_font_montserrat_12, 0);
    lv_dropdown_set_options(lv_st_weather_drop, "5\n10\n30\n60");
    uint8_t min = network_weather_get_update_time();
    switch (min)
    {
    case 5:
        lv_dropdown_set_selected(lv_st_weather_drop, 0);
        break;
     case 10:
         lv_dropdown_set_selected(lv_st_weather_drop, 1);
         break;
    case 30:
        lv_dropdown_set_selected(lv_st_weather_drop, 2);
        break;
    case 60:
        lv_dropdown_set_selected(lv_st_weather_drop, 3);
        break;
    default:
        lv_dropdown_set_selected(lv_st_weather_drop, 1);
        break;
    }
    lv_dropdown_set_dir(lv_st_weather_drop, LV_DIR_BOTTOM);
    lv_dropdown_set_symbol(lv_st_weather_drop, LV_SYMBOL_DOWN);
    lv_obj_set_width(lv_st_weather_drop, 100);
    lv_obj_set_height(lv_st_weather_drop, LV_SIZE_CONTENT);
    lv_obj_align(lv_st_weather_drop, LV_ALIGN_BOTTOM_MID, 15, -5);
    lv_obj_add_event_cb(lv_st_weather_drop, ui_app_st_weather_drop_cb, LV_EVENT_VALUE_CHANGED, NULL);

    //创建亮度设置
    lv_st_brightness_panel  = ui_app_setting_create_list_item_panel(lv_setting_list, 240, 64);
    lv_obj_t *brightness_lb = ui_app_setting_create_list_item_title_icon(lv_st_brightness_panel, "亮度设置", &img_szp_brightness);
    lv_obj_align(brightness_lb, LV_ALIGN_TOP_MID, 15, 0);
    lv_st_brightness_slider = lv_slider_create(lv_st_brightness_panel);
    lv_slider_set_range(lv_st_brightness_slider, 1, 100);
    lv_slider_set_value(lv_st_brightness_slider, 50, LV_ANIM_OFF);
    if (lv_slider_get_mode(lv_st_brightness_slider) == LV_SLIDER_MODE_RANGE)
        lv_slider_set_left_value(lv_st_brightness_slider, 0, LV_ANIM_OFF);
    lv_obj_set_width(lv_st_brightness_slider, 130);
    lv_obj_set_height(lv_st_brightness_slider, 15);
    lv_obj_clear_flag(lv_st_brightness_slider, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_align(lv_st_brightness_slider, LV_ALIGN_BOTTOM_MID, 10, -15);
     lv_obj_add_event_cb(lv_st_brightness_slider, ui_app_st_bn_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_st_brightness_lb = lv_label_create(lv_st_brightness_panel);
    lv_label_set_text(lv_st_brightness_lb, "50");
    lv_obj_align_to(lv_st_brightness_lb, lv_st_brightness_slider,LV_ALIGN_OUT_RIGHT_MID, 15, 0);

    // 创建关于
    lv_st_about_panel = ui_app_setting_create_list_item_panel(lv_setting_list, 180, 64);
    ui_app_setting_create_list_item_title_icon(lv_st_about_panel, "关于", &img_szp_about);
    lv_obj_t *project_lb = lv_label_create(lv_st_about_panel);
    lv_label_set_text(project_lb, "Project:实战派");
    lv_obj_align(project_lb, LV_ALIGN_BOTTOM_MID, 10, -25);
    lv_obj_t *author_lb = lv_label_create(lv_st_about_panel);
    lv_label_set_text(author_lb, "Author:果条");
    lv_obj_align_to(author_lb, project_lb,LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    //初始化时调用一次滚动
    lv_event_t ev;
    ev.target = lv_setting_list;
    ui_app_setting_list_scroll_cb(&ev);
}
