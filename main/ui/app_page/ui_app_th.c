#include "lvgl.h"

//父界面删除事件
static void ui_app_th_parent_del(lv_event_t *ev)
{
  
}

void ui_app_th_setup(lv_obj_t* parent)
{
    lv_obj_add_event_cb(parent, ui_app_th_parent_del, LV_EVENT_DELETE, NULL);
}