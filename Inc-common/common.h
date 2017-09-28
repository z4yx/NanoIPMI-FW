#ifndef __COMMON__H__
#define __COMMON__H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define LOGLEVEL 3

#define LOG_ERR(...) do{}while(0)
#define LOG_WARN(...) do{}while(0)
#define LOG_INFO(...) do{}while(0)
#define LOG_DBG(...) do{}while(0)
#define LOG_VERBOSE(...) do{}while(0)

#if LOGLEVEL>=1
#undef LOG_ERR
#define LOG_ERR(M, ...) printf("[ERROR] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#if LOGLEVEL>=2
#undef LOG_WARN
#define LOG_WARN(M, ...) printf("[WARN] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#if LOGLEVEL>=3
#undef LOG_INFO
#define LOG_INFO(M, ...) printf("[INFO] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#if LOGLEVEL>=4
#undef LOG_DBG
#define LOG_DBG(M, ...) printf("[DBG] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#if LOGLEVEL>=5
#undef LOG_VERBOSE
#define LOG_VERBOSE(M, ...) printf("[VERBOSE] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif
#endif
#endif
#endif
#endif


inline uint32_t rbit(uint32_t input){
    uint32_t output;
    __asm__("rbit %0, %1\n" : "=r"(output) : "r"(input));
    return output;
}

void Chip_GetUniqueID(uint32_t ChipUniqueID[3]);
int32_t ADC_getTemperatureReading(void);

#endif /* __COMMON__H__ */