#pragma once

#include "esp_err.h"
#include "esp_gatt_defs.h"

#define SZP_BLE_GATTS_CHAR_BUF_MAX_LEN       ESP_GATT_MAX_ATTR_LEN        //读写char缓存最大长度
#define SZP_GATTS_ADV_SERVICE_UUID16_1          0x17        //服务UUID1
#define SZP_GATTS_ADV_SERVICE_UUID16_2          0x71        //服务UUID2

#define SZP_GATTS_SERVICE_UUID                          0x0077              //服务UUID
#define SZP_GATTS_DEVICE_NAME                           "Szp-X-Pad"      //蓝牙名称

#define SZP_BLE_GATTS_CHECK_TIMER_ID                      77              //定时器ID
#define SZP_BLE_GATTS_LOST_CONNECT_TIME_S           60*1          //丢失连接时间,单位秒(超过自动关闭蓝牙)

//gatts char和descr配置结构体
typedef struct
{
     /********CHAR********/
    uint16_t char_uuid;
    esp_gatt_perm_t               char_perm;   //char权限
    esp_gatt_char_prop_t       char_property;//char 属性
    
    /********DESCR********/
    bool                            add_descr;
    uint16_t                       descr_uuid;
    //descr为自动回复需要填写值
    const char*                 descr_value;    //descr值

}szp_gatts_char_descr;

//char读回调
typedef size_t (*szp_ble_gatts_char_read)(uint16_t uuid, char *value, int maxlen);
//char写回调
typedef void (*szp_ble_gatts_char_write)(uint16_t uuid,const char *value);

//char读写回调结构体
typedef struct 
{
    szp_ble_gatts_char_read char_read;  
    szp_ble_gatts_char_write char_write;
}szp_ble_gatts_char_cb_t;

//ble-gatts事件
typedef enum
{
    EV_SZP_BLE_GATTS_STOP,  //蓝牙关闭
    EV_SZP_BLE_GATTS_START, //蓝牙开启
} SzpBleGattsEvent;
//ble-gatts事件回调函数
typedef void (*szp_ble_gatts_event_cb)(SzpBleGattsEvent e);

//蓝牙GATTS服务启动
esp_err_t szp_ble_gatts_start(void);
//蓝牙GATTS服务停止
esp_err_t szp_ble_gatts_stop(void);
//注册蓝牙事件回调
void szp_ble_gatts_register_event_cb(szp_ble_gatts_event_cb event_cb);
//获取当前蓝牙状态
SzpBleGattsEvent szp_ble_gatts_get_current_event(void);
//注册CHAR/DESCR回调
void szp_ble_gatts_register_char_cb(szp_ble_gatts_char_cb_t char_cb);
//添加服务的char/descr
bool szp_ble_gatts_add_char(szp_gatts_char_descr char_descr);
//删除所有char
bool szp_ble_gatts_del_all_char();
//监控设置(开启则在1分钟内无连接则关闭gatts) 注:开启时需要修改sdkconfig中的BTC Task的栈大小
void szp_ble_gatts_set_check_enable(bool enable);