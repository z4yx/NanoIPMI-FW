#ifndef SETTINGS_H__
#define SETTINGS_H__ 

#include "wizchip_conf.h"

#define SETTINGS_MAGIC 0xdead4545

struct saved_settings_t
{
    uint32_t magic;
    uint32_t checksum;
    wiz_NetInfo netconf;
};

bool Settings_IsValid(void);
wiz_NetInfo Settings_getNetworkConf(void);
void Settings_updateNetworkConf(const wiz_NetInfo* newConf);


#endif