#ifndef __COMMON__H__
#define __COMMON__H__

#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_utils.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


#define LOG_ERR(M, ...) printf("[ERROR] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(M, ...) printf("[WARN] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(M, ...) printf("[INFO] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_DBG(M, ...) printf("[DBG] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
 
void RCC_GPIOClockCmd(GPIO_TypeDef* GPIOx, FunctionalState state);
void RCC_USARTClockCmd(USART_TypeDef* USARTx, FunctionalState state);
void Timer_16bit_Calc(int freq, uint16_t *period, uint16_t *prescaler);
void Chip_GetUniqueID(uint32_t ChipUniqueID[3]);

#endif /* __COMMON__H__ */