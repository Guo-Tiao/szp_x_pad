#include "lvgl_manager.h"
#include "lvgl.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include <inttypes.h>

#include "drivers/szp_lcd.h"
#include "drivers/szp_touch.h"

#define SZP_LVGL_TICK_PERIOD_MS    2

static lv_disp_draw_buf_t               szp_lvgl_disp_buf;           //显示缓存
static lv_disp_drv_t                         szp_lvgl_disp_drv;           //显示驱动 
static lv_disp_t                                *szp_lvgl_disp=NULL;           //显示屏幕 

/*************LCD回调*************/
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


