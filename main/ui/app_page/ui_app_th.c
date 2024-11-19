#include "lvgl.h"
#include "assets/szp_assets_def.h"
#include "sensor/szp_sensor_manager.h"

//温度
lv_obj_t *lv_th_temp_meter;//温度计表
lv_meter_scale_t *lv_th_temp_scale;//温度刻度
lv_meter_indicator_t *lv_th_temp_value;//温度值
lv_obj_t *lv_th_temp_lb;//温度标签
//湿度
lv_obj_t *lv_th_humi_meter;//湿度计
lv_meter_scale_t *lv_th_humi_scale;//湿度刻度
lv_meter_indicator_t *lv_th_humi_value;//湿度值
lv_obj_t *lv_th_humi_lb;//湿度标签

lv_timer_t *lv_th_timer;//更新定时器

//父界面删除事件
static void ui_app_th_parent_del(lv_event_t *ev)
{   
    if(lv_th_timer)
    {
        lv_timer_del(lv_th_timer);
        lv_th_timer = NULL;
    }
}

static void ui_app_th_update_timer_cb(lv_timer_t *t)
{
    int temp = szp_sensor_th_gxhtc3.iTemp;
    lv_label_set_text_fmt(lv_th_temp_lb, "温度:%d°", temp);
    lv_meter_set_indicator_value(lv_th_temp_meter, lv_th_temp_value, temp);

    int humi = szp_sensor_th_gxhtc3.iHumi;
    lv_label_set_text_fmt(lv_th_humi_lb, "湿度:%d%%", humi);
    lv_meter_set_indicator_end_value(lv_th_humi_meter, lv_th_humi_value, humi);
}

void ui_app_th_setup(lv_obj_t* parent)
{
    lv_obj_add_event_cb(parent, ui_app_th_parent_del, LV_EVENT_DELETE, NULL);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    
    // 创建温度表盘
    lv_th_temp_meter = lv_meter_create(parent);
    lv_obj_center(lv_th_temp_meter);
    lv_obj_set_size(lv_th_temp_meter, 230, 230);
    lv_obj_set_style_bg_opa(lv_th_temp_meter, LV_OPA_30, 0);
    lv_obj_set_style_text_color(lv_th_temp_meter, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_width(lv_th_temp_meter, 0, 0);
    lv_obj_set_style_pad_all(lv_th_temp_meter, 0, 0);
    lv_obj_remove_style(lv_th_temp_meter, NULL, LV_PART_INDICATOR);
    lv_obj_center(lv_th_temp_meter);

    // 创建冷热范围
    lv_th_temp_scale = lv_meter_add_scale(lv_th_temp_meter);
    lv_meter_set_scale_ticks(lv_th_temp_meter, lv_th_temp_scale, 80+ 1, 1, 8, lv_color_hex(0xffffff));
    lv_meter_set_scale_range(lv_th_temp_meter, lv_th_temp_scale, -30, 50, 190, 175);
    lv_meter_set_scale_major_ticks(lv_th_temp_meter, lv_th_temp_scale, 10, 2, 25, lv_color_hex(0xffffff), 12);

    lv_meter_indicator_t *lv_th_temp_cool_indicator=lv_meter_add_arc(lv_th_temp_meter,lv_th_temp_scale,10, lv_palette_main(LV_PALETTE_BLUE),-8);
    lv_meter_set_indicator_start_value(lv_th_temp_meter, lv_th_temp_cool_indicator, -50);
    lv_meter_set_indicator_end_value(lv_th_temp_meter, lv_th_temp_cool_indicator, 15);

    lv_meter_indicator_t *lv_th_temp_comfort_indicator=lv_meter_add_arc(lv_th_temp_meter,lv_th_temp_scale,10,lv_palette_main(LV_PALETTE_GREEN),-8);
    lv_meter_set_indicator_start_value(lv_th_temp_meter, lv_th_temp_comfort_indicator, 15);
    lv_meter_set_indicator_end_value(lv_th_temp_meter, lv_th_temp_comfort_indicator, 25);

    lv_meter_indicator_t *lv_th_temp_hot_indicator=lv_meter_add_arc(lv_th_temp_meter,lv_th_temp_scale,10,lv_palette_main(LV_PALETTE_RED),-8);
    lv_meter_set_indicator_start_value(lv_th_temp_meter, lv_th_temp_hot_indicator, 25);
    lv_meter_set_indicator_end_value(lv_th_temp_meter, lv_th_temp_hot_indicator, 50);

    //创建温度指针
    lv_th_temp_value= lv_meter_add_needle_img(lv_th_temp_meter, lv_th_temp_scale, &img_szp_th_pointer,8, 55);
    lv_meter_set_indicator_value(lv_th_temp_meter, lv_th_temp_value, 10);

    //创建温度标签
    lv_th_temp_lb = lv_label_create(lv_th_temp_meter);
    lv_obj_set_style_text_font(lv_th_temp_lb, &font_szp_Harmony_14, 0);
    lv_label_set_text(lv_th_temp_lb, "温度:10°");
    lv_obj_align(lv_th_temp_lb, LV_ALIGN_CENTER, 0, 20);

    // 创建湿度表盘
    lv_th_humi_meter = lv_meter_create(parent);
    lv_obj_center(lv_th_humi_meter);
    lv_obj_align(lv_th_humi_meter, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(lv_th_humi_meter, 230, 230);
    lv_obj_set_style_bg_opa(lv_th_humi_meter, LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_color(lv_th_humi_meter, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_width(lv_th_humi_meter, 0, 0);
    lv_obj_set_style_pad_all(lv_th_humi_meter, 8, 0);
    lv_obj_remove_style(lv_th_humi_meter, NULL, LV_PART_INDICATOR);

    lv_th_humi_scale = lv_meter_add_scale(lv_th_humi_meter);
    lv_meter_set_scale_ticks(lv_th_humi_meter, lv_th_humi_scale, 50+1, 2, 10,  lv_color_hex(0xffffff)); 
    lv_meter_set_scale_range(lv_th_humi_meter, lv_th_humi_scale, 0, 100, 140, 20);
    lv_meter_set_scale_major_ticks(lv_th_humi_meter, lv_th_humi_scale, 5, 2, 12, lv_color_hex(0xffffff), 12); 
    
    lv_meter_indicator_t * humi_indic;
    humi_indic = lv_meter_add_arc(lv_th_humi_meter, lv_th_humi_scale, 3, lv_palette_main(LV_PALETTE_BLUE),0); 
    lv_meter_set_indicator_start_value(lv_th_humi_meter, humi_indic, 0);
    lv_meter_set_indicator_end_value(lv_th_humi_meter, humi_indic, 40);
    humi_indic = lv_meter_add_scale_lines(lv_th_humi_meter, lv_th_humi_scale, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_BLUE), false, 0);
    lv_meter_set_indicator_start_value(lv_th_humi_meter, humi_indic, 0);
    lv_meter_set_indicator_end_value(lv_th_humi_meter, humi_indic, 40);


    humi_indic = lv_meter_add_arc(lv_th_humi_meter, lv_th_humi_scale, 3, lv_palette_main(LV_PALETTE_GREEN), 0); 
    lv_meter_set_indicator_start_value(lv_th_humi_meter, humi_indic, 40);
    lv_meter_set_indicator_end_value(lv_th_humi_meter, humi_indic, 70);
    humi_indic = lv_meter_add_scale_lines(lv_th_humi_meter, lv_th_humi_scale, lv_palette_main(LV_PALETTE_GREEN), lv_palette_main(LV_PALETTE_GREEN), false, 0);
    lv_meter_set_indicator_start_value(lv_th_humi_meter, humi_indic, 40);
    lv_meter_set_indicator_end_value(lv_th_humi_meter, humi_indic, 70);

    humi_indic = lv_meter_add_arc(lv_th_humi_meter, lv_th_humi_scale, 3, lv_palette_main(LV_PALETTE_RED), 0); 
    lv_meter_set_indicator_start_value(lv_th_humi_meter, humi_indic, 70);
    lv_meter_set_indicator_end_value(lv_th_humi_meter, humi_indic, 100);
    humi_indic = lv_meter_add_scale_lines(lv_th_humi_meter, lv_th_humi_scale, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false, 0);
    lv_meter_set_indicator_start_value(lv_th_humi_meter, humi_indic, 70);
    lv_meter_set_indicator_end_value(lv_th_humi_meter, humi_indic, 100);
    
    //创建湿度条
    lv_th_humi_value = lv_meter_add_arc(lv_th_humi_meter, lv_th_humi_scale, 5, lv_palette_main(LV_PALETTE_ORANGE), 7);
    lv_meter_set_indicator_start_value(lv_th_humi_meter, lv_th_humi_value, 0);
    lv_meter_set_indicator_end_value(lv_th_humi_meter, lv_th_humi_value, 0); 
    
    //创建湿度标签
    lv_th_humi_lb = lv_label_create(lv_th_humi_meter);
    lv_obj_set_style_text_font(lv_th_humi_lb, &font_szp_Harmony_14, 0);
    lv_label_set_text(lv_th_humi_lb, "湿度:0%");
    lv_obj_align(lv_th_humi_lb, LV_ALIGN_CENTER, 0, 40);

    //创建定时器
    lv_th_timer = lv_timer_create(ui_app_th_update_timer_cb, 500, NULL);
    ui_app_th_update_timer_cb(NULL);
}