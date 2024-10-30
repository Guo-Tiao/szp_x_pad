#include "szp_uart.h"
#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "string.h"
#include "common/common_macro.h"

#if  CONFIG_SZP_EXP_ITF_FUNC_UART
#include "driver/uart.h"
#include "driver/gpio.h"

//扩展串口引脚配置
#if CONFIG_SZP_EXP_ITF_UART_RX_TX_1
#define SZP_UART_RX GPIO_NUM_18
#define SZP_UART_TX GPIO_NUM_19
#elif CONFIG_SZP_EXP_ITF_UART_RX_TX_2
#define SZP_UART_RX GPIO_NUM_19
#define SZP_UART_TX GPIO_NUM_18
#endif
//扩展串口波特率
#define SZP_UART_BAUD CONFIG_SZP_EXP_ITF_UART_BAUD
//扩展串口缓存
#define SZP_UART_RX_BUFFER_SIZE CONFIG_SZP_EXP_ITF_UART_RX_BUFFER
#define SZP_UART_TX_BUFFER_SIZE  CONFIG_SZP_EXP_ITF_UART_TX_BUFFER
//扩展串口号(串口0是调试串口,已被占用)
#define SZP_UART_NUM  UART_NUM_1

esp_err_t szp_uart_init(void)
{
    //配置串口
    const uart_config_t uart_config = {
        .baud_rate = SZP_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    //设置缓存和使能配置
    SZP_ESP_ERR_CHECK(uart_driver_install(SZP_UART_NUM, SZP_UART_RX_BUFFER_SIZE * 2, SZP_UART_TX_BUFFER_SIZE * 2, 0, NULL, 0));
    SZP_ESP_ERR_CHECK(uart_param_config(SZP_UART_NUM, &uart_config));
    SZP_ESP_ERR_CHECK(uart_set_pin(SZP_UART_NUM, SZP_UART_TX, SZP_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    return ESP_OK;
}

int szp_uart_send_data(const char *data)
{
    const int len= strlen(data);
    return uart_write_bytes(SZP_UART_NUM, data, len);
}

int szp_uart_receive_data(char *data, uint32_t len,uint16_t waitMs)
{
    if(NULL==data)
    {
        return 0;
    }
    int bytes= uart_read_bytes(SZP_UART_NUM, data, len, SZP_MS_TO_TICK(waitMs));
     if (bytes > 0)
    {
        data[bytes] = 0;
    }
    return bytes;
}

#endif