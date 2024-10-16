#pragma once

#include "stdint.h"

#define SZP_KEY_IDLE_TIME                                             400                                     //空闲时间(释放*ms后)
#define SZP_KEY_DOUBLE_CLICKED_INTERVAL            1000                                    //按键双击间隔时间(ms)
#define SZP_KEY_TREBLE_CLICKED_INTERVAL              2000                                    //按键三击间隔时间(ms)
#define SZP_KEY_HOLD_LONG_MS                                 2000                                    //长按时间(ms)




//实战派按钮事件
typedef enum e_szp_key_event
{
    EV_SZP_KEY_IDLE=0x01,                               //按键释放(空闲态)
    EV_SZP_KEY_CLICKED=0x02,                        //按键单击
    EV_SZP_KEY_DOUBLE_CLICKED=0x04,        //按键双击 短时间连续按下两次
    EV_SZP_KEY_TREBLE_CLICKED=0x08,          //按键三击 短时间连续按下三次
    EV_SZP_KEY_LONG_HOLDING=0x10,           //按键长按 

} SzpKeyEvent;

//实战派用户按钮初始化
void szp_key_init(void);
//实战派事件等待
uint32_t szp_key_wait_event(SzpKeyEvent  evnet,uint8_t clearbit,uint32_t waitTime);
