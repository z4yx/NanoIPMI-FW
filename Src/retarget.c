#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_usart.h"

/* Retargeting functions for gcc-arm-embedded */

int _write (int fd, char *ptr, int len)
{
  /* Write "len" of char from "ptr" to file id "fd"
   * Return number of char written.
   * Need implementing with UART here. */
  int i;
  for (i = 0; i < len; ++i)
  {
    _ttywrch(ptr[i]);
  }
  return len;
}

int _read (int fd, char *ptr, int len)
{
  /* Read "len" of char to "ptr" from file id "fd"
   * Return number of char read.
   * Need implementing with UART here. */
  return len;
}

void _ttywrch(int ch) {
  /* Write one char "ch" to the default console
   * Need implementing with UART here. */
  while(!LL_USART_IsActiveFlag_TXE (USART1));
  LL_USART_TransmitData8 (USART1, ch);
}
