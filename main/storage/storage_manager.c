#include "storage_manager.h"

//导出nvs初始化
extern void szp_nvs_init(void);

void storage_init(void)
{
    //nvs初始化
    szp_nvs_init();
}