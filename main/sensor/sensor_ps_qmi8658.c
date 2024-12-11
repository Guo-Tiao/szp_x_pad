#include "szp_sensor_def.h"
#include "driver/i2c.h"
#include <inttypes.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "drivers/szp_i2c.h"
#include "common/common_macro.h"

#define QS_GRAVITY_COFF			      (9.807f)                                                  //重力系数
#define QS_ACC_SCALE		        	(8192)                                                  //加速度比例(4g为8192)
#define QS_ACC_COFF		        	(QS_GRAVITY_COFF/QS_ACC_SCALE)                          //加速度系数(QS_GRAVITY_COFF/QS_ACC_COFF)
#define QS_M_PI                                 (3.141592f)                                         //圆周率
#define QS_GYR_SCALE		        	(64)                                                        //陀螺仪比例(512dps为64)
#define QS_GYR_COFF		        	(QS_M_PI/(QS_GYR_SCALE*180))             //陀螺仪系数


//姿态传感器
SensorPsQmi8658 szp_sensor_ps_qmi8658;

//传感器地址
#define  QMI8658_SENSOR_ADDR       0x6A

//传感器寄存器
enum qmi8658a_reg
{
    QMI8658_WHO_AM_I,
    QMI8658_REVISION_ID,
    QMI8658_CTRL1,
    QMI8658_CTRL2,
    QMI8658_CTRL3,
    QMI8658_CTRL4,
    QMI8658_CTRL5,
    QMI8658_CTRL6,
    QMI8658_CTRL7,
    QMI8658_CTRL8,
    QMI8658_CTRL9,
    QMI8658_CATL1_L,
    QMI8658_CATL1_H,
    QMI8658_CATL2_L,
    QMI8658_CATL2_H,
    QMI8658_CATL3_L,
    QMI8658_CATL3_H,
    QMI8658_CATL4_L,
    QMI8658_CATL4_H,
    QMI8658_FIFO_WTM_TH,
    QMI8658_FIFO_CTRL,
    QMI8658_FIFO_SMPL_CNT,
    QMI8658_FIFO_STATUS,
    QMI8658_FIFO_DATA,
    QMI8658_I2CM_STATUS = 44,
    QMI8658_STATUSINT,
    QMI8658_STATUS0,
    QMI8658_STATUS1,
    QMI8658_TIMESTAMP_LOW,
    QMI8658_TIMESTAMP_MID,
    QMI8658_TIMESTAMP_HIGH,
    QMI8658_TEMP_L,
    QMI8658_TEMP_H,
    QMI8658_AX_L,
    QMI8658_AX_H,
    QMI8658_AY_L,
    QMI8658_AY_H,
    QMI8658_AZ_L,
    QMI8658_AZ_H,
    QMI8658_GX_L,
    QMI8658_GX_H,
    QMI8658_GY_L,
    QMI8658_GY_H,
    QMI8658_GZ_L,
    QMI8658_GZ_H,
    QMI8658_MX_L,
    QMI8658_MX_H,
    QMI8658_MY_L,
    QMI8658_MY_H,
    QMI8658_MZ_L,
    QMI8658_MZ_H,
    QMI8658_dQW_L = 73,
    QMI8658_dQW_H,
    QMI8658_dQX_L,
    QMI8658_dQX_H,
    QMI8658_dQY_L,
    QMI8658_dQY_H,
    QMI8658_dQZ_L,
    QMI8658_dQZ_H,
    QMI8658_dVX_L,
    QMI8658_dVX_H,
    QMI8658_dVY_L,
    QMI8658_dVY_H,
    QMI8658_dVZ_L,
    QMI8658_dVZ_H,
    QMI8658_AE_REG1,
    QMI8658_AE_REG2,
    QMI8658_RESET = 96
};

//寄存器读取
esp_err_t qmi8658_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(SZP_I2C_NUM, QMI8658_SENSOR_ADDR,  &reg_addr, 1, data, len, SZP_MS_TO_TICK(SZP_I2C_TIMEOUT_MS));
}
//寄存器写入
esp_err_t qmi8658_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_write_to_device(SZP_I2C_NUM, QMI8658_SENSOR_ADDR, write_buf, sizeof(write_buf), SZP_MS_TO_TICK(SZP_I2C_TIMEOUT_MS));
}




//更新数据
bool sensor_qs_qmi8658_update()
{
    uint8_t status=0;
    int16_t buf[6];

    esp_err_t ret = qmi8658_register_read(QMI8658_STATUS0, &status, 1); // 读状态寄存器
    if(ret!=ESP_OK)
    {
        return false;
    }
    if (status & 0x03) // 判断加速度和陀螺仪数据是否可读
    {
        ret=qmi8658_register_read(QMI8658_AX_L, (uint8_t *)buf, 12); // 读加速度值
        if(ret!=ESP_OK)
        {
            return false;
        }
        szp_sensor_ps_qmi8658.data.acc.x = (float) (buf[0]*QS_ACC_COFF);
        szp_sensor_ps_qmi8658.data.acc.y = (float) (buf[1]*QS_ACC_COFF);
        szp_sensor_ps_qmi8658.data.acc.z = (float) (buf[2]*QS_ACC_COFF);
        szp_sensor_ps_qmi8658.data.gyr.x = (float)(buf[3]*QS_GYR_COFF);
        szp_sensor_ps_qmi8658.data.gyr.y = (float)(buf[4]*QS_GYR_COFF);
        szp_sensor_ps_qmi8658.data.gyr.z = (float)(buf[5]*QS_GYR_COFF);
    }
    else
    {
       return false;
    }
    return true;
}

//初始化
void sensor_qs_qmi8658a_init(void)
{
    uint8_t id = 0;
    while (id != 0x05)
    {
        qmi8658_register_read(QMI8658_WHO_AM_I, &id ,1);
        vTaskDelay(SZP_MS_TO_TICK(10));
    }
    qmi8658_register_write_byte(QMI8658_RESET, 0xb0);  // 复位
    vTaskDelay(SZP_MS_TO_TICK(10));
    qmi8658_register_write_byte(QMI8658_CTRL1, 0x40); // CTRL1 设置地址自动增加
    qmi8658_register_write_byte(QMI8658_CTRL7, 0x03); // CTRL7 允许加速度和陀螺仪
    qmi8658_register_write_byte(QMI8658_CTRL2, 0x95); // CTRL2 设置ACC 4g 250Hz
    qmi8658_register_write_byte(QMI8658_CTRL3, 0xd5); // CTRL3 设置GRY 512dps 250Hz

    szp_sensor_ps_qmi8658.update = &sensor_qs_qmi8658_update;
}