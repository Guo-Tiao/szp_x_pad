#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "driver/rmt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "common_macro.h"
#include "szp_ble_gatts.h"
#include "szp_key.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "storage_manager.h"
#include "network_manager.h"

//CHAR读回调测试
size_t szp_ble_gatts_char_read_callback_test(uint16_t uuid, char *value, int maxlen)
{
    if(uuid==0xFF00)
    {
    printf("szp_ble_gatts_char_read_callback uuid:0x%x\r\n",uuid);
    size_t ssid_len= szp_nvs_read_str(Nvs_NameSpace_Network, Nvs_Key_Wifi_Ssid, value,maxlen);
    return ssid_len;
    }
    else
    {
        return 0;
    }
}
void szp_ble_gatts_char_write_callback_test(uint16_t uuid, const char *value)
{
    if(uuid==0xFF00)
    {
        printf("szp_ble_gatts_char_write_callback uuid:0x%x  value:%s\r\n",uuid,value);
        szp_nvs_write_str(Nvs_NameSpace_Network, Nvs_Key_Wifi_Ssid, value);
    }
    else
    {
    }
}

//SZP-BLE-gatts注册
void szp_ble_gatts_register()
{
    for (size_t i = 0; i < 3; i++)
    {
        szp_gatts_char_descr char_descr =
        {
            .char_uuid = 0xFF00+i,
            .char_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            .char_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,

            .add_descr =i%2?true:false,
            .descr_uuid = 0x2700+i,
            /*
            .descr_value.attr_max_len=0,
            .descr_value.attr_len= 0,
            .descr_value.attr_value=NULL,
            */
        };
        szp_ble_gatts_add_char(char_descr);
    }
    szp_ble_gatts_char_cb_t cb_t=
        {
            .char_read = szp_ble_gatts_char_read_callback_test,
            .char_write = szp_ble_gatts_char_write_callback_test,
        };
    szp_ble_gatts_register_char_cb(cb_t);
    
}


//蓝牙demo
void _4_demo_ble_run(void)
{
    szp_key_init();
    storage_init();
    szp_ble_gatts_register();
    szp_ble_gatts_set_check_enable(true);
    szp_ble_gatts_start();


    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        uint32_t event= szp_key_wait_event(EV_SZP_KEY_CLICKED|EV_SZP_KEY_DOUBLE_CLICKED|EV_SZP_KEY_LONG_HOLDING, pdTRUE, portMAX_DELAY);

        printf("event:%"PRId32"\r\n", event);
        if(event&EV_SZP_KEY_CLICKED)
        {
           esp_err_t ret=  szp_ble_gatts_start();
           printf("szp_ble_gatts_start:%d\r\n", ret);
        }
          if(event&EV_SZP_KEY_DOUBLE_CLICKED)
        {
            esp_err_t ret= szp_ble_gatts_stop();
               printf("szp_ble_gatts_stop:%d\r\n", ret);
               szp_ble_gatts_del_all_char();
        }
        if(event&EV_SZP_KEY_LONG_HOLDING)
        {
            printf("EV_SZP_KEY_LONG_HOLDING\r\n");
            szp_ble_gatts_register();
        }
        
    }
        
}