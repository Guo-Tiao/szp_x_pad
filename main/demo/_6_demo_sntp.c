#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_netif_sntp.h"
#include "lwip/ip_addr.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include "work_controller.h"
#include "network_manager.h"
#include "storage_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>


#define DEMO_SNTP_TAG               "DEMO_SNTP"     //日志使用的TAG
#define SNTP_TIME_SERVER        "pool.ntp.org"

#define INET6_ADDRSTRLEN 48
static void print_servers(void)
{
    ESP_LOGI(DEMO_SNTP_TAG, "List of configured NTP servers:");

    for (uint8_t i = 0; i < SNTP_MAX_SERVERS; ++i){
        if (esp_sntp_getservername(i)){
            ESP_LOGI(DEMO_SNTP_TAG, "server %d: %s", i, esp_sntp_getservername(i));
        } else {
            // we have either IPv4 or IPv6 address, let's print it
            char buff[INET6_ADDRSTRLEN];
            ip_addr_t const *ip = esp_sntp_getserver(i);
            if (ipaddr_ntoa_r(ip, buff, INET6_ADDRSTRLEN) != NULL)
                ESP_LOGI(DEMO_SNTP_TAG, "server %d: %s", i, buff);
        }
    }
}
void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(DEMO_SNTP_TAG, "Notification of a time synchronization event tv:%lld",tv->tv_sec);
}

static void obtain_time(void)
{

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(SNTP_TIME_SERVER);
        config.sync_cb = time_sync_notification_cb;     // Note: This is only needed if we want
    esp_netif_sntp_init(&config);

    print_servers();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 15;
    while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < retry_count) {
        ESP_LOGI(DEMO_SNTP_TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    }
    printf("retry:%d\r\n", retry);
    time(&now);
    localtime_r(&now, &timeinfo);

    esp_netif_sntp_deinit();

       ESP_LOGI(DEMO_SNTP_TAG, "obtain_time now:%lld",now);
}



void _6_demo_sntp_run(void)
{
    storage_init();
    network_init();
    network_wifi_connect();
    network_start_sntp_task();

     for (int i = 20; i >= 0; i--) 
     {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    esp_restart();//测试授时判断
    return;

    uint32_t event = network_wait_wifi_event(EV_SZP_WIFI_CONNECT_SUCCESS, 60 * 1000); // 等待一分钟
    printf("network_wait_wifi_event:%"PRId32"\r\n", event);
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(DEMO_SNTP_TAG, "now:%lld,timeinfo:%d %d %d",now,
    timeinfo.tm_year,
    timeinfo.tm_mon,
    timeinfo.tm_mday);
    obtain_time();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
}



