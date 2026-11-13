
#include "MS51_16K.H"
bit PRINTFG = 0, uart0_receive_flag = 0, uart1_receive_flag;
unsigned char uart0_receive_data, uart1_receive_data;

// void Serial_ISR(void) interrupt 4
// {
//   _push_(SFRS);

//   if (RI)
//   {
//     uart0_receive_flag = 1;
//     uart0_receive_data = SBUF;
//     clr_SCON_RI; // Clear RI (Receive Interrupt).
//   }

//   if (TI)
//   {
//     if (!PRINTFG)
//     {
//       TI = 0;
//     }
//   }

//   _pop_(SFRS);
// }


// void SerialPort1_ISR(void) interrupt 15
// {
//   _push_(SFRS);

//   if (RI_1)
//   {
//     uart1_receive_data = SBUF_1;
//     uart1_receive_flag = 1;
//     clr_SCON_1_RI_1; /* clear reception flag for next reception */
//   }

//   if (TI_1 == 1)
//   {
//     if (!PRINTFG)
//     {
//       clr_SCON_1_TI_1; /* if emission occur */
//     }
//   }

//   _pop_(SFRS);
// }

/*MS51 new version buadrate */
void UART_Open(unsigned long u32SysClock, unsigned char u8UARTPort, unsigned long u32Baudrate)
{
  switch (u8UARTPort)
  {
  case UART0_Timer1:
    SCON = 0x50;   // UART0 Mode1,REN=1,TI=1
    TMOD |= 0x20;  // Timer1 Mode1
    set_PCON_SMOD; // UART0 Double Rate Enable
    set_CKCON_T1M;
    clr_T3CON_BRCK; // Serial port 0 baud rate clock source = Timer1
    TH1 = 256 - (u32SysClock / 16 / u32Baudrate);
    set_TCON_TR1;
    break;

  case UART0_Timer3:
    SCON = 0x50;    // UART0 Mode1,REN=1,TI=1
    set_PCON_SMOD;  // UART0 Double Rate Enable
    T3CON &= 0xF8;  // T3PS2=0,T3PS1=0,T3PS0=0(Prescale=1)
    set_T3CON_BRCK; // UART0 baud rate clock source = Timer3
    RH3 = HIBYTE(65536 - (u32SysClock / 16 / u32Baudrate));
    RL3 = LOBYTE(65536 - (u32SysClock / 16 / u32Baudrate));
    set_T3CON_TR3; // Trigger Timer3
    break;

  case UART1_Timer3:
    SCON_1 = 0x50; // UART1 Mode1,REN_1=1,TI_1=1
    T3CON = 0x80;  // T3PS2=0,T3PS1=0,T3PS0=0(Prescale=1), UART1 in MODE 1
    RH3 = HIBYTE(65536 - (u32SysClock / 16 / u32Baudrate));
    RL3 = LOBYTE(65536 - (u32SysClock / 16 / u32Baudrate));
    set_T3CON_TR3; // Trigger Timer3
    break;
  }
}

#if 0 
unsigned char Receive_Data(unsigned char UARTPort)
{
    UINT8 c;
    switch (UARTPort)
    {
      case UART0:
        while (!RI);
        c = SBUF;
        RI = 0;
      break;
      case UART1:
        while (!RI_1);
        c = SBUF_1;
        RI_1 = 0;
      break;
    }
    return (c);
}
#endif

void UART_Send_Data(UINT8 UARTPort, UINT8 c)
{
  switch (UARTPort)
  {
  case UART0:
    {
    TI    =   0;              //清零串口发送完成中断请求标志
    SBUF   = c;
	while(TI ==0);              //等待发送完成 	
    TI    =   0;              //清零串口发送完成中断请求标志

}
    break;
  case UART1:
    SBUF_1 = c;
    break;
  }
}

void Enable_UART0_VCOM_printf_24M_115200(void)
{
  P06_QUASI_MODE;
  P07_QUASI_MODE;
  P06_PUSHPULL_MODE;
  UART_Open(24000000, UART0_Timer3, 115200);
  ENABLE_UART0_INTERRUPT;
  ENABLE_UART0_PRINTF;
}
void Enable_UART0_VCOM_printf_24M_9600(void)
{
  P06_QUASI_MODE;
  P07_QUASI_MODE;
	P06_PUSHPULL_MODE;
	// P07_PUSHPULL_MODE;
  UART_Open(24000000, UART0_Timer3, 9600);
  ENABLE_UART0_INTERRUPT;
  ENABLE_UART0_PRINTF;
}
void UART0_send_string(unsigned char *buf)
{
  while (*buf)
  {
    UART_Send_Data(UART0, *buf++);
  }
}

// 辅助函数：将一个字节转换为两个ASCII字符
void byte_to_hex(unsigned char byte, char *hex_str) {
    const char hex_chars[] = "0123456789ABCDEF";
    hex_str[0] = hex_chars[(byte >> 4) & 0x0F];
    hex_str[1] = hex_chars[byte & 0x0F];
}

// 发送16进制数
void uart_send_hex(unsigned char byte) {
    char hex_str[2]; // 两个字符加一个空字符
        byte_to_hex(byte, hex_str);
        UART0_send_string(hex_str);
    
}
// 手动实现将字节转换为十进制字符串
void byte_to_dec(unsigned char byte, char *dec_str) {
    if (byte >= 100) {
        dec_str[0] = '0' + (byte / 100);
        dec_str[1] = '0' + ((byte % 100) / 10);
        dec_str[2] = '0' + (byte % 10);
        dec_str[3] = '\0'; // 字符串结束符
    } else if (byte >= 10) {
        dec_str[0] = '0' + (byte / 10);
        dec_str[1] = '0' + (byte % 10);
        dec_str[2] = '\0'; // 字符串结束符
    } else {
        dec_str[0] = '0' + byte;
        dec_str[1] = '\0'; // 字符串结束符
    }
}
void uint16_to_dec(uint16_t value, char *dec_str) {
    // 确保输入值在0-65535范围内（16位无符号整数）
    if (value >= 10000) {
        dec_str[0] = '0' + (value / 10000);               // 万位
        dec_str[1] = '0' + ((value % 10000) / 1000);      // 千位
        dec_str[2] = '0' + ((value % 1000) / 100);        // 百位
        dec_str[3] = '0' + ((value % 100) / 10);          // 十位
        dec_str[4] = '0' + (value % 10);                  // 个位
        dec_str[5] = '\0';                                // 字符串结束符
    } else if (value >= 1000) {
        dec_str[0] = '0' + (value / 1000);                // 千位
        dec_str[1] = '0' + ((value % 1000) / 100);        // 百位
        dec_str[2] = '0' + ((value % 100) / 10);          // 十位
        dec_str[3] = '0' + (value % 10);                  // 个位
        dec_str[4] = '\0';                                // 字符串结束符
    } else if (value >= 100) {
        dec_str[0] = '0' + (value / 100);                 // 百位
        dec_str[1] = '0' + ((value % 100) / 10);          // 十位
        dec_str[2] = '0' + (value % 10);                  // 个位
        dec_str[3] = '\0';                                // 字符串结束符
    } else if (value >= 10) {
        dec_str[0] = '0' + (value / 10);                  // 十位
        dec_str[1] = '0' + (value % 10);                  // 个位
        dec_str[2] = '\0';                                // 字符串结束符
    } else {
        dec_str[0] = '0' + value;                          // 个位
        dec_str[1] = '\0';                                // 字符串结束符
    }
}

// 发送十进制数
void uart_send_dec(unsigned char byte) {
    char dec_str[4]; // 最多3位十进制数（0-255）加一个空字符
    byte_to_dec(byte, dec_str); // 将字节转换为十进制字符串
    UART0_send_string(dec_str); // 发送字符串
}
void uart_send_16byte_dec(uint16_t value) {
    char dec_str[7]; // 最多需要6个字符（5位数字 + 结束符'\0'）
    uint16_to_dec(value, dec_str); // 将16位无符号整数转换为十进制字符串
    UART0_send_string(dec_str);    // 发送字符串
}

void uart_send_byte(unsigned char send_data)
{
  UART_Send_Data(UART0, send_data);
}