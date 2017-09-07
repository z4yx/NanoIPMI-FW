#include "common.h"
#include "host_uart.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_usart.h"
#include "ipmi-app.h"
#include "circular_buffer.h"
#include "socket.h"
#include "net_conf.h"
#include "network.h"
#include <stdlib.h>

static int sock_fd;
static uint32_t throttle_timer;
static bool connected, wait_connect;
struct CircularBuffer* gBuf;

void HostUART_Init(void)
{
    gBuf = CircularBuffer_New(2048);
    LL_USART_EnableIT_RXNE(USART2);
}

void HostUART_Send(uint8_t data)
{
    if(!LL_USART_IsActiveFlag_TXE (USART2))
        return;
    LL_USART_TransmitData8 (USART2, data);

}

void HostUART_Recv_IT(void)
{
    uint8_t d = LL_USART_ReceiveData8 (USART2);
    IPMIApp_HostUART_RecvCallback(d);
    CircularBuffer_Push(gBuf, d, true); //IRQ already disable in IRQ context
}

void HostUART_InitSol(uint16_t port)
{
    int rc = -1;
    connected = false;
    wait_connect = false;
    throttle_timer = 0;
    sock_fd = socket(SOCK_SOL, Sn_MR_TCP, (rand()&0x7fff)+0x8000, SF_IO_NONBLOCK/*|SF_TCP_NODELAY*/);
    if(sock_fd >= 0)
        rc = connect(sock_fd, Network_GetMQTTBrokerIP(), port);
    if(rc == SOCK_OK)
        connected = true;
    else if(rc == SOCK_BUSY)
        wait_connect = true;
}

void HostUART_Task(void)
{
    uint16_t size, i;
    uint8_t burst[128];
    if(wait_connect){
        uint8_t status;
        getsockopt(sock_fd, SO_STATUS, &status);
        if(status == SOCK_ESTABLISHED){
            connected = true;
            wait_connect = false;
            LOG_INFO("HostUART connected");
        }
    }
    if(!connected)
        return;

    while(CircularBuffer_Size(gBuf) > 0){
        getsockopt(sock_fd, SO_SENDBUF, &size);
        if(size > sizeof(burst))
            size = sizeof(burst);
        for (i = 0; i < size; ++i)
            if(!CircularBuffer_Pop(gBuf, &burst[i]))
                break;
        LOG_VERBOSE("SOL send %u", i);
        send(sock_fd, burst, i);
    }

    // if(HAL_GetTick() - throttle_timer < 1)
    //     return;
    int rc = recv(sock_fd, burst, sizeof(burst));
    if (rc == SOCK_BUSY){
    }
    else if(rc > 0){
        LOG_VERBOSE("SOL recv %u",rc);
        for (i = 0; i < rc; ++i){
            HAL_Delay(1);
            HostUART_Send(burst[i]);
        }
        // throttle_timer = HAL_GetTick();
    } else {
        LOG_WARN("SOL recv with %d", rc);
        connected = false;
    }
}
