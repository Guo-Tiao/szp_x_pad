#include "szp_wifi.h"
#include "common_macro.h"
#include <inttypes.h>
#include <string.h>
#include "esp_log.h"
#include "esp_wifi.h" 
#include "esp_netif.h"
#include "esp_event.h"
#include "freertos/event_groups.h"

static esp_netif_t *szp_tutorial_netif = NULL;  
static esp_event_handler_instance_t szp_ip_event_handler;
static esp_event_handler_instance_t szp_wifi_event_handler;
SzpWifiConnectConfig szp_wifi_connect_config;   //wifi连接配置
uint8_t szp_wifi_retry_count;   //wifi重连次数

#define SZP_WIFI_TAG "szp_wifi"

//ip事件回调
static void szp_ip_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case (IP_EVENT_STA_GOT_IP)://获取wifi才默认连接成功
        szp_wifi_retry_count=0;
        if(szp_wifi_connect_config.wifi_connect_cb)
        {
            szp_wifi_connect_config.wifi_connect_cb(SZP_WIFI_CONNECTED);
        }
        break;
    case (IP_EVENT_GOT_IP6):
       szp_wifi_retry_count=0;
        if(szp_wifi_connect_config.wifi_connect_cb)
        {
            szp_wifi_connect_config.wifi_connect_cb(SZP_WIFI_CONNECTED);
        }
        break;
    }
}
//wifi事件回调
static void szp_wifi_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case (WIFI_EVENT_STA_START)://开始连接
        esp_wifi_connect();
        break;

    case (WIFI_EVENT_STA_DISCONNECTED)://断连事件
        if(szp_wifi_retry_count<szp_wifi_connect_config.retry_count)
        {
            esp_wifi_connect();
            szp_wifi_retry_count++;
            if(szp_wifi_connect_config.wifi_connect_cb)
            {
                szp_wifi_connect_config.wifi_connect_cb(SZP_WIFI_RECONNECTING);
            }
        }
        else
        {
            if(szp_wifi_connect_config.wifi_connect_cb)
            {
                szp_wifi_connect_config.wifi_connect_cb(SZP_WIFI_DISCONNECTED);
            }
        }
        break;
    }
}


esp_err_t szp_wifi_init(void)
{

    SZP_ESP_ERR_CHECK(esp_netif_init());

    SZP_ESP_ERR_CHECK(esp_event_loop_create_default());

    SZP_ESP_ERR_CHECK(esp_wifi_set_default_wifi_sta_handlers());

    szp_tutorial_netif = esp_netif_create_default_wifi_sta();
    if (szp_tutorial_netif == NULL) 
    {
        ESP_LOGE("szp_wifi_init", "Failed to create default WiFi STA interface");
        return ESP_FAIL;
    }

    // Wi-Fi stack configuration parameters
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    SZP_ESP_ERR_CHECK(esp_wifi_init(&cfg));

    SZP_ESP_ERR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &szp_wifi_event_cb,
                                                        NULL,
                                                        &szp_wifi_event_handler));
    SZP_ESP_ERR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &szp_ip_event_cb,
                                                        NULL,
                                                        &szp_ip_event_handler));
    return ESP_OK;
}

esp_err_t szp_wifi_connect(SzpWifiConnectConfig config)
{
    szp_wifi_connect_config.wifi_connect_cb = config.wifi_connect_cb;
    szp_wifi_connect_config.retry_count = config.retry_count;
    szp_wifi_retry_count = 0;
    wifi_config_t wifi_config = {
        .sta = {
            // this sets the weakest authmode accepted in fast scan mode (default)
            .threshold.authmode = WIFI_AUTH_OPEN,
        },
    };

    strncpy((char*)wifi_config.sta.ssid, config.ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, config.password, sizeof(wifi_config.sta.password));

    SZP_ESP_ERR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE)); // default is WIFI_PS_MIN_MODEM
    SZP_ESP_ERR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM)); // default is WIFI_STORAGE_FLASH

    SZP_ESP_ERR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    SZP_ESP_ERR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_LOGI(SZP_WIFI_TAG, "开始连接wifi,ssid:%s 密码:%s", wifi_config.sta.ssid,wifi_config.sta.password);
    SZP_ESP_ERR_CHECK(esp_wifi_start());

    return ESP_OK;
}

esp_err_t szp_wifi_disconnect(void)
{
    return esp_wifi_disconnect();
}

esp_err_t szp_wifi_uninit(void)
{
    esp_err_t ret = esp_wifi_stop();
    if (ret == ESP_ERR_WIFI_NOT_INIT) 
    {
        ESP_LOGE(SZP_WIFI_TAG, "Wi-Fi未初始化");
        return ret;
    }

    //wifi反初始
    SZP_ESP_ERR_CHECK(esp_wifi_deinit());
    SZP_ESP_ERR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(szp_tutorial_netif));
    esp_netif_destroy(szp_tutorial_netif);
    //注销事件句柄
    SZP_ESP_ERR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, ESP_EVENT_ANY_ID, szp_ip_event_handler));
    SZP_ESP_ERR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, szp_wifi_event_handler));

    return ESP_OK;
}
