#include "common.h"
#include "host_uart.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_usart.h"
#include "ipmi-app.h"

void HostUART_Send_NoWait(uint8_t data)
{
    if(!LL_USART_IsActiveFlag_TXE (USART2))
        return;
    LL_USART_TransmitData8 (USART2, data);

}

void HostUART_Recv_IT(void)
{
    uint8_t d = LL_USART_ReceiveData8 (USART2);
    IPMIApp_CDC_RecvCallback(d);
}
