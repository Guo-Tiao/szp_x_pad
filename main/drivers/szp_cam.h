#pragma once
#include "esp_err.h"
#include "esp_camera.h"

//摄像头初始化
esp_err_t szp_cam_init(void);
//摄像头帧获取
camera_fb_t *szp_cam_fb_get(void);
//摄像头帧返回
void szp_cam_fb_return(camera_fb_t *fb);