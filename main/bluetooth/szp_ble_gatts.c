#include "szp_ble_gatts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "esp_log.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gatt_common_api.h"

#include "common_macro.h"

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"

/* 蓝牙事件流程
T:GATTS 
P:GAP
初始化和创建
T   ESP_GATTS_REG_EVT
P   ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT
P   ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT
T   ESP_GATTS_CREATE_EVT
P   ESP_GAP_BLE_ADV_START_COMPLETE_EVT
T   ESP_GATTS_START_EVT
T   ESP_GATTS_ADD_CHAR_EVT
T   ESP_GATTS_ADD_CHAR_DESCR_EVT
连接
T   ESP_GATTS_CONNECT_EVT
P   ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT
T   ESP_GATTS_MTU_EVT
断开
T   ESP_GATTS_DISCONNECT_EVT
P   ESP_GAP_BLE_ADV_START_COMPLETE_EVT
 */
#define SZP_GATTS_TAG               "SZP_BLE_GATTS"     //日志使用的TAG
#define SZP_BLE_GATTS_APP_ID     0x00       //Gatt服务器ID
#define SZP_GATTS_PROFILE_NUM   1         //Gatt属性数量

/*****************蓝牙信息状态*****************/
static uint8_t szp_gatts_adv_config_done = 0;   //广播数据是否配置完成
#define szp_gatts_adv_config_flag (1 << 0)      //配置广播标志位
#define szp_gatts_scan_rsp_config_flag (1 << 1) //扫描标志位
/***********************************************/

/*****************蓝牙配置*****************/
/*********************服务配置*********************/

//descr默认回复值
static const uint8_t descr_default_value[]={0x00};

//gatts-char实例结构体
typedef struct 
{
    /********CHAR********/
    uint16_t                            char_handle;       //char句柄
    esp_bt_uuid_t                   char_uuid;    //charID(不可重复)
    esp_gatt_perm_t               char_perm;   //char权限
    esp_gatt_char_prop_t       char_property;//char 属性
    /********DESCR********/
    bool                                   add_descr; //是否添加描述
    //自动回复不使用句柄 uint16_t                             descr_handle;      //descr句柄
    esp_bt_uuid_t                    descr_uuid;   //descID(不可重复)
    esp_attr_control_t              descr_control;//descr控制
    esp_attr_value_t                 descr_value;//descr自动回复值
}szp_gatts_char_inst;


//char实例链表
typedef struct szp_gatts_char_list_node 
{
    szp_gatts_char_inst char_inst;
    struct szp_gatts_char_list_node* next;  //指向下一个节点
} SzpGattsCharNode;  

//gatts配置结构体(从官方样例中拷贝gatts_profile_inst)
struct szp_gatts_profile_inst 
{
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    SzpGattsCharNode *szp_gatts_char_list;
    uint16_t szp_gatts_handle_num;// Gatts句柄数 char数*2+2
    szp_ble_gatts_char_cb_t char_cb;    //char/descr读写回调
};
//szp-gatts最终实现回调声明
static void szp_gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
//全局gatts属性组(此处保留使用数组,目前只有单个APP,后续可扩展)
static struct szp_gatts_profile_inst gl_szp_gatts_profile_tab[SZP_GATTS_PROFILE_NUM] ={
    [SZP_BLE_GATTS_APP_ID]= {
        .gatts_if = ESP_GATT_IF_NONE,
        .gatts_cb=szp_gatts_profile_event_handler,   
        .szp_gatts_char_list=NULL,
    }
};


/***********************************************/

/*****************蓝牙广播数据*****************/
//ble-gatts服务UUID
static uint8_t adv_service_uuid128[32] = {
      //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, SZP_GATTS_ADV_SERVICE_UUID16_1, 0x00, 0x00, 0x00,
    //second uuid, 32bit, [12], [13], [14], [15] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, SZP_GATTS_ADV_SERVICE_UUID16_2, 0x00, 0x00, 0x00,
};
//广播数据
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,  //是否设置扫描响应数据 
    .include_name = true,   // 是否包含设备名称
    .include_txpower = false,    // 是否包含发射功率等级 
    .min_interval = 0x0006, // 广播最小间隔（若不可连接，则忽略） Time = min_interval * 1.25 msec
    .max_interval = 0x0010, // 广播最大间隔（若不可连接，则忽略）Time = max_interval * 1.25 msec
    .appearance = 0x00, // 外观值（用于表示设备类型）  
    .manufacturer_len = 0,   // 制造商数据长度  
    .p_manufacturer_data =  NULL,  // 指向制造商数据的指针  
    .service_data_len = 0,   // 服务数据长度
    .p_service_data = NULL, // 指向服务数据的指针
    .service_uuid_len = sizeof(adv_service_uuid128),     // 服务UUID长度  
    .p_service_uuid = adv_service_uuid128,     // 指向服务UUID的指针  
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),     // 广播标志 :一般可发现模式/不支持BR/EDR(不支持经典蓝牙)
};
//扫描响应数据
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,   
    .include_name = true,
    .include_txpower = true,
    //.min_interval = 0x0006,
    //.max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0, 
    .p_manufacturer_data =  NULL, 
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
//广播参数
static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x20, //广播间隔最小值
    .adv_int_max        = 0x40, //广播间隔最大值
    .adv_type           = ADV_TYPE_IND, //广播类型   ADV_TYPE_IND(非定向广播)
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC, //广播使用地址类型 BLE_ADDR_TYPE_PUBLIC(公共地址)
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map        = ADV_CHNL_ALL,     //使用的广播信道 ADV_CHNL_ALL(所有信道37,38,39)
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY, //广播的过滤策略 ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY(允许任何设备扫描和连接)
};
/***********************************************/

/*****************蓝牙定时监控*****************/
bool szp_ble_gatts_check_enable=false;//是否使能检查
TimerHandle_t szp_ble_gatts_check_timer_handle=NULL;

typedef struct 
{
    bool is_connect;//是否连接
    TickType_t szp_ble_gatts_current_connect_time;//当前连接开始时间
    TickType_t szp_ble_gatts_current_disconnect_time;//当前未连接开始时间
}szp_ble_gatts_check_time_t;
//检查时间结构体
szp_ble_gatts_check_time_t szp_ble_gatts_check_time;
//定时器回调()
static void szp_ble_gatts_check_timer_callback(TimerHandle_t handle)
{
    if(!szp_ble_gatts_check_time.is_connect)//未连接状态
    {
       
        TickType_t current_time =  xTaskGetTickCount();
        TickType_t disconnect_time = current_time - szp_ble_gatts_check_time.szp_ble_gatts_current_disconnect_time;//未连接时间时长
        TickType_t disconnect_time_ms=pdTICKS_TO_MS(disconnect_time);
        if(disconnect_time_ms>SZP_BLE_GATTS_LOST_CONNECT_TIME_S*1000)
        {
            ESP_LOGI(SZP_GATTS_TAG, "蓝牙超时未有客户端连接,自动关闭");
            szp_ble_gatts_stop();
        }
    }

}
//更新连接时间点
static void szp_ble_gatts_update_check_time(bool isconnect)
{
    if(szp_ble_gatts_check_enable)
    {
        szp_ble_gatts_check_time.is_connect = isconnect;
        if(isconnect)
            szp_ble_gatts_check_time.szp_ble_gatts_current_connect_time=xTaskGetTickCount();
        else
            szp_ble_gatts_check_time.szp_ble_gatts_current_disconnect_time=xTaskGetTickCount();
    }
}
//开启检查定时器
static bool szp_ble_gatts_start_check_timer()
{
    if(szp_ble_gatts_check_enable&&szp_ble_gatts_check_timer_handle!=NULL)
    {
        xTimerStart(szp_ble_gatts_check_timer_handle, portMAX_DELAY);
        return true;
    }
    return false;
}
//关闭检查定时器
static bool szp_ble_gatts_stop_check_timer()
{
    if(szp_ble_gatts_check_enable&&szp_ble_gatts_check_timer_handle!=NULL)
    {
        szp_ble_gatts_update_check_time(false);
        xTimerStop(szp_ble_gatts_check_timer_handle, portMAX_DELAY);
        return true;
    }
    return false;
}
/***********************************************/



//查找char节点通过charuuid
static inline SzpGattsCharNode*  szp_gatts_find_char_node_by_charuuid(uint16_t uuid16)
{
    if (gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].szp_gatts_char_list != NULL)
    {
        SzpGattsCharNode *node = gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].szp_gatts_char_list;
        while (node!=NULL)
        {
            if(node->char_inst.char_uuid.uuid.uuid16==uuid16)
            {
                return node;
            }
            node = node->next;
        }
    }
    return NULL;
}

//查找descr节点通过descruuid
static inline SzpGattsCharNode*  szp_gatts_find_char_node_by_descruuid(uint16_t uuid16)
{
    if (gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].szp_gatts_char_list != NULL)
    {
        SzpGattsCharNode *node = gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].szp_gatts_char_list;
        while (node!=NULL)
        {
            if(node->char_inst.descr_uuid.uuid.uuid16==uuid16)
            {
                return node;
            }
            node = node->next;
        }
    }
    return NULL;
}

//查找char节点通过char句柄
static inline SzpGattsCharNode*  szp_gatts_find_char_node_by_char_handle(uint16_t handle)
{
    if (gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].szp_gatts_char_list != NULL)
    {
        SzpGattsCharNode *node = gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].szp_gatts_char_list;
        while (node!=NULL)
        {
            if(node->char_inst.char_handle==handle)
            {
                return node;
            }
            node = node->next;
        }
    }
    return NULL;
}

//计算服务所需要的句柄数
static inline uint16_t szp_gatts_calc_handle_num()
 {
    uint16_t num = 2;//初始需要两个句柄
    SzpGattsCharNode *node = gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].szp_gatts_char_list; // 获取头节点
    while (node!=NULL)
    {
        num += 2;
        if(node->char_inst.add_descr)   //有添加descr需要+2
        {
            num += 2;
        }
        node = node->next;
    }
    return num;
 }

//GAP事件回调
static void szp_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) 
    {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT: //广播数据设置完毕
        {
            ESP_LOGI(SZP_GATTS_TAG, "ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT\n");
            szp_gatts_adv_config_done &= (~szp_gatts_adv_config_flag);
             if (szp_gatts_adv_config_done == 0)
             {
                esp_ble_gap_start_advertising(&adv_params);
            }
        }
        break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:    //扫描回复数据设置完毕
        {
            ESP_LOGI(SZP_GATTS_TAG, "ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT\n");
            szp_gatts_adv_config_done &= (~szp_gatts_scan_rsp_config_flag);
            if (szp_gatts_adv_config_done == 0)
            {
                esp_ble_gap_start_advertising(&adv_params);
            }
        }
        break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:    //开启完毕
        {
            ESP_LOGI(SZP_GATTS_TAG, "ESP_GAP_BLE_ADV_START_COMPLETE_EVT\n");
             if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) 
             {
                ESP_LOGE(SZP_GATTS_TAG, "Advertising start failed\n");
            }
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
    {
        ESP_LOGI(SZP_GATTS_TAG, "ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT\n");
        ESP_LOGI(SZP_GATTS_TAG, "update connection params: status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                  param->update_conn_params.status,
                  param->update_conn_params.min_int,
                  param->update_conn_params.max_int,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
    }
        break;
    default:
        break;
    }
}
//szp-gatts最终实现回调声明
static void szp_gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
     //创建及初始化事件   
    case ESP_GATTS_REG_EVT:     //Gatts注册事件
    { 
        ESP_LOGI(SZP_GATTS_TAG, "ESP_GATTS_REG_EVT, app_id %"PRIu16", status 0x%x\n", param->reg.app_id, param->reg.status);
        gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].service_id.is_primary = true;
        gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].service_id.id.inst_id = 0x00;
        gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
        gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].service_id.id.uuid.uuid.uuid16 = SZP_GATTS_SERVICE_UUID;

        //设置蓝牙名称(设备名称)
        esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(SZP_GATTS_DEVICE_NAME);
        if (set_dev_name_ret) ESP_LOGE(SZP_GATTS_TAG, "set device name failed, error code = %x", set_dev_name_ret);
        //配置广播数据
        esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
        if (ret)  ESP_LOGE(SZP_GATTS_TAG, "config adv data failed, error code = 0x%x", ret);
        szp_gatts_adv_config_done |= szp_gatts_adv_config_flag;
        //配置扫描回复
        ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
        if (ret)  ESP_LOGE(SZP_GATTS_TAG, "config raw scan response data failed, error code = 0x%x", ret);
        szp_gatts_adv_config_done |= szp_gatts_scan_rsp_config_flag;
        //创建gatts服务
        gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].szp_gatts_handle_num = szp_gatts_calc_handle_num();
        ret = esp_ble_gatts_create_service(gatts_if, &gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].service_id, gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].szp_gatts_handle_num);
        if (ret)  ESP_LOGE(SZP_GATTS_TAG,"create attr table failed, error code = 0x%x", ret);
    }
        break;
    case ESP_GATTS_CREATE_EVT:    //Gatts创建事件
    {
        ESP_LOGI(SZP_GATTS_TAG, "ESP_GATTS_CREATE_EVT, status 0x%x,  service_handle %"PRIu16"\n", param->create.status, param->create.service_handle);
        //开启服务
        gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].service_handle = param->create.service_handle;
        esp_ble_gatts_start_service(gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].service_handle);

        // 设置添加服务的char和descr
        if(gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].szp_gatts_char_list!=NULL)
        {
            SzpGattsCharNode *node = gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].szp_gatts_char_list;
            while (node!=NULL)
            {
                uint16_t char_uuid16 = node->char_inst.char_uuid.uuid.uuid16;
                uint16_t descr_uuid16 = node->char_inst.descr_uuid.uuid.uuid16;
                //添加char
                esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].service_handle, &node->char_inst.char_uuid,
                                                                                                    node->char_inst.char_perm,
                                                                                                    node->char_inst.char_property,
                                                                                                    NULL, NULL);
                if (add_char_ret)    ESP_LOGE(SZP_GATTS_TAG, "add char failed, error code =0x%x  uuid=0x%x",add_char_ret,char_uuid16);
                
                //添加descr
                if(node->char_inst.add_descr)
                {
                    esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].service_handle, 
                                                                                                                &node->char_inst.descr_uuid,
                                                                                                                ESP_GATT_PERM_READ, 
                                                                                                                &node->char_inst.descr_value, 
                                                                                                                &node->char_inst.descr_control);
                    if (add_descr_ret)  ESP_LOGE(SZP_GATTS_TAG, "add char descr failed, error code =0x%x char_uuid16=0x%x descr_uuid16=0x%x", add_descr_ret,char_uuid16,descr_uuid16);
                }

                node = node->next;
            }
            
        }
    }
        break;
    case ESP_GATTS_START_EVT:   //gatt服务开始事件
    {
        ESP_LOGI(SZP_GATTS_TAG, "ESP_GATTS_START_EVT, status 0x%x, service_handle %"PRIu16"\n",
                 param->start.status, param->start.service_handle);
        gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].conn_id = 0x00;
        if(szp_ble_gatts_start_check_timer())//开启服务时同步开启检查定时器
        {
            szp_ble_gatts_update_check_time(false);
        }
    }
        break;
    case ESP_GATTS_ADD_CHAR_EVT:    //char添加事件
    {
        ESP_LOGI(SZP_GATTS_TAG, "ESP_GATTS_ADD_CHAR_EVT, add_char_uuid16 0x%x,status 0x%x,  attr_handle %"PRIu16", service_handle %"PRIu16"\n",
                param->add_char.char_uuid.uuid.uuid16,
                param->add_char.status, 
                param->add_char.attr_handle, 
                param->add_char.service_handle);
         //成功才添加CHAR句柄
        uint16_t uuid16 = param->add_char.char_uuid.uuid.uuid16;
        if(param->add_char.status==ESP_GATT_OK)
        {
            SzpGattsCharNode *node = szp_gatts_find_char_node_by_charuuid(uuid16);
            if(node!=NULL) node->char_inst.char_handle = param->add_char.attr_handle;       //赋值句柄  
            else ESP_LOGE(SZP_GATTS_TAG, "add char evt failed, char_uuid=0x%x not found",uuid16);
        }
        else
        {
            ESP_LOGE(SZP_GATTS_TAG, "add char evt failed, error status =0x%x  char_uuid=0x%x",param->add_char.status,uuid16);
        }
    }
        break;    
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:  //descr添加事件
    {
        ESP_LOGI(SZP_GATTS_TAG, "ESP_GATTS_ADD_CHAR_DESCR_EVT, add_char_descr_uuid16 0x%x,status %d, attr_handle %"PRIu16", service_handle %"PRIu16"\n",
                param->add_char_descr.descr_uuid.uuid.uuid16,
                param->add_char_descr.status, 
                param->add_char_descr.attr_handle, 
                param->add_char_descr.service_handle);
         //成功才添加descr句柄
        /*  自动回复不需要用到句柄
        uint16_t uuid16 = param->add_char_descr.descr_uuid.uuid.uuid16;
        if(param->add_char_descr.status==ESP_GATT_OK)
        {
            SzpGattsCharNode *node = szp_gatts_find_char_node_by_descruuid(uuid16);
            if(node!=NULL) node->char_inst.descr_handle = param->add_char_descr.attr_handle;       //赋值句柄  
            else ESP_LOGE(SZP_GATTS_TAG, "add char descr evt failed, descr_uuid=0x%x not found",uuid16);
        }
        else
        {
            ESP_LOGE(SZP_GATTS_TAG, "add char descr evt failed, error status =0x%x  descr_uuid=0x%x",param->add_char_descr.status,uuid16);
        }
        */
    }
        break;    
    case ESP_GATTS_READ_EVT:    // 读操作
    {
        ESP_LOGI(SZP_GATTS_TAG, "GATT_READ_EVT, conn_id %"PRIu16", trans_id %"PRIu32", handle %"PRIu16"\n", param->read.conn_id, param->read.trans_id, param->read.handle);

        //通过句柄查找char节点,descr不回复(已自动)
        uint16_t handle=param->read.handle;
        SzpGattsCharNode *node = szp_gatts_find_char_node_by_char_handle(handle);
        if(node!=NULL)
        {
            szp_ble_gatts_char_read char_read = gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].char_cb.char_read;
            esp_gatt_rsp_t rsp;
            memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
            rsp.attr_value.handle = handle;
            if (char_read)
            {
                char buf[SZP_BLE_GATTS_CHAR_BUF_MAX_LEN];
                size_t len = char_read(node->char_inst.char_uuid.uuid.uuid16, buf, SZP_BLE_GATTS_CHAR_BUF_MAX_LEN);
                if(len>0)
                {
                    rsp.attr_value.len = len;
                    memcpy(rsp.attr_value.value, buf, rsp.attr_value.len);
                }
        }
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);
        }
    }
        break;
    case ESP_GATTS_WRITE_EVT:   // 写操作
    {
        ESP_LOGI(SZP_GATTS_TAG, "ESP_GATTS_WRITE_EVT, conn_id %"PRIu16" trans_id %"PRIu32", handle %"PRIu16"", param->write.conn_id, param->write.trans_id, param->write.handle);
        //通过句柄查找char节点,descr不回复(已自动)
        uint16_t handle=param->write.handle;
        SzpGattsCharNode *node = szp_gatts_find_char_node_by_char_handle(handle);
        if(node!=NULL)
        {
            szp_ble_gatts_char_write char_write = gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].char_cb.char_write;
            char buf[SZP_BLE_GATTS_CHAR_BUF_MAX_LEN];
            memcpy(buf,param->write.value,param->write.len);
            buf[param->write.len]=0;
            if(char_write)
            {
                char_write(node->char_inst.char_uuid.uuid.uuid16, buf);
            }
            if(node->char_inst.char_property & ESP_GATT_CHAR_PROP_BIT_NOTIFY) //需要通知
            {
                esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, node->char_inst.char_handle,
                                            strlen(buf), (uint8_t*)buf, false);
            }
        }
        if (param->write.need_rsp)
        {
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        }
    }
        break;
    case ESP_GATTS_EXEC_WRITE_EVT:
     {
         ESP_LOGI(SZP_GATTS_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
         esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
     }   
        break;
    //连接事件
    case ESP_GATTS_CONNECT_EVT: //连接 
    {
        ESP_LOGI(SZP_GATTS_TAG, "ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
                 param->connect.conn_id,
                 param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
                 param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
        szp_ble_gatts_update_check_time(true);
        gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].conn_id = param->connect.conn_id;
    }   
        break;
    //断开事件
    case ESP_GATTS_DISCONNECT_EVT:
    {
        ESP_LOGI(SZP_GATTS_TAG, "ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x", param->disconnect.reason);
        szp_ble_gatts_update_check_time(false);
        gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].conn_id = 0x00;
        esp_ble_gap_start_advertising(&adv_params);
    }
        break;
    //其他
    case ESP_GATTS_MTU_EVT:
    {
        ESP_LOGI(SZP_GATTS_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
    }   
        break;
    case ESP_GATTS_UNREG_EVT:
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
    case ESP_GATTS_DELETE_EVT:
    case ESP_GATTS_STOP_EVT:
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    default:
        break;
    }

}
//GATTS事件回调
static void szp_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
  if (event == ESP_GATTS_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            gl_szp_gatts_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        }
    }
    do
    {
        int idx;
        for (idx = 0; idx < SZP_GATTS_PROFILE_NUM; idx++)
        {
            if (gatts_if == ESP_GATT_IF_NONE || 
                gatts_if == gl_szp_gatts_profile_tab[idx].gatts_if)
            {
                if (gl_szp_gatts_profile_tab[idx].gatts_cb)
                {
                    gl_szp_gatts_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

//BLE-GATTS启动
esp_err_t szp_ble_gatts_start(void)
{
    //先获取蓝牙状态,确保未初始化才开启
    esp_bluedroid_status_t status = esp_bluedroid_get_status();
    if(status!=ESP_BLUEDROID_STATUS_UNINITIALIZED)  //不等于未初始化返回
    {
        return ESP_FAIL;
    }

    //使能检查时创建定时器
    if(szp_ble_gatts_check_enable&&szp_ble_gatts_check_timer_handle==NULL)
    {
        szp_ble_gatts_check_timer_handle=xTimerCreate(
            "szp_ble_gatts_check_timer", 
            pdMS_TO_TICKS(1000), 
            pdTRUE, 
            (void *)SZP_BLE_GATTS_CHECK_TIMER_ID, 
            szp_ble_gatts_check_timer_callback);
    }

    // 蓝牙初始化
    // 释放经典蓝牙内存
    SZP_ESP_ERR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    //初始化蓝牙控制器
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    SZP_ESP_ERR_CHECK( esp_bt_controller_init(&bt_cfg));
    //开启低功耗蓝牙
    SZP_ESP_ERR_CHECK( esp_bt_controller_enable(ESP_BT_MODE_BLE));
    //bluedroid初始化和使能
    SZP_ESP_ERR_CHECK( esp_bluedroid_init());
    SZP_ESP_ERR_CHECK( esp_bluedroid_enable());
//注册蓝牙回调GATTS/GAP
    SZP_ESP_ERR_CHECK(esp_ble_gatts_register_callback(szp_gatts_event_handler));
    SZP_ESP_ERR_CHECK(esp_ble_gap_register_callback(szp_gap_event_handler));
//注册应用程序标识符
    SZP_ESP_ERR_CHECK(esp_ble_gatts_app_register(SZP_BLE_GATTS_APP_ID));
//配置本地MTU大小
    SZP_ESP_ERR_CHECK(esp_ble_gatt_set_local_mtu(500));
//打印蓝牙MAC
    const uint8_t *addr = esp_bt_dev_get_address();
    ESP_LOGI(SZP_GATTS_TAG, "SZP_BLE_GATTS MAC:  %02X.%02X.%02X.%02X.%02X.%02X",addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);

    return ESP_OK;
}
//BLE-GATTS关闭
esp_err_t szp_ble_gatts_stop(void)
{
    esp_bluedroid_status_t status=esp_bluedroid_get_status();
    //确保开启才进行关闭
    if(status==ESP_BLUEDROID_STATUS_ENABLED)
    {
        //关闭和删除定时器
        szp_ble_gatts_stop_check_timer();
        if(szp_ble_gatts_check_enable&&szp_ble_gatts_check_timer_handle!=NULL)
        {
            xTimerDelete(szp_ble_gatts_check_timer_handle, portMAX_DELAY);
             szp_ble_gatts_check_timer_handle = NULL;
        }
        //bluedroid反初始化和关闭
        SZP_ESP_ERR_CHECK(esp_bluedroid_disable());
        SZP_ESP_ERR_CHECK(esp_bluedroid_deinit());
        //蓝牙控制器反初始化和关闭
        SZP_ESP_ERR_CHECK(esp_bt_controller_disable());
        SZP_ESP_ERR_CHECK(esp_bt_controller_deinit());
        return ESP_OK;
    }
    else
    {
        return ESP_FAIL;
    }
    return ESP_OK;
}
//char读写回调注册
void szp_ble_gatts_register_char_cb(szp_ble_gatts_char_cb_t char_cb)
{
    gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].char_cb = char_cb;
}

//创建char节点
static SzpGattsCharNode* szp_gatts_create_char_node(szp_gatts_char_descr char_descr)
{
    SzpGattsCharNode* new_node= (SzpGattsCharNode *)malloc(sizeof(SzpGattsCharNode));
    //CHAR
    new_node->char_inst.char_handle=0x00;//待赋值
    new_node->char_inst.char_uuid.len = ESP_UUID_LEN_16;
    new_node->char_inst.char_uuid.uuid.uuid16 = char_descr.char_uuid;
    new_node->char_inst.char_perm = char_descr.char_perm;
    new_node->char_inst.char_property = char_descr.char_property;

    //DESCR
    new_node->char_inst.add_descr = char_descr.add_descr;
    //new_node->char_inst.descr_handle = 0x00;//待赋值
    if(char_descr.add_descr)
    {
        new_node->char_inst.descr_uuid.len = ESP_UUID_LEN_16;
        new_node->char_inst.descr_uuid.uuid.uuid16 = char_descr.descr_uuid;

        if(char_descr.descr_value==NULL)
        {
            new_node->char_inst.descr_value.attr_max_len = sizeof(descr_default_value);
            new_node->char_inst.descr_value.attr_len = sizeof(descr_default_value);
            new_node->char_inst.descr_value.attr_value = descr_default_value;
        }
        else
        {
            new_node->char_inst.descr_value.attr_max_len = strlen(char_descr.descr_value)*2;
            new_node->char_inst.descr_value.attr_len = strlen(char_descr.descr_value);
            new_node->char_inst.descr_value.attr_value = char_descr.descr_value;
        }


        new_node->char_inst.descr_control.auto_rsp = ESP_GATT_AUTO_RSP;
    }

    new_node->next = NULL;
    return new_node;
}
//添加char节点
bool szp_ble_gatts_add_char(szp_gatts_char_descr char_descr)
{
    esp_bluedroid_status_t status=esp_bluedroid_get_status();
    //开启时不能添加char
    if(status==ESP_BLUEDROID_STATUS_ENABLED)
    {
        return false;
    }
    uint16_t char_uuid = char_descr.char_uuid;
    uint16_t descr_uuid = char_descr.descr_uuid;
    SzpGattsCharNode *new_node = szp_gatts_create_char_node(char_descr);
    SzpGattsCharNode **head_ref = &gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].szp_gatts_char_list;//二级指针获取头节点地址
    SzpGattsCharNode *last = *head_ref;
    //头节点为空则赋值
    if(*head_ref==NULL)
    {
        *head_ref = new_node;
        return true;
    }
    else
    {
        SzpGattsCharNode *node = last;
        while (last!=NULL)
        {
            if(last->char_inst.char_uuid.uuid.uuid16==char_uuid)//不添加重复charid
            {
                return false;
             }
            if(char_descr.add_descr)    //有添加descr才判断i
            {
                if(last->char_inst.descr_uuid.uuid.uuid16==descr_uuid)//不添加重复descrid
                {
                    return false;
                }
            }
            node = last;
            last = last->next;
        }
        node->next = new_node;
    }
    
    return true;
}

bool szp_ble_gatts_del_all_char()
{
     esp_bluedroid_status_t status=esp_bluedroid_get_status();
    //开启时不能删除char
    if(status==ESP_BLUEDROID_STATUS_ENABLED)
    {
        return false;
    }
    SzpGattsCharNode **head_ref = &gl_szp_gatts_profile_tab[SZP_BLE_GATTS_APP_ID].szp_gatts_char_list; // 二级指针获取头节点地址
    SzpGattsCharNode* temp;
    SzpGattsCharNode *node = *head_ref;
    while (node!=NULL)
    {
        temp = node;
        node = node->next;
        free(temp);
        temp = NULL;
    }
    *head_ref = NULL;
    return true;
}

void szp_ble_gatts_set_check_enable(bool enable)
{
    esp_bluedroid_status_t status = esp_bluedroid_get_status();
    if(status==ESP_BLUEDROID_STATUS_ENABLED)  //开启时无法设置
    {
        return;
    }
    szp_ble_gatts_check_enable = enable;
}
