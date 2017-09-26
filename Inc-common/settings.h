#ifndef SETTINGS_H__
#define SETTINGS_H__ 

#include "wizchip_conf.h"
#include <stdbool.h>

#define SETTINGS_MAGIC 0xdead4545

enum {UPGRADE_FLAG_UNDEF=0xffffffff, UPGRADE_FLAG_COPY = 0x00ff55aa};

struct saved_settings_t
{
    uint32_t magic;
    uint32_t checksum;
    uint32_t upgrade_flags;
    uint32_t crc_new_fw;
    wiz_NetInfo netconf;
};

bool Settings_IsValid(void);
wiz_NetInfo Settings_getNetworkConf(void);
void Settings_updateNetworkConf(const wiz_NetInfo* newConf);
uint32_t Settings_GetUpgradeFlag(void);
void Settings_EraseUpgradeFlag(void);
uint32_t Settings_GetNewFWCrc(void);
void Settings_SetUpgrade(uint32_t flag, uint32_t crc);


#endif