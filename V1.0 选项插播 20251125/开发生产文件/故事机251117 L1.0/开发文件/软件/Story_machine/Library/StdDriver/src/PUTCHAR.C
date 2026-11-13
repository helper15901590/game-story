

//#include <reg51.h>
#include "MS51_16K.H"
extern bit printf_IT;
/****************************************************************************/
/* Define putchar send from UART1, printf function will send from P1.6(TXD_1)
/* NOTICE: Since UART1 pin is multi-function with OCD DATA/CLK pin.
/* Suggest download than use run with realchip but not OCD mode.
/****************************************************************************/

/*
 * putchar (mini version): outputs charcter only
 */

char putchar (char c)  {
  while (!printf_IT);
  printf_IT = 0;
  return (SBUF = c);
}

