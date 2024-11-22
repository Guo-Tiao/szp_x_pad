#pragma once
#include <stdint.h>
/*
公共数据定义
*/
//浮点坐标系3D坐标
typedef struct coord_3d_f_t
{
    float x;
    float y;
    float z;
}coord_3d_f;

//整数坐标系3D坐标
typedef struct coord_3d_i16_t
{
    int16_t x;
    int16_t y;
    int16_t z;
}coord_3d_i16;

//IMU数据
typedef struct imu_data_t
{
    coord_3d_f acc;//加速度
    coord_3d_f gyr;//角速度
} imu_data;

//欧拉角
typedef struct euler_angles_t
{
    float roll; //滚动角
    float pitch;//俯仰角
    float yaw ;//偏航角
} euler_angles;