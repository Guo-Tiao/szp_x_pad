#pragma once
#include "esp_err.h"
//天气API


//全天天气信息
typedef struct szp_weather_daily_t
{
    char sunrise_time[10];  // 当天日出时间
    char sunset_time[10];   // 当天日落时间
    int    temp_max;             //当天最高气温
    int    temp_min;               // 当天最低气温
}SzpWeatherDaily;

//实时天气信息
typedef struct szp_weather_now_t
{
    int    temp;            //实时温度
    int    humi;            //实时湿度
    int    weather_type;      //实时天气类型
}SzpWeatherNow;
   
 //空气质量
typedef struct szp_weather_air_t
{
    int    level;           //空气质量
    int    pm25;         //PM2.5范围
}SzpWeatherAir;

typedef struct szp_weather_info_t
{
    SzpWeatherDaily   daily;    //每日天气
    SzpWeatherNow   now;    //实时天气
    SzpWeatherAir       air;       //空气质量
}SzpWeatherInfo;
//更新地址
void szp_weather_update_url(void);
//获取每日天气
esp_err_t szp_weather_api_get_daily(SzpWeatherDaily *daily);
//获取当前天气
esp_err_t szp_weather_api_get_now(SzpWeatherNow *now);
//获取空气质量
esp_err_t szp_weather_api_get_air(SzpWeatherAir *air);
