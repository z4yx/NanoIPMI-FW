#ifndef IPMI_APP_H
#define IPMI_APP_H 

void IPMIApp_Task(void);
void IPMIApp_HostUART_RecvCallback(uint8_t data);
void IPMIApp_CDC_RecvCallback(uint8_t *buf, uint32_t len);
void IPMIApp_EventCallback(uint8_t event);


#endif