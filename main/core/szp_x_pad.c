#include "szp_x_pad.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "inttypes.h"

#include "common/common_macro.h"
#include "drivers/drv_manager.h"
#include "sensor/szp_sensor_manager.h"
#include "storage/storage_manager.h"
#include "network/network_manager.h"
#include "work_controller.h"
#include "ui/ui_manager.h"


void app_core_init()
{
   // 驱动初始化
   drv_init();
   //传感器初始化
   szp_sensor_init();
   //存储初始化
   storage_init();
   //界面(LVGL)初始化
   szp_lvgl_init();
#if CONFIG_ENABLE_SZP_IOT
   //网络初始化
   network_init();
#endif
   //工作控制器初始化
   szp_work_init();

}

extern void _demo_ui_test(void);
void app_core_run(void)
{
   szp_ui_sys_setup();
   for (;;)
   {
      vTaskDelay(SZP_MS_TO_TICK(10));
      szp_lvgl_timer_handler();
   }
}
