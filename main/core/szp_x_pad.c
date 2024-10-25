#include "szp_x_pad.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "inttypes.h"
#include "drv_manager.h"
#include "szp_sensor_manager.h"
#include "storage_manager.h"
#include "network_manager.h"
#include "work_controller.h"
#include "lvgl_manager.h"


void app_core_init()
{
   // 驱动初始化
   drv_init();
   //传感器初始化
   szp_sensor_init();
   //音频初始化 
/*TODO*/
   //存储初始化
   storage_init();
   //网络初始化
   network_init();
   //界面(LVGL)初始化
   szp_lvgl_init();
   //工作控制器初始化
   szp_work_init();
}

extern void ui_main_setup();//ui测试
void app_core_run(void)
{
   ui_main_setup();
   for (;;)
   {
      vTaskDelay(pdMS_TO_TICKS(10));
      szp_lvgl_timer_handler();
   }
}
