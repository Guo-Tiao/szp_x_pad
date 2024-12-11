#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"

#include "freertos/queue.h"
#include "common/common_macro.h"
#include "ui/ui_manager.h"
#include "drivers/drv_manager.h"
#include "drivers/szp_cam.h"
#include "drivers/szp_lcd.h"

//摄像头帧队列
static QueueHandle_t xQueueCamFrame = NULL;
// lcd处理任务
static void task_show_frame(void *arg)
{
    camera_fb_t *frame = NULL;

    while (true)
    {
        if (xQueueReceive(xQueueCamFrame, &frame, portMAX_DELAY))
        {
            szp_lcd_draw_bitmap((uint16_t *)frame->buf, 0, 0, frame->width, frame->height);
            szp_cam_fb_return(frame);
        }
    }
}

// 摄像头处理任务
static void task_get_frame(void *arg)
{
    while (true)
    {
        camera_fb_t *frame = szp_cam_fb_get();
        if (frame)
        {
            xQueueSend(xQueueCamFrame, &frame, portMAX_DELAY);
        }
    }
}

// 摄像头运行
void demo_cam_create_task(void)
{
    xQueueCamFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xTaskCreatePinnedToCore(task_show_frame, "task_show_frame", 3 * 1024, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(task_get_frame, "task_get_frame", 4 * 1024, NULL, 5, NULL, 0);
}

void _5_demo_cam_run(void)
{
        // 驱动初始化
   drv_init();
    //界面(LVGL)初始化
   szp_lvgl_init();

    //创建线程
   demo_cam_create_task();
   while (1)
   {
     vTaskDelay(SZP_MS_TO_TICK(10));
      szp_lvgl_timer_handler();
   }
   
}