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


/********************************LV对象成员********************************/
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
 
}

void ui_home_page_setup(lv_obj_t *parent)
{
    //安装首页界面
    ui_home_page_init_complete = false;
    ui_home_page_init(parent);
    ui_home_page_init_complete = true;
}


