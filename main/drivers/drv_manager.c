#include "drv_manager.h"
#include "szp_key.h"
#include "szp_i2c.h"


#if   CONFIG_SZP_EXP_ITF_FUNC_UART
#include "szp_uart.h"
#endif

void drv_init(void)
{
    //初始化按键
    szp_key_init();
    //初始化I2C
    szp_i2c_init();

#if   CONFIG_SZP_EXP_ITF_FUNC_UART
     //初始化串口
    szp_uart_init();
#endif

}