#pragma once
#include "esp_err.h"
#include "stdbool.h"
#include "sdkconfig.h"

//NVS配置
#define Nvs_NameSpace_Network     "network"                 //网络命名空间
#define Nvs_Key_Wifi_Ssid                   "wifi_ssid"                 //wifi名称
#define Nvs_Key_Wifi_Password          "wifi_password"       //wifi密码

#define Nvs_Key_Weather_Api_Key     "weather_key"          //天气API密钥
#define Nvs_Key_Weather_Api_City     "weather_city"          //天气API城市


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
typedef void (*network_wifi_event_cb)(SzpWifiStateEvent e);
//实战派WIFI事件等待
uint32_t network_wait_wifi_event(SzpWifiStateEvent  evnet,uint32_t waitTimeMs);

//获取当前wifi时间状态
SzpWifiStateEvent network_wifi_current_state();
//wifi事件回调
void network_wifi_register_event_cb(network_wifi_event_cb event_cb);
//等待wifi连接成功
bool network_wait_wifi_connect_success(uint32_t waitMs);

/*************** wifi操作 ***************/



/*************** mqtt操作 ***************/

#if CONFIG_USE_SZP_MQTT
//开启MQTT连接任务
void network_start_mqtt_task();
#endif
/*************** mqtt操作 ***************/


/*************** web服务操作 ***************/
//开启SNTP授时任务(一次性任务)
void network_start_sntp_task();
//开启天气更新任务(定时十分钟)
bool network_start_weather_timer_task();
//关闭天气更新任务
bool network_stop_weather_timer_task();
/*************** web服务操作 ***************/