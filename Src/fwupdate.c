#include <stdlib.h>
#include "stm32f1xx_ll_crc.h"
#include "stm32f1xx_hal.h"
#include "common.h"
#include "network.h"
#include "socket.h"
#include "net_conf.h"
#include "settings.h"
#include "eeprom.h"
#include "fwupdate.h"

enum {S_IDLE, S_CMD_RECVED, S_CONNECTING, S_RECEIVING, S_RESET};

static uint32_t page_addr, buf_offset, fw_size, _crc;
static uint8_t _ip[4];
static uint16_t _port;
static uint8_t state;
static int8_t my_socket = -1;
static uint8_t databuf[FLASH_PAGE_SIZE] __attribute__ ((aligned (4)));
static const volatile __attribute__((section(".new_firmware"))) uint32_t new_firmware_1st;

static bool verify_crc(uint32_t* end, uint32_t crc)
{
    uint32_t *i = &new_firmware_1st;
    uint32_t calc_crc;

    LL_CRC_ResetCRCCalculationUnit(CRC);
    while(i < end){
        LL_CRC_FeedData32(CRC, rbit(*(i++)));
    }
    calc_crc = rbit(~LL_CRC_ReadData32(CRC));
    LOG_DBG("Flash CRC=%x", calc_crc);
    if(crc != calc_crc){
        LOG_ERR("CRC error %x vs %x", crc, calc_crc);
        return false;
    }
    return true;
}

void FWUpdate_StartUpgrade(uint32_t ip, uint16_t port, uint32_t crc)
{
    _ip[3] = ip & 0xff;
    _ip[2] = ip >> 8;
    _ip[1] = ip >> 16;
    _ip[0] = ip >> 24;
    _port = port;
    _crc = crc;
    state = S_CMD_RECVED;
}

void FWUpdate_Task(void)
{
    uint8_t sock_status;
    int rc = -1;
    switch(state){
        case S_IDLE:
            break;
        case S_CMD_RECVED:
            buf_offset = 0;
            page_addr = (uint32_t)&new_firmware_1st;
            fw_size = 0;
            if(my_socket != -1){
                close(my_socket);
                my_socket = -1;
            }
            LOG_INFO("Opening socket for fwupdate");
            my_socket = socket(SOCK_FWUPGRADE, Sn_MR_TCP, (rand()&0x7fff)+0x8000, SF_IO_NONBLOCK/*|SF_TCP_NODELAY*/);
            if (my_socket >= 0){
                rc = connect(my_socket, _ip, _port);
                if(rc == SOCK_BUSY)
                    state = S_CONNECTING;
                else if(rc == SOCK_OK)
                    state = S_RECEIVING;
                else{
                    LOG_ERR("fwupdate: failed to connect");
                    state = S_IDLE;
                }
            }
            else{
                LOG_ERR("SOCK_FWUPGRADE failure");
                state = S_IDLE;
            }
            break;
        case S_CONNECTING:
            getsockopt(my_socket, SO_STATUS, &sock_status);
            if(sock_status == SOCK_ESTABLISHED
                || sock_status == SOCK_CLOSE_WAIT){
                LOG_DBG("SOCK_ESTABLISHED");
                LL_CRC_ResetCRCCalculationUnit(CRC);
                state = S_RECEIVING;
            }else if(sock_status == SOCK_CLOSED){
                LOG_ERR("fwupdate connecting failed");
                state = S_IDLE;
            }else{
                static uint8_t sock_status_last;
                if(sock_status_last != sock_status){
                    LOG_DBG("sock_status: %d", sock_status);
                    sock_status_last = sock_status;
                }
            }
            break;
        case S_RECEIVING:
            rc = recv(my_socket, databuf+buf_offset, sizeof(databuf)-buf_offset);
            if(rc > 0){
                buf_offset += rc;
                if(buf_offset == FLASH_PAGE_SIZE){
                    putchar('.');
                    fflush(stdout);
                    FlashEEP_WriteHalfWords(databuf, buf_offset/2, page_addr);
                    // for (int i = 0; i < FLASH_PAGE_SIZE; i+=4)
                    // {
                    //     LL_CRC_FeedData32(CRC, rbit(*(uint32_t*)(databuf+i)));
                    // }
                    page_addr += buf_offset;
                    fw_size += buf_offset;
                    buf_offset = 0;
                }
            }else{
                //check if connection closed (i.e. completed transfer)
                getsockopt(my_socket, SO_STATUS, &sock_status);
                if(sock_status == SOCK_CLOSE_WAIT || sock_status == SOCK_CLOSED){
                    printf(".\r\n");
                    if(buf_offset > 0){
                        while(buf_offset & 3){
                            //padding 0xff
                            databuf[buf_offset++] = 0xff;
                        }
                        FlashEEP_WriteHalfWords(databuf, buf_offset/2, page_addr);
                        // for (int i = 0; i < buf_offset; i+=4)
                        // {
                        //     LL_CRC_FeedData32(CRC, rbit(*(uint32_t*)(databuf+i)));
                        // } 
                        page_addr += buf_offset;
                        fw_size += buf_offset;
                        buf_offset = 0;
                    }
                    // LOG_DBG("page_addr=%x, crc=0x%x", page_addr, rbit(~LL_CRC_ReadData32(CRC)));
                    close(my_socket);
                    my_socket = -1;
                    if(verify_crc((uint32_t*)page_addr, _crc)){
                        LOG_INFO("CRC OK");
                        Settings_SetUpgrade(UPGRADE_FLAG_COPY, _crc, fw_size);
                        state = S_RESET;
                    }else{
                        state = S_IDLE;
                    }
                }
            }
            break;
        case S_RESET:
            LOG_INFO("Reseting...");
            HAL_NVIC_SystemReset(); //shouldn't return
            state = S_IDLE;
            break;
    }
}