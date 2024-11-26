#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "common/common_macro.h"
#include "assets/szp_assets_def.h"

lv_obj_t *lv_uart_log_obj; //串口日志显示

lv_obj_t *lv_uart_send_area;//发送区域
lv_obj_t *lv_uart_send_txt;//文本框
lv_obj_t *lv_uart_send_btn;//发送按钮

lv_obj_t *lv_uart_keyboard;//发送小键盘

TaskHandle_t task_ui_app_uart_recv_handle;//接收线程

//添加日志
static void ui_app_uart_add_log(const char *str,bool fRtS)
{
    lv_obj_t *log = lv_label_create(lv_uart_log_obj);
    lv_obj_set_height(log, LV_SIZE_CONTENT);
    lv_label_set_long_mode(log, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_bg_opa(log, LV_OPA_TRANSP, 0);
    if(fRtS)//FALSE:接收(蓝色) TRUE:发送(绿色)
    {
        lv_obj_set_style_text_color(log, lv_palette_main(LV_PALETTE_GREEN), 0);
        lv_label_set_text_fmt(log, "send:%s", str);
    }
    else
    {
        lv_obj_set_style_text_color(log, lv_palette_main(LV_PALETTE_BLUE), 0);
        lv_label_set_text_fmt(log, "recv:%s", str);
    }

    uint32_t cnt = lv_obj_get_child_cnt(lv_uart_log_obj);
    if (cnt > 8) 
    {
        lv_obj_del(lv_obj_get_child(lv_uart_log_obj, 0));
    }
    lv_obj_scroll_to_y(lv_uart_log_obj, LV_COORD_MAX, LV_ANIM_ON);
}


//父界面删除事件
static void ui_app_uart_parent_del(lv_event_t *ev) 
{
    if(task_ui_app_uart_recv_handle)
    {
        vTaskDelete(task_ui_app_uart_recv_handle);
        task_ui_app_uart_recv_handle = NULL;
    }
};


static void ui_app_uart_send_btn_cb(lv_event_t *ev)
{
    const char *str = lv_textarea_get_text(lv_uart_send_txt);
    printf("%s\n", str);
    ui_app_uart_add_log(str, true);
    lv_textarea_set_text(lv_uart_send_txt, "");

}



//显示键盘
static void ui_app_uart_kb_show_cb(lv_event_t *ev)
{
    lv_obj_clear_flag(lv_uart_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(lv_uart_send_area, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_align(lv_uart_log_obj, LV_ALIGN_BOTTOM_MID, 0, -10);
}
//隐藏键盘
static void ui_app_uart_kb_hide_cb(lv_event_t *ev)
{

    uint16_t btn_id         = lv_btnmatrix_get_selected_btn(lv_uart_keyboard);
    if (btn_id == LV_BTNMATRIX_BTN_NONE) return;

    const char *txt = lv_btnmatrix_get_btn_text(lv_uart_keyboard, lv_btnmatrix_get_selected_btn(lv_uart_keyboard));

    if (txt == NULL) return;

    if (strcmp(txt, LV_SYMBOL_KEYBOARD) == 0) 
    {
        lv_obj_add_flag(lv_uart_keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(lv_uart_log_obj, LV_ALIGN_TOP_MID, 0, 10);
        lv_obj_align(lv_uart_send_area, LV_ALIGN_BOTTOM_MID, 0, -10);
        return;
    }
    else if (strcmp(txt, LV_SYMBOL_OK) == 0) 
    {
        ui_app_uart_send_btn_cb(NULL);
        lv_obj_add_flag(lv_uart_keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(lv_uart_log_obj, LV_ALIGN_TOP_MID, 0, 10);
        lv_obj_align(lv_uart_send_area, LV_ALIGN_BOTTOM_MID, 0, -10);
        return;
    }
}

static void task_ui_app_uart_recv_data(void *arg)
{   
      while (1)
    {
        char str[1024];
        int len=scanf("%s", str);
        if (len > 0)
        {
            ui_app_uart_add_log(str, false);
        }
        vTaskDelay(SZP_MS_TO_TICK(200));
    }
}

void ui_app_uart_setup(lv_obj_t* parent)
{
    lv_obj_add_event_cb(parent, ui_app_uart_parent_del, LV_EVENT_DELETE, NULL);

    //初始化日志显示
    static lv_style_t obj_style;
    lv_style_init(&obj_style);
    lv_style_set_radius(&obj_style, 2);
    lv_style_set_bg_opa( &obj_style, LV_OPA_COVER);
    lv_style_set_text_color(&obj_style, lv_color_hex(0xA0A0A0));
    lv_style_set_text_font(&obj_style, &lv_font_montserrat_12);
    lv_style_set_border_width(&obj_style, 0);
    lv_style_set_bg_color(&obj_style, lv_color_hex(0xffffff));

    lv_uart_log_obj = lv_obj_create(parent); // 创建父对象
    lv_obj_add_style(lv_uart_log_obj, &obj_style, 0);
    lv_obj_set_size(lv_uart_log_obj, 250, 170); // 设置父对象大小
    lv_obj_align(lv_uart_log_obj, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_scrollbar_mode(lv_uart_log_obj, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_flex_flow(lv_uart_log_obj, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_all(lv_uart_log_obj, 5, 0);
    lv_obj_set_style_pad_row(lv_uart_log_obj, 5, 0);

    lv_uart_send_area = lv_obj_create(parent);
    lv_obj_set_size(lv_uart_send_area, lv_obj_get_width(parent), 40);
    lv_obj_set_style_radius(lv_uart_send_area,0,0);
    lv_obj_set_style_bg_opa(lv_uart_send_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(lv_uart_send_area, 0, 0);
    lv_obj_set_style_pad_all(lv_uart_send_area, 0, 0);
    lv_obj_align(lv_uart_send_area, LV_ALIGN_BOTTOM_MID, 0, -10);
    //初始化发送文本框
    lv_uart_send_txt = lv_textarea_create(lv_uart_send_area);
    lv_obj_add_style(lv_uart_send_txt, &obj_style, 0);
    lv_obj_set_size(lv_uart_send_txt, 185, 40);
    lv_obj_align(lv_uart_send_txt, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_add_event_cb(lv_uart_send_txt, ui_app_uart_kb_show_cb, LV_EVENT_CLICKED, NULL);
    static lv_style_t btn_style;
    lv_style_init(&btn_style);
    lv_style_set_radius(&btn_style, 5);
    lv_style_set_bg_opa( &btn_style, LV_OPA_COVER);
    lv_style_set_text_color(&btn_style, lv_color_hex(0xA0A0A0));
    lv_style_set_text_font(&btn_style, &lv_font_montserrat_12);
    lv_style_set_border_width(&btn_style, 0);
    lv_style_set_bg_color(&btn_style, lv_palette_main(LV_PALETTE_LIGHT_BLUE));
    lv_uart_send_btn=lv_btn_create(lv_uart_send_area);
    lv_obj_add_style(lv_uart_send_btn, &btn_style,0);
    lv_obj_set_size(lv_uart_send_btn, 60, 40);
    lv_obj_align_to(lv_uart_send_btn, lv_uart_send_txt,LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_add_event_cb(lv_uart_send_btn,ui_app_uart_send_btn_cb  , LV_EVENT_CLICKED, NULL);
   lv_obj_t *img = lv_img_create(lv_uart_send_btn);
    lv_img_set_src(img, &img_szp_uart_send);
    lv_obj_center(img);
    //初始化键盘
    lv_uart_keyboard=  lv_keyboard_create(parent);
    lv_keyboard_set_textarea(lv_uart_keyboard, lv_uart_send_txt);
    lv_obj_add_flag(lv_uart_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(lv_uart_keyboard, ui_app_uart_kb_hide_cb, LV_EVENT_CLICKED, NULL);

    //开启线程
    xTaskCreate(task_ui_app_uart_recv_data, "tk_ui_uart_recv_data", 4096, NULL, 10, task_ui_app_uart_recv_handle);
}