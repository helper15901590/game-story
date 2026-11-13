/*
 * @LastEditors: ???
 * @LastEditTime: 2025-03-12 17:22:30
 */


#define UART0_Timer1  0
#define UART0_Timer3  1
#define UART1_Timer3  2
#define UART0 0
#define UART1 1



#if UART1
#define uartRx1_on()   io_RX1  = 1
#define	uartRx1_off()  io_RX1  = 0
#define uartTx1_on()   io_TX1  = 1
#define	uartTx1_off()  io_TX1  = 0
#endif

void uart_send_16byte_dec(uint16_t value);
void UART_Open(unsigned long u32SysClock, unsigned char u8UARTPort,unsigned long u32Baudrate);
void UART_Send_Data(unsigned char UARTPort, unsigned char c);
unsigned char Receive_Data(unsigned char UARTPort);
void Enable_UART0_VCOM_printf_24M_115200(void);
void Enable_UART0_VCOM_printf_24M_9600(void);

extern   bit PRINTFG,uart0_receive_flag,uart1_receive_flag;
extern   unsigned char uart0_receive_data,uart1_receive_data;
void UART0_send_string(unsigned char *buf);
void uart_send_hex(unsigned char byte);
void uart_send_dec(unsigned char byte);
void uart_send_byte(unsigned char send_data);