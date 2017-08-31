#ifndef __COMMON__H__
#define __COMMON__H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


#define LOG_ERR(M, ...) printf("[ERROR] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(M, ...) printf("[WARN] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(M, ...) printf("[INFO] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_DBG(M, ...) printf("[DBG] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
 
void Chip_GetUniqueID(uint32_t ChipUniqueID[3]);
int32_t ADC_getTemperatureReading(void);

#endif /* __COMMON__H__ */