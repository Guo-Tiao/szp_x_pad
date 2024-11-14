#include "szp_weather_api.h"

#include "esp_log.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_tls.h"

#include "cJSON.h"
#include "zlib.h"

#include "common/common_macro.h"
#include "storage/storage_manager.h"
#include "network/network_manager.h"

#define SZP_WEATHER_API_TAG    "SZP_WEATHER_API"

#define SZP_WEATHER_MAX_HTTP_OUTPUT_BUFFER      2048

#define SZP_WEATHER_DAILY_URL   "https://devapi.qweather.com/v7/weather/3d?&location=%s&key=%s"        //3日天气预报URL
#define SZP_WEATHER_NOW_URL     "https://devapi.qweather.com/v7/weather/now?&location=%s&key=%s"    //实时天气URL
#define SZP_WEATHER_AIR_URL         "https://devapi.qweather.com/v7/air/now?&location=%s&key=%s"                   //空气质量URL

char szp_weather_daily_url[256];
char szp_weather_now_url[256];
char szp_weather_air_url[256];

// GZIP解压函数
static int gzip_decompress(char *src, int srcLen, char *dst, int* dstLen)
{
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    strm.avail_in = srcLen;
    strm.avail_out = *dstLen;
    strm.next_in = (Bytef *)src;
    strm.next_out = (Bytef *)dst;

    int err = -1;
    err = inflateInit2(&strm, 31); // 初始化
    if (err == Z_OK)
    {
        err = inflate(&strm, Z_FINISH); // 解压gzip数据
        if (err == Z_STREAM_END) // 解压成功
        { 
            *dstLen = strm.total_out; 
        } 
        else // 解压失败
        {
        }
        inflateEnd(&strm);
    } 
    else
    {
        ESP_LOGE(SZP_WEATHER_API_TAG,"gzip_decompress inflateInit2 err! err=%d", err);
    }
    return err;
}

//HTTP事件回调
static esp_err_t szp_weather_http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer; 
    static int output_len;      
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
    case HTTP_EVENT_ON_CONNECTED:
    case HTTP_EVENT_HEADER_SENT:
    case HTTP_EVENT_ON_HEADER:
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(SZP_WEATHER_API_TAG, "szp_weather_http_event_handler event:%d",evt->event_id);
        break;
    case HTTP_EVENT_ON_DATA:
    {
        ESP_LOGD(SZP_WEATHER_API_TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            int copy_len = 0;
            if (evt->user_data)
            {
                copy_len = MIN(evt->data_len, (SZP_WEATHER_MAX_HTTP_OUTPUT_BUFFER - output_len));
                if (copy_len)
                {
                    memcpy(evt->user_data + output_len, evt->data, copy_len);
                }
            }
            else
            {
                const int buffer_len = esp_http_client_get_content_length(evt->client);
                if (output_buffer == NULL)
                {
                    output_buffer = (char *)malloc(buffer_len);
                    output_len = 0;
                    if (output_buffer == NULL)
                    {
                        ESP_LOGE(SZP_WEATHER_API_TAG, "Failed to allocate memory for output buffer");
                        return ESP_FAIL;
                    }
                }
                copy_len = MIN(evt->data_len, (buffer_len - output_len));
                if (copy_len)
                {
                    memcpy(output_buffer + output_len, evt->data, copy_len);
                }
            }
            output_len += copy_len;
        }
    }
        break;
    case HTTP_EVENT_ON_FINISH:
    {
        ESP_LOGD(SZP_WEATHER_API_TAG, "HTTP_EVENT_ON_FINISH");
        if (output_buffer != NULL)
        {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
    }
        break;
    case HTTP_EVENT_DISCONNECTED:
    {
        ESP_LOGI(SZP_WEATHER_API_TAG, "HTTP_EVENT_DISCONNECTED");
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
        if (err != 0)
        {
            ESP_LOGI(SZP_WEATHER_API_TAG, "Last esp error code: 0x%x", err);
            ESP_LOGI(SZP_WEATHER_API_TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
        }
        if (output_buffer != NULL)
        {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
    }
        break;
    }

    return ESP_OK;
}


//http-get请求
esp_err_t szp_weather_api_http_get(const char* url, char* data,int size)
{
    //配置http
    char local_response_buffer[SZP_WEATHER_MAX_HTTP_OUTPUT_BUFFER] = {0};
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = szp_weather_http_event_handler,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .user_data = local_response_buffer,     
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    //执行http
    esp_err_t ret = esp_http_client_perform(client);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(SZP_WEATHER_API_TAG, "szp_weather_api_http_get-请求错误:%s", esp_err_to_name(ret));
        return ret;
    } 
    //获取请求数据并处理
    int client_code = esp_http_client_get_status_code(client);
    int64_t   gzip_len = esp_http_client_get_content_length(client);
    esp_http_client_cleanup(client);
    if(client_code!=200)
    {
        return ESP_FAIL;
    }
    //对数据进行解压
    int zRet = gzip_decompress(local_response_buffer, gzip_len, data, &size);
    if(Z_STREAM_END!=zRet)
    {
        ESP_LOGE(SZP_WEATHER_API_TAG, "szp_weather_api_http_get-数据解压失败:%d", zRet);
        return ESP_FAIL;
    }
    return ESP_OK;
}

void szp_weather_update_url(void)
{
    char key[64];
    szp_nvs_read_str(Nvs_NameSpace_Network, Nvs_Key_Weather_Api_Key, &key, 64);
    char city[32];
    szp_nvs_read_str(Nvs_NameSpace_Network, Nvs_Key_Weather_Api_City, &city, 32);

    sprintf(szp_weather_daily_url, SZP_WEATHER_DAILY_URL,city,key);
    sprintf(szp_weather_now_url, SZP_WEATHER_NOW_URL, city,key);
    sprintf(szp_weather_air_url, SZP_WEATHER_AIR_URL,city, key);
}

esp_err_t szp_weather_api_get_daily(SzpWeatherDaily *daily)
{
    if(daily==NULL)
    {
        return ESP_FAIL;
    }
    int data_size = SZP_WEATHER_MAX_HTTP_OUTPUT_BUFFER;
    char* data = (char*)malloc(SZP_WEATHER_MAX_HTTP_OUTPUT_BUFFER);
    memset(data, 0, SZP_WEATHER_MAX_HTTP_OUTPUT_BUFFER);
    esp_err_t ret= szp_weather_api_http_get(szp_weather_daily_url, data, data_size);//URL需要修改
    if(ret!=ESP_OK)
    {
        free(data);
        return ESP_FAIL;
    }
    //解析返回的数据
    cJSON *json_root = cJSON_Parse(data);
    cJSON *json_daily = cJSON_GetObjectItem(json_root,"daily");
    cJSON *json_today = cJSON_GetArrayItem(json_daily, 0);
    if(json_today==NULL)
    {
        cJSON_Delete(json_root);
        ESP_LOGE(SZP_WEATHER_API_TAG, "szp_weather_api_get_daily-解析json失败");
        free(data);
        return ESP_FAIL;
    }
    char *temp_max = cJSON_GetObjectItem(json_today,"tempMax")->valuestring;
    char *temp_min = cJSON_GetObjectItem(json_today,"tempMin")->valuestring;
    char *sunset = cJSON_GetObjectItem(json_today,"sunset")->valuestring;
    char *sunrise = cJSON_GetObjectItem(json_today,"sunrise")->valuestring;
    //赋值数据
    daily->temp_max= atoi(temp_max);
    daily->temp_min= atoi(temp_min);
    strcpy(daily->sunrise_time, sunrise);
    strcpy(daily->sunset_time, sunset);
    cJSON_Delete(json_root);
    free(data);
    return ESP_OK;
}

esp_err_t szp_weather_api_get_now(SzpWeatherNow *now)
{
    if(now==NULL)
    {
        return ESP_FAIL;
    }
    int data_size = SZP_WEATHER_MAX_HTTP_OUTPUT_BUFFER;
    char* data = (char*)malloc(SZP_WEATHER_MAX_HTTP_OUTPUT_BUFFER);
    memset(data, 0, SZP_WEATHER_MAX_HTTP_OUTPUT_BUFFER);
    esp_err_t ret= szp_weather_api_http_get(szp_weather_now_url, data, data_size);//URL需要修改
    if(ret!=ESP_OK)
    {
        free(data);
        return ESP_FAIL;
    }
    //解析返回的数据

    cJSON *json_root = cJSON_Parse(data);
    cJSON *json_now = cJSON_GetObjectItem(json_root,"now");
    if(json_now==NULL)
    {
        cJSON_Delete(json_root);
        ESP_LOGE(SZP_WEATHER_API_TAG, "szp_weather_api_get_now-解析json失败");
        free(data);
        return ESP_FAIL;
    }
    char *temp = cJSON_GetObjectItem(json_now,"temp")->valuestring;
    char *humidity = cJSON_GetObjectItem(json_now,"humidity")->valuestring;
    char *icon = cJSON_GetObjectItem(json_now, "icon")->valuestring;
    //赋值数据
    now->temp= atoi(temp);
    now->humi= atoi(humidity);
    now->weather_type= atoi(icon);
    cJSON_Delete(json_root);
    free(data);
    return ESP_OK;
}

esp_err_t szp_weather_api_get_air(SzpWeatherAir *air)
{
    if(air==NULL)
    {
        return ESP_FAIL;
    }
    int data_size = SZP_WEATHER_MAX_HTTP_OUTPUT_BUFFER;
    char* data = (char*)malloc(SZP_WEATHER_MAX_HTTP_OUTPUT_BUFFER);
    memset(data, 0, SZP_WEATHER_MAX_HTTP_OUTPUT_BUFFER);
    esp_err_t ret= szp_weather_api_http_get(szp_weather_air_url, data, data_size);//URL需要修改
    if(ret!=ESP_OK)
    {
        free(data);
        return ESP_FAIL;
    }
    //解析返回的数据

    cJSON *json_root = cJSON_Parse(data);
    cJSON *json_now = cJSON_GetObjectItem(json_root,"now");
    if(json_now==NULL)
    {
        cJSON_Delete(json_root);
        ESP_LOGE(SZP_WEATHER_API_TAG, "szp_weather_api_get_air-解析json失败");
        free(data);
        return ESP_FAIL;
    }
    char *level = cJSON_GetObjectItem(json_now, "level")->valuestring;
    char *pm2p5 = cJSON_GetObjectItem(json_now, "pm2p5")->valuestring;
    //赋值数据
    air->level= atoi(level);
    air->pm25= atoi(pm2p5);
    cJSON_Delete(json_root);
    free(data);
    return ESP_OK;
}
