/*
 * @LastEditors: ≈£¬Ì»À
 * @LastEditTime: 2025-03-07 10:38:19
 */


void main_Init(void);
void DATA_clr(void);
void uart_clr(void);

// #define LED0_off P12=1;
// #define LED0_on P12=0;
// #define LED0_toggle P12=~P12;
#define CALL_isr_UART()         TI = 1

// enum MODE
// {
//     MODE1,MODE2,MODE3,MODE4
// };