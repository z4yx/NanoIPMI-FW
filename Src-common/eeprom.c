#include "common.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_rcc.h"
#include "eeprom.h"
#include <string.h>

#define PAGE_SIZE FLASH_PAGE_SIZE

static uint8_t pageBuf[PAGE_SIZE] __attribute__ ((aligned (4)));

static int programPage(uint32_t addr, uint8_t* data)
{
    for (uint32_t i = 0; i < PAGE_SIZE; i+=4){
        HAL_FLASH_Program (FLASH_TYPEPROGRAM_WORD, addr+i, *(uint32_t*)(data+i));
    }
    for (uint32_t i = 0; i < PAGE_SIZE; i+=4){
        if(*(uint32_t*)(data+i) != *(uint32_t*)(addr+i)){
            LOG_ERR("%08x: %08x vs %08x\r\n", addr+i, *(uint32_t*)(data+i), *(uint32_t*)(addr+i));
            return -1;
        }
    }
    return 0;
}

static int erasePage(uint32_t addr)
{
    FLASH_EraseInitTypeDef pEraseInit;
    uint32_t PageError = 0;
    HAL_StatusTypeDef status;

    pEraseInit.Banks = FLASH_BANK_1;
    pEraseInit.NbPages = 1;
    pEraseInit.PageAddress = addr;
    pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);

    if(status != HAL_OK){
        LOG_ERR("erase failed at %08x, status=%x, PageError=%x",
            status, PageError);
        return -1;
    }
    return 0;
}

void FlashEEP_WriteHalfWords(uint16_t* data, uint32_t length, uint32_t addr)
{
    uint32_t page, off, page_start;
    page = (addr - FLASH_BASE)/PAGE_SIZE;
    page_start = page*PAGE_SIZE + FLASH_BASE;
    off = addr - page_start;
    if((uintptr_t)data & 1){
        LOG_ERR("unaligned");
        return;
    }
    LOG_DBG("Write Flash Addr %08x @ Page %lu Off=%lu", addr, page, off);
    LL_RCC_HSI_Enable();
    while(!LL_RCC_HSI_IsReady());
    HAL_FLASH_Unlock();
    while(length > 0){
        int retry;
        LOG_DBG("page_start=%08x off=%lu size=%lu", page_start, off, length*sizeof(uint16_t));
        if(off>0 || length*sizeof(uint16_t)<PAGE_SIZE)
            memcpy(pageBuf, (void*)page_start, PAGE_SIZE);
        while(off < PAGE_SIZE && length > 0){
            *(uint16_t*)(pageBuf+off) = *(data++);
            off += 2;
            length--;
        }
        for(retry = 3; retry--;){
            if(0 == erasePage(page_start)){
                LOG_DBG("erased");
            }else
                continue;
            if(0 == programPage(page_start, pageBuf)){
                LOG_DBG("programmed");
                break;
            }
        }
        if(retry == -1)
            break;
        off = 0;
        page_start += PAGE_SIZE;
    }
    HAL_FLASH_Lock();
}

