#pragma once
#include "szp_sensor_def.h"

//传感器初始化
void szp_sensor_init(void);

//导出温湿度传感器
extern SensorThGxhtc3 szp_sensor_th_gxhtc3;
//导出姿态传感器
extern SensorPsQmi8658c szp_sensor_ps_qmi8658c;
//导出地磁传感器
extern SensorMgQmc5883L szp_sensor_mg_qmc5883l;

