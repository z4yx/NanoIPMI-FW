#ifndef LED_H__
#define LED_H__ 

#include "stm32f1xx_hal.h"

#define LED_ON  0
#define LED_OFF 1

#define LED_Board(ch, s) \
do {\
  if (s) /*LED_OFF*/ \
    HAL_GPIO_WritePin(GPIOB, ((ch)==1) ? LED1_Pin : LED2_Pin, GPIO_PIN_SET); \
  else \
    HAL_GPIO_WritePin(GPIOB, ((ch)==1) ? LED1_Pin : LED2_Pin, GPIO_PIN_RESET); \
}while(0)


#endif
