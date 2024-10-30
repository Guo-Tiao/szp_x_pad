#include "szp_i2c.h"
#include "driver/i2c.h"
#include "common/common_macro.h"

esp_err_t szp_i2c_init(void)
{
    int i2c_master_port = SZP_I2C_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SZP_I2C_SDA_IO,
        .scl_io_num = SZP_I2C_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = SZP_I2C_FREQ_HZ,
    };

    SZP_ESP_ERR_CHECK(i2c_param_config(i2c_master_port, &conf));
    SZP_ESP_ERR_CHECK(i2c_driver_install(i2c_master_port, conf.mode, SZP_I2C_RX_BUF_DISABLE, SZP_I2C_TX_BUF_DISABLE, 0));

    return ESP_OK;
}