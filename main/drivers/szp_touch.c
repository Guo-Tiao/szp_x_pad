#include "szp_touch.h"
#include "esp_lcd_touch_ft5x06/esp_lcd_touch_ft5x06.h"
#include "common/common_macro.h"

#include "szp_lcd.h"
#include "sdkconfig.h"

esp_lcd_touch_handle_t szp_touch_handle = NULL;
esp_err_t szp_touch_init(void)
{
    // 初始化触摸屏
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
    esp_lcd_touch_config_t tp_cfg = 
    {

#if CONFIG_SZP_LVGL_VER_DISP
    .x_max = SZP_LCD_H_RES,
    .y_max = SZP_LCD_V_RES,
    .flags = {
            .swap_xy = 0,                                           
            .mirror_x = 0,
            .mirror_y = 0,
        },
#else
    .x_max =SZP_LCD_V_RES ,
    .y_max = SZP_LCD_H_RES,
    .flags = {
            .swap_xy = 1,                                           
            .mirror_x = 1,
            .mirror_y = 0,
        },
#endif
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
     
    };
    SZP_ESP_ERR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg,&szp_touch_handle));
    
    return ESP_OK;
}

bool szp_touch_get_coord(uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    esp_lcd_touch_read_data(szp_touch_handle);
    return esp_lcd_touch_get_coordinates(szp_touch_handle,x, y, strength, point_num, max_point_num);
}
