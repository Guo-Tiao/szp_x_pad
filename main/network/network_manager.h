#pragma once
#include "esp_err.h"
#include "sdkconfig.h"

//NVS配置
#define Nvs_NameSpace_Network     "network"                 //网络命名空间
#define Nvs_Key_Wifi_Ssid                   "wifi_ssid"                 //wifi名称
#define Nvs_Key_Wifi_Password          "wifi_password"       //wifi密码



//网络初始化
void network_init(void);

/*************** wifi操作 ***************/
//wifi连接设置
void network_wifi_set_config(const char* ssid,const char* password);
//wifi设置ssid
void network_wifi_set_ssid(const char* ssid);
//wifi设置密码
void network_wifi_set_password(const char* password);
//wifi获取ssid
size_t network_wifi_get_ssid(char* value,int maxlen);

//wifi网络连接
esp_err_t network_wifi_connect();
//wifi网络断开
esp_err_t network_wifi_disconnect();

//实战派WIFI连接事件
typedef enum
{
    EV_SZP_WIFI_CONNECT_SUCCESS=0x01,    // wifi连接成功
    EV_SZP_WIFI_CONNECT_FAIL=0x02,            // wifi连接失败
    EV_SZP_WIFI_RECONNECTING=0x04,          //wifi重连中
} SzpWifiStateEvent;

//实战派WIFI事件等待
uint32_t network_wait_wifi_event(SzpWifiStateEvent  evnet,uint32_t waitTimeMs);

//获取当前wifi时间状态
SzpWifiStateEvent network_wifi_current_state();


/*************** mqtt操作 ***************/

#if CONFIG_USE_SZP_MQTT
//开启MQTT连接任务
void network_start_mqtt_task();
#endif