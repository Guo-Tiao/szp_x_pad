#include "szp_x_pad.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "inttypes.h"
#include "drv_manager.h"
#include "szp_sensor_manager.h"
#include "lvgl_manager.h"


void app_core_init()
{
    //驱动初始化
   drv_init();
   //传感器初始化
   szp_sensor_init();
   //音频初始化 TODO

   //界面(LVGL)初始化
   szp_lvgl_init();
}


void app_core_run(void)
{

   for (;;)
   {
      vTaskDelay(pdMS_TO_TICKS(10));
      szp_lvgl_timer_handler();
   }
}
