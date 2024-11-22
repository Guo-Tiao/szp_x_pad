#include "lvgl.h"
#include "sensor/szp_sensor_manager.h"
#include "utils/imu_tool.h"
#include "common/common_macro.h"

lv_obj_t *lv_gyro_chart;
lv_chart_series_t *lv_gyro_pitch;//俯仰角
lv_chart_series_t *lv_gyro_roll;//滚动角
lv_obj_t *lv_gyro_pitch_lb;//俯仰角标签
lv_obj_t *lv_gyro_roll_lb;//滚动角标签
lv_timer_t *lv_gyro_timer;//定时器

static void ui_app_gyro_update_data(lv_timer_t *timer)
{
    euler_angles e = u_calc_euler_angles(szp_sensor_ps_qmi8658c.data);
    int16_t pitch =(int16_t)(e.pitch * Deg_To_Angle_Conff);
    int16_t roll =(int16_t)(e.roll * Deg_To_Angle_Conff)-5;
    lv_chart_set_next_value(lv_gyro_chart, lv_gyro_pitch, pitch);
    lv_chart_set_next_value(lv_gyro_chart, lv_gyro_roll, roll);
    lv_label_set_text_fmt(lv_gyro_pitch_lb, "%d°", pitch);
    lv_label_set_text_fmt(lv_gyro_roll_lb, "%d°", roll);
}

//父界面删除事件
static void ui_app_gyro_parent_del(lv_event_t *ev)
{
    if(lv_gyro_timer)
    {
        lv_timer_del(lv_gyro_timer);
    }
    szp_sensor_PsQmi8658c_stop_task();
}

void ui_app_gyro_setup(lv_obj_t* parent)
{
    lv_obj_add_event_cb(parent, ui_app_gyro_parent_del, LV_EVENT_DELETE, NULL);
    
    //创建图表
    lv_gyro_chart = lv_chart_create(parent); 
    lv_obj_set_size(lv_gyro_chart, 230, 180);      
    lv_obj_align(lv_gyro_chart,LV_ALIGN_CENTER,15,-15);              
    lv_chart_set_update_mode(lv_gyro_chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_range(lv_gyro_chart, LV_CHART_AXIS_PRIMARY_Y, -100, 100);
    lv_obj_set_grid_cell(lv_gyro_chart, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_chart_set_div_line_count(lv_gyro_chart, 10, 20);
    lv_obj_set_style_radius(lv_gyro_chart, 5, 0);
    // 添加刻度
    lv_chart_set_axis_tick(lv_gyro_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 0, 11, 1, true, 30);
    //添加曲线
    lv_gyro_pitch = lv_chart_add_series(lv_gyro_chart,
                              lv_palette_main(LV_PALETTE_GREEN),
                              LV_CHART_AXIS_SECONDARY_X);
    lv_gyro_roll = lv_chart_add_series(lv_gyro_chart,
                              lv_palette_main(LV_PALETTE_BLUE),
                              LV_CHART_AXIS_SECONDARY_X);
    //添加定时器
    lv_gyro_timer=lv_timer_create(ui_app_gyro_update_data, 100, NULL); 

    //刷新图表
    lv_chart_refresh(lv_gyro_chart);          


    //创建标签
    lv_obj_t *lv_pitch_lbs = lv_label_create(parent);
    lv_obj_align(lv_pitch_lbs, LV_ALIGN_BOTTOM_LEFT, 40, -15);
    lv_label_set_text(lv_pitch_lbs, "pitch:");
    lv_gyro_pitch_lb=lv_label_create(parent);
    lv_obj_align_to(lv_gyro_pitch_lb, lv_pitch_lbs, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_label_set_text(lv_gyro_pitch_lb, "0°");

    lv_obj_t *lv_roll_lbs = lv_label_create(parent);
    lv_obj_align(lv_roll_lbs, LV_ALIGN_BOTTOM_RIGHT, -100, -15);
    lv_label_set_text(lv_roll_lbs, "roll:");
    lv_gyro_roll_lb = lv_label_create(parent);
    lv_obj_align_to(lv_gyro_roll_lb, lv_roll_lbs, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_label_set_text(lv_gyro_roll_lb, "0°");

    szp_sensor_PsQmi8658c_start_task();
}