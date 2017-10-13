#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_adc.h"
#include "common.h"

extern ADC_HandleTypeDef hadc1;

#define VDDA_APPLI                       ((uint32_t)3300)
#define INTERNAL_TEMPSENSOR_AVGSLOPE   ((int32_t) 4300)        /* Internal temperature sensor, parameter Avg_Slope (unit: uV/DegCelsius). Refer to device datasheet for min/typ/max values. */
#define INTERNAL_TEMPSENSOR_V25        ((int32_t) 1430)        /* Internal temperature sensor, parameter V25 (unit: mV). Refer to device datasheet for min/typ/max values. */
#define INTERNAL_TEMPSENSOR_V25_TEMP   ((int32_t)   25)
#define INTERNAL_TEMPSENSOR_V25_VREF   ((int32_t) 3300)


//获得芯片的唯一ID
void Chip_GetUniqueID(uint32_t ChipUniqueID[3])
{
	ChipUniqueID[2] = *(uint32_t *)(0X1FFFF7F0);
    ChipUniqueID[1] = *(uint32_t *)(0X1FFFF7EC);
    ChipUniqueID[0] = *(uint32_t *)(0X1FFFF7E8);
}

int32_t ADC_getTemperatureReading(void)
{
    while (LL_ADC_IsCalibrationOnGoing(hadc1.Instance) != 0);
    HAL_ADC_Start(&hadc1);
    if(HAL_OK != HAL_ADC_PollForConversion(&hadc1, 10)){
        LOG_ERR("ADC failed");
        return 0;
    }
    uint16_t val = HAL_ADC_GetValue(&hadc1);
    LOG_DBG("ADC=%hu", val);
    HAL_ADC_Stop(&hadc1);
    return __LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(INTERNAL_TEMPSENSOR_AVGSLOPE, 
                                                INTERNAL_TEMPSENSOR_V25,
                                                INTERNAL_TEMPSENSOR_V25_TEMP,
                                                VDDA_APPLI,
                                                val,
                                                LL_ADC_RESOLUTION_12B);
}