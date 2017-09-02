#include "common.h"
#include "atx.h"
#include "stm32f1xx_hal.h"

typedef enum { 
    STATE_DOWN, 
    STATE_NORMAL, 
    STATE_WAIT_CHECK,
    STATE_PWR_SHORT_PRESSED,
    STATE_PWR_LONG_PRESSED,
    STATE_RST_SHORT_PRESSED,
};

uint8_t atx_state = STATE_DOWN;
uint32_t pressed_timer;

void ATX_PowerCommand(Command_PowerCommand_PowerOp op)
{
    switch(op){
        case Command_PowerCommand_PowerOp_NOOP:
            break;
        case Command_PowerCommand_PowerOp_ON:
            if(atx_state == STATE_DOWN){
                HAL_GPIO_WritePin(GPIOB, PWR_SW_Pin, GPIO_PIN_RESET);
                pressed_timer = HAL_GetTick();
                atx_state = STATE_PWR_SHORT_PRESSED;
            }
            break;
        case Command_PowerCommand_PowerOp_OFF:
            if(atx_state == STATE_NORMAL){
                HAL_GPIO_WritePin(GPIOB, PWR_SW_Pin, GPIO_PIN_RESET);
                pressed_timer = HAL_GetTick();
                atx_state = STATE_PWR_LONG_PRESSED;
            }
            break;
        case Command_PowerCommand_PowerOp_RESET:
            if(atx_state == STATE_NORMAL){
                HAL_GPIO_WritePin(GPIOB, RST_SW_Pin, GPIO_PIN_RESET);
                pressed_timer = HAL_GetTick();
                atx_state = STATE_RST_SHORT_PRESSED;
            }
            break;
        default:
            LOG_WARN("unknown PowerCommand %d", op);
    }
}
bool ATX_GetPowerOnState(void)
{
    return HAL_GPIO_ReadPin(PERST_GPIO_Port, PERST_Pin); //PCI-E reset is active low
}

void ATX_Task(void)
{
    switch(atx_state){
        case STATE_PWR_SHORT_PRESSED:
            if(HAL_GetTick() - pressed_timer >= 100){
                atx_state = STATE_WAIT_CHECK;
                HAL_GPIO_WritePin(GPIOB, PWR_SW_Pin, GPIO_PIN_SET);
            }
            break;
        case STATE_PWR_LONG_PRESSED:
            if(HAL_GetTick() - pressed_timer >= 6000
                || !ATX_GetPowerOnState()){
                atx_state = STATE_WAIT_CHECK;
                HAL_GPIO_WritePin(GPIOB, PWR_SW_Pin, GPIO_PIN_SET);
            }
            break;
        case STATE_RST_SHORT_PRESSED:
            if(HAL_GetTick() - pressed_timer >= 100){
                atx_state = STATE_WAIT_CHECK;
                HAL_GPIO_WritePin(GPIOB, RST_SW_Pin, GPIO_PIN_SET);
            }
            break;
        case STATE_WAIT_CHECK:
        case STATE_NORMAL:
        case STATE_DOWN:
            if(ATX_GetPowerOnState()){
                if(atx_state != STATE_NORMAL){
                    LOG_INFO("Powered on");
                    atx_state = STATE_NORMAL;
                }
            }else{
                if(atx_state != STATE_DOWN){
                    LOG_INFO("Powered off");
                    atx_state = STATE_DOWN;
                }
            }
            break;
    }
}
