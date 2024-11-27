#include "ui_szp_monitor_page.h"
#include "lvgl.h"
#include "assets/szp_assets_def.h"
#include "ui_common_def.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "common/common_macro.h"

//监控页面成员
static lv_style_t lv_monitor_page_style;//app页面样式
lv_obj_t * lv_monitor_page_obj;//app视图对象

lv_obj_t *lv_monitor_cpu_bar;//cpu使用bar
lv_obj_t *lv_monitor_cpu_lb;//cpu使用lb

lv_obj_t *lv_monitor_gpu_bar;//gpu使用bar
lv_obj_t *lv_monitor_gpu_lb;//gpu使用lb


lv_obj_t *lv_monitor_mem_bar;//内存使用bar
lv_obj_t *lv_monitor_mem_lb;//内存使用lb

lv_obj_t *lv_monitor_net_down_lb;//网络上传网速
lv_obj_t *lv_monitor_net_up_lb;//网络下载网速


//监控线程
TaskHandle_t ui_monitor_task_handle;
static void task_ui_monitor_read_data(void *arg)
{
    while (1)
    {
        float cpu, gpu, mem, send_kbyte, recv_kbyte;
        int len=scanf("%f,%f,%f,%f,%f", &cpu,&gpu,&mem,&send_kbyte,&recv_kbyte);
        if (len > 0)
        {
            lv_bar_set_value(lv_monitor_cpu_bar, (int)cpu, true);
            lv_label_set_text_fmt(lv_monitor_cpu_lb,"%d%%", (int)cpu);

            lv_bar_set_value(lv_monitor_gpu_bar,  (int)gpu, true);
            lv_label_set_text_fmt(lv_monitor_gpu_lb, "%d%%",  (int)gpu);

            lv_bar_set_value(lv_monitor_mem_bar,  (int)mem, true);
            lv_label_set_text_fmt(lv_monitor_mem_lb, "%d%%",  (int)mem);
            if(send_kbyte>1024)
            {
                lv_label_set_text_fmt(lv_monitor_net_up_lb,"%dMb/s",(int)(send_kbyte/1024));
            }
            else
            {
                 lv_label_set_text_fmt(lv_monitor_net_up_lb,"%dKb/s",(int)send_kbyte);
            }
            if(recv_kbyte>1024)
            {
                lv_label_set_text_fmt(lv_monitor_net_down_lb,"%dMb/s",(int)(recv_kbyte/1024));
            }
            else
            {
                 lv_label_set_text_fmt(lv_monitor_net_down_lb,"%dKb/s",(int)recv_kbyte);
            }
            
        }

        vTaskDelay(SZP_MS_TO_TICK(500));
    }
        
}
//线程控制
static void ui_monitor_page_task_control(bool run)
{
    if(ui_monitor_task_handle)
    {
        eTaskState state = eTaskGetState(ui_monitor_task_handle);
        if(run&&state==eSuspended)  //停止态则运行 
        {
            vTaskResume(ui_monitor_task_handle);
        }
        else if(!run&&(state==eRunning||state==eReady||state==eBlocked))    //运行/准备/阻塞则进行暂停
        {
            vTaskSuspend(ui_monitor_task_handle);
        }
      
    }

}

//开启或者关闭线程
static void ui_monitor_page_move_cb(lv_event_t* ev)
{
    lv_area_t area;
    lv_obj_get_coords(lv_monitor_page_obj, &area);
    if(area.x1==0)      //页面显示
    {
        ui_monitor_page_task_control(true);
    }
    else if(area.x1<=-(lv_obj_get_width(lv_monitor_page_obj)-5)) //页面隐藏
    {
        ui_monitor_page_task_control(false);
    }
}

void ui_monitor_page_setup(lv_obj_t *parent)
{
   // 初始化视图样式和对象
    lv_style_init(&lv_monitor_page_style);
    lv_style_set_radius(&lv_monitor_page_style, 0);
    lv_style_set_bg_opa(&lv_monitor_page_style, LV_OPA_90);
    lv_style_set_bg_color(&lv_monitor_page_style, lv_color_hex(0x000000));
    lv_style_set_text_color(&lv_monitor_page_style, lv_color_hex(0xffffff));
    lv_style_set_border_width(&lv_monitor_page_style, 0);
    lv_style_set_pad_all(&lv_monitor_page_style, 0);
    lv_style_set_width(&lv_monitor_page_style, SZP_LV_UI_HOR);
    lv_style_set_height(&lv_monitor_page_style, SZP_LV_UI_VER);

    lv_monitor_page_obj = lv_obj_create(parent);
    lv_obj_add_style(lv_monitor_page_obj, &lv_monitor_page_style, 0);
    lv_obj_set_scrollbar_mode(lv_monitor_page_obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(lv_monitor_page_obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(lv_monitor_page_obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(lv_monitor_page_obj, 10, 0);
    lv_obj_set_style_pad_top(lv_monitor_page_obj, 20, 0);
    lv_obj_set_style_pad_bottom(lv_monitor_page_obj, 20, 0);
    lv_obj_set_scroll_snap_y(lv_monitor_page_obj, LV_SCROLL_SNAP_CENTER);
    lv_obj_center(lv_monitor_page_obj);

    static lv_style_t area_style;
    lv_style_init(&area_style);
    lv_style_set_radius(&area_style, 0);
    lv_style_set_bg_opa(&area_style, LV_OPA_TRANSP);
    lv_style_set_text_color(&area_style, lv_color_hex(0xffffff));
    lv_style_set_border_width(&area_style, 0);
    lv_style_set_text_font(&area_style, &font_szp_Harmony_14);
    lv_style_set_pad_all(&area_style, 0);
    lv_style_set_width(&area_style, SZP_LV_UI_HOR-20);
    lv_style_set_height(&area_style, 40);
    lv_style_set_border_color(&area_style, lv_color_hex(0xFFFFFF));
    lv_style_set_border_opa(&area_style, LV_OPA_COVER);
    lv_style_set_border_width(&area_style, 0);
   // lv_style_set_border_side(&area_style, LV_BORDER_SIDE_BOTTOM);

    // 创建区域
    lv_obj_t *cpu_area = lv_obj_create(lv_monitor_page_obj);
    lv_obj_add_style(cpu_area, &area_style, 0);
    
    lv_obj_t *gpu_area = lv_obj_create(lv_monitor_page_obj);
    lv_obj_add_style(gpu_area, &area_style, 0);

    lv_obj_t *mem_area = lv_obj_create(lv_monitor_page_obj);
    lv_obj_add_style(mem_area, &area_style, 0);

    lv_obj_t *net_area = lv_obj_create(lv_monitor_page_obj);
    lv_obj_set_style_height(net_area, 50,0);
    lv_obj_add_style(net_area, &area_style, 0);



    //创建进度条样式
    static lv_style_t style_bar_bg;
    static lv_style_t style_bar_indic;
    lv_style_init(&style_bar_bg);
    lv_style_set_border_width(&style_bar_bg, 2);
    lv_style_set_pad_all(&style_bar_bg, 5); 
    lv_style_set_radius(&style_bar_bg, 5);
    lv_style_init(&style_bar_indic);
    lv_style_set_bg_opa(&style_bar_indic, LV_OPA_COVER);
    lv_style_set_radius(&style_bar_indic, 3);


    //CPU
    lv_obj_t *cpu_lb = lv_label_create(cpu_area);
    lv_obj_align(cpu_lb, LV_ALIGN_LEFT_MID, 10, 0);
    lv_label_set_text(cpu_lb, "CPU:");

    lv_monitor_cpu_bar = lv_bar_create(cpu_area);
    lv_bar_set_range(lv_monitor_cpu_bar, 0, 100);
    lv_obj_remove_style_all(lv_monitor_cpu_bar);
    lv_obj_add_style(lv_monitor_cpu_bar, &style_bar_bg, 0);
    lv_obj_set_style_border_color(lv_monitor_cpu_bar, lv_palette_main(LV_PALETTE_BLUE),0);
    lv_obj_add_style(lv_monitor_cpu_bar, &style_bar_indic, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(lv_monitor_cpu_bar, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR);
    lv_obj_set_size(lv_monitor_cpu_bar, 180, 15);
    lv_obj_align(lv_monitor_cpu_bar,LV_ALIGN_CENTER,0,0);
    lv_bar_set_value(lv_monitor_cpu_bar, 0,true);

    lv_monitor_cpu_lb = lv_label_create(cpu_area);
    lv_obj_align(lv_monitor_cpu_lb, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_label_set_text(lv_monitor_cpu_lb, "0%");
 
    //GPU
    lv_obj_t *gpu_lb = lv_label_create(gpu_area);
    lv_obj_align(gpu_lb, LV_ALIGN_LEFT_MID, 10, 0);
    lv_label_set_text(gpu_lb, "GPU:");

    lv_monitor_gpu_bar = lv_bar_create(gpu_area);
    lv_bar_set_range(lv_monitor_gpu_bar, 0, 100);
    lv_obj_remove_style_all(lv_monitor_gpu_bar);
    lv_obj_add_style(lv_monitor_gpu_bar, &style_bar_bg, 0);
    lv_obj_set_style_border_color(lv_monitor_gpu_bar, lv_palette_main(LV_PALETTE_YELLOW),0);
    lv_obj_add_style(lv_monitor_gpu_bar, &style_bar_indic, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(lv_monitor_gpu_bar, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_INDICATOR);
    lv_obj_set_size(lv_monitor_gpu_bar, 180, 15);
    lv_obj_align(lv_monitor_gpu_bar,LV_ALIGN_CENTER,0,0);
    lv_bar_set_value(lv_monitor_gpu_bar, 0,true);

    lv_monitor_gpu_lb = lv_label_create(gpu_area);
    lv_obj_align(lv_monitor_gpu_lb, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_label_set_text(lv_monitor_gpu_lb, "0%");

    //内存
    lv_obj_t *mem_lb = lv_label_create(mem_area);
    lv_obj_align(mem_lb, LV_ALIGN_LEFT_MID, 10, 0);
    lv_label_set_text(mem_lb, "Mem:");

    lv_monitor_mem_bar = lv_bar_create(mem_area);
    lv_bar_set_range(lv_monitor_mem_bar, 0, 100);
    lv_obj_remove_style_all(lv_monitor_mem_bar);
    lv_obj_add_style(lv_monitor_mem_bar, &style_bar_bg, 0);
    lv_obj_set_style_border_color(lv_monitor_mem_bar, lv_palette_main(LV_PALETTE_PURPLE),0);
    lv_obj_add_style(lv_monitor_mem_bar, &style_bar_indic, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(lv_monitor_mem_bar, lv_palette_main(LV_PALETTE_PURPLE), LV_PART_INDICATOR);
    lv_obj_set_size(lv_monitor_mem_bar, 180, 15);
    lv_obj_align(lv_monitor_mem_bar,LV_ALIGN_CENTER,0,0);
    lv_bar_set_value(lv_monitor_mem_bar, 0,true);

    lv_monitor_mem_lb = lv_label_create(mem_area);
    lv_obj_align(lv_monitor_mem_lb, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_label_set_text(lv_monitor_mem_lb, "0%");




    //网速
    lv_obj_t *net_lb = lv_label_create(net_area);
    lv_obj_align(net_lb, LV_ALIGN_LEFT_MID, 10, 0);
    lv_label_set_text(net_lb, "Net:");

    lv_obj_t *net_up_lb = lv_label_create(net_area);
    lv_obj_align_to(net_up_lb, net_lb,LV_ALIGN_OUT_RIGHT_TOP, 15, 0);
    lv_label_set_text(net_up_lb, "Up:");
    lv_monitor_net_up_lb = lv_label_create(net_area);
    lv_obj_align_to(lv_monitor_net_up_lb, net_up_lb, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_label_set_text(lv_monitor_net_up_lb, "0b/s");

    lv_obj_t *net_down_lb = lv_label_create(net_area);
    lv_obj_align(net_down_lb, LV_ALIGN_CENTER, 40, 0);
    lv_label_set_text(net_down_lb, "Down:");
    lv_monitor_net_down_lb = lv_label_create(net_area);
    lv_obj_align_to(lv_monitor_net_down_lb, net_down_lb, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_label_set_text(lv_monitor_net_down_lb, "0b/s");

    //创建线程
    xTaskCreate(task_ui_monitor_read_data, "tk_ui_monitor", 4096, NULL, 10, ui_monitor_task_handle);
    ui_monitor_page_task_control(false);
    //创建页面控制
    lv_obj_add_event_cb(lv_monitor_page_obj, ui_monitor_page_move_cb, LV_EVENT_DRAW_PART_END, NULL);
}