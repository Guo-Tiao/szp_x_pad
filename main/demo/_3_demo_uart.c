#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"

#include "freertos/queue.h"
#include "common_macro.h"

// for uart_1
static const int RX_BUF_SIZE = 1024;

#define TXD_PIN (GPIO_NUM_18) // to  B7
#define RXD_PIN (GPIO_NUM_19) //   B6


int sendData(const char *data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    return txBytes;
}

static void tx_task(void *arg)
{
    while (1)
    {
        sendData( "led on"); 
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        sendData("led off"); 
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


static void rx_task(void *arg)
{
    uint8_t *data = (uint8_t *)malloc(RX_BUF_SIZE + 1); // 申请一块内存空间 与free()成对出现

    while (1)
    {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, SZP_MS_TO_TICK(1000)); // important 重要函数 接收
        if (rxBytes > 0)
        {
            data[rxBytes] = 0;
            printf("Rx:%s\r\n",data);
        }
    }
    free(data);
}


void _3_demo_uart_run(void)
{

     const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);


    xTaskCreate(rx_task, "uart_rx_task", 1024 * 5, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024 * 5, NULL, configMAX_PRIORITIES - 1, NULL);
}