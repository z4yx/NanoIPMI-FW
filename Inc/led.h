#ifndef LED_H__
#define LED_H__ 


#define LED_ON  0
#define LED_OFF 1

enum{
    COLOR_OFF,
    COLOR_RED,
    COLOR_GREEN,
};


// void LED_Board(uint8_t ch, uint8_t s);
void LED_SetColor(uint8_t color_);
void LED_SetFlashing(uint8_t flashing_);
void LED_Task(void);


#endif
