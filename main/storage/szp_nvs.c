#include "storage_manager.h"
#include "nvs_flash.h"
#include "esp_err.h"

#include "common/common_macro.h"

//实战派自定义NVS区
#define SZP_NVS         "szp_nvs"

//从nvs中读取字符值
size_t szp_nvs_read_str(const char* namespace,const char* key,char* value,int maxlen)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret_val = ESP_FAIL;
    size_t required_size = 0;
    SZP_ESP_ERR_CHECK(nvs_open_from_partition(SZP_NVS,namespace, NVS_READWRITE, &nvs_handle));
    ret_val = nvs_get_str(nvs_handle, key, NULL, &required_size);
    if(ret_val == ESP_OK && required_size <= maxlen)
    {
        nvs_get_str(nvs_handle,key,value,&required_size);
    }
    else
        required_size = 0;
    nvs_close(nvs_handle);
    return required_size;
}

//写入字符值到NVS
esp_err_t szp_nvs_write_str(const char* namespace,const char* key,const char* value)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;
    SZP_ESP_ERR_CHECK(nvs_open_from_partition(SZP_NVS,namespace, NVS_READWRITE, &nvs_handle));
    
    ret = nvs_set_str(nvs_handle, key, value);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return ret;
}


//从nvs中读取bool
size_t szp_nvs_read_blob(const char* namespace,const char* key,uint8_t *value,int maxlen)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret_val = ESP_FAIL;
    size_t required_size = 0;
    SZP_ESP_ERR_CHECK(nvs_open_from_partition(SZP_NVS,namespace, NVS_READWRITE, &nvs_handle));
    ret_val = nvs_get_blob(nvs_handle, key, NULL, &required_size);
    if(ret_val == ESP_OK && required_size <= maxlen)
    {
        nvs_get_blob(nvs_handle,key,value,&required_size);
    }
    else
        required_size = 0;
    nvs_close(nvs_handle);
    return required_size;
}

//写入bool值到NVS
esp_err_t szp_nvs_write_blob(const char* namespace,const char* key,uint8_t* value,size_t len)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;
    SZP_ESP_ERR_CHECK(nvs_open_from_partition(SZP_NVS,namespace, NVS_READWRITE, &nvs_handle));
    ret = nvs_set_blob(nvs_handle, key, value,len);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return ret;
}

//擦除nvs区中某个键
esp_err_t szp_nvs_erase_key(const char* namespace,const char* key)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret_val = ESP_FAIL;
    SZP_ESP_ERR_CHECK(nvs_open_from_partition(SZP_NVS,namespace, NVS_READWRITE, &nvs_handle));
    ret_val = nvs_erase_key(nvs_handle,key);
    ret_val = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return ret_val;
}
//擦除nvs区中某个空间下所有数据
esp_err_t szp_nvs_erase_all(const char* namespace)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret_val = ESP_FAIL;
    SZP_ESP_ERR_CHECK(nvs_open_from_partition(SZP_NVS,namespace, NVS_READWRITE, &nvs_handle));
    ret_val = nvs_erase_all(nvs_handle);
    ret_val = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return ret_val;
}

void szp_nvs_init(void)
{
    //初始化自带NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        //NVS出现错误，执行擦除
        SZP_ESP_ERR_CHECK(nvs_flash_erase());
        //重新尝试初始化
        SZP_ESP_ERR_CHECK(nvs_flash_init());
    }

    //初始化自定义NVS
    ret = nvs_flash_init_partition(SZP_NVS);
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        //NVS出现错误，执行擦除
        SZP_ESP_ERR_CHECK(nvs_flash_erase_partition(SZP_NVS));
        //重新尝试初始化
        SZP_ESP_ERR_CHECK(nvs_flash_init_partition(SZP_NVS));
    }

}