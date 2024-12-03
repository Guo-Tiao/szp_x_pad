#include "szp_lcd.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "common/common_macro.h"

#include "lvgl.h"
#include "szp_lcd.h"
#include "szp_ioext.h"
#include "szp_touch.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"

/************** LCD背光 **************/
#define SZP_LCD_BACKLIGHT_NUM     (GPIO_NUM_42)             //背光IO口
#define SZP_LCD_LEDC_CH                   LEDC_CHANNEL_0         //背光通道
/**************************************/


/************** LCD定义 **************/
//屏幕SPI
#define SZP_LCD_SPI_HOST                    SPI3_HOST   //spi号

#define SZP_LCD_SPI_SCLK_NUM           GPIO_NUM_41    //spi-sclk引脚
#define SZP_LCD_SPI_MOSI_NUM          GPIO_NUM_40   //spi-mosi引脚

#define SZP_LCD_DC_IO_NUM                   (GPIO_NUM_39)
#define SZP_LCD_CS_IO_NUM                    (GPIO_NUM_NC)
#define SZP_LCD_RST_IO_NUM                  (GPIO_NUM_NC)
#define SZP_LCD_PIXEL_CLOCK_HZ            (80 * 1000 * 1000)   //屏幕像素时钟频率
#define SZP_LCD_CMD_BITS                       (8)           //屏幕命令位
#define SZP_LCD_PARAM_BITS                   (8)           //屏幕参数位

#define SZP_LCD_BITS_PER_PIXEL              (16)        //屏幕像素
/**************************************/
szp_lcd_on_flush_ready_cb on_szp_lcd_on_flush_ready_cb;
esp_lcd_panel_handle_t szp_lcd_panel_handle = NULL;






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

void szp_lcd_set_brightness(uint16_t val)
{
   if (val > 100) {
        val = 100;
    } else if (val < 0) {
        val = 0;
    }
    uint32_t duty_cycle = (1023 * val) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, SZP_LCD_LEDC_CH, duty_cycle);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, SZP_LCD_LEDC_CH);
}

//初始化背光
static esp_err_t szp_lcd_brightness_init(void)
{
    // Setup LEDC peripheral for PWM backlight control
    const ledc_channel_config_t lcd_backlight_channel = {
        .gpio_num = SZP_LCD_BACKLIGHT_NUM,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = SZP_LCD_LEDC_CH,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = 1,
        .duty = 0,
        .hpoint = 0,
        .flags.output_invert = true
    };
    const ledc_timer_config_t lcd_backlight_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = 1,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    SZP_ESP_ERR_CHECK(ledc_timer_config(&lcd_backlight_timer));
    SZP_ESP_ERR_CHECK(ledc_channel_config(&lcd_backlight_channel));

    return ESP_OK;
}
//屏幕初始化
esp_err_t szp_lcd_init(void)
{
    // 背光初始化
    SZP_ESP_ERR_CHECK(szp_lcd_brightness_init());
    //初始化屏幕SPI通讯
    spi_bus_config_t spi_cfg = {
        .sclk_io_num = SZP_LCD_SPI_SCLK_NUM,
        .mosi_io_num = SZP_LCD_SPI_MOSI_NUM,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = SZP_LCD_H_RES * SZP_LCD_V_RES * sizeof(uint16_t),
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
        .spi_mode = 2,
        .trans_queue_depth = 10,
        .on_color_trans_done = szp_lcd_on_color_trans_done_cb,
    };
    SZP_ESP_ERR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SZP_LCD_SPI_HOST, &io_config, &io_handle));

    //初始化st7789驱动
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = SZP_LCD_RST_IO_NUM,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = SZP_LCD_BITS_PER_PIXEL,
    };
    SZP_ESP_ERR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &szp_lcd_panel_handle));

    //LCD画板初始化
    SZP_ESP_ERR_CHECK(esp_lcd_panel_reset(szp_lcd_panel_handle));
    //拉低CS引脚
    szp_ioext_lcd_cs(0);
    SZP_ESP_ERR_CHECK(esp_lcd_panel_init(szp_lcd_panel_handle));
    SZP_ESP_ERR_CHECK(esp_lcd_panel_invert_color(szp_lcd_panel_handle, true));
    SZP_ESP_ERR_CHECK(esp_lcd_panel_swap_xy(szp_lcd_panel_handle,true));
    SZP_ESP_ERR_CHECK(esp_lcd_panel_mirror(szp_lcd_panel_handle, true, false));
    SZP_ESP_ERR_CHECK(esp_lcd_panel_disp_on_off(szp_lcd_panel_handle, true));
    //打开背光
    szp_lcd_set_brightness(100);
    return ESP_OK;
}