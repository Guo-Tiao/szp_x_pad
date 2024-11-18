#include "lvgl.h"
#include "math.h"
#include "assets/szp_assets_def.h"
#include "sensor/szp_sensor_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "common/common_macro.h"


lv_obj_t *lv_compass_main_meter;//主盘
lv_obj_t *lv_compass_vice_meter;//副盘
lv_meter_scale_t *lv_compass_small_scale;//小指针
lv_meter_scale_t *lv_compass_big_scale;//大指针
lv_meter_scale_t *lv_compass_vice_scale;//副盘指针
lv_meter_scale_t *lv_compass_vice_hide_scale;//副盘隐藏指针
lv_meter_indicator_t *lv_compass_indicator; //索引盘
lv_obj_t *lv_compass_arrow_img;//索引箭头
lv_obj_t *lv_compass_direction_lb;//方向标签
lv_obj_t *lv_compass_angle_lb;//角度标签
lv_timer_t *lv_compass_timer;//刷新定时器

//父界面删除事件
static void ui_app_compass_parent_del(lv_event_t *ev)
{
    if(lv_compass_timer)
    {
        lv_timer_del(lv_compass_timer);
        lv_compass_timer = NULL;
    }
    szp_sensor_MgQmc5883L_stop_task();
}
static void ui_app_compass_update_timer_cb(lv_timer_t* ev)
{
    int comp_angle = 0; 
    comp_angle = round(szp_sensor_mg_qmc5883l.azimuth) + 160;
    comp_angle = (comp_angle) % 360;
    lv_label_set_text_fmt(lv_compass_angle_lb, "%d°", comp_angle);
    lv_meter_set_scale_range(lv_compass_main_meter, lv_compass_small_scale, 0, 360, 360, -comp_angle + 270);
    lv_meter_set_scale_range(lv_compass_main_meter, lv_compass_big_scale, 0, 360, 360, -comp_angle + 270);
    lv_meter_set_scale_range(lv_compass_vice_meter, lv_compass_vice_scale, 0, 330, 330, -comp_angle + 270);
    if ((comp_angle <= 30 && comp_angle >= 0) || (comp_angle <= 360 && comp_angle >= 330))
    {
        lv_label_set_text(lv_compass_direction_lb, "北");
    }
    else if (comp_angle > 30 && comp_angle < 60)
    {
        lv_label_set_text(lv_compass_direction_lb, "东北");
    }
    else if (comp_angle <= 120 && comp_angle >= 60)
    {
        lv_label_set_text(lv_compass_direction_lb, "东");
    }
    else if (comp_angle > 120 && comp_angle < 150)
    {
        lv_label_set_text(lv_compass_direction_lb, "东南");
    }
    else if (comp_angle <= 210 && comp_angle >= 150)
    {
        lv_label_set_text(lv_compass_direction_lb, "南");
    }
    else if (comp_angle > 210 && comp_angle < 240)
    {
        lv_label_set_text(lv_compass_direction_lb, "西南");
    }
    else if (comp_angle <= 300 && comp_angle >= 240)
    {
        lv_label_set_text(lv_compass_direction_lb, "西");
    }
    else if (comp_angle > 240 && comp_angle < 330)
    {
        lv_label_set_text(lv_compass_direction_lb, "西北");
    }
     
     
    
}
//指南针app安装
void ui_app_compass_setup(lv_obj_t* parent)
{
    lv_obj_add_event_cb(parent, ui_app_compass_parent_del, LV_EVENT_DELETE, NULL);
    // 创建主表盘
    lv_compass_main_meter = lv_meter_create(parent);
    lv_obj_center(lv_compass_main_meter);
    lv_obj_set_size(lv_compass_main_meter, 230, 230);
    lv_obj_set_style_bg_opa(lv_compass_main_meter, LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_color(lv_compass_main_meter, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_width(lv_compass_main_meter, 0, 1);
    lv_obj_set_style_pad_all(lv_compass_main_meter, 0, 0);
    lv_obj_remove_style(lv_compass_main_meter, NULL, LV_PART_INDICATOR);
    lv_obj_center(lv_compass_main_meter);
    // 创建指针
    lv_compass_small_scale = lv_meter_add_scale(lv_compass_main_meter);
    lv_meter_set_scale_ticks(lv_compass_main_meter, lv_compass_small_scale, 45 * 4 + 1, 1, 5, lv_color_hex(0xffffff));
    lv_meter_set_scale_range(lv_compass_main_meter, lv_compass_small_scale, 0, 360, 360, 270);
    lv_compass_big_scale = lv_meter_add_scale(lv_compass_main_meter);
    lv_meter_set_scale_ticks(lv_compass_main_meter, lv_compass_big_scale, 9 * 4 + 1, 2, 10, lv_color_hex(0xffffff));
    lv_meter_set_scale_range(lv_compass_main_meter, lv_compass_big_scale, 0, 360, 360, 270);

    // 创建副表盘
    lv_compass_vice_meter = lv_meter_create(parent);
    lv_obj_center(lv_compass_vice_meter);
    lv_obj_set_size(lv_compass_vice_meter, 200, 200);
    lv_obj_set_style_bg_opa(lv_compass_vice_meter, LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_color(lv_compass_vice_meter, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_width(lv_compass_vice_meter, 0, 0);
    lv_obj_set_style_pad_all(lv_compass_vice_meter, 0, 0);
    lv_obj_remove_style(lv_compass_vice_meter, NULL, LV_PART_INDICATOR);
    lv_obj_set_style_text_font(lv_compass_vice_meter, &lv_font_montserrat_12, 0);
    lv_obj_center(lv_compass_vice_meter);
    // 创建刻度指针
    lv_compass_vice_scale = lv_meter_add_scale(lv_compass_vice_meter);
    lv_meter_set_scale_ticks(lv_compass_vice_meter, lv_compass_vice_scale, 11 + 1, 2, 15, lv_color_hex(0xffffff));
    lv_meter_set_scale_range(lv_compass_vice_meter, lv_compass_vice_scale, 0, 330, 330, 270);
    lv_meter_set_scale_major_ticks(lv_compass_vice_meter, lv_compass_vice_scale, 1, 2, 15, lv_color_hex(0xffffff), 12);
    // 创建隐藏指针供indicator使用
    lv_compass_vice_hide_scale = lv_meter_add_scale(lv_compass_vice_meter);
    lv_meter_set_scale_ticks(lv_compass_vice_meter, lv_compass_vice_hide_scale, 0, 2, 15, lv_color_hex(0xffffff));
    lv_meter_set_scale_range(lv_compass_vice_meter, lv_compass_vice_hide_scale, 0, 360, 360, 270);
    lv_compass_indicator = lv_meter_add_arc(lv_compass_vice_meter, lv_compass_vice_hide_scale, 15, lv_color_hex(0xff0000), 0);
    lv_meter_set_indicator_end_value(lv_compass_vice_meter, lv_compass_indicator, 360);

    // 创建指南箭头
    lv_compass_arrow_img = lv_img_create(lv_compass_vice_meter);
    lv_img_set_src(lv_compass_arrow_img, &img_szp_compass_pointer);
    lv_obj_align(lv_compass_arrow_img, LV_ALIGN_CENTER, 0, -45);
    // 创建方向和角度标签
    lv_compass_direction_lb = lv_label_create(lv_compass_vice_meter);
    lv_obj_set_style_text_font(lv_compass_direction_lb, &font_szp_compass_24, 0);
    lv_label_set_text(lv_compass_direction_lb, "北");
    lv_obj_align(lv_compass_direction_lb, LV_ALIGN_CENTER, 0, -10);
    lv_compass_angle_lb = lv_label_create(lv_compass_vice_meter);
    lv_obj_set_style_text_font(lv_compass_angle_lb, &font_szp_compass_24, 0);
    lv_label_set_text(lv_compass_angle_lb, "360°");
    lv_obj_align(lv_compass_angle_lb, LV_ALIGN_CENTER, 2, 20);

    //开启地磁更新线程
    szp_sensor_MgQmc5883L_start_task();
    lv_compass_timer = lv_timer_create(ui_app_compass_update_timer_cb, 150, NULL);

}