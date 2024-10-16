
#include "sdkconfig.h"
#include "core/szp_x_pad.h"


void app_main(void)
{
#if CONFIG_SZP_PROJECT_RUN_DEMO
   extern void _demo_run_(void);
   _demo_run_();
   return;
#endif
   //核心初始化
   app_core_init();
   //核心运行
   app_core_run();
   
}
