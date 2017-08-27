#include "stm32f1xx_hal.h"
#include "common.h"

//获得芯片的唯一ID
void Chip_GetUniqueID(uint32_t ChipUniqueID[3])
{
	ChipUniqueID[2] = *(uint32_t *)(0X1FFFF7F0);
    ChipUniqueID[1] = *(uint32_t *)(0X1FFFF7EC);
    ChipUniqueID[0] = *(uint32_t *)(0X1FFFF7E8);
}