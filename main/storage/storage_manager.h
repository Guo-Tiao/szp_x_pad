#pragma once
#include "esp_err.h"

//存储初始化
void storage_init(void);


/**************************** NSV操作 ****************************/
/** 从nvs中读取字符值
 * @param namespace NVS命名空间
 * @param key 要读取的键值
 * @param value 读到的值
 * @param maxlen 外部存储数组的最大值
 * @return 读取到的字节数
*/
size_t szp_nvs_read_str(const char *namespace, const char *key, char *value, int maxlen);
/** 写入字符值到NVS
 * @param namespace NVS命名空间
 * @param key NVS键值
 * @param value 需要写入的值
 * @return ESP_OK or ESP_FAIL
*/
esp_err_t szp_nvs_write_str(const char *namespace, const char *key, const char *value);

/** 从nvs中读取bool
 * @param namespace NVS命名空间
 * @param key 要读取的键值
 * @param value 读到的值
 * @param maxlen 外部存储数组的最大值
 * @return 读取到的字节数
*/
size_t szp_nvs_read_blob(const char *namespace, const char *key, uint8_t *value, int maxlen);
/** 写入bool值到NVS
 * @param namespace NVS命名空间
 * @param key NVS键值
 * @param value 需要写入的值
 * @return ESP_OK or ESP_FAIL
*/
esp_err_t szp_nvs_write_blob(const char *namespace, const char *key, uint8_t *value, size_t len);

/** 擦除nvs区中某个键
 * @param namespace NVS命名空间
 * @param key 要读取的键值
 * @return 错误值
*/
esp_err_t szp_nvs_erase_key(const char *namespace, const char *key);
/** 擦除nvs区中某个空间下所有数据
 * @param namespace NVS命名空间
 * @return 错误值
*/
esp_err_t szp_nvs_erase_all(const char *namespace);