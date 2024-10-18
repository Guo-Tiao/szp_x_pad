#pragma once
#include "esp_err.h"
#include "stdbool.h"
//实战派WIFI连接事件
typedef enum
{
    SZP_WIFI_CONNECTED,    // wifi连接成功
    SZP_WIFI_DISCONNECTED, // wifi连接断开
    SZP_WIFI_RECONNECTING, //wifi重连中
} SzpWifiConnectEvent;
//实战派wifi连接事件回调
typedef void (*szp_wifi_connect_cb)(SzpWifiConnectEvent);
//实战派wifi连接结构体
typedef struct 
{
    const char* ssid; //wifi名
    const char* password;//wifi密码
    szp_wifi_connect_cb wifi_connect_cb;    //wifi回调
    uint8_t retry_count;    //重试次数(超过次数则触发断开事件)
} SzpWifiConnectConfig;

//wifi初始化
esp_err_t szp_wifi_init(void);
//wifi连接
esp_err_t szp_wifi_connect(SzpWifiConnectConfig config);
//wifi断开
esp_err_t szp_wifi_disconnect(void);
//wifi反初始化
esp_err_t szp_wifi_uninit(void);