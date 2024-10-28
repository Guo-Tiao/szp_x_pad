#include "network_manager.h"
#include "szp_wifi.h"
#include "storage_manager.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "common_macro.h"

#include "esp_log.h"

#define SZP_NETWORK_TAG         "SZP_NETWORK"

#define DEFAULT_SZP_WIFI_SSID                       CONFIG_SZP_WIFI_SSID
#define DEFAULT_SZP_WIFI_PASSWORD           CONFIG_SZP_WIFI_PASSWORD
#define DEFAULT_SZP_WIFI_RETRY_COUNT      CONFIG_SZP_WIFI_RETRY_COUNT

//实战派wifi事件组
EventGroupHandle_t event_group_szp_wifi;
//当前wifi状态
SzpWifiStateEvent network_current_wifi_state;

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
}

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
#if CONFIG_USE_SZP_MQTT
#include "szp_mqtt.h"
//线程等待wifi 开启mqtt
static void task_network_mqtt_start()
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