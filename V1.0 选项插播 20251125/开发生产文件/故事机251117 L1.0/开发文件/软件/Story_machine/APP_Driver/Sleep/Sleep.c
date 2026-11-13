#include "Sleep.h"
#include "MS51_16K.H"
#include "main.h"
#include "RC522.h"
#include "RC522_Function.h"
#include "Function_Loop.h"
extern uint8_t u8_Card_Number;
// 按键值
extern UINT8 ucKeyValue;
/******************************************************************************
**函数：guanjiao_wakeup_on
**描述：关唤醒中断
**入口：无
**出口：无
*******************************************************************************/
void guanjiao_wakeup_on(void)
{

    // 电池管理
    // sbit STDBY        = P1^4;//充电完成标识，完成变0，平时为1
    // sbit CHRG         = P2^5;//充电进行标识，充电变0，平时为1

    // // ALL_GPIO_QUASI_MODE;

    // clr_PIPS1;
    // clr_PIPS0; // 选择端口0为中断输入源

    // ENABLE_BIT3_BOTHEDGE_TRIG;
    // ENABLE_BIT1_BOTHEDGE_TRIG;
    // ENABLE_BIT2_BOTHEDGE_TRIG;

    // Timer1_Delay(24000000, 20, 1000);
    // set_EPI; // 开启管脚中断使能

    P34_INPUT_MODE;
    P25_INPUT_MODE;
    P14_INPUT_MODE;
    P34=1;
    P25=1;
    P14=1;

    // P34_QUASI_MODE;
    // P25_QUASI_MODE;
    // P14_QUASI_MODE;
    // P01_QUASI_MODE;
    // P04_QUASI_MODE;

    // ENABLE_P01_PULLUP; // 上拉
    ENABLE_P34_PULLUP;
    ENABLE_P25_PULLUP;
    ENABLE_P14_PULLUP;
    // ENABLE_P04_PULLUP;
    // DISABLE_P01_PULLUP;
    //  DISABLE_P03_PULLUP;

    // ENABLE_PIT1_P01_BOTHEDGE; // 双边沿
    // ENABLE_PIT0_P04_BOTHEDGE;

    // ENABLE_PIT2_P03_FALLINGEDGE; // 下降沿
    // ENABLE_PIT0_P34_FALLINGEDGE;
    // ENABLE_PIT1_P25_FALLINGEDGE;
    // ENABLE_PIT2_P14_FALLINGEDGE;

    ENABLE_PIT2_P34_LOWLEVEL;
    ENABLE_PIT1_P25_LOWLEVEL;
    ENABLE_PIT0_P14_LOWLEVEL;

    ENABLE_PIN_INTERRUPT;    // Enable pin interrupt
    ENABLE_GLOBAL_INTERRUPT; // global enable bit
    set_PCON_PD;
}

/******************************************************************************
**函数：guanjiao_wakeup_off
**描述：关唤醒中断
**入口：无
**出口：无
*******************************************************************************/
void guanjiao_wakeup_off(void)
{
    DISABLE_PIN_INTERRUPT;
}

/**
 * @brief 进入休眠模式
 *
 * 该函数将使单片机进入休眠状态，以节省电力。在进入休眠后，单片机将停止执行代码，直到有外部事件唤醒它。
 *
 * @param 无
 * @return 无
 */
void sleep_on(void)
{
    PCON |= 0x02; // 设置PCON寄存器，使单片机进入掉电模式的休眠状态
}

void UART0_Close(void)
{
    // 禁用UART0中断
    DISABLE_UART0_INTERRUPT;

    // 清除UART0接收和发送中断标志
    clr_SCON_RI; // 清除接收中断标志
    clr_SCON_TI; // 清除发送中断标志

    // 清零UART0控制寄存器
    SCON = 0x00; // 禁用UART0

    // 关闭UART0相关的时钟源（假设使用Timer1）
    clr_TCON_TR1; // 停止Timer1
}
void Timer0_Close(void)
{
    // 禁用定时器0中断
    DISABLE_TIMER0_INTERRUPT;

    // 清除定时器0中断标志
    clr_TCON_TF0; // 清除定时器0溢出标志

    // 停止定时器0运行
    clr_TCON_TR0; // 停止定时器0

    // 清零定时器0寄存器
    TH0 = 0x00; // 清零高字节
    TL0 = 0x00; // 清零低字节
}
void Timer1_Close(void)
{
    // 禁用定时器1中断
    DISABLE_TIMER1_INTERRUPT;

    // 清除定时器1中断标志
    clr_TCON_TF1; // 清除定时器1溢出标志

    // 停止定时器1运行
    clr_TCON_TR1; // 停止定时器1

    // 清零定时器1寄存器
    TH1 = 0x00; // 清零高字节
    TL1 = 0x00; // 清零低字节
}
void sleep_mode(void)
{

    RC522_Off(); // 关闭NFC

    Timer0_Close(); // 关闭定时器0
    Timer1_Close();
    clr_ADCEN;     // 关闭ADC
    clr_BODEN;     // 关闭BOD欠压检测
    UART0_Close(); // 关闭UART0

    // 必须处理好每个引脚的模式和状态。每个引脚状态需要外部上、下拉一致，
    // 比如上拉就要输出1，下拉就要输出0。如果引脚是悬浮的，建议用户配置端口为准双向模式。

    // 烧写口浮空（P2.0、P02、P16）设置准双向
    P20_QUASI_MODE;
    P02_QUASI_MODE;
    P16_QUASI_MODE;

    // 按键设置准双向
    P03_QUASI_MODE;
    P34_QUASI_MODE;
    P33_QUASI_MODE;

    // 语音芯片（UART通信 P06(TX)、P07(RX)）06得置1，7待定 （busy（P34）、P15 ）
    P06_QUASI_MODE;
    // ENABLE_P06_PULLUP;
    P06 = 1;
    P07_INPUT_MODE;
    P07 = 0;
    P04_INPUT_MODE;
    Voices_BUSY = 0;

    // NFC模块  （P1.0、P00、P01、P33（rst得置0）、P11、P12、P35、P21、P22、P23、P24、P13（IRQ） ）
    P23_QUASI_MODE;
    MF522_RST = 0;
    MF522_SCK = 0;
    MF522_SO = 0;
    MF522_SI = 0;
    MF522_NSS1 = 0;

    P01_QUASI_MODE;
    P00_QUASI_MODE;
    P10_QUASI_MODE;
    LED_R = 0;
    LED_G = 0;
    LED_B = 0;

    // 电池   （P25、P14）输入，外部都为1
    P14_QUASI_MODE;
    P25_QUASI_MODE;
    STDBY = 1;
    CHRG = 1;

    // ADC
    P05_QUASI_MODE;

    // 编码器
    P17 = 1;
    P30 = 1;
    ENABLE_P17_PULLUP;
    ENABLE_P30_PULLUP;

    guanjiao_wakeup_on(); // 开启引脚中断

    sleep_on();
    // NOP_Delay_10us(20);
    guanjiao_wakeup_off();
    // P06 = 1;
    // P07 = 1;

    main_Init();
   
   
    #if 1//UART0_DEBUG
        UART0_send_string("\r\n OUT Sleep");
    #endif
}