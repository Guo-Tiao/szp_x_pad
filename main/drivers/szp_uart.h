#pragma once
#include "esp_err.h"
#if   CONFIG_SZP_EXP_ITF_FUNC_UART

//实战派串口初始化
esp_err_t szp_uart_init(void);
//发送数据
int szp_uart_send_data(const char* data);
//获取数据
int szp_uart_receive_data(char* data,uint32_t len,uint16_t waitMs);
#endif