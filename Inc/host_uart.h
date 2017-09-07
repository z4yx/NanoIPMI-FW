#ifndef HOST_UART_H
#define HOST_UART_H 

void HostUART_Init(void);
void HostUART_Send(uint8_t data);
void HostUART_Recv_IT(void);
void HostUART_InitSol(uint16_t port);

#endif