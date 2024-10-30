#include "szp_sensor_def.h"
#include "driver/i2c.h"
#include <inttypes.h>
#include <math.h>
#include "szp_sensor_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "drivers/szp_i2c.h"
#include "common/common_macro.h"

//地磁传感器
SensorMgQmc5883L szp_sensor_mg_qmc5883l;

//传感器地址
#define  QMC5883L_SENSOR_ADDR    0x0D

enum qmc5883l_reg
{
    QMC5883L_XOUT_L,
    QMC5883L_XOUT_H,
    QMC5883L_YOUT_L,
    QMC5883L_YOUT_H,
    QMC5883L_ZOUT_L,
    QMC5883L_ZOUT_H,
    QMC5883L_STATUS,
    QMC5883L_TOUT_L,
    QMC5883L_TOUT_H,
    QMC5883L_CTRL1,
    QMC5883L_CTRL2,
    QMC5883L_FBR,
    QMC5883L_CHIPID = 13
};


esp_err_t qmc5883L_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(SZP_I2C_NUM, QMC5883L_SENSOR_ADDR,  &reg_addr, 1, data, len, SZP_MS_TO_TICK(SZP_I2C_TIMEOUT_MS));
}

esp_err_t qmc5883L_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_write_to_device(SZP_I2C_NUM, QMC5883L_SENSOR_ADDR, write_buf, sizeof(write_buf), SZP_MS_TO_TICK(SZP_I2C_TIMEOUT_MS));
}



bool sensor_mg_qmc5833l_update(void)
{
    uint8_t status=0;
    int16_t mag_reg[3];

    esp_err_t ret= qmc5883L_register_read(QMC5883L_STATUS, &status, 1); // 读状态寄存器 
    if(ret!=ESP_OK)
    {
        return false;
    }
    if (status & 0x01)
    {
        ret=qmc5883L_register_read(QMC5883L_XOUT_L, (uint8_t *)mag_reg, 6);
        if(ret!=ESP_OK)
        {
            return false;
        }
        szp_sensor_mg_qmc5883l.mag_x = mag_reg[0];
        szp_sensor_mg_qmc5883l.mag_y = mag_reg[1];
        szp_sensor_mg_qmc5883l.mag_z = mag_reg[2];
        szp_sensor_mg_qmc5883l.azimuth = (float)atan2(szp_sensor_mg_qmc5883l.mag_y, szp_sensor_mg_qmc5883l.mag_x) * 180.0 / 3.1415926 + 180.0;
    }
    else
    {
        return false;
    }
    return true;
}

void sensor_th_qmc5883l_init(void)
{
    uint8_t id = 0;

    while (id != 0xff)  // 确定ID号是否正确
    {
        qmc5883L_register_read(QMC5883L_CHIPID, &id ,1);
        vTaskDelay(SZP_MS_TO_TICK(500));
    }
    qmc5883L_register_write_byte(QMC5883L_CTRL2, 0x80); // 复位芯片 
    vTaskDelay(SZP_MS_TO_TICK(10));
    qmc5883L_register_write_byte(QMC5883L_CTRL1, 0x05); //Continuous模式 50Hz 
    qmc5883L_register_write_byte(QMC5883L_CTRL2, 0x00); 
    qmc5883L_register_write_byte(QMC5883L_FBR, 0x01); 
    
    szp_sensor_mg_qmc5883l.update = &sensor_mg_qmc5833l_update;
}