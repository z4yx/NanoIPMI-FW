#include "stm32f1xx_hal.h"
#include "common.h"
#include "led.h"

static uint8_t color,flashing,off_state;
static uint32_t tick;

static void LED_Board(uint8_t ch, uint8_t s) {
  if (s) /*LED_OFF*/
    HAL_GPIO_WritePin(GPIOB, ((ch)==1) ? LED1_Pin : LED2_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(GPIOB, ((ch)==1) ? LED1_Pin : LED2_Pin, GPIO_PIN_RESET);
}

void LED_SetColor(uint8_t color_)
{
    color = color_;
}

void LED_SetFlashing(uint8_t flashing_)
{
    flashing = flashing_;
}

void LED_Task(void)
{
    uint32_t t = HAL_GetTick();
    if(t-tick > 300){
        tick = t;
        off_state = ~off_state;
        if(color == COLOR_OFF || (off_state && flashing)){
            LED_Board(1, LED_OFF);
            LED_Board(2, LED_OFF);
        }else if(color == COLOR_GREEN){
            LED_Board(1, LED_OFF);
            LED_Board(2, LED_ON);
        }else if(color == COLOR_RED){
            LED_Board(1, LED_ON);
            LED_Board(2, LED_OFF);
        }
    }
}