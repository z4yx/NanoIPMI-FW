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

bool Settings_IsValid(void)
{
	LOG_DBG("magic=%x", savedSettings.magic);
	if(savedSettings.magic != SETTINGS_MAGIC)
		return 0;
	if(calc_checksum(&savedSettings) != savedSettings.checksum)
		return 0;
	return 1;
}

wiz_NetInfo Settings_getNetworkConf(void)
{
	return savedSettings.netconf;
}

void Settings_updateNetworkConf(const wiz_NetInfo* newConf)
{
	tmpSettings = savedSettings;
	tmpSettings.magic = SETTINGS_MAGIC;
	tmpSettings.netconf = *newConf;
	tmpSettings.checksum = calc_checksum(&tmpSettings);
	FlashEEP_WriteHalfWords((uint16_t*)&tmpSettings, (sizeof(tmpSettings)+1)/2, &savedSettings);
	LOG_DBG("magic=%x", savedSettings.magic);
}