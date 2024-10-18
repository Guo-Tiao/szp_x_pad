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