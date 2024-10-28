#pragma once
#include "esp_err.h"
#include "sdkconfig.h"
#if CONFIG_USE_SZP_MQTT

//开启MQTT服务
esp_err_t szp_mqtt_start(void);



#endif