#include "szp_lcd.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "common_macro.h"

#include "lvgl.h"
#include "szp_lcd.h"
#include "szp_touch.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"

szp_lcd_on_flush_ready_cb on_szp_lcd_on_flush_ready_cb;
esp_lcd_panel_handle_t szp_lcd_panel_handle = NULL;
//屏幕IO
#define SZP_LCD_BACK_LIGHT_IO_NUM   2   //屏幕背光IO
#define SZP_LCD_BACK_LIGHT_ON_LEVEL  0  //屏幕背光开启电平
#define SZP_LCD_BACK_LIGHT_OFF_LEVEL !SZP_LCD_BACK_LIGHT_ON_LEVEL

#define SZP_LCD_DC_IO_NUM                   6
#define SZP_LCD_RST_IO_NUM                  -1
#define SZP_LCD_CS_IO_NUM                     4
//屏幕SPI
#define SZP_LCD_SPI_HOST                    SPI2_HOST   //spi号
#define SZP_LCD_SPI_SCLK_NUM           3    //spi-sclk引脚
#define SZP_LCD_SPI_MOSI_NUM           5   //spi-mosi引脚
#define SZP_LCD_SPI_MISO_NUM          -1  //spi-miso引脚


//转换完成回调
static bool szp_lcd_on_color_trans_done_cb(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    if(on_szp_lcd_on_flush_ready_cb!=NULL)
    {
        on_szp_lcd_on_flush_ready_cb();
    }
    return false;
}
//回调注册
void szp_lcd_flush_ready_cb_register(szp_lcd_on_flush_ready_cb cb)
{
    on_szp_lcd_on_flush_ready_cb = cb;
}

// 绘制图像
void szp_lcd_draw_bitmap(const void *bitmap,int x_start, int y_start, int x_end, int y_end)
{
    esp_lcd_panel_draw_bitmap(szp_lcd_panel_handle, x_start, y_start, x_end, y_end, bitmap);
}

extern void example_lvgl_demo_ui(lv_disp_t *disp);

//屏幕初始化
esp_err_t szp_lcd_init(void)
{
    //初始化屏幕SPI通讯
    spi_bus_config_t spi_cfg = {
        .sclk_io_num = SZP_LCD_SPI_SCLK_NUM,
        .mosi_io_num = SZP_LCD_SPI_MOSI_NUM,
        .miso_io_num = SZP_LCD_SPI_MISO_NUM,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SZP_LCD_H_RES * 80 * sizeof(uint16_t),
    };
    SZP_ESP_ERR_CHECK(spi_bus_initialize(SZP_LCD_SPI_HOST, &spi_cfg, SPI_DMA_CH_AUTO));

    //初始化ESP-LCD-Panel
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = SZP_LCD_DC_IO_NUM,
        .cs_gpio_num = SZP_LCD_CS_IO_NUM,
        .pclk_hz = SZP_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = SZP_LCD_CMD_BITS,
        .lcd_param_bits = SZP_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = szp_lcd_on_color_trans_done_cb,
    };
    SZP_ESP_ERR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SZP_LCD_SPI_HOST, &io_config, &io_handle));

    //初始化st7789驱动
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = SZP_LCD_RST_IO_NUM,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    SZP_ESP_ERR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &szp_lcd_panel_handle));

    //LCD画板初始化
    SZP_ESP_ERR_CHECK(esp_lcd_panel_reset(szp_lcd_panel_handle));
    SZP_ESP_ERR_CHECK(esp_lcd_panel_init(szp_lcd_panel_handle));
    SZP_ESP_ERR_CHECK(esp_lcd_panel_mirror(szp_lcd_panel_handle, false, false));
    SZP_ESP_ERR_CHECK(esp_lcd_panel_invert_color(szp_lcd_panel_handle, true));
    SZP_ESP_ERR_CHECK(esp_lcd_panel_disp_on_off(szp_lcd_panel_handle, true));
    //背光初始化
    //初始化屏幕背光
    gpio_config_t bl_gpio_config = 
    {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << SZP_LCD_BACK_LIGHT_IO_NUM
    };
    SZP_ESP_ERR_CHECK(gpio_config(&bl_gpio_config));
    gpio_set_level(SZP_LCD_BACK_LIGHT_IO_NUM, SZP_LCD_BACK_LIGHT_ON_LEVEL);

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_13_BIT, // Set duty resolution to 13 bits,
        .freq_hz          = 5000, // Frequency in Hertz. Set frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    SZP_ESP_ERR_CHECK(ledc_timer_config(&ledc_timer));
    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SZP_LCD_BACK_LIGHT_IO_NUM,
        .duty           = 0, // Set duty
        .hpoint         = 0
    };
    SZP_ESP_ERR_CHECK(ledc_channel_config(&ledc_channel));
    // 设置占空比
    SZP_ESP_ERR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 8191*(0.5)));  
    // 更新背光
    SZP_ESP_ERR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
    
    return ESP_OK;
}