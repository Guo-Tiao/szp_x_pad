#pragma once

#include "stdbool.h"

/*
业务控制器
功能:
1.管理蓝牙开关,配置参数和执行命令
2.开启wifi和网络服务等
*/

//工作控制器初始化
void szp_work_init();


//开启蓝牙
bool szp_work_ble_start(void);
//关闭蓝牙
bool szp_work_ble_stop(void);