#include "szp_sensor_def.h"
#include "szp_i2c.h"
#include "driver/i2c.h"
#include <inttypes.h>
#include <math.h>
#include "szp_sensor_manager.h"

//温湿度传感器
SensorThGxhtc3 szp_sensor_th_gxhtc3;


#define GXHTC3_I2C_ADDR 0x70

#define POLYNOMIAL  0x31 // P(x) = x^8 + x^5 + x^4 + 1 = 00110001

//CRC校验
uint8_t gxhtc3_calc_crc(uint8_t *crcdata, uint8_t len)
{
    uint8_t crc = 0xFF; 
  
    for(uint8_t i = 0; i < len; i++)
    {
        crc ^= (crcdata[i]);
        for(uint8_t j = 8; j > 0; --j)
        {
            if(crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
            else           crc = (crc << 1);
        }
    }
    return crc;
}

//获取ID
esp_err_t sensor_th_gxhtc3_read_id()
{
    esp_err_t ret;
    uint8_t data[3];

    //查询ID命令
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, GXHTC3_I2C_ADDR << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0xEF, true);
    i2c_master_write_byte(cmd, 0xC8, true);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(SZP_I2C_NUM, cmd, pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) 
    {
        i2c_cmd_link_delete(cmd);
        return ret;
    }
    //获取查询结果
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, GXHTC3_I2C_ADDR << 1 | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 3, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(SZP_I2C_NUM, cmd, pdMS_TO_TICKS(1000));

    if(data[2]!=gxhtc3_calc_crc(data,2))
    {     
        ret = ESP_FAIL;
    }
    else
    {
        szp_sensor_th_gxhtc3.id = data[0] << 8 | data[1];
    }
    i2c_cmd_link_delete(cmd);
    return ret;
}

// 休眠
esp_err_t gxhtc3_sleep(void)
{
    int ret;
    //休眠命令
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, GXHTC3_I2C_ADDR << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0xB0, true);
    i2c_master_write_byte(cmd, 0x98, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(SZP_I2C_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

//唤醒
esp_err_t gxhtc3_wake_up(void)
{
    int ret;
    //唤醒命令
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, GXHTC3_I2C_ADDR << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x35, true);
    i2c_master_write_byte(cmd, 0x17, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(SZP_I2C_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

// 测量
esp_err_t gxhtc3_measure(void)
{
    int ret;
    //测量命令
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x70 << 1 | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x7c, true);
    i2c_master_write_byte(cmd, 0xa2, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(SZP_I2C_NUM, cmd,  pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

uint8_t gxhtc3_tah_data[6];//读取的数据
//读取数据
esp_err_t gxhtc3_read_tah(void)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x70 << 1 | I2C_MASTER_READ, true);
    i2c_master_read(cmd, gxhtc3_tah_data, 6, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(SZP_I2C_NUM, cmd,pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}
//更新数据
esp_err_t sensor_th_gxhtc3_update()
{
    esp_err_t ret;
    gxhtc3_wake_up();
    gxhtc3_measure();
    vTaskDelay(pdMS_TO_TICKS(20));
    gxhtc3_read_tah();
    gxhtc3_sleep();

      if((gxhtc3_tah_data[2]!=gxhtc3_calc_crc(gxhtc3_tah_data,2)
          ||(gxhtc3_tah_data[5]!=gxhtc3_calc_crc(&gxhtc3_tah_data[3],2))))
      {    
        szp_sensor_th_gxhtc3.fTemp = 0;
        szp_sensor_th_gxhtc3.fHumi = 0;
        szp_sensor_th_gxhtc3.iTemp = 0;
        szp_sensor_th_gxhtc3.iHumi = 0;
        ret = ESP_FAIL;
    }
    else
    {
        uint16_t rawValueTemp = (gxhtc3_tah_data[0]<<8) | gxhtc3_tah_data[1];
        uint16_t rawValueHumi = (gxhtc3_tah_data[3]<<8) | gxhtc3_tah_data[4];
        szp_sensor_th_gxhtc3.fTemp = (175.0 * (float)rawValueTemp) / 65535.0 - 45.0; 
        szp_sensor_th_gxhtc3.fHumi = (100.0 * (float)rawValueHumi) / 65535.0;
        szp_sensor_th_gxhtc3.iTemp  = round(szp_sensor_th_gxhtc3.fTemp );
        szp_sensor_th_gxhtc3.iHumi  = round(szp_sensor_th_gxhtc3.fHumi );
        ret = ESP_OK;
    }
    return ret;
}


void sensor_th_gxhtc3_init(void)
{
    szp_sensor_th_gxhtc3.id = 0;
    szp_sensor_th_gxhtc3.read_id = &sensor_th_gxhtc3_read_id;
    szp_sensor_th_gxhtc3.update = &sensor_th_gxhtc3_update;
}