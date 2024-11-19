#include "szp_sensor_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "common/common_macro.h"

extern void sensor_th_gxhtc3_init(void);
extern void sensor_qs_qmi8658c_init(void);
extern void sensor_th_qmc5883l_init(void);

void szp_sensor_init(void)
{
    //温湿度传感器初始化
    sensor_th_gxhtc3_init();
    //姿态传感器初始化
    sensor_qs_qmi8658c_init();
    //地磁传感器初始化
    sensor_th_qmc5883l_init();
}


//温度传感器更新线程句柄
TaskHandle_t sensor_ThGxhtc3_update_handle=NULL;
//温度传感器更新线程
static void task_sensor_ThGxhtc3_update(void *arg)
{
    if(szp_sensor_th_gxhtc3.update==NULL)
    {
        sensor_ThGxhtc3_update_handle = NULL;
        vTaskDelete(NULL);
    }
    for (;;)
    {
        szp_sensor_th_gxhtc3.update();
        vTaskDelay(SZP_MS_TO_TICK(1000));
    }
    
}
void szp_sensor_ThGxhtc3_start_task()
{
    if(sensor_ThGxhtc3_update_handle==NULL)
    {
        xTaskCreate(task_sensor_ThGxhtc3_update, "snr_ThGxhtc3_update,", 2048, NULL, 10, sensor_ThGxhtc3_update_handle);
    }
}

void szp_sensor_ThGxhtc3_stop_task()
{
    if(sensor_ThGxhtc3_update_handle!=NULL)
    {
        vTaskDelete(sensor_ThGxhtc3_update_handle);
        sensor_ThGxhtc3_update_handle = NULL;
    }
}

//姿态传感器更新线程句柄
TaskHandle_t sensor_PsQmi8658c_update_handle=NULL;
//姿态传感器更新线程
static void task_sensor_PsQmi8658c_update(void *arg)
{
    if(szp_sensor_ps_qmi8658c.update==NULL)
    {
        sensor_PsQmi8658c_update_handle = NULL;
        vTaskDelete(NULL);
    }
    for (;;)
    {
        szp_sensor_ps_qmi8658c.update();
        vTaskDelay(SZP_MS_TO_TICK(100));
    }
    
}

void szp_sensor_PsQmi8658c_start_task()
{
    if(sensor_PsQmi8658c_update_handle==NULL)
    {
        xTaskCreate(task_sensor_PsQmi8658c_update, "snr_PsQmi8658c_update,", 2048, NULL, 10, sensor_PsQmi8658c_update_handle);
    }
}

void szp_sensor_PsQmi8658c_stop_task()
{
    if(sensor_PsQmi8658c_update_handle!=NULL)
    {
        vTaskDelete(sensor_PsQmi8658c_update_handle);
        sensor_PsQmi8658c_update_handle = NULL;
    }
}

//地磁传感器更新线程句柄
TaskHandle_t sensor_MgQmc5883L_update_handle=NULL;
//温度传感器更新线程
static void task_sensor_MgQmc5883L_update(void *arg)
{
    if(szp_sensor_mg_qmc5883l.update==NULL)
    {
        sensor_MgQmc5883L_update_handle = NULL;
        vTaskDelete(NULL);
    }
    for (;;)
    {
        szp_sensor_mg_qmc5883l.update();
        vTaskDelay(SZP_MS_TO_TICK(100));
    }
    
}

void szp_sensor_MgQmc5883L_start_task()
{
     if(sensor_MgQmc5883L_update_handle==NULL)
    {
        xTaskCreate(task_sensor_MgQmc5883L_update, "snr_MgQmc_update,", 2048, NULL, 10, sensor_MgQmc5883L_update_handle);
    }
}

void szp_sensor_MgQmc5883L_stop_task()
{
    if(sensor_MgQmc5883L_update_handle!=NULL)
    {
        vTaskDelete(sensor_MgQmc5883L_update_handle);
        sensor_MgQmc5883L_update_handle = NULL;
    }
}
