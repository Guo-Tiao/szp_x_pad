#include "stdio.h"

extern void _1_print_chip_info(void);//打印芯片信息
extern void _2_demo_gpio_run(void);//运行GPIO-demo
extern void _3_demo_uart_run(void);//运行串口-demo
extern void _4_demo_ble_run(void);//运行蓝牙-demo
extern void _5_demo_timer_run(void);//运行定时器-demo
extern void _6_demo_sntp_run(void);//运行STNP-demo
extern void _7_demo_imu_run(void);//运行IMU-demo

void _demo_run_(void)
{
    printf("_demo_run_\r\n");
    _7_demo_imu_run();
}