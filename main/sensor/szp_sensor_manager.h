#pragma once
#include "szp_sensor_def.h"

//传感器初始化
void szp_sensor_init(void);
//开启温度传感器更新线程
void szp_sensor_ThGxhtc3_start_task();
//关闭温度传感器更新线程
void szp_sensor_ThGxhtc3_stop_task();

//开启姿态传感器更新线程
void szp_sensor_PsQmi8658c_start_task();
//关闭姿态传感器更新线程
void szp_sensor_PsQmi8658c_stop_task();

//开启地磁传感器更新线程
void szp_sensor_MgQmc5883L_start_task();
//关闭地磁传感器更新线程
void szp_sensor_MgQmc5883L_stop_task();

//导出温湿度传感器
extern SensorThGxhtc3 szp_sensor_th_gxhtc3;
//导出姿态传感器
extern SensorPsQmi8658c szp_sensor_ps_qmi8658c;
//导出地磁传感器
extern SensorMgQmc5883L szp_sensor_mg_qmc5883l;

