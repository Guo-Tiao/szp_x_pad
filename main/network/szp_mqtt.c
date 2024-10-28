#include "szp_mqtt.h"

#if CONFIG_USE_SZP_MQTT

#include "mqtt_client.h"
#include "esp_log.h"

#define SZP_MQTT_TAG    "'SZP_MQTT"

#define SZP_MQTT_URL                                    CONFIG_SZP_MQTT_BROKER_URI      
#define SZP_MQTT_PORT                                  CONFIG_SZP_MQTT_BROKER_PORK
#define SZP_MQTT_CLIENT_ID                          CONFIG_SZP_MQTT_CLIENT_ID
#define SZP_MQTT_CLIENT_USERNAME          CONFIG_SZP_MQTT_CLIENT_USERNAME
#define SZP_MQTT_CLIENT_PASSWORD           CONFIG_SZP_MQTT_CLIENT_PASSWORD

//todo 后续MQTT根据业务待完成订阅操作,现为测试订阅
#define MQTT_PUBLIC_TOPIC      "/test/topic1"       //测试用的,推送消息主题
#define MQTT_SUBSCRIBE_TOPIC    "/test/topic2"      //测试用的,需要订阅的主题

//MQTT客户端操作句柄
static esp_mqtt_client_handle_t     szp_mqtt_client = NULL;
//MQTT连接标志
static bool   is_mqtt_connected = false;

//mqtt事件处理
static void szp_mqtt_event_handler(void* event_handler_arg,esp_event_base_t event_base,int32_t event_id,void* event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) 
    {
        case MQTT_EVENT_CONNECTED:  //连接成功
        {
            ESP_LOGI(SZP_MQTT_TAG, "MQTT_EVENT_CONNECTED");
            is_mqtt_connected = true;
            //连接成功后，订阅测试主题
            esp_mqtt_client_subscribe_single(szp_mqtt_client,MQTT_SUBSCRIBE_TOPIC,1);
        }
            break;
        case MQTT_EVENT_DISCONNECTED:   //连接断开
        {
            ESP_LOGI(SZP_MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
            is_mqtt_connected = false;
        }
            break;
        case MQTT_EVENT_SUBSCRIBED:     //收到订阅消息ACK
        {
            ESP_LOGI(SZP_MQTT_TAG, "MQTT_EVENT_SUBSCRIBED msg_id=%d", event->msg_id);
        }
            break;
        case MQTT_EVENT_UNSUBSCRIBED:   //收到解订阅消息ACK
            break;
        case MQTT_EVENT_PUBLISHED:      //收到发布消息ACK
        {
            ESP_LOGI(SZP_MQTT_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        }
            break;
        case MQTT_EVENT_DATA:
        {
            ESP_LOGI(SZP_MQTT_TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);       //收到Pub消息直接打印出来
            printf("DATA=%.*s\r\n", event->data_len, event->data);
        }
            break;
        case MQTT_EVENT_ERROR:
        {
            ESP_LOGI(SZP_MQTT_TAG, "MQTT_EVENT_ERROR");
        }
            break;
        default:
            break;
    }
}
//开启MQTT服务
esp_err_t szp_mqtt_start(void)
{
   
    esp_mqtt_client_config_t mqtt_cfg = {0};
    mqtt_cfg.broker.address.uri = SZP_MQTT_URL;
    mqtt_cfg.broker.address.port = SZP_MQTT_PORT;
    mqtt_cfg.credentials.client_id = SZP_MQTT_CLIENT_ID;
    mqtt_cfg.credentials.username = SZP_MQTT_CLIENT_USERNAME;
    mqtt_cfg.credentials.authentication.password = SZP_MQTT_CLIENT_PASSWORD;

    ESP_LOGI(SZP_MQTT_TAG,"mqtt connect->clientId:%s,username:%s,password:%s",
    mqtt_cfg.credentials.client_id,
    mqtt_cfg.credentials.username,
    mqtt_cfg.credentials.authentication.password);

    //设置mqtt配置，返回mqtt操作句柄
    szp_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    //注册mqtt事件回调函数
    esp_mqtt_client_register_event(szp_mqtt_client, ESP_EVENT_ANY_ID, szp_mqtt_event_handler, szp_mqtt_client);
    //启动mqtt连接
    return esp_mqtt_client_start(szp_mqtt_client);
}


#endif