#pragma once
#include "esp_err.h"
#include "stdbool.h"
#include "common/common_data.h"

/****************** 温湿度传感器GXHTC3******************/
//温湿度传感器结构体
typedef struct sensor_th_gxhtc3_t
{
    //传感器数据
    uint16_t id;    //传感器ID
    float fTemp;    //浮点温度
    float fHumi;    //浮点湿度
    uint16_t iTemp; //整数温度
    uint16_t iHumi; //整数湿度
    
/*传感器方法*/
    //读取ID
    esp_err_t (*read_id)(void);
    //更新数据
    esp_err_t (*update)(void);
}SensorThGxhtc3;


/****************** 姿态传感器QMI8658C******************/


//姿态传感器结构体
typedef struct sensor_ps_qmi8658c_t
{
    //传感器数据
    imu_data data;
    //更新数据
    bool (*update)(void);
} SensorPsQmi8658c;

/****************** 地磁传感器QMC5883L******************/
//姿态传感器结构体
typedef struct sensor_mg_qmc5883l_t
{
    coord_3d_i16 mag;//地磁坐标
    float   azimuth;
    //更新数据
    bool (*update)(void);
} SensorMgQmc5883L;