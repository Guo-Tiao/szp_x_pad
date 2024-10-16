#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define DEMO_GPIO_KEY_NUM   GPIO_NUM_9

//按键中断队列
static QueueHandle_t  s_demo_gpio_key_evt_queue = NULL;

//Key线程
static void demo_gpio_key_task(void* arg)
{
    uint32_t io_num;
    for(;;) 
    {
        if(xQueueReceive(s_demo_gpio_key_evt_queue, &io_num, portMAX_DELAY)) 
        {
            printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));
        }
    }
}

//Key中断事件
static void IRAM_ATTR demo_gpio_key_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(s_demo_gpio_key_evt_queue, &gpio_num, NULL);
}
    //实力派User-Key初始化
void demo_gpio_key_init()
{
    gpio_config_t key_cfg =
    {
            .intr_type = GPIO_INTR_NEGEDGE, //配置中断类型
            .mode = GPIO_MODE_INPUT,    //配置输入输出模式
            .pin_bit_mask = 1ULL << DEMO_GPIO_KEY_NUM, //配置引脚
            .pull_down_en = GPIO_PULLDOWN_DISABLE,//下拉电阻是否使能
            .pull_up_en = GPIO_PULLUP_ENABLE    //上拉电阻是否使能
    };
    //配置GPIO
    gpio_config(&key_cfg);

    //配置中断服务和线程
    //创建队列
    s_demo_gpio_key_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //创建线程
    xTaskCreate(demo_gpio_key_task, "demo_gpio_key_task", 2048, NULL, 10, NULL);    //栈不能太小，否则会崩溃
    //安装GPIO中断服务程序，即使能中断
    gpio_install_isr_service(0);
    //添加中断函数
    gpio_isr_handler_add(DEMO_GPIO_KEY_NUM, demo_gpio_key_isr_handler, (void *)DEMO_GPIO_KEY_NUM);
}


void _2_demo_gpio_run()
{
    //实力派User-Key初始化
    demo_gpio_key_init();
}