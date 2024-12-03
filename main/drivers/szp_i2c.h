#pragma once

#include "esp_err.h"

#define SZP_I2C_SDA_IO           GPIO_NUM_1      /*!< GPIO number used for I2C master data  */
#define SZP_I2C_SCL_IO           GPIO_NUM_2      /*!< GPIO number used for I2C master clock */
#define SZP_I2C_NUM              0               /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define SZP_I2C_FREQ_HZ          100000          /*!< I2C master clock frequency */
#define SZP_I2C_TX_BUF_DISABLE   0               /*!< I2C master doesn't need buffer */
#define SZP_I2C_RX_BUF_DISABLE   0               /*!< I2C master doesn't need buffer */
#define SZP_I2C_TIMEOUT_MS       1000

//实战派I2C初始化
esp_err_t  szp_i2c_init(void);

