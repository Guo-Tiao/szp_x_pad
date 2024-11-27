import os
import _thread
import time

import psutil #CPU库
import GPUtil #GPU库

import serial # type: ignore
import serial.tools.list_ports # type: ignore

pc_data={
    "CPU":0,
    "GPU":0,
    "Mem":0,
    "Net_Up":0.0,
    "Net_Down":0.0
    }

#获取CPU占用
def get_cpu_usage(interval=0.5):
    return psutil.cpu_percent(interval=interval)*5 #win11上计算CPU占用率比实际低

#获取GPU占用
def get_gpu_usage():
    gpus = GPUtil.getGPUs()
    if len(gpus)<1:
            return 0
    else:
        gpu=gpus[0]
        return   (gpu.load*100)
    
#获取内存占用
def get_mem_usage():
    memory_info = psutil.virtual_memory()
    return  memory_info.percent


        
#获取网速        
def get_net_speed(interval=1):
    
    net_io_1= psutil.net_io_counters()
    send_byte1=net_io_1.bytes_sent
    recv_byte1=net_io_1.bytes_recv
    
    time.sleep(interval)
    
    net_io_2= psutil.net_io_counters()
    send_byte2=net_io_2.bytes_sent
    recv_byte2=net_io_2.bytes_recv
    
    return (float((send_byte2-send_byte1)/1024),float((recv_byte2-recv_byte1)/1024))


#串口设置
com='COM7'
ser:serial.Serial
# 初始化串口
def serial_init():
        global  ser
        ser = serial.Serial(com, 115200)
        if ser.is_open:
            return True
        else:
            return False

#数据更新
def thread_update_data():
    while True:
        pc_data["CPU"]=get_cpu_usage()
        pc_data["GPU"]=get_gpu_usage()
        pc_data["Mem"]=get_mem_usage()
        speed=get_net_speed()
        pc_data["Net_Up"]=speed[0]
        pc_data["Net_Down"]=speed[1]
        time.sleep(0.5) 

#数据发送
def thread_send_data():
    while True:
        global ser
        result = ' '.join([f"{value}," for key,value in pc_data.items()])
        result = result[:-1]
        result=result.replace(' ','')
        ser.write(result.encode())
        print("send:"+result)
        time.sleep(0.5) #每0.5秒发送一次
    
    
    
if __name__ == '__main__':
    #初始化失败退出
    if not serial_init():
        print("初始化串口失败")
        exit()
    _thread.start_new_thread( thread_update_data,())
    _thread.start_new_thread( thread_send_data,())
    while True:
        pass
        
    