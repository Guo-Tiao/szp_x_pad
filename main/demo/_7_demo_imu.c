#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "string.h"
#include "common/common_macro.h"
#include "drivers/drv_manager.h"
#include "sensor/szp_sensor_manager.h"
#include "utils/imu_tool.h"
#include "drivers/szp_uart.h"


#define Deg_To_Angle_Conff 57.29579143313326

static void task_printf_qmi8658c(void *arg)
{
    while (1)
    {
        euler_angles e = u_calc_euler_angles(szp_sensor_ps_qmi8658c.data);
        printf("%f, %f, %f \n",(e.roll*Deg_To_Angle_Conff),(e.yaw*Deg_To_Angle_Conff),(e.pitch*Deg_To_Angle_Conff));

        vTaskDelay(SZP_MS_TO_TICK(100));
    }
    
}

static void task_recv_uart(void *arg)
{
    while (1)
    {
        float a, b,c;
        int len=scanf("%f %f %f", &a,&b,&c);
        if (len > 0)
        {
            //获取PID系数
        }
        vTaskDelay(SZP_MS_TO_TICK(200));
    }
    
}

void _7_demo_imu_run(void)
{
    // 驱动初始化
   drv_init();
   //传感器初始化
   szp_sensor_init();


    szp_sensor_PsQmi8658c_start_task();
    xTaskCreate(task_printf_qmi8658c, "task_printf_qmi8658c", 4098, NULL, 10, NULL);
    xTaskCreate(task_recv_uart, "task_recv_uart", 4098, NULL, 10, NULL);
    while (1)
    {
        vTaskDelay(SZP_MS_TO_TICK(500));
    }

    szp_sensor_MgQmc5883L_stop_task();
}