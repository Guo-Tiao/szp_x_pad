#include "szp_key.h"
#include "common/common_macro.h"
#include <stdio.h>
#include <inttypes.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"


#define SZP_KEY_NUM   GPIO_NUM_9
#define SZP_KEY_MONITOR_TASK_PRIO                         10                                        //监控任务优先级
#define SZP_KEY_MONITOR_TASK_DELAY                      10                                        //监控任务睡眠


#define SZP_KEY_PRESSING_LEVEL         0       //按下状态
#define SZP_KEY_RELEASE_LEVEL           1       //松开状态
//实战派按键状态
typedef enum e_szp_key_state
{
    WaitCheck,//等待检查

    PressDown,//按钮第一次按下
    Pressing,//按下中

    PressUp,//按下弹起
    Upping,//未按下

} SzpKeyState;

//实战派按键数据
typedef struct szp_key_t
{
    SzpKeyState state;//按钮状态
    uint8_t current_level;    // 按钮当前电平;
    uint8_t last_level;    // 按钮上一个电平;

    uint32_t check_start_time;//触发检测的时间点
    uint32_t last_down_time;//上一次按下的时间点
    uint32_t last_up_time;//上一次按下的时间点

    uint32_t current_pressing_ms;//当前按下的时间
    uint32_t current_releasing_ms;//当前松开的时间

    uint16_t clicked_count;//检测周期内按下后松开的次数
    SzpKeyEvent event;//按钮事件
}SzpKey;

//实战派按钮
SzpKey _szp_key_;


//实战派按键事件组
EventGroupHandle_t event_group_szp_key;
// 按键任务句柄
TaskHandle_t szp_key_monitor_task_handle = NULL; 
//按键中断和监控任务事件队列
static QueueHandle_t  szp_key_monitor_evt_queue = NULL;

//Key中断事件
static void IRAM_ATTR szp_key_isr_handler(void* arg)
{
    //等待检测才启动
    if(_szp_key_.state==WaitCheck)
    {
         uint32_t io_num = (uint32_t) arg;
        xQueueSendFromISR(szp_key_monitor_evt_queue, &io_num, NULL);
    }
}
//实战派按钮设置事件
void szp_key_set_event(SzpKeyEvent event)
{
    _szp_key_.event = event;
   xEventGroupSetBits(event_group_szp_key, event);
}

//实战派按键初始化
void szp_key_t_init()
{
    _szp_key_.state = WaitCheck;
    _szp_key_.current_level = SZP_KEY_RELEASE_LEVEL;
    _szp_key_.last_level = SZP_KEY_RELEASE_LEVEL;

    _szp_key_.check_start_time = 0;
    _szp_key_.last_down_time = 0;
    _szp_key_.last_up_time = 0;

    _szp_key_.current_pressing_ms = 0;
    _szp_key_.current_releasing_ms = 0;
    _szp_key_.clicked_count = 0;
    xEventGroupClearBits(event_group_szp_key, 0x00ffffff);
}
//Key监控按钮任务
static void szp_key_monitor_task(void *params)
{
    uint32_t io_num;
    //状态机判断
    for (;;)
    { 
         if(xQueueReceive(szp_key_monitor_evt_queue, &io_num, portMAX_DELAY)) 
        {
            TickType_t current_time = SZP_TICK_TO_MS(xTaskGetTickCount());//获取按下的时间ms记为上次ms
            //初始化状态    
            _szp_key_.state = PressDown;
            _szp_key_.current_level = SZP_KEY_PRESSING_LEVEL;
            _szp_key_.last_level = SZP_KEY_RELEASE_LEVEL;
            _szp_key_.check_start_time = current_time;
            _szp_key_.last_down_time =current_time;
            _szp_key_.last_up_time = 0;

            _szp_key_.current_pressing_ms = 0;
            _szp_key_.current_releasing_ms = 0;
            _szp_key_.clicked_count = 0;

            {
                //进入检测阶段
                while (1)
                {
                    vTaskDelay(SZP_MS_TO_TICK(SZP_KEY_MONITOR_TASK_DELAY));
                    _szp_key_.last_level = _szp_key_.current_level;//保存上一个电平状态
                    _szp_key_.current_level = gpio_get_level(SZP_KEY_NUM); //获取按键状态
                    current_time = SZP_TICK_TO_MS(xTaskGetTickCount());   // 获取当前计时

                    
                    switch (_szp_key_.state)
                    {
                        case WaitCheck:
                        {
                            // 退出检测
                            szp_key_t_init();
                            break;
                        }
                            break;
                        case PressDown://按下瞬间的状态
                        {    
                            _szp_key_.last_down_time = current_time;
                            _szp_key_.current_pressing_ms = 0;
                            _szp_key_.current_releasing_ms = 0;
                            _szp_key_.state = Pressing;
                             continue;
                         
                        }
                            break;
                        case Pressing://按下状态
                        {
                            if(_szp_key_.current_level==SZP_KEY_PRESSING_LEVEL)
                            {
                                _szp_key_.current_pressing_ms = (current_time - _szp_key_.last_down_time);   
                                _szp_key_.state = Pressing;
                                if(_szp_key_.current_pressing_ms>=SZP_KEY_HOLD_LONG_MS) //触发长按,退出
                                {
                                    szp_key_set_event(EV_SZP_KEY_LONG_HOLDING);
                                    _szp_key_.state = WaitCheck;
                                }
                            }
                            else
                            {
                                _szp_key_.state = PressUp;
                            }
                            continue;
                        }
                            break;
                        case PressUp://弹起瞬间的状态
                        {
                            _szp_key_.last_up_time = current_time;
                            _szp_key_.state = Upping;
                             _szp_key_.clicked_count++;//点击次数++
                             if(1==_szp_key_.clicked_count)
                             {
                                szp_key_set_event(EV_SZP_KEY_CLICKED);
                             }
                             else if(2==_szp_key_.clicked_count)
                             {
                                  if( (current_time-_szp_key_.check_start_time)<SZP_KEY_DOUBLE_CLICKED_INTERVAL)
                                  {
                                        szp_key_set_event(EV_SZP_KEY_DOUBLE_CLICKED);
                                  }
                                  else
                                  {
                                        szp_key_set_event(EV_SZP_KEY_CLICKED);
                                  }
                             }
                             else if(3==_szp_key_.clicked_count)
                             {
                                 if( (current_time-_szp_key_.check_start_time)<SZP_KEY_TREBLE_CLICKED_INTERVAL)
                                  {
                                        szp_key_set_event(EV_SZP_KEY_TREBLE_CLICKED);
                                  }
                                  else
                                  {
                                        szp_key_set_event(EV_SZP_KEY_CLICKED);
                                  }
                                  _szp_key_.state = WaitCheck;
                             }

                             _szp_key_.current_pressing_ms = 0;
                            _szp_key_.current_releasing_ms = 0;
                            continue;
                        }
                            break;
                        case Upping://弹起状态
                        {
                             if(_szp_key_.current_level==SZP_KEY_RELEASE_LEVEL)
                            {
                                _szp_key_.current_releasing_ms = (current_time - _szp_key_.last_up_time);   
                                _szp_key_.state = Upping;
                                if(_szp_key_.current_releasing_ms>=SZP_KEY_IDLE_TIME) //大于空闲时间退出
                                {
                                    _szp_key_.state = WaitCheck;
                                }
                            }
                            else
                            {
                                _szp_key_.state = PressDown;
                            }
                            continue;
                        }
                            break;
                    }
                   
                    if(_szp_key_.state==WaitCheck)
                    {
                         szp_key_set_event(EV_SZP_KEY_IDLE);
                        break;
                    }
                    
                }
            }
        }
    }
}

void szp_key_init(void)
{
    gpio_config_t key_cfg =
    {
            .intr_type = GPIO_INTR_NEGEDGE, //配置中断类型
            .mode = GPIO_MODE_INPUT,    //配置输入输出模式
            .pin_bit_mask = 1ULL << SZP_KEY_NUM, //配置引脚
            .pull_down_en = GPIO_PULLDOWN_DISABLE,//下拉电阻是否使能
            .pull_up_en = GPIO_PULLUP_ENABLE    //上拉电阻是否使能
    };
    //配置GPIO
    SZP_ESP_ERR_CHECK(gpio_config(&key_cfg));
    //安装GPIO中断服务程序，即使能中断
    SZP_ESP_ERR_CHECK(gpio_install_isr_service(0));
    //添加中断函数
    SZP_ESP_ERR_CHECK(gpio_isr_handler_add(SZP_KEY_NUM, szp_key_isr_handler, (void *)SZP_KEY_NUM));


    //创建事件组
    event_group_szp_key = xEventGroupCreate();
    //初始化实战派按键
    szp_key_t_init();
     //创建队列
    szp_key_monitor_evt_queue = xQueueCreate(1, sizeof(uint32_t));
    //创建线程
    xTaskCreate(szp_key_monitor_task, "szp_key_monitor_task", 2048, NULL, SZP_KEY_MONITOR_TASK_PRIO, &szp_key_monitor_task_handle);

}

uint32_t szp_key_wait_event(SzpKeyEvent evnet, uint8_t clearbit, uint32_t waitTimeMs)
{
    TickType_t time=waitTimeMs;
    if(waitTimeMs!=SZP_WAIT_FOR_INFINITE)   //非永久则转换为TICK
    {
        time = SZP_MS_TO_TICK(waitTimeMs);
    }

    uint32_t event_bit= xEventGroupWaitBits(event_group_szp_key,
                               (EventBits_t)evnet,
                               (BaseType_t)clearbit,
                               (BaseType_t)SZP_OS_FALSE,
                               (TickType_t)time);
    return event_bit;
}
