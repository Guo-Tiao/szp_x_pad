#pragma once

#define SZP_ESP_ERR_CHECK(x)                \
    {                                                                 \
        esp_err_t ret = (x);                                \
        if (ESP_OK != ret)                                  \
        {                                                             \
            return ret;                                         \
        }                                                              \
    }
    