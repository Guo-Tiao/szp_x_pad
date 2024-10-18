#include "network_manager.h"
#include "szp_wifi.h"
#include "storage_manager.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define DEFAULT_SZP_WIFI_SSID                       CONFIG_SZP_WIFI_SSID
#define DEFAULT_SZP_WIFI_PASSWORD           CONFIG_SZP_WIFI_PASSWORD
#define DEFAULT_SZP_WIFI_RETRY_COUNT      CONFIG_SZP_WIFI_RETRY_COUNT

//实战派wifi事件组
EventGroupHandle_t event_group_szp_wifi;

void network_init(void)
{
    //wifi初始化
    esp_err_t ret= szp_wifi_init();
    if(ret!=ESP_OK)
    {
        szp_wifi_disconnect();
    }
    else
    {
        //创建wifi事件组
        event_group_szp_wifi = xEventGroupCreate();
    }
}

static void cb_szp_network_wifi_event(SzpWifiConnectEvent e)
{
    if(event_group_szp_wifi)
    {
        printf("cb_szp_network_wifi_event:%d\r\n", e);
        switch (e)
        {
        case SZP_WIFI_CONNECTED:
            xEventGroupSetBits(event_group_szp_wifi, EV_SZP_WIFI_CONNECT_SUCCESS);
            break;
        case SZP_WIFI_DISCONNECTED:
            xEventGroupSetBits(event_group_szp_wifi, EV_SZP_WIFI_CONNECT_FAIL);
            break;
        case SZP_WIFI_RECONNECTING:
            xEventGroupSetBits(event_group_szp_wifi, EV_SZP_WIFI_RECONNECTING);
            break;
        }
    }
}

esp_err_t network_wifi_connect()
{
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
    return szp_wifi_disconnect();
}

uint32_t network_wait_wifi_event(SzpWifiEvent evnet, uint32_t waitTime)
{
    if(!event_group_szp_wifi)
    {
        return 0;
    }
    return xEventGroupWaitBits(event_group_szp_wifi,
                               (EventBits_t)evnet,
                               (BaseType_t)pdTRUE,
                               (BaseType_t)pdFALSE,
                               (TickType_t)waitTime);

}
