#include "szp_ioext.h"
#include "driver/i2c.h"
#include "drivers/szp_i2c.h"
#include "common/common_macro.h"
/***************  IO扩展芯片PCA9557 ***************/
#define PCA9557_INPUT_PORT                             0x00
#define PCA9557_OUTPUT_PORT                          0x01
#define PCA9557_POLARITY_INVERSION_PORT   0x02
#define PCA9557_CONFIGURATION_PORT           0x03

#define PCA9557_LCD_CS_GPIO                 BIT(0)    // PCA9557_GPIO_NUM_1
#define PCA9557_PA_EN_GPIO                  BIT(1)    // PCA9557_GPIO_NUM_2
#define PCA9557_DVP_PWDN_GPIO               BIT(2)    // PCA9557_GPIO_NUM_3

#define PCA9557_SENSOR_ADDR             0x19        /*!< Slave address of the MPU9250 sensor */

#define PCA9557_SET_BITS(_m, _s, _v)  ((_v) ? (_m)|((_s)) : (_m)&~((_s)))
/*************************************/


// 读取PCA9557寄存器的值
esp_err_t pca9557_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(SZP_I2C_NUM, PCA9557_SENSOR_ADDR,  &reg_addr, 1, data, len, SZP_TICK_TO_MS(1000));
}

// 给PCA9557的寄存器写值
esp_err_t pca9557_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};

    return i2c_master_write_to_device(SZP_I2C_NUM, PCA9557_SENSOR_ADDR, write_buf, sizeof(write_buf), SZP_TICK_TO_MS(1000));
}



void szp_ioext_init(void)
{
    // 写入控制引脚默认值 DVP_PWDN=1  PA_EN = 0  LCD_CS = 1
    pca9557_register_write_byte(PCA9557_OUTPUT_PORT, 0x05);  
    // 把PCA9557芯片的IO1 IO1 IO2设置为输出 其它引脚保持默认的输入
    pca9557_register_write_byte(PCA9557_CONFIGURATION_PORT, 0xf8); 
}



// 设置PCA9557芯片的某个IO引脚输出高低电平
esp_err_t szp_ioext_set_output(uint8_t gpio_bit, uint8_t level)
{
    uint8_t data;
    esp_err_t res = ESP_FAIL;

    pca9557_register_read(PCA9557_OUTPUT_PORT, &data, 1);
    res = pca9557_register_write_byte(PCA9557_OUTPUT_PORT, PCA9557_SET_BITS(data, gpio_bit, level));

    return res;
}

// 控制 PCA9557_LCD_CS 引脚输出高低电平 参数0输出低电平 参数1输出高电平 
void szp_ioext_lcd_cs(uint8_t level)
{
    szp_ioext_set_output(PCA9557_LCD_CS_GPIO, level);
}

// 控制 PCA9557_PA_EN 引脚输出高低电平 参数0输出低电平 参数1输出高电平 
void szp_ioext_pa_en(uint8_t level)
{
    szp_ioext_set_output(PCA9557_PA_EN_GPIO, level);
}

// 控制 PCA9557_DVP_PWDN 引脚输出高低电平 参数0输出低电平 参数1输出高电平 
void szp_ioext_dvp_pwdn(uint8_t level)
{
    szp_ioext_set_output(PCA9557_DVP_PWDN_GPIO, level);
}