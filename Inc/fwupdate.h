#ifndef FW_UPDATE_H__
#define FW_UPDATE_H__ 

void FWUpdate_Task(void);
void FWUpdate_StartUpgrade(uint32_t ip, uint16_t port, uint32_t crc);

#endif