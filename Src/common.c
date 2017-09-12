#include "stm32f1xx_hal.h"
#include "common.h"

extern ADC_HandleTypeDef hadc1;

//获得芯片的唯一ID
void Chip_GetUniqueID(uint32_t ChipUniqueID[3])
{
	ChipUniqueID[2] = *(uint32_t *)(0X1FFFF7F0);
    ChipUniqueID[1] = *(uint32_t *)(0X1FFFF7EC);
    ChipUniqueID[0] = *(uint32_t *)(0X1FFFF7E8);
}

int32_t ADC_getTemperatureReading(void)
{
    HAL_ADC_Start(&hadc1);
    if(HAL_OK != HAL_ADC_PollForConversion(&hadc1, 10)){
        LOG_ERR("ADC failed");
        return 0;
    }
    uint32_t val = HAL_ADC_GetValue(&hadc1);
    LOG_DBG("ADC=%u", val);
    return (int)((1.43 - 3.3*val/4096)/4.3)+25;
}