#pragma once
#include "sdkconfig.h"

#if CONFIG_USE_SZP_ESP_ERR_CHECK
#define SZP_ESP_ERR_CHECK(x)                \
    {                                                                 \
        esp_err_t ret = (x);                                \
        if (ESP_OK != ret)                                  \
        {                                                             \
            return ret;                                         \
        }                                                              \
    }
#else
#define SZP_ESP_ERR_CHECK  ESP_ERROR_CHECK
#endif


#include "freertos/FreeRTOS.h"
#define SZP_OS_TRUE                         pdTRUE                  
#define SZP_OS_FALSE                       pdFALSE
#define SZP_WAIT_FOR_INFINITE       portMAX_DELAY            //永久等待
#define SZP_TICK_TO_MS(tick)           pdTICKS_TO_MS(tick)       //tick转换为ms
#define SZP_MS_TO_TICK(ms)             pdMS_TO_TICKS(ms)       //ms转换为tick