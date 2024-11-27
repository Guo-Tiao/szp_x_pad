#include "network_manager.h"
#include "szp_wifi.h"
#include "storage/storage_manager.h"

#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"

#include "common/common_macro.h"

#include "esp_sntp.h"
#include "esp_netif_sntp.h"
#include "esp_log.h"

#define SZP_NETWORK_TAG         "SZP_NETWORK"
//WIFI
#define DEFAULT_SZP_WIFI_SSID                       CONFIG_SZP_WIFI_SSID
#define DEFAULT_SZP_WIFI_PASSWORD           CONFIG_SZP_WIFI_PASSWORD
#define DEFAULT_SZP_WIFI_RETRY_COUNT      CONFIG_SZP_WIFI_RETRY_COUNT
//SNTP
#define SZP_SNTP_SERVER_URI                         CONFIG_SZP_SNTP_SERVER_URI


//实战派wifi事件组
EventGroupHandle_t event_group_szp_wifi;
//当前wifi状态
SzpWifiStateEvent network_current_wifi_state;
//wifi事件回调
network_wifi_event_cb network_wifi_event_callback;
//SNTP授时回调
sntp_complete_cb sntp_complete_callback;

void network_init(void)
{
    //wifi初始化
    esp_err_t ret= szp_wifi_init();
    if(ret==ESP_OK)
    {
        //创建wifi事件组
        event_group_szp_wifi = xEventGroupCreate();
    }
    network_current_wifi_state = EV_SZP_WIFI_CONNECT_FAIL;

    //时间设置:设置为东八区
    setenv("TZ", "CST-8", 1);
    tzset();
}

/*************** wifi操作 ***************/
static void cb_szp_network_wifi_event(SzpWifiConnectEvent e)
{
    if(event_group_szp_wifi)
    {
        switch (e)
        {
        case SZP_WIFI_CONNECTED:
            network_current_wifi_state = EV_SZP_WIFI_CONNECT_SUCCESS;
            xEventGroupSetBits(event_group_szp_wifi, EV_SZP_WIFI_CONNECT_SUCCESS);
            break;
        case SZP_WIFI_DISCONNECTED:
            network_current_wifi_state = EV_SZP_WIFI_CONNECT_FAIL;
            xEventGroupSetBits(event_group_szp_wifi, EV_SZP_WIFI_CONNECT_FAIL);
            break;
        case SZP_WIFI_RECONNECTING:
            network_current_wifi_state = EV_SZP_WIFI_RECONNECTING;
            xEventGroupSetBits(event_group_szp_wifi, EV_SZP_WIFI_RECONNECTING);
            break;
        }
        if(network_wifi_event_callback)
        {
            network_wifi_event_callback(network_current_wifi_state);
        }
    }
}

void network_wifi_set_config(const char *ssid, const char *password)
{
    szp_nvs_write_str(Nvs_NameSpace_Network,Nvs_Key_Wifi_Ssid,ssid);
    szp_nvs_write_str(Nvs_NameSpace_Network,Nvs_Key_Wifi_Password,password);
}

void network_wifi_set_ssid(const char *ssid)
{
    szp_nvs_write_str(Nvs_NameSpace_Network, Nvs_Key_Wifi_Ssid, ssid);
}

void network_wifi_set_password(const char *password)
{
    szp_nvs_write_str(Nvs_NameSpace_Network, Nvs_Key_Wifi_Password, password);
}

size_t network_wifi_get_ssid(char *value, int maxlen)
{
    return szp_nvs_read_str(Nvs_NameSpace_Network, Nvs_Key_Wifi_Ssid, value,maxlen);
}

esp_err_t network_wifi_connect()
{
    if(network_current_wifi_state==EV_SZP_WIFI_CONNECT_SUCCESS)
    {
        return ESP_FAIL;
    }
    //查询NVS中的wifi数据,没有则使用配置中的wifi
    char wifi_ssid[32];
    char wifi_password[64];
    size_t ssid_len= szp_nvs_read_str(Nvs_NameSpace_Network, Nvs_Key_Wifi_Ssid, wifi_ssid,32);
    szp_nvs_read_str(Nvs_NameSpace_Network, Nvs_Key_Wifi_Password, wifi_password,32);
    if(ssid_len==0)
    {
        strncpy(wifi_ssid, DEFAULT_SZP_WIFI_SSID, 32);
        strncpy(wifi_password, DEFAULT_SZP_WIFI_PASSWORD, 64);
    }
    SzpWifiConnectConfig cfg =
   {
      .ssid = wifi_ssid,
      .password = wifi_password,
      .wifi_connect_cb = cb_szp_network_wifi_event,
      .retry_count = DEFAULT_SZP_WIFI_RETRY_COUNT,
    };
    //wifi数据记录到NVS
    szp_nvs_write_str(Nvs_NameSpace_Network,Nvs_Key_Wifi_Ssid,wifi_ssid);
    szp_nvs_write_str(Nvs_NameSpace_Network,Nvs_Key_Wifi_Password,wifi_password);
    return szp_wifi_connect(cfg);
}


esp_err_t network_wifi_disconnect()
{
    if(network_current_wifi_state==EV_SZP_WIFI_CONNECT_SUCCESS)
    {
        return szp_wifi_disconnect();
    }
    return ESP_FAIL;
}


uint32_t network_wait_wifi_event(SzpWifiStateEvent evnet, uint32_t waitTimeMs)
{
    if(!event_group_szp_wifi)
    {
        return 0;
    }
    TickType_t time=waitTimeMs;
    if(waitTimeMs!=SZP_WAIT_FOR_INFINITE)   //非永久则转换为TICK
    {
        time = SZP_MS_TO_TICK(waitTimeMs);
    }

    return xEventGroupWaitBits(event_group_szp_wifi,
                               (EventBits_t)evnet,
                               (BaseType_t)SZP_OS_TRUE,
                               (BaseType_t)SZP_OS_FALSE,
                               (TickType_t)time);

}

SzpWifiStateEvent network_wifi_current_state()
{
    return network_current_wifi_state;
}

void network_wifi_register_event_cb(network_wifi_event_cb event_cb)
{
    network_wifi_event_callback = event_cb;
}

//等待wifi连接成功
bool network_wait_wifi_connect_success(uint32_t waitMs)
{
     bool is_wifi_connect = false;
    //wifi未连接进入等待
    if(network_wifi_current_state()!=EV_SZP_WIFI_CONNECT_SUCCESS)
    {
        uint32_t event = network_wait_wifi_event(EV_SZP_WIFI_CONNECT_SUCCESS, waitMs); // 等待一分钟
        if (event & EV_SZP_WIFI_CONNECT_SUCCESS)
        {
            is_wifi_connect = true;
        }
    }
    else
    {
        is_wifi_connect = true;
    }
    return is_wifi_connect;
}
/*************** wifi操作 ***************/



/*************** mqtt操作 ***************/
#if CONFIG_USE_SZP_MQTT
#include "szp_mqtt.h"
//线程等待wifi 开启mqtt
static void task_network_mqtt_start(void *arg)
{
    bool is_wifi_connect = false;
    //wifi未连接进入等待
    if(network_wifi_current_state()!=EV_SZP_WIFI_CONNECT_SUCCESS)
    {
        uint32_t event = network_wait_wifi_event(EV_SZP_WIFI_CONNECT_SUCCESS, 60 * 1000); // 等待一分钟
        if (event & EV_SZP_WIFI_CONNECT_SUCCESS)
        {
            is_wifi_connect = true;
        }
    }
    else
    {
        is_wifi_connect = true;
    }
    if(is_wifi_connect)
    {
        esp_err_t ret = szp_mqtt_start();
        if(ret!=ESP_OK)
        {
            ESP_LOGE(SZP_NETWORK_TAG,"启动MQTT服务失败,ret=0x%x",ret);
        }
    }
    else
    {
        ESP_LOGE(SZP_NETWORK_TAG,"wifi未连接,启动MQTT服务失败");
    }
    vTaskDelete(NULL);
}

void network_start_mqtt_task()
{
    xTaskCreate(task_network_mqtt_start, "task_network_mqtt_start", 4096, NULL, 5, NULL);
}

#endif

/*************** mqtt操作 ***************/

/*************** web服务操作 ***************/
//SNTP授时
static void task_network_sntp_get_time(void *arg)
{
    //判断是否需要授时
    time_t now = 0;
    struct tm time_info = { 0 };
    time(&now);
    localtime_r(&now, &time_info);
    if(time_info.tm_year>(2000-1900))
    {
        ESP_LOGI(SZP_NETWORK_TAG,"当前已获取实时时间,无需SNTP授时,当前时间:%d-%d-%d %d:%d:%d",
                     time_info.tm_year+1900, time_info.tm_mon+1, time_info.tm_mday,
                     time_info.tm_hour, time_info.tm_min, time_info.tm_sec);
        vTaskDelete(NULL);
        return;
    }

    //wifi未连接进入等待
    if(network_wait_wifi_connect_success(60 * 1000))// 等待一分钟
    {
        //配置SNTP服务
        esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(SZP_SNTP_SERVER_URI);
        esp_netif_sntp_init(&config);
        //尝试获取时间
        int retry = 0;
        const int retry_count = 15;
        while (esp_netif_sntp_sync_wait(SZP_MS_TO_TICK(2000)) == ESP_ERR_TIMEOUT && ++retry < retry_count)
        {
            ESP_LOGI(SZP_NETWORK_TAG, "等待SNTP授时....尝试次数:(%d/%d)", retry, retry_count);
        }
        esp_netif_sntp_deinit();
        if(retry<retry_count)//授时成功
        {
            time_t now = 0;
            struct tm time_info;
            time(&now);
            localtime_r(&now, &time_info);
            ESP_LOGI(SZP_NETWORK_TAG, "SNTP授时成功,当前时间:%d-%d-%d %d:%d:%d",
                     time_info.tm_year+1900, time_info.tm_mon+1, time_info.tm_mday,
                     time_info.tm_hour, time_info.tm_min, time_info.tm_sec);
            if(sntp_complete_callback)
            {
                sntp_complete_callback();
            }        
        }
        else//授时失败
        {
            ESP_LOGE(SZP_NETWORK_TAG, "SNTP网络授时失败,尝试超过次数");
        }
    }
    else
    {
        ESP_LOGE(SZP_NETWORK_TAG,"wifi未连接,SNTP网络授时失败");
    }
    vTaskDelete(NULL);
}

void network_start_sntp_task()
{
    xTaskCreate(task_network_sntp_get_time, "nw_sntp_get_time", 2048, NULL, 5, NULL);
}

void network_sntp_complete_register_cb(sntp_complete_cb cb)
{
    sntp_complete_callback = cb;
}
/*************** web服务操作 ***************/