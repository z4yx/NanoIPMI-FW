#include "common.h"
#include "settings.h"

static const volatile __attribute__((section(".eeprom"))) 
    struct saved_settings_t savedSettings;

static struct saved_settings_t tmpSettings;

uint32_t calc_checksum(struct saved_settings_t *settings)
{
	uint32_t sum = 0;
	for (int i = 0; i < sizeof(*settings); ++i)
	{
		sum += ((uint8_t*)settings)[i];
	}
	for (int i = 0; i < sizeof(settings->checksum); ++i)
	{
		sum -= ((uint8_t*)&settings->checksum)[i];
	}
	return sum;
}

static void populate_default(struct saved_settings_t *settings)
{
	settings->magic = SETTINGS_MAGIC;
	settings->upgrade_flags = UPGRADE_FLAG_UNDEF;
	settings->crc_new_fw = 0;
	settings->sz_new_fw = 0;
	settings->netconf = (wiz_NetInfo){ 
							.mac = {0x78,0xCA,0x83,0x40,0x01,0x08},
                            .ip = {192, 168, 1, 80},
                            .sn = {255, 255, 255, 0},
                            .gw = {192, 168, 1, 1},
                            .dns = {8, 8, 8, 8},
                            .logserver = {192, 168, 1, 1},
                            .dhcp = NETINFO_DHCP
                        };
}

bool Settings_IsValid(void)
{
	LOG_DBG("magic=%x", savedSettings.magic);
	if(savedSettings.magic != SETTINGS_MAGIC)
		return 0;
	if(calc_checksum(&savedSettings) != savedSettings.checksum)
		return 0;
	return 1;
}

uint32_t Settings_GetNewFWInfo(uint32_t *crc, uint32_t *sz_new_fw)
{
	*crc = savedSettings.crc_new_fw;
	*sz_new_fw = savedSettings.sz_new_fw;
}

wiz_NetInfo Settings_getNetworkConf(void)
{
	return savedSettings.netconf;
}

uint32_t Settings_GetUpgradeFlag(void)
{
	return savedSettings.upgrade_flags;
}

void Settings_SetUpgrade(uint32_t flag, uint32_t crc, uint32_t sz_new_fw)
{
	tmpSettings = savedSettings;
	if(!Settings_IsValid())
		populate_default(&tmpSettings);
	tmpSettings.upgrade_flags = flag;
	tmpSettings.sz_new_fw = sz_new_fw;
	tmpSettings.crc_new_fw = crc;
	tmpSettings.checksum = calc_checksum(&tmpSettings);
	FlashEEP_WriteHalfWords((uint16_t*)&tmpSettings, (sizeof(tmpSettings)+1)/2, &savedSettings);
}

void Settings_EraseUpgradeFlag(void)
{
	if(!Settings_IsValid())
		return;
	tmpSettings = savedSettings;
	tmpSettings.upgrade_flags = UPGRADE_FLAG_UNDEF;
	tmpSettings.sz_new_fw = 0;
	tmpSettings.crc_new_fw = 0;
	tmpSettings.checksum = calc_checksum(&tmpSettings);
	FlashEEP_WriteHalfWords((uint16_t*)&tmpSettings, (sizeof(tmpSettings)+1)/2, &savedSettings);
}

void Settings_updateNetworkConf(const wiz_NetInfo* newConf)
{
	tmpSettings = savedSettings;
	if(!Settings_IsValid())
		populate_default(&tmpSettings);
	tmpSettings.netconf = *newConf;
	tmpSettings.checksum = calc_checksum(&tmpSettings);
	FlashEEP_WriteHalfWords((uint16_t*)&tmpSettings, (sizeof(tmpSettings)+1)/2, &savedSettings);
	LOG_DBG("magic=%x", savedSettings.magic);
}