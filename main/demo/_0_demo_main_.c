#include "stdio.h"

extern void _1_print_chip_info(void);//打印芯片信息
extern void _2_demo_gpio_run(void);//运行GPIO-demo
extern void _3_demo_uart_run(void);//运行串口-demo
extern void _4_demo_ui_run(void);//运行ui-demo
extern void _5_demo_cam_run(void);//运行cam-demo


void _demo_run_(void)
{
    printf("_demo_run_\r\n");
    _5_demo_cam_run();
}