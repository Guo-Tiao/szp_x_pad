#include "work_controller.h"
#include "network/network_manager.h"
#include "storage/storage_manager.h"
#include "bluetooth/szp_ble_gatts.h"
#include "ui/ui_manager.h"
#include "szp_config_def.h"

#include "string.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SZP_WORK_TAG               "SZP_WORK"     //日志使用的TAG

/******************************************* BLE-GATTS *******************************************/

#define SZP_GATTS_CHAR_ID_WIFI_SSID                  0xFF71             //wifi-ssid可读写
#define SZP_GATTS_DESCR_ID_WIFI_SSID                0x2771            //描述

#define SZP_GATTS_CHAR_ID_WIFI_PASSWORD      0xFF72             //wifi-密码只写
#define SZP_GATTS_DESCR_ID_WIFI_PASSWORD    0x2772             //描述

#define SZP_GATTS_CHAR_ID_WIFI_STATE                0xFF73             //wifi-连接状态 可读写(1连接/2断开/4重连中)
#define SZP_GATTS_DESCR_ID_WIFI_STATE              0x2773             //描述

#define SZP_GATTS_CHAR_ID_WIFI_AUTO_CONNECT                0xFF74             //wifi-自动连接 可读写(1自动连接/0不自动连接)
#define SZP_GATTS_DESCR_ID_WIFI_AUTO_CONNECT              0x2774             //描述

#define SZP_GATTS_CHAR_ID_WEATHER_API_KEY      0xFF81             //天气API-key
#define SZP_GATTS_DESCR_ID_WEATHER_API_KEY    0x2781             //描述

#define SZP_GATTS_CHAR_ID_WEATHER_API_CITY      0xFF82             //天气城市代码
#define SZP_GATTS_DESCR_ID_WEATHER_API_CITY    0x2782             //描述

//GATTS事件回调
static void szp_work_ble_gatts_event_callback(SzpBleGattsEvent e)
{
    //UI刷新
    szp_ui_update_ble_gatts_evnet(e);
}
//GATTS读char回调
static size_t szp_work_ble_gatts_char_read_callback(uint16_t uuid, char *value, int maxlen)
{
    switch (uuid)
    {
    case SZP_GATTS_CHAR_ID_WIFI_SSID:
        return network_wifi_get_ssid(value, maxlen);
    case SZP_GATTS_CHAR_ID_WIFI_STATE:
        {
            SzpWifiStateEvent state = network_wifi_current_state();
            char buf[10];
            sprintf(buf, "%d", state);
            size_t len = strlen(buf);
            memcpy(value, buf, len);
            return len;
        }
    case SZP_GATTS_CHAR_ID_WIFI_AUTO_CONNECT:
        {
            uint8_t wifi_auto_connect = 0;
            szp_nvs_read_blob(Nvs_NameSpace_App, Nvs_Key_Wifi_AutoConnect, &wifi_auto_connect, 1);
            char buf[10];
            sprintf(buf, "%d", wifi_auto_connect);
            size_t len = strlen(buf);
            memcpy(value, buf, len);
            return len;
        }
    case SZP_GATTS_CHAR_ID_WEATHER_API_CITY:
        return szp_nvs_read_str(Nvs_NameSpace_Network, Nvs_Key_Weather_Api_City, value,maxlen);
    default:
        break;
    }
    return 0;
}
//GATTS写char回调
static void szp_work_ble_gatts_char_write_callback(uint16_t uuid, const char *value)
{
    switch (uuid)
    {
    case SZP_GATTS_CHAR_ID_WIFI_SSID:
        {
            network_wifi_set_ssid(value);
        }
        break;
    case SZP_GATTS_CHAR_ID_WIFI_PASSWORD:
        {
            network_wifi_set_password(value);
        }
        break;
    case SZP_GATTS_CHAR_ID_WIFI_STATE:
        {
            char buf[10];
            size_t len = strlen(value);
            memcpy(buf,value,len);
            int state = atoi(buf);
            if(state==EV_SZP_WIFI_CONNECT_SUCCESS)
            {
                network_wifi_connect();
            }
            else if(state==EV_SZP_WIFI_CONNECT_FAIL)
            {
                network_wifi_disconnect();
            }
        }
        break;
    case SZP_GATTS_CHAR_ID_WIFI_AUTO_CONNECT:
        {
            char buf[10];
            size_t len = strlen(value);
            memcpy(buf,value,len);
            int state = atoi(buf);
            if(state==0)
            {
                uint8_t val = 0;
                szp_nvs_write_blob(Nvs_NameSpace_App, Nvs_Key_Wifi_AutoConnect, &val, 1);
            }
            else if(state==1)
            {
                uint8_t val = 1;
                szp_nvs_write_blob(Nvs_NameSpace_App, Nvs_Key_Wifi_AutoConnect, &val, 1);
            }
        }
        break;
    case SZP_GATTS_CHAR_ID_WEATHER_API_KEY:
        {
            szp_nvs_write_str(Nvs_NameSpace_Network, Nvs_Key_Weather_Api_Key, value);
        }
        break;
    case SZP_GATTS_CHAR_ID_WEATHER_API_CITY:
        {
            szp_nvs_write_str(Nvs_NameSpace_Network, Nvs_Key_Weather_Api_City, value);
        }
        break;
    default:
            break;
    }
}
//GATTS开启(后续建议改为Protocol Buffer,减少charID)
static void szp_work_ble_gatts_start()
{
    //先注册char和设置回调
    szp_gatts_char_descr char_wifi_ssid =
        {
            .char_uuid = SZP_GATTS_CHAR_ID_WIFI_SSID,
            .char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            .char_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,

            .add_descr =true,
            .descr_uuid = SZP_GATTS_DESCR_ID_WIFI_SSID,
            .descr_value= "wifi ssid",
            
        };
    szp_ble_gatts_add_char(char_wifi_ssid);
    

    szp_gatts_char_descr char_wifi_password =
        {
            .char_uuid = SZP_GATTS_CHAR_ID_WIFI_PASSWORD,
            .char_perm =   ESP_GATT_PERM_WRITE,
            .char_property = ESP_GATT_CHAR_PROP_BIT_WRITE ,

            .add_descr =true,
            .descr_uuid = SZP_GATTS_DESCR_ID_WIFI_PASSWORD,
            .descr_value= "wifi密码",
            
        };
    szp_ble_gatts_add_char(char_wifi_password);

    szp_gatts_char_descr char_wifi_state =
        {
            .char_uuid = SZP_GATTS_CHAR_ID_WIFI_STATE,
            .char_perm =   ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
            .char_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,

            .add_descr =true,
            .descr_uuid = SZP_GATTS_DESCR_ID_WIFI_STATE,
            .descr_value="wifi状态",
            
        };
    szp_ble_gatts_add_char(char_wifi_state);

    szp_gatts_char_descr char_wifi_auto_connect =
        {
            .char_uuid = SZP_GATTS_CHAR_ID_WIFI_AUTO_CONNECT,
            .char_perm =   ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
            .char_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,

            .add_descr =true,
            .descr_uuid = SZP_GATTS_DESCR_ID_WIFI_AUTO_CONNECT,
            .descr_value="wifi自动连接",
            
        };
    szp_ble_gatts_add_char(char_wifi_auto_connect);

    szp_gatts_char_descr char_weather_api_key =
        {
            .char_uuid = SZP_GATTS_CHAR_ID_WEATHER_API_KEY,
            .char_perm = ESP_GATT_PERM_WRITE,
            .char_property = ESP_GATT_CHAR_PROP_BIT_WRITE,

            .add_descr = true,
            .descr_uuid = SZP_GATTS_DESCR_ID_WEATHER_API_KEY,
            .descr_value = "天气API-Key",

        };
    szp_ble_gatts_add_char(char_weather_api_key);

    szp_gatts_char_descr char_weather_api_city =
        {
            .char_uuid = SZP_GATTS_CHAR_ID_WEATHER_API_CITY,
            .char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            .char_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,

            .add_descr = true,
            .descr_uuid = SZP_GATTS_DESCR_ID_WEATHER_API_CITY,
            .descr_value = "天气API-城市代码",

        };
    szp_ble_gatts_add_char(char_weather_api_city);

    //注册读写回调
    szp_ble_gatts_char_cb_t cb_t=
        {
            .char_read = szp_work_ble_gatts_char_read_callback,
            .char_write = szp_work_ble_gatts_char_write_callback,
        };
    szp_ble_gatts_register_char_cb(cb_t);
    //事件注册回调
    szp_ble_gatts_register_event_cb(szp_work_ble_gatts_event_callback);
    //开启蓝牙检查
    szp_ble_gatts_set_check_enable(true);
    //开启蓝牙服务
    szp_ble_gatts_start();
}
//GATTS关闭
static void szp_work_ble_gatts_stop()
{
    //关闭蓝牙
    szp_ble_gatts_stop();
    //注销回调
    szp_ble_gatts_char_cb_t cb_t=
    {
            .char_read = NULL,
            .char_write = NULL,
    };
    szp_ble_gatts_register_char_cb(cb_t);
    szp_ble_gatts_register_event_cb(NULL);
    //清除char
    szp_ble_gatts_del_all_char();
}

bool szp_work_ble_start(void)
{
    if(szp_ble_gatts_get_current_event()!=EV_SZP_BLE_GATTS_STOP)
    {
        return false;
    }
    szp_work_ble_gatts_start();
    return true;
}

bool szp_work_ble_stop(void)
{
    if(szp_ble_gatts_get_current_event()!=EV_SZP_BLE_GATTS_START)
    {
        return false;
    }
    szp_work_ble_gatts_stop();
    return true;
}

/******************************************* BLE-GATTS *******************************************/

/******************************************* Networkl *******************************************/
//GATTS事件回调
static void szp_work_network_wifi_event_callback(SzpWifiStateEvent e)
{
    //UI刷新
    szp_ui_update_network_wifi_evnet(e);
}
//网络服务开启
static void szp_work_network_start()
{
    //注册回调
    network_wifi_register_event_cb(szp_work_network_wifi_event_callback);
    //查找是否自动连接wifi
    uint8_t wifi_auto_connect = 0;
    size_t read_len= szp_nvs_read_blob(Nvs_NameSpace_App, Nvs_Key_Wifi_AutoConnect, &wifi_auto_connect, 1);
    if(read_len==0) szp_nvs_write_blob(Nvs_NameSpace_App, Nvs_Key_Wifi_AutoConnect, &wifi_auto_connect, 1);
    if(wifi_auto_connect)   network_wifi_connect();
    //启动MQTT
#if CONFIG_USE_SZP_MQTT
    if(wifi_auto_connect)
    {
        network_start_mqtt_task();
    }
#endif
    //开启SNTP授时
   network_start_sntp_task();

   //开启天气获取定时任务
   network_start_weather_timer_task();
}
/******************************************* Networkl *******************************************/


void szp_work_init()
{
#if CONFIG_ENABLE_SZP_IOT
    //开启蓝牙服务
    //初始化不打开蓝牙,需要设置参数再打开,后续使用UI开关 szp_work_ble_gatts_start();
    //开启网络服务
    szp_work_network_start();
#endif

}
