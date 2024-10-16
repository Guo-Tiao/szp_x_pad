#include "szp_sensor_manager.h"

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