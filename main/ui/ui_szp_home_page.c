#include "ui_szp_home_page.h"
#include "lvgl.h"
#include "ui_common_def.h"
#include "common/common_macro.h"
#include "sensor/szp_sensor_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "time.h"

#include "assets/szp_assets_def.h"

/********************************LV对象成员********************************/
bool ui_home_page_init_complete=false;//初始化完成
//首页面成员
static lv_style_t lv_home_page_style;//首页样式
static lv_obj_t * lv_home_page_obj;//首页本体对象


/**首页控件**/
//顶部控件
//天气
lv_obj_t *lv_weather_icon_lb;//天气图标
lv_obj_t *lv_weather_text_lb;//天气文本
lv_obj_t *lv_temp_range_lb;//温度范围
lv_obj_t *lv_sunrise_lb;//日出时间
lv_obj_t *lv_sunset_lb;//日落时间
//空气
lv_obj_t *lv_air_level_obj;//空气质量背景
lv_obj_t *lv_air_levle_lb;//空气质量
lv_obj_t *lv_pm25_lb;//PM2.5值
//时间
lv_obj_t *lv_week_lb;//星期X标签
lv_obj_t *lv_date_lb;//日期标签
lv_obj_t *lv_time_lb;//时间标签
//中心分割线
lv_obj_t *lv_home_center_line; 
//底部控件
lv_obj_t *lv_home_page_gif; //左下角动图
lv_obj_t *indoor_temp_val_lb;//室内温度
lv_obj_t *indoor_humi_val_lb;//室内湿度
lv_obj_t *outdoor_temp_val_lb;//室外温度
lv_obj_t *outdoor_humi_val_lb;//室外湿度

/********************************LV对象成员********************************/
//更新日期
void ui_home_page_upate_date()
{
 if(!ui_home_page_init_complete)
  {
    return;
  }
  time_t now = 0;
  struct tm time_info = {0};
  time(&now);
  localtime_r(&now, &time_info);
  //刷新日期
  lv_label_set_text_fmt(lv_date_lb, "%d年%02d月%02d日", time_info.tm_year+1900, time_info.tm_mon+1, time_info.tm_mday);
  switch (time_info.tm_wday)
  {
      case 0: lv_label_set_text(lv_week_lb, "星期日"); break;
      case 1: lv_label_set_text(lv_week_lb, "星期一"); break;
      case 2: lv_label_set_text(lv_week_lb, "星期二"); break;
      case 3: lv_label_set_text(lv_week_lb, "星期三"); break;
      case 4: lv_label_set_text(lv_week_lb, "星期四"); break; 
      case 5: lv_label_set_text(lv_week_lb, "星期五"); break;
      case 6: lv_label_set_text(lv_week_lb, "星期六"); break;
      default: lv_label_set_text(lv_week_lb, "未知"); break;
  }
}

// 定时器刷新时间
static void timer_cb_update_info(struct _lv_timer_t *)
{
  time_t now = 0;
  struct tm time_info = {0};
  time(&now);
  localtime_r(&now, &time_info);
  char buf[20];
  //刷新时间
  strftime(buf, sizeof(buf), "%H:%M:%S", &time_info);
  lv_label_set_text(lv_time_lb, buf);

  //刷新室内温湿度
  lv_label_set_text_fmt(indoor_temp_val_lb, "%d°C",szp_sensor_th_gxhtc3.iTemp);
  lv_label_set_text_fmt(indoor_humi_val_lb, "%d",szp_sensor_th_gxhtc3.iHumi);
}
//更新天气图标文本
static void update_weather_icon_text(int type)
{
    switch (type)
    {
        case 100: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x81"); lv_label_set_text(lv_weather_text_lb, "晴"); break;
        case 101: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x82"); lv_label_set_text(lv_weather_text_lb, "多云"); break;
        case 102: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x83"); lv_label_set_text(lv_weather_text_lb, "少云"); break;
        case 103: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x84"); lv_label_set_text(lv_weather_text_lb, "晴间多云"); break;
        case 104: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x85"); lv_label_set_text(lv_weather_text_lb, "阴"); break;
        case 150: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x86"); lv_label_set_text(lv_weather_text_lb, "晴"); break;
        case 151: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x87"); lv_label_set_text(lv_weather_text_lb, "多云"); break;
        case 152: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x88"); lv_label_set_text(lv_weather_text_lb, "少云"); break;
        case 153: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x89"); lv_label_set_text(lv_weather_text_lb, "晴间多云"); break;
        case 300: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x8A"); lv_label_set_text(lv_weather_text_lb, "阵雨"); break;
        case 301: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x8B"); lv_label_set_text(lv_weather_text_lb, "强阵雨"); break;
        case 302: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x8C"); lv_label_set_text(lv_weather_text_lb, "雷阵雨"); break;
        case 303: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x8D"); lv_label_set_text(lv_weather_text_lb, "强雷阵雨"); break;
        case 304: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x8E"); lv_label_set_text(lv_weather_text_lb, "雷阵雨伴冰雹"); break;
        case 305: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x8F"); lv_label_set_text(lv_weather_text_lb, "小雨"); break;
        case 306: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x90"); lv_label_set_text(lv_weather_text_lb, "中雨"); break;
        case 307: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x91"); lv_label_set_text(lv_weather_text_lb, "大雨"); break;
        case 308: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x92"); lv_label_set_text(lv_weather_text_lb, "极端降雨"); break;
        case 309: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x93"); lv_label_set_text(lv_weather_text_lb, "毛毛雨"); break;
        case 310: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x94"); lv_label_set_text(lv_weather_text_lb, "暴雨"); break;
        case 311: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x95"); lv_label_set_text(lv_weather_text_lb, "大暴雨"); break;
        case 312: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x96"); lv_label_set_text(lv_weather_text_lb, "特大暴雨"); break;
        case 313: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x97"); lv_label_set_text(lv_weather_text_lb, "冻雨"); break;
        case 314: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x98"); lv_label_set_text(lv_weather_text_lb, "小到中雨"); break;
        case 315: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x99"); lv_label_set_text(lv_weather_text_lb, "中到大雨"); break;
        case 316: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x9A"); lv_label_set_text(lv_weather_text_lb, "大到暴雨"); break;
        case 317: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x9B"); lv_label_set_text(lv_weather_text_lb, "暴雨到大暴雨"); break;
        case 318: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x9C"); lv_label_set_text(lv_weather_text_lb, "大到特大暴雨"); break;
        case 350: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x9D"); lv_label_set_text(lv_weather_text_lb, "阵雨"); break;
        case 351: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x9E"); lv_label_set_text(lv_weather_text_lb, "强阵雨"); break;
        case 399: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\x9F"); lv_label_set_text(lv_weather_text_lb, "雨"); break;
        case 400: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xA0"); lv_label_set_text(lv_weather_text_lb, "小雪"); break;
        case 401: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xA1"); lv_label_set_text(lv_weather_text_lb, "中雪"); break;
        case 402: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xA2"); lv_label_set_text(lv_weather_text_lb, "大雪"); break;
        case 403: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xA3"); lv_label_set_text(lv_weather_text_lb, "暴雪"); break;
        case 404: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xA4"); lv_label_set_text(lv_weather_text_lb, "雨夹雪"); break;
        case 405: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xA5"); lv_label_set_text(lv_weather_text_lb, "雨雪天气"); break;
        case 406: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xA6"); lv_label_set_text(lv_weather_text_lb, "阵雨夹雪"); break;
        case 407: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xA7"); lv_label_set_text(lv_weather_text_lb, "阵雪"); break;
        case 408: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xA8"); lv_label_set_text(lv_weather_text_lb, "小到中雪"); break;
        case 409: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xA9"); lv_label_set_text(lv_weather_text_lb, "中到大雪"); break;
        case 410: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xAA"); lv_label_set_text(lv_weather_text_lb, "大到暴雪"); break;
        case 456: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xAB"); lv_label_set_text(lv_weather_text_lb, "阵雨夹雪"); break;
        case 457: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xAC"); lv_label_set_text(lv_weather_text_lb, "阵雪"); break;
        case 499: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xAD"); lv_label_set_text(lv_weather_text_lb, "雪"); break;
        case 500: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xAE"); lv_label_set_text(lv_weather_text_lb, "薄雾"); break;
        case 501: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xAF"); lv_label_set_text(lv_weather_text_lb, "雾"); break;
        case 502: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xB0"); lv_label_set_text(lv_weather_text_lb, "霾"); break;
        case 503: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xB1"); lv_label_set_text(lv_weather_text_lb, "扬沙"); break;
        case 504: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xB2"); lv_label_set_text(lv_weather_text_lb, "浮尘"); break;
        case 507: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xB3"); lv_label_set_text(lv_weather_text_lb, "沙尘暴"); break;
        case 508: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xB4"); lv_label_set_text(lv_weather_text_lb, "强沙尘暴"); break;
        case 509: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xB5"); lv_label_set_text(lv_weather_text_lb, "浓雾"); break;
        case 510: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xB6"); lv_label_set_text(lv_weather_text_lb, "强浓雾"); break;
        case 511: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xB7"); lv_label_set_text(lv_weather_text_lb, "中度霾"); break;
        case 512: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xB8"); lv_label_set_text(lv_weather_text_lb, "重度霾"); break;
        case 513: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xB9"); lv_label_set_text(lv_weather_text_lb, "严重霾"); break;
        case 514: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xBA"); lv_label_set_text(lv_weather_text_lb, "大雾"); break;
        case 515: lv_label_set_text(lv_weather_icon_lb, "\xEF\x84\xBB"); lv_label_set_text(lv_weather_text_lb, "特强浓雾"); break;
        case 900: lv_label_set_text(lv_weather_icon_lb, "\xEF\x85\x84"); lv_label_set_text(lv_weather_text_lb, "热"); break;
        case 901: lv_label_set_text(lv_weather_icon_lb, "\xEF\x85\x85"); lv_label_set_text(lv_weather_text_lb, "冷"); break;
    
        default:
            lv_label_set_text(lv_weather_icon_lb, "\xEF\x85\x86");
            lv_label_set_text(lv_weather_text_lb, "未知天气");
            break;
    }
} 
//更新空气质量
static void update_air_level(int level)
{
    switch (level)
    {
        case 1: 
            lv_label_set_text(lv_air_levle_lb, "优"); 
            lv_obj_set_style_bg_color(lv_air_level_obj, lv_palette_main(LV_PALETTE_GREEN), 0); 
            lv_obj_set_style_text_color(lv_air_levle_lb, lv_color_hex(0xFFFFFF), 0);
            break;
        case 2: 
            lv_label_set_text(lv_air_levle_lb, "良"); 
            lv_obj_set_style_bg_color(lv_air_level_obj, lv_palette_main(LV_PALETTE_YELLOW), 0); 
            lv_obj_set_style_text_color(lv_air_levle_lb, lv_color_hex(0x000000), 0);
            break;
        case 3: 
            lv_label_set_text(lv_air_levle_lb, "轻");
            lv_obj_set_style_bg_color(lv_air_level_obj, lv_palette_main(LV_PALETTE_ORANGE), 0); 
            lv_obj_set_style_text_color(lv_air_levle_lb, lv_color_hex(0xFFFFFF), 0); 
            break;
        case 4: 
            lv_label_set_text(lv_air_levle_lb, "中"); 
            lv_obj_set_style_bg_color(lv_air_level_obj, lv_palette_main(LV_PALETTE_RED), 0); 
            lv_obj_set_style_text_color(lv_air_levle_lb, lv_color_hex(0xFFFFFF), 0);
            break; 
        case 5: 
            lv_label_set_text(lv_air_levle_lb, "重"); 
            lv_obj_set_style_bg_color(lv_air_level_obj, lv_palette_main(LV_PALETTE_PURPLE), 0); 
            lv_obj_set_style_text_color(lv_air_levle_lb, lv_color_hex(0xFFFFFF), 0);
            break;
        case 6: 
            lv_label_set_text(lv_air_levle_lb, "严"); 
            lv_obj_set_style_bg_color(lv_air_level_obj, lv_palette_main(LV_PALETTE_BROWN), 0); 
            lv_obj_set_style_text_color(lv_air_levle_lb, lv_color_hex(0xFFFFFF), 0);
            break;
        default: 
            lv_label_set_text(lv_air_levle_lb, "未"); 
            lv_obj_set_style_bg_color(lv_air_level_obj, lv_palette_main(LV_PALETTE_GREEN), 0); 
            lv_obj_set_style_text_color(lv_air_levle_lb, lv_color_hex(0xFFFFFF), 0);
            break;
    }
}
//更新天气
void ui_home_page_weather_update_info(SzpWeatherInfo info)
{
  if(!ui_home_page_init_complete)
  {
    return;
  }
  //更新天气图标文本
  update_weather_icon_text(info.now.weather_type);
  //更新室外温湿度
  lv_label_set_text_fmt(outdoor_temp_val_lb, "%d°C",info.now.temp);
  lv_label_set_text_fmt(outdoor_humi_val_lb, "%d",info.now.humi);
  //更新日出日落时间
  lv_label_set_text_fmt(lv_sunrise_lb, "%s", info.daily.sunrise_time);
  lv_label_set_text_fmt(lv_sunset_lb, "%s", info.daily.sunset_time);
  //更新温度范围
  lv_label_set_text_fmt(lv_temp_range_lb, "%d-%d°C", info.daily.temp_min, info.daily.temp_max);
  //更新空气
  update_air_level(info.air.level);
  lv_label_set_text_fmt(lv_pm25_lb, "PM2.5:%d", info.air.pm25);
}
static void ui_home_page_top_init()
{
  //左侧上方日出
  lv_obj_t* lv_sunrise_lbs = lv_label_create(lv_home_page_obj);
  lv_obj_set_style_text_font(lv_sunrise_lbs, &font_szp_Harmony_14, 0);
  lv_obj_align(lv_sunrise_lbs, LV_ALIGN_TOP_LEFT, 15, 5);
  lv_label_set_text(lv_sunrise_lbs, "日出");
  lv_sunrise_lb = lv_label_create(lv_home_page_obj);
  lv_obj_set_style_text_font(lv_sunrise_lb, &font_szp_Harmony_14, 0);
  lv_obj_align_to(lv_sunrise_lb,lv_sunrise_lbs, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
  lv_label_set_text(lv_sunrise_lb, "06:00");
  //左侧下方天气
  lv_weather_icon_lb = lv_label_create(lv_home_page_obj);
  lv_obj_set_style_text_font(lv_weather_icon_lb, &icon_szp_weather, 0);
  lv_obj_align_to(lv_weather_icon_lb,lv_sunrise_lb, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
  lv_label_set_text(lv_weather_icon_lb, "\xEF\x85\x86");
  lv_weather_text_lb = lv_label_create(lv_home_page_obj);
  lv_obj_set_style_text_font(lv_weather_text_lb, &font_szp_Harmony_12, 0);
  lv_obj_align_to(lv_weather_text_lb, lv_weather_icon_lb,LV_ALIGN_OUT_BOTTOM_LEFT, 0, 3);
  lv_label_set_text(lv_weather_text_lb, "未知天气");

  //右侧上方日落
  lv_obj_t *lv_sunset_lbs = lv_label_create(lv_home_page_obj);
  lv_obj_set_style_text_font(lv_sunset_lbs, &font_szp_Harmony_14, 0);
  lv_obj_align(lv_sunset_lbs, LV_ALIGN_TOP_RIGHT, -15, 5);
  lv_label_set_text(lv_sunset_lbs, "日落");
  lv_sunset_lb = lv_label_create(lv_home_page_obj);
  lv_obj_set_style_text_font(lv_sunset_lb, &font_szp_Harmony_14, 0);
  lv_obj_align_to(lv_sunset_lb,lv_sunset_lbs, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 10);
  lv_label_set_text(lv_sunset_lb, "18:00");
  //右侧下方温度
  lv_obj_t* lv_temp_lb= lv_label_create(lv_home_page_obj);
  lv_obj_set_style_text_font(lv_temp_lb, &font_szp_Harmony_14, 0);
  lv_obj_align(lv_temp_lb,LV_ALIGN_RIGHT_MID, -13, -30);
  lv_label_set_text(lv_temp_lb, "今日温度");
  lv_temp_range_lb= lv_label_create(lv_home_page_obj);
  lv_obj_set_style_text_font(lv_temp_range_lb, &font_szp_Harmony_14, 0);
  lv_obj_align_to(lv_temp_range_lb,lv_temp_lb,LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
  lv_label_set_text(lv_temp_range_lb, "未知");

  //中心时间
  lv_time_lb = lv_label_create(lv_home_page_obj);
  lv_obj_set_style_text_font(lv_time_lb, &font_szp_digital, 0);
  lv_label_set_text(lv_time_lb, "00:00:00");
  lv_obj_align(lv_time_lb, LV_ALIGN_CENTER, 0, -50);
  lv_timer_create(timer_cb_update_info, 500, NULL);
  //时间上侧日期
  lv_date_lb=lv_label_create(lv_home_page_obj);
  lv_obj_set_style_text_font(lv_date_lb, &font_szp_Harmony_14, 0);
  lv_label_set_text(lv_date_lb, "2024年1月1日");
  lv_obj_align_to(lv_date_lb, lv_time_lb,LV_ALIGN_OUT_TOP_LEFT, 0, -10);
  lv_week_lb = lv_label_create(lv_home_page_obj);
  lv_obj_set_style_text_font(lv_week_lb, &font_szp_Harmony_14, 0);
  lv_label_set_text(lv_week_lb, "星期一");
  lv_obj_align_to(lv_week_lb, lv_time_lb,LV_ALIGN_OUT_TOP_RIGHT, 0, -10);

  //时间下侧空气
  static lv_style_t lv_air_level_style;
  lv_style_init(&lv_air_level_style);
  lv_style_set_radius(&lv_air_level_style, 10);                                  // 设置圆角半径
  lv_style_set_bg_color(&lv_air_level_style, lv_palette_main(LV_PALETTE_GREEN)); // 绿色
  lv_style_set_text_color(&lv_air_level_style, lv_color_hex(0xffffff));          // 白色
  lv_style_set_border_width(&lv_air_level_style, 0);
  lv_style_set_pad_all(&lv_air_level_style, 0);
  lv_style_set_width(&lv_air_level_style, 50);  // 设置宽
  lv_style_set_height(&lv_air_level_style, 26); // 设置高
  lv_air_level_obj = lv_obj_create(lv_home_page_obj);
  lv_obj_add_style(lv_air_level_obj, &lv_air_level_style, 0);
  lv_obj_align_to(lv_air_level_obj, lv_time_lb,LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
  lv_air_levle_lb = lv_label_create(lv_air_level_obj);
  lv_obj_set_style_text_font(lv_air_levle_lb, &font_szp_Harmony_16, 0);
  lv_label_set_text(lv_air_levle_lb, "无");
  lv_obj_align(lv_air_levle_lb, LV_ALIGN_CENTER, 0, 0);

  lv_pm25_lb = lv_label_create(lv_home_page_obj);
  lv_obj_set_style_text_font(lv_pm25_lb, &font_szp_Harmony_16, 0);
  lv_label_set_text(lv_pm25_lb, "PM2.5:0");
  lv_obj_align_to(lv_pm25_lb, lv_air_level_obj,LV_ALIGN_OUT_RIGHT_BOTTOM, 5, 0);
}
static void ui_home_page_down_init()
{
  //创建左下角GIF
  lv_home_page_gif = lv_gif_create(lv_home_page_obj);
  lv_gif_set_src(lv_home_page_gif, &gif_szp_duckyo);
  lv_obj_align(lv_home_page_gif, LV_ALIGN_BOTTOM_LEFT, 10, -5);
  
  //室内温湿度
  //室内控件块
  static lv_style_t indoor_style;
  lv_style_init(&indoor_style);
  lv_style_set_radius(&indoor_style, 10);  // 设置圆角半径
  lv_style_set_bg_color(&indoor_style, lv_color_hex(0xFFF0C8)); 
  lv_style_set_text_color(&indoor_style, lv_color_hex(0x112233)); 
  lv_style_set_border_width(&indoor_style, 0);
  lv_style_set_pad_all(&indoor_style, 5);
  lv_style_set_width(&indoor_style, 210);  // 设置宽
  lv_style_set_height(&indoor_style, 35); // 设置高
  lv_obj_t * indoor_obj = lv_obj_create(lv_home_page_obj);
  lv_obj_add_style(indoor_obj, &indoor_style, 0);
  lv_obj_align(indoor_obj, LV_ALIGN_BOTTOM_RIGHT, -10, -5);
  lv_obj_set_flex_flow(indoor_obj, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(indoor_obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  //室内温度
  lv_obj_t *indoor_temp_txt_lb = lv_label_create(indoor_obj);
  lv_obj_set_style_text_font(indoor_temp_txt_lb, &font_szp_Harmony_12, 0);
  lv_label_set_text(indoor_temp_txt_lb, "室内温度:");
  indoor_temp_val_lb = lv_label_create(indoor_obj);
  lv_obj_set_style_text_font(indoor_temp_val_lb, &font_szp_Harmony_12, 0);
  lv_label_set_text(indoor_temp_val_lb, "未知");
  //室内湿度
  lv_obj_t *indoor_humi_txt_lb = lv_label_create(indoor_obj);
  lv_obj_set_style_text_font(indoor_humi_txt_lb, &font_szp_Harmony_12, 0);
  lv_label_set_text(indoor_humi_txt_lb, "室内湿度:");
  indoor_humi_val_lb = lv_label_create(indoor_obj);
  lv_obj_set_style_text_font(indoor_humi_val_lb, &font_szp_Harmony_12, 0);
  lv_label_set_text(indoor_humi_val_lb, "未知");

  // 室外温湿度
  // 室外控件块
   static lv_style_t outdoor_style;
  lv_style_init(&outdoor_style);
  lv_style_set_radius(&outdoor_style, 10);  // 设置圆角半径
  lv_style_set_bg_color(&outdoor_style, lv_color_hex(0xF0FF8C)); 
  lv_style_set_text_color(&outdoor_style, lv_color_hex(0x112233)); 
  lv_style_set_border_width(&outdoor_style, 0);
  lv_style_set_pad_all(&outdoor_style, 5);
  lv_style_set_width(&outdoor_style, 210);  // 设置宽
  lv_style_set_height(&outdoor_style, 35); // 设置高
  lv_obj_t * outdoor_obj = lv_obj_create(lv_home_page_obj);
  lv_obj_add_style(outdoor_obj, &outdoor_style, 0);
  lv_obj_align_to(outdoor_obj, indoor_obj,LV_ALIGN_OUT_TOP_MID, 0, -5);
  lv_obj_set_flex_flow(outdoor_obj, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(outdoor_obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  //室外温度
  lv_obj_t *outdoor_temp_txt_lb = lv_label_create(outdoor_obj);
  lv_obj_set_style_text_font(outdoor_temp_txt_lb, &font_szp_Harmony_12, 0);
  lv_label_set_text(outdoor_temp_txt_lb, "室外温度:");
  outdoor_temp_val_lb = lv_label_create(outdoor_obj);
  lv_obj_set_style_text_font(outdoor_temp_val_lb, &font_szp_Harmony_12, 0);
  lv_label_set_text(outdoor_temp_val_lb, "未知");
  //室外湿度
  lv_obj_t *outdoor_humi_txt_lb = lv_label_create(outdoor_obj);
  lv_obj_set_style_text_font(outdoor_humi_txt_lb, &font_szp_Harmony_12, 0);
  lv_label_set_text(outdoor_humi_txt_lb, "室外湿度:");
  outdoor_humi_val_lb = lv_label_create(outdoor_obj);
  lv_obj_set_style_text_font(outdoor_humi_val_lb, &font_szp_Harmony_12, 0);
  lv_label_set_text(outdoor_humi_val_lb, "未知");
}

//初始化首页界面
static void ui_home_page_init(lv_obj_t *parent)
{
  //初始化首页样式和对象
  lv_style_init(&lv_home_page_style);
  lv_style_set_radius(&lv_home_page_style, 0);
  lv_style_set_bg_opa(&lv_home_page_style, LV_OPA_TRANSP);
  lv_style_set_border_width(&lv_home_page_style, 0);
  lv_style_set_pad_all(&lv_home_page_style, 0);
  lv_style_set_width(&lv_home_page_style, SZP_LV_UI_HOR);  
  lv_style_set_height(&lv_home_page_style, SZP_LV_UI_VER-lv_obj_get_height(szp_ui_get_sys_title()));

  //初始化首页对象
  lv_home_page_obj = lv_obj_create(parent);
  lv_obj_add_style(lv_home_page_obj, &lv_home_page_style, 0);
  lv_obj_align(lv_home_page_obj, LV_ALIGN_BOTTOM_MID, 0, 0);

 /**************顶部**************/
  ui_home_page_top_init();

  /**************中间分割线**************/
  static lv_point_t line_points[] = {{0, 0}, {300, 0}};
  static lv_style_t style_line;
  lv_style_init(&style_line);
  lv_style_set_line_width(&style_line, 2);
  lv_style_set_line_opa(&style_line, LV_OPA_50);
  lv_style_set_line_color(&style_line, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_line_rounded(&style_line, true);

  lv_home_center_line = lv_line_create(lv_home_page_obj);
  lv_line_set_points(lv_home_center_line, line_points, 2); 
  lv_obj_add_style(lv_home_center_line, &style_line, 0);
  lv_obj_align(lv_home_center_line, LV_ALIGN_CENTER, 0, 15);

  /**************底部**************/
  ui_home_page_down_init();
 
}

void ui_home_page_setup(lv_obj_t *parent)
{
    //安装首页界面
    ui_home_page_init_complete = false;
    ui_home_page_init(parent);
    ui_home_page_init_complete = true;
}


