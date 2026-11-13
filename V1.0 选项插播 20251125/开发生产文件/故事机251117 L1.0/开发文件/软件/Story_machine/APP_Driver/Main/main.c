
#include "MS51_16K.H"
#include "main.h"
#include "RC522_Function.h"
#include "RC522.h"
#include "Key.h"
#include "Sleep.h"
#include "LED.h"
#include "WT2003.h"
#define UART_TIMEOUT 1000
#define TH0_INIT 0x10
#define TL0_INIT 0x00
#define TH1_INIT 0x10
#define TL1_INIT 0x00
#define VOL_control_firstpree 0x08

uint8_t NFC_password1[4] = {0x31, 0x35, 0x39, 0x30}; // NFC密码
// 电压检测
#define VOLTAGE_BUFFER_SIZE 8
#define LOW_BATTERY_THRESHOLD33 2100 // 需要根据实际测试调整
#define LOW_BATTERY_THRESHOLD35 2200 // 需要根据实际测试调整
#define HYSTERESIS_MARGIN 50         // 迟滞值，防止LED闪烁

// 电压检测
UINT16 voltage_buffer[VOLTAGE_BUFFER_SIZE];
UINT8 voltage_buffer_index = 0;
uint8_t low_battery_flag = 0; // 低电量标志 1为低于3.3V 2低于3.5V  0为正常状态
uint8_t low_battery_cnt = 0;

// UART变量
unsigned char RevBuffer[30];
unsigned char idata Uart_data_lenght = 0;
unsigned char data SerBuffer[20];
bit Timer_cnt_start;
unsigned char Timer_cnt = 0;
extern bit UART0_receive_flag;
char temp;
bit printf_IT;

// 100ms时间计数
UINT16 u16time_cnt2 = 0;
bit Bit_time_100ms_flag = 0;
bit Bit_time_100ms_flag_2 = 0;
UINT8 u8Cycle_Time;

UINT16 u8LED_Time;
UINT8 ui_LED_time_cnt = 0;

// 定时器中断计时
UINT16 uitime_cnt;
bit uitime_1S_flag;

// 1s时间计数
UINT16 uitime_1S_cnts;

// 按键值
extern UINT8 ucKeyValue;

bit CHRG_flag = 0; // 关充电灯标识

bit BIT_NFC_LED_Flag = 0; // 识别灯标识

bit bit_Run_flag; // 开机标识 1为开机状态 0为关机状态

bit bit_open_flag = 0; // 开机不可打断标识

// 编码器相关定义
#define ENCODER_CW_KEY 0xD0  // 顺时针旋转键值
#define ENCODER_CCW_KEY 0xD1 // 逆时针旋转键值

// 编码器引脚定义
#define ENCODER_A P30 // 编码器A相连接到P3.0 (INT0)
#define ENCODER_B P17 // 编码器B相连接到P1.7

// 编码器状态变量
volatile bit encoder_event_flag = 0; // 编码器事件标志
UINT16 encoder_lock_counter = 0;     // 锁定计数器

// 音量控制变量
volatile uint8_t volume_control_cnt = 25;

// 保存nfc相关数据
uint16_t Story_Number = 0;      // 故事编号
uint8_t Story_allnodes = 0;     // 故事节点总数量
uint8_t Story_nodes_state = 0;  // 故事当前所处节点
uint8_t branches_ALLNumber = 0; // 当前节点分支总数量
uint8_t branches_Number = 0;    // 当前节点所处分支

uint8_t branches_play_flag = 0; // 节点播放中标识 0没有播放 1播放中 2播放暂停中 3播放验证通过音频

bit volume_control_flag = 0;     // 音量声音标识
uint16_t volume_control_num = 0; // 音量延时

bit NFC_flag = 0;          // 当开机时卡已经插下则自动识别
uint8_t End_play_flag = 0; // 结束音效播放标识
uint16_t Busy_time = 0;    // 等待busy反应时间变量
uint8_t uitime_1S_60s_flag = 0;

bit b_Automatic_Continuation = 0; // 自动播放下一节点

bit B_Skipped_flag = 0; // 跳过标识

bit KEY_NFC_flag = 0; // 播放结束进入待机标识 播放完故事到关机，或者松开NFC按键则清0 清0后待机状态由NFC轻触控制 识别失败也进入待机模式

unsigned char NFCdata_Save[17]; /*
 05	0：节点编号：            1：分支数量：	         2：插播索引地址高位：     3：插播索引地址低位：
 06	4：插播播放地址高位：     5：插播播放地址低位：	  6：分支1播放地址:	        7： 下个节点编号：
 07	8：分支2播放地址:	     9：下个节点编号：	     10：分支3播放地址:	       11： 下个节点编号：
 08	12：分支4播放地址:	     13：下个节点编号：	     14：分支5播放地址:	       15： 下个节点编号：
 */

void Voltage_Monitor_Function(void);

/// @brief 定时中断0
void Timer0_ISR(void) interrupt 1 // interrupt address is 0x000B   2.738ms
{
    _push_(SFRS);

    /* if define TIMER0_FSYS_DIV12, timer = (0xFFFF-0x1000)*12/24MHz = 36.58ms */
    /* if define TIMER0_FSYS, timer = (0xFFFF-0x1000)/24MHz = 2.548ms */
    TH0 = TH0_INIT;
    TL0 = TL0_INIT;
    TF0 = 0;
    uitime_cnt++;
    if (uitime_cnt >= 390) // 1s
    {
        // LED2=~LED2;
        uitime_cnt = 0;
        uitime_1S_flag = 1;
    }

    _pop_(SFRS);
}

/// @brief 定时中断1
void Timer1_ISR(void) interrupt 3 // interrupt address is 0x000B
{
    _push_(SFRS);

    TH1 = TH1_INIT; // 2.73ms
    TL1 = TL1_INIT;
    TF1 = 0;
    ui_LED_time_cnt++;
    if (ui_LED_time_cnt >= 39) // 100ms
    {
        //  LED1=~LED1;
        ui_LED_time_cnt = 0;
        Bit_time_100ms_flag_2 = 1;
    }
    _pop_(SFRS);
}
void Serial_ISR(void) interrupt 4
{
    _push_(SFRS);

    if (RI) // 接收中断
    {
        RevBuffer[Uart_data_lenght] = SBUF;
        Uart_data_lenght++;
        if (Uart_data_lenght >= 80) // 一次只能接收30个字节；
        {
            Uart_data_lenght = 80;
        }
        Timer_cnt_start = 1;
        clr_SCON_RI; // 清除接收中断标志位
    }

    if (TI) // 发送中断
    {
        // if (!PRINTFG)
        // {
        //     TI = 0;
        // }
        // 完善发送中断处理逻辑
        // ...
        TI = 0;
        printf_IT = 1;
    }

    _pop_(SFRS);
}
//-------管脚中断中断服务程序---------
void PinInterrupt(void) interrupt 7
{
    _push_(SFRS);
    if ((PIF | CLR_BIT0) == 0xFF)
    {
        PIF = 0;
    }
    if ((PIF | CLR_BIT1) == 0xFF)
    {
        PIF = 0;
    }
    if ((PIF | CLR_BIT2) == 0xFF)
    {
        PIF = 0;
    }
    if ((PIF | CLR_BIT3) == 0xFF)
    {
        PIF = 0;
    }
    _pop_(SFRS);
}

// 外部中断
/* External pin interrupt INT0 subroutine */
void INT0_ISR(void) interrupt 0 // Vector @  0x03
{
    _push_(SFRS);

    clr_TCON_IE0; // 清除INT0中断标志位

    // 检查编码器事件标志，防止重复触发
    if (encoder_event_flag == 0)
    {
        // 检查B相状态来确认是逆时针转动
        if (ENCODER_B == 1)
        {
            // B相为高电平，确认为逆时针转动
            ucKeyValue = ENCODER_CCW_KEY; // 逆时针转动键值
            encoder_event_flag = 1;
        }
    }

    _pop_(SFRS);
}
void INT1_ISR(void) interrupt 2 // Vector @  0x03
{
    _push_(SFRS);

    clr_TCON_IE1; // 清除INT1中断标志位

    // 检查编码器事件标志，防止重复触发
    if (encoder_event_flag == 0)
    {
        // 检查A相状态来确认是顺时针转动
        if (ENCODER_A == 1)
        {
            // A相为高电平，确认为顺时针转动
            ucKeyValue = ENCODER_CW_KEY; // 顺时针转动键值
            encoder_event_flag = 1;
        }
    }

    _pop_(SFRS);
}

// 外部中断初始化
void INT_Intrerupt_Init(void)
{

    P30_INPUT_MODE; // setting INT0 pin P3.0 as Quasi mode with internal pull high
    ENABLE_P30_PULLUP;
    P30 = 1;
    INT0_FALLING_EDGE_TRIG; // setting trig condition level or edge
    set_IE_EX0;             // INT0_Enable;

    P17_INPUT_MODE; // setting INT0 pin P1.7 as Quasi mode with internal pull high
    ENABLE_P17_PULLUP;
    P17 = 1;
    INT1_FALLING_EDGE_TRIG; // setting trig condition level or edge
    set_IE_EX1;             // INT0_Enable;

    ENABLE_GLOBAL_INTERRUPT; // Global interrupt enable
}

void Data_Init(void)//开机时用到
{
    unsigned char i;
    // 按键值
    bit_open_flag = 0; // 开机不可打断标识

    // 编码器状态变量
    encoder_event_flag = 0;   // 编码器事件标志
    encoder_lock_counter = 0; // 锁定计数器

    // 保存nfc相关数据
    Story_Number = 0;             // 故事编号
    Story_allnodes = 0;           // 故事节点总数量
    Story_nodes_state = 0;        // 故事当前所处节点
    branches_ALLNumber = 0;       // 当前节点分支总数量
    branches_Number = 0;          // 当前节点所处分支
    b_Automatic_Continuation = 0; // 自动播放下一节点标识

    branches_play_flag = 0; // 节点播放中标识 0没有播放 1播放中 2播放暂停中

    volume_control_flag = 0; // 音量声音标识，启动发送音量音频的标识
    volume_control_num = 0;  // 音量延时，延时发送音量音频计数

    NFC_flag = 0;      // 当开机时卡已经插下则自动识别
    End_play_flag = 0; // 结束音效播放标识
    Busy_time = 0;     // 等待busy反应时间变量
    uitime_1S_60s_flag = 0;

    B_Skipped_flag=0;//跳过标识

    for (i = 0; i < 17; i++)
    {
        NFCdata_Save[i] = 0;
    }

    for (i = 0; i < 30; i++)
    {
        RevBuffer[i] = 0;
    }
}
// 数据清除
void Data_clear(void)//松开NFC时，播放结束时，插入NFC时
{
    unsigned char i;
    for (i = 0; i < 30; i++)
    {
        RevBuffer[i] = 0;
    }
    for (i = 0; i < 17; i++)
    {
        NFCdata_Save[i] = 0;
    }

    Story_Number = 0;
    Story_allnodes = 0;
    Story_nodes_state = 0;
    branches_ALLNumber = 0;
    branches_Number = 0;
    b_Automatic_Continuation = 0; // 自动播放下一节点标识

    NFC_flag = 0;
    branches_play_flag = 0;
    End_play_flag = 0;

    B_Skipped_flag=0;//跳过标识
}

/**
 * @brief 编码器处理函数
 * 在主循环中调用，处理编码器事件
 */
void Encoder_Process(void)
{
    // 处理编码器事件
    if (encoder_event_flag)
    {
        encoder_lock_counter++;
        if (encoder_lock_counter > 10000)
        {
            encoder_event_flag = 0;
            encoder_lock_counter = 0;
        }
    }
    else
    {
        encoder_lock_counter = 0;
    }
}

// @brief
void uart_clr(void)
{
    unsigned char i;
    for (i = 0; i < 30; i++)
    {
        RevBuffer[i] = 0;
    }
    Uart_data_lenght = 0;
    UART0_receive_flag = 0;
    Timer_cnt_start = 0;
}

/// @brief

/// @brief 电池管理芯片LED控制
void CHRG_function(void)
{
    if (CHRG == 0) // 正在充电
    {
        CHRG_flag = 1;
        uitime_1S_cnts = 0; // 不进入休眠计时
        if (STDBY == 0)     // 充满电了
        {
            LED_mode_set(6); // 绿灯亮
        }
        else
        {
            LED_mode_set(5); // 红灯亮
        }
    }
    else // 不充电时灭
    {

        if (STDBY == 0) // 充满电了
        {
            CHRG_flag = 1;
            LED_mode_set(6); // 绿灯亮
        }
        else
        {
            if (CHRG_flag)
            {
                CHRG_flag = 0;
            }
        }
    }
}

/// @brief 休眠定时函数
void utime_1s_function(void)
{
    // UINT8 i;

    if (uitime_1S_flag) // 休眠
    {
        uitime_1S_flag = 0;
        uitime_1S_cnts++;
        // #if UART0_DEBUG
        //         UART0_send_string("CHRG:");
        //         uart_send_dec(CHRG);
        //         UART0_send_string("\r\nSTDBY:");
        //         uart_send_dec(STDBY);
        //         UART0_send_string("\r\n");
        // #endif
        if (bit_Run_flag == 1)
        {
            if (CHRG)
            {
                Voltage_Monitor_Function(); // 电量检测
            }
            if (uitime_1S_cnts == 60)
            {
                volume_Play_TF_SYS(0, 0, 0, 8);
                uitime_1S_60s_flag++;
            }
            if (uitime_1S_cnts == 120)
            {
                volume_Play_TF_SYS(0, 0, 0, 8);
                uitime_1S_60s_flag++;
            }
            if (uitime_1S_cnts >= 180) // 休眠时间
            {
                uitime_1S_60s_flag = 0;
                uitime_1S_cnts = 0;
                // 进入休眠
                // UART0_send_string("\r\nInto Sleep");
                bit_Run_flag = 0;               // 关机
                volume_Play_TF_SYS(0, 0, 0, 2); // 关机音效
                LED_mode_select_clr(1);         // 关灯
                // UART0_send_string("Out Sleep\r\n");
            }
        }
        else
        {
            if (uitime_1S_cnts >= 10) // 休眠时间
            {
                uitime_1S_cnts = 0;
                // 进入休眠
                // UART0_send_string("\r\nInto Sleep");
                sleep_mode();
                // UART0_send_string("Out Sleep\r\n");
            }
        }
    }
    if ((ucKeyValue > 0) || (Voices_BUSY == 1)) // 当有操作或者语音播放时
    {
        if (uitime_1S_60s_flag == 0) // 无操作少于60s时
        {
            uitime_1S_cnts = 0; // 正常清零
        }
        else // 当大于60s时
        {
            if (ucKeyValue > 0) // 当有操作时
            {
                uitime_1S_cnts = 0; // 正常清零
                uitime_1S_60s_flag = 0;
            }
        }
    }
}

void LED_function(void)
{

    if (bit_Run_flag) // 开机状态
    {
        if (low_battery_flag == 1) // 低电状态
        {
            if ((bit_Run_flag == 1) && (bit_open_flag == 0)) // 开机状态
            {
                if (Story_Number == 0) // 未插卡
                {
                    LED_mode_select_clr(6); // 红灯闪两下接呼吸灯
                }
                else
                {
                    LED_mode_select_clr(11); // 红灯闪两下接白灯
                }
                volume_Play_TF_SYS(0, 0, 0, 9);
                low_battery_flag = 2; // 已经警示过了
            }
        }
        else if (low_battery_flag == 3)
        {
            if ((bit_Run_flag == 1) && (bit_open_flag == 0)) // 开机状态
            {
                LED_mode_select_clr(5); // 红灯闪两下接灭
                volume_Play_TF_SYS(0, 0, 1, 0);
                bit_Run_flag = 0; // 关机
            }
        }
        else // 正常状态
        {
        }
    }
    else // 关机状态
    {
        if ((CHRG == 0) && (STDBY == 1)) // 正在充电
        {
            uitime_1S_cnts = 0; // 不进入休眠
            low_battery_flag = 0;
            if ((LED_G == 0) && (LED_B == 0) && (LED_R == 1))
            {
                /* code */
            }
            else
            {
                LED_mode_set(7); // 红灯亮
            }
        }
        else if ((CHRG == 1) && (STDBY == 0)) // 充满电了
        {
            uitime_1S_cnts = 0; // 不进入休眠
            low_battery_flag = 0;
            if ((LED_G == 1) && (LED_B == 0) && (LED_R == 0))
            {
                /* code */
            }
            else
            {
                LED_mode_set(8); // 绿灯亮
            }
        }
        else if ((CHRG == 1) && (STDBY == 1))
        {
            if ((LED_G == 0) && (LED_B == 0) && (LED_R == 0))
            {
                /* code */
            }
            else
            {
                LED_mode_set(1); // 灭
            }
        }
        else // 故障
        {
            if ((LED_G == 0) && (LED_B == 0) && (LED_R == 0))
            {
                /* code */
            }
            else
            {
                LED_mode_set(1); // 灭
            }
            // LED_B = CHRG;
            // LED_R = CHRG;
            // LED_G = CHRG;
            // LED_mode_set(9); // 蓝灯亮
        }
    }
}

void NFCdata_Save_clr(void)
{
    unsigned char i;
    for (i = 0; i < 30; i++)
    {
        RevBuffer[i] = 0;
    }
}

unsigned int base_decoding(uint8_t input_high, uint8_t input_low)
{
    unsigned int result = 0;

    // 处理高字节的十位
    result += (input_high / 16) * 1000; // 高字节的高4位作为千位
    result += (input_high % 16) * 100;  // 高字节的低4位作为百位

    // 处理低字节的十位
    result += (input_low / 16) * 10; // 低字节的高4位作为十位
    result += (input_low % 16);      // 低字节的低4位作为个位

    return result;
}

/// @brief 按键功能
void KEY_function(void)
{
    char status;
    //////////////////////////////////////////////////////
    if (key_get_value() > 0)
    {
        if (bit_open_flag == 1) // 开机音效
        {
            if (ucKeyValue == 0xC1) // 关机可以打断
            {
                /* code */
            }
            else
            {
                ucKeyValue = 0;
            }
        }
        if (End_play_flag == 2) // 结束音效
        {
            if ((ucKeyValue == 0xC1) || (ucKeyValue == 0xB0)) // 关机或者松开NFC可以打断
            {
                /* code */
            }
            else
            {
                ucKeyValue = 0;
            }
        }

        switch (key_get_value())
        {
        case 0xB0: // NFC轻触按键松开
#if UART0_DEBUG
            UART0_send_string("NFC_Loosen\r\n");
#endif
            if (bit_Run_flag) // 运行中
            {
                // 恢复呼吸灯
                volume_Play_TF_SYS(0, 0, 0, 6);
                LED_mode_select_clr(2); // 呼吸灯
                TIPLED_MODE_Select_clr(2); //关灯
                Story_Number = 0;       // 故事编号清0
                branches_play_flag = 0; // 播放状态清0
                Data_clear();
                KEY_NFC_flag = 0;
            }
            else
            {
            }
            break;
        case 0xB1: // NFC轻触按键按下
#if UART0_DEBUG
            UART0_send_string("NFC_Press\r\n");
#endif
            if (bit_Run_flag) // 运行中
            {
                Data_clear();
                status = Rc522_NTAG216_Password_Verify(NFC_password1);
                //                 if (status != 0)
                //                 {
                // #if UART0_DEBUG
                //                     UART0_send_string("Errd:Password verification failed 1 !!\r\n");
                // #endif
                //                     status = Rc522_NTAG216_Password_Verify(NFC_password);
                //                     if (status != 0)
                //                     {
                // #if UART0_DEBUG
                //                         UART0_send_string("Errd:Password verification failed 2 !!\r\n");
                // #endif
                //                         status = Rc522_NTAG216_Password_Verify(NFC_password);
                //                     }
                //                 }
                if (status == 0)
                {

                    LED_mode_select_clr(4);                                   // 白灯亮
                    Rc522_Read_NTAG216_Full(4);                               // 读取故事编号
                    Story_Number = base_decoding(RevBuffer[0], RevBuffer[1]); // 保存故事编号用于语音地址索引
                    NFCdata_Save_clr();
#if UART0_DEBUG
                    UART0_send_string("Story_Number : ");
                    uart_send_16byte_dec(Story_Number);
                    UART0_send_string("\r\n");
#endif
                    Rc522_Read_NTAG216_Full(5);        // 读取第一个节点
                    branches_ALLNumber = RevBuffer[0]; // 保存当前节点分支数量
                    Story_nodes_state = 1;             // 当前节点为1
                    branches_play_flag = 3;            // 播放播放验证通过时
                    branches_Number = 0;
                    volume_Play_TF_SYS(0, 0, 0, 5); // 验证通过
                    Timer2_Delay(24000000, 128, 100, 1000);
                }
                else
                {
#if UART0_DEBUG
                    UART0_send_string("Errd:Password verification failed 3 !!\r\n");
#endif
                    LED_mode_select_clr(6);         // 识别失败 红灯闪两下恢复呼吸灯
                    volume_Play_TF_SYS(0, 0, 0, 4); // 识别错误音效
                    KEY_NFC_flag = 1;
                }
                RC522_Off();
            }

            break;
        case 0xC0: // 旋钮按键短按
#if UART0_DEBUG
            UART0_send_string("P3_short\r\n");
#endif
            if (bit_Run_flag) // 已开机
            {
                if (Story_Number > 0) // 已刷卡
                {
                    if ((branches_play_flag == 1) && (Voices_BUSY == 1))
                    {
#if UART0_DEBUG
                        UART0_send_string("Tip:Volume pause!!\r\n ");
                        UART0_send_string("\r\n");
#endif

                        volume_pause();         // 暂停播放
                        branches_play_flag = 2; // 播放暂停中
                    }
                    else if ((branches_play_flag == 2) && (Voices_BUSY == 0)) // 播放暂停中 则续播
                    {

#if UART0_DEBUG
                        UART0_send_string("Tip:Volume continue!!\r\n ");
                        UART0_send_string("\r\n");
#endif
                        volume_pause(); // 继续播放
                        branches_play_flag = 1;
                    }
                    else if (branches_play_flag == 0) // 播放结束
                    {
                        if (branches_Number > 0) // 已经选择选项音频
                        {
                            switch (branches_Number)
                            {
                            case 1:
                                Story_nodes_state = RevBuffer[1]; // 选项音频1的下一主音频位置
                                break;
                            case 2:
                                Story_nodes_state = RevBuffer[2]; // 选项音频2
                                break;
                            case 3:
                                Story_nodes_state = RevBuffer[3]; // 选项音频3
                                break;
                            default:

                                break;
                            }
                            NFCdata_Save_clr();
                            Rc522_Read_NTAG216_Full((Story_nodes_state + 4));   // 读取相应节点
                            volume_Play_TF(Story_Number, Story_nodes_state, 0); // 播放节点1音频
                            branches_play_flag = 1;                             // 播放中
                            branches_Number = 0;                                // 选项音频清零
                            LED_mode_select_clr(4);                             // 白灯亮
                            TIPLED_MODE_Select_clr(2);          //关灯
                            branches_ALLNumber = RevBuffer[0];                  // 保存当前节点分支数量
                            if (branches_ALLNumber == 0)                        // 如果没有选项音频
                            {
                                if (RevBuffer[1] == 0) // 如果没有选项音频，也没有下一节点，说明是结束音效
                                {
                                    End_play_flag = 1;
#if UART0_DEBUG
                                    UART0_send_string("Tip:End play!!\r\n ");
#endif
                                }
                                else
                                {
                                    b_Automatic_Continuation = 1;     // 自动播放下一节点
                                    Story_nodes_state = RevBuffer[1]; // 跳转下一节点
                                    NFCdata_Save_clr();
                                    Rc522_Read_NTAG216_Full((Story_nodes_state + 4)); // 读取相应节点
                                    branches_ALLNumber = RevBuffer[0];                // 保存当前节点分支数量
                                }
                            }
                            else
                            {
#if UART0_DEBUG
                                UART0_send_string("Story_nodes_state: ");
                                uart_send_dec(Story_nodes_state);
                                UART0_send_string("\r\n");
#endif
                            }
                        }
                    }
                    else
                    {
                    }
                }
            }
            else // 未开机
            {
            }
            break;
        case 0xC1:            // 旋钮按键长按
                              // #if UART0_DEBUG
                              //             UART0_send_string("P3_long\r\n");
                              // #endif
            if (bit_Run_flag) //
            {
                bit_Run_flag = 0;               // 关机
                LED_mode_select_clr(1);         // 关灯
                TIPLED_MODE_Select_clr(2);  //关灯
                volume_Play_TF_SYS(0, 0, 0, 2); // 关机音效
                Story_Number = 0;               // 故事编号清0
                branches_play_flag = 0;         // 播放状态清0
                bit_open_flag = 0;
                NFCdata_Save_clr(); // 清除读取的数据
                KEY_NFC_flag = 0;
            }
            else //
            {
                if (low_battery_flag == 3)
                {
                    LED_mode_select_clr(5); // 红灯闪两下接灭
                    volume_Play_TF_SYS(0, 0, 1, 1);
                }
                else
                {
                    Data_Init();
                    bit_Run_flag = 1;               // 开机
                    volume_Play_TF_SYS(0, 0, 0, 1); // 开机音效
                    bit_open_flag = 1;              // 不可打断标识
                    LED_mode_select_clr(2);         // 呼吸灯
                    if (KEY_NFC == 1)               // NFC未插入
                    {
                        NFC_flag = 0;
                    }
                    else // NFC已插入
                    {
                        NFC_flag = 1;
                    }
                }
            }
            break;
        case 0xD0: // 旋钮顺时针旋转
#if UART0_DEBUG
            UART0_send_string("Knob++\r\n");
#endif
            if (bit_Run_flag)
            {
                if ((KEY_NFC == 1) || (KEY_NFC_flag == 1)) // NFC未插入  调节音量功能
                {

                    if (volume_control_cnt >= 10)
                    {
                        volume_control_cnt = 10;
                    }
                    else
                    {
                        volume_control_cnt++;
                    }
#if UART0_DEBUG
                    UART0_send_string("volume level: ");
                    uart_send_dec(volume_control_cnt);
                    UART0_send_string("\r\n");
#endif
                    switch (volume_control_cnt)
                    {
                    case 1:
                        volume_control(0);
                        break;
                    case 2:
                        volume_control(5);
                        break;
                    case 3:
                        volume_control(10);
                        break;
                    case 4:
                        volume_control(13);
                        break;
                    case 5:
                        volume_control(16);
                        break;
                    case 6:
                        volume_control(19);
                        break;
                    case 7:
                        volume_control(22);
                        break;
                    case 8:
                        volume_control(25);
                        break;
                    case 9:
                        volume_control(28);
                        break;
                    case 10:
                        volume_control(31);
                        break;

                    default:
                        break;
                    }
                    volume_control_flag = 1;
                    if (volume_control_cnt == 10)
                    {
                        LED_mode_select_clr(6);
                    }
                }
                else // NFC已插入  分支选择功能
                {
                    if (Story_Number > 0) // 读取到故事编号
                    {
                        if (branches_play_flag == 0) // 主音频已经播完
                        {

                            if (branches_Number >= branches_ALLNumber)
                            {
                                branches_Number = 1;
                            }
                            else
                            {
                                branches_Number++;
                            }
#if UART0_DEBUG
                            UART0_send_string("branches_Number: ");
                            uart_send_dec(branches_Number);
                            UART0_send_string("\r\n");
#endif
                            volume_Play_TF(Story_Number, Story_nodes_state, branches_Number); // 播放节点1音频
                            TIPLED_MODE_Select_clr(1);                                          // 提示按键灯灭5000后亮
                        }
                    }
                }
            }
            break;
        case 0xD1: // 旋钮逆时针旋转
#if UART0_DEBUG
            UART0_send_string("Knob--\r\n");
#endif
            if (bit_Run_flag)
            {
                if ((KEY_NFC == 1) || (KEY_NFC_flag == 1)) // NFC未插入  调节音量功能
                {

                    if ((volume_control_cnt > 10) || (volume_control_cnt <= 1))
                    {
                        volume_control_cnt = 1;
                    }
                    else
                    {
                        volume_control_cnt--;
                    }
#if UART0_DEBUG
                    UART0_send_string("volume level: ");
                    uart_send_dec(volume_control_cnt);
                    UART0_send_string("\r\n");
#endif
                    switch (volume_control_cnt)
                    {
                    case 1:
                        volume_control(0);
                        break;
                    case 2:
                        volume_control(5);
                        break;
                    case 3:
                        volume_control(10);
                        break;
                    case 4:
                        volume_control(13);
                        break;
                    case 5:
                        volume_control(16);
                        break;
                    case 6:
                        volume_control(19);
                        break;
                    case 7:
                        volume_control(22);
                        break;
                    case 8:
                        volume_control(25);
                        break;
                    case 9:
                        volume_control(28);
                        break;
                    case 10:
                        volume_control(31);
                        break;

                    default:
                        break;
                    } // 调节音量
                    volume_control_flag = 1;
                    if (volume_control_cnt == 1)
                    {  
                        LED_mode_select_clr(6);
                    }
                }
                else // NFC已插入 分支选择功能,无法打断节点音频
                {
                    if (branches_play_flag == 0) // 主音频已经播完
                    {
                        if (Story_Number > 0) // 读取到故事编号
                        {

                            if ((branches_Number <= 1) || (branches_Number > branches_ALLNumber)) // 当分支为0时，则分支为最大分支数
                            {
                                branches_Number = branches_ALLNumber;
                            }
                            else
                            {
                                branches_Number--;
                            }
#if UART0_DEBUG
                            UART0_send_string("branches_Number: ");
                            uart_send_dec(branches_Number);
                            UART0_send_string("\r\n");
#endif
                            volume_Play_TF(Story_Number, Story_nodes_state, branches_Number); // 播放节点1音频
                            TIPLED_MODE_Select_clr(1);                                          // 提示按键灯灭5000后亮
                        }
                    }
                }
            }

            break;
        case 0XE0: // 插播按键短按
#if UART0_DEBUG
            UART0_send_string("Inter_short\r\n");
#endif
            if (bit_Run_flag) //
            {
                if (KEY_NFC == 0) //
                {
                    if (Story_Number > 0) // 读取到故事编号
                    {
                        if (branches_Number > 0)
                        {
                            volume_Play_TF_option(Story_Number, Story_nodes_state, branches_Number);
                            TIPLED_MODE_Select_clr(2); // 提示按键灯熄灭
                        }

                        // if (branches_play_flag == 1) // 播放中
                        // {
                        //     // volume_insert(RevBuffer[1], RevBuffer[2]);
                        // }
                        // else if ((branches_play_flag == 3) || (branches_play_flag == 2)) // 播放验证音频时
                        // {
                        // }
                        // else // 主音频已经播完 或者 暂停
                        // {
                        //     volume_Play_TF_A4(RevBuffer[1], RevBuffer[2]);
                        // }
                    }
                }
            }
            else
            {
                // LED_KEY = 1;
                // volume_Play_TF(2, 1, 0);
                //   volume_pause();
                //   Timer2_Delay(24000000, 128, 200, 1000);
                //    volume_Play_TF_SYS(0, 0, 0, 2);
                //   volume_insert(0,2);
            }
            break;
        case 0XE1: // 插播按键长按
#if UART0_DEBUG
            UART0_send_string("Inter_long\r\n");
#endif
            if (bit_Run_flag) // 开机中
            {
                if ((branches_play_flag == 1) || (branches_play_flag == 2))
                {
                    if (End_play_flag == 1)
                    {
                        // 正在播放结束音频，不可跳过
                    }
                    else if (b_Automatic_Continuation == 1) // 自动播放下一节点
                    {
                        if (B_Skipped_flag == 0)
                        {
                            volume_Play_TF_SYS(0, 0, 0, 7); // 跳过音频
                            B_Skipped_flag = 1;
                        }
                    }
                    else // 跳过
                    {
                        volume_Play_TF_SYS(0, 0, 0, 7); // 跳过音频
                        branches_play_flag = 0;
                        LED_mode_select_clr(2);
                    }
                }
            }
            else
            {
                // LED_KEY = 0;
                low_battery_flag = 0;
                // volume_Play_TF_SYS(0, 0, 0, 1);
            }
            // volume_Play_TF(1, 2, 1);
            // // Rc522_Read_NTAG216_Full(0x04);

            // Rc522_Test();      // NFC测试
            // Rc522_Read_NTAG216_Full(0x04);
            // volume_Play_TF(2, 2, 0);
            break;
        default:
            break;
        }
    }
    key_clr();
    //////////////////////////////////////////////////
}

void volume_control_funtion(void) // 音量控制声效
{

    if (volume_control_flag)
    {
        volume_control_num++;
        if (volume_control_num > 5000)
        {
            volume_control_num = 0;
            volume_control_flag = 0;
            volume_Play_TF_SYS(0, 0, 0, 3); // 噔
        }
    }
}

/**
 * ADC初始化函数
 *
 * 初始化ADC模块，配置为读取内部通道(CH11)用于检测供电电压
 * CH11通常连接到内部VDD电压分压电路
 */
void ADC_Init(void)
{
    ENABLE_ADC_CH4;
    ADCCON1 |= 0x01; // 设置ADC时钟分频为1/2
    ADCCON2 |= 0x01; // 选择ADC转换时钟源
    ADCCON0 |= 0x80; // 使能ADC模块
}

/**
 * ADC读取供电电压函数
 */
float ADC_Read_VDD_Voltage(void)
{
    UINT16 adc_result;
    UINT16 timeout = 0;
    clr_ADCF; // 清除ADC转换完成标志
    set_ADCS; // 启动ADC转换

    // 等待转换完成，设置超时防止死循环
    timeout = 0;
    while (ADCF == 0 && timeout < 5000)
    {
        timeout++;
    }
    // 检查是否超时
    // 检查是否超时
    if (ADCF != 0)
    {
        // 读取ADC转换结果 (12位ADC)
        adc_result = (ADCRH << 4) | (ADCRL & 0x0F);
    }
    else
    {
        // 超时则使用默认值
        // adc_result = 2200;
    }
    return adc_result;
}

/**
 * 更新电压缓冲区并计算平均值
 * 使用移动平均滤波减少电压波动
 */
void Update_Voltage_Buffer(UINT16 new_voltage)
{
    static UINT32 voltage_sum = 0;
    UINT16 voltage_average;
    static bit buffer_full = 0;

    // 更新缓冲区和总和
    voltage_sum = voltage_sum - voltage_buffer[voltage_buffer_index] + new_voltage;
    voltage_buffer[voltage_buffer_index] = new_voltage;
    voltage_buffer_index++;

    if (voltage_buffer_index >= VOLTAGE_BUFFER_SIZE)
    {
        voltage_buffer_index = 0;
        buffer_full = 1;
    }

    // 计算平均电压值
    if (buffer_full)
    {
        voltage_average = voltage_sum / VOLTAGE_BUFFER_SIZE;
    }
    else
    {
        voltage_average = voltage_sum / voltage_buffer_index;
    }
    // 使用迟滞比较判断低电量状态
    if ((low_battery_flag == 0) && (voltage_average < LOW_BATTERY_THRESHOLD35))
    {
        low_battery_cnt++;
        if (low_battery_cnt > 10)
        {
            low_battery_cnt = 0;
            low_battery_flag = 1; // 电量低于3.5V，进入低电量模式
        }
    }
    else if ((low_battery_flag == 2) && (voltage_average < LOW_BATTERY_THRESHOLD33))
    {
        low_battery_cnt++;
        if (low_battery_cnt > 10)
        {
            low_battery_cnt = 0;
            low_battery_flag = 3; // 电量低于3.3V，进入低电量模式
        }
    }
    else if (voltage_average > LOW_BATTERY_THRESHOLD35 + 100)
    {
        low_battery_cnt = 0;
        low_battery_flag = 0;
    }
    // #if UART0_DEBUG
    //     UART0_send_string("Voltage avg: ");
    //     uart_send_16byte_dec(voltage_average);
    //     UART0_send_string("\r\n");
    // #endif
}

/**
 * 电压监测函数
 *
 * 定期调用此函数来监测电压状态
 */
void Voltage_Monitor_Function(void)
{
    UINT16 current_adc_value;

    // 读取当前ADC值
    current_adc_value = ADC_Read_VDD_Voltage();

    // #if 1
    //     UART0_send_string("Current ADC value: ");
    //     uart_send_16byte_dec(current_adc_value);
    //     UART0_send_string("\r\n");
    //      UART0_send_string("low_battery_flag: ");
    //     uart_send_16byte_dec(low_battery_flag);
    //     UART0_send_string("\r\n");
    // #endif

    // 更新电压缓冲区 
    Update_Voltage_Buffer(current_adc_value);
}
 
// busy结束清0变量函数
void busy_end_clr(void)
{
    if ((branches_play_flag == 1) || (branches_play_flag == 3) || (bit_open_flag == 1) || (End_play_flag == 2)) // 主音频播放   开机音效播放   结束音频播放
    {

        if (Busy_time >= 6000) // 延时时间防止busy还未拉高
        {

            if (Voices_BUSY == 0) // 播放结束就清0
            {
                Busy_time = 0;
                if (bit_open_flag == 1)
                {
                    bit_open_flag = 0;
                    if (NFC_flag == 1) // 已经插卡开始识别
                    {
                        ucKeyValue = 0xB1;
                        NFC_flag = 0;
                    }
                }
                if (branches_play_flag == 1)
                {
                    branches_play_flag = 0;
                    if (End_play_flag == 1) // 在最后一节点播放结束开始播放结束音频
                    {
                        volume_Play_TF_SYS(0, 0, 1, 1); // 结束音频
                        Busy_time = 0;
                        End_play_flag = 2; // 结束音频播放完毕，重新开始
                    }
                    else if (b_Automatic_Continuation == 1) // 自动播放下一节点
                    {
                        B_Skipped_flag=0;//可以跳过标识
                        b_Automatic_Continuation = 0;
                        volume_Play_TF(Story_Number, Story_nodes_state, 0); // 播放节点1音频
                        branches_play_flag = 1;                             // 播放中
                        branches_Number = 0;                                // 选项音频清零

                        if (branches_ALLNumber == 0) // 如果没有选项音频
                        {
                            if (RevBuffer[1] == 0) // 如果没有选项音频，也没有下一节点，说明是结束音效
                            {
                                End_play_flag = 1;
#if UART0_DEBUG
                                UART0_send_string("Tip:End play!!\r\n ");
#endif
                            }
                            else
                            {
                                b_Automatic_Continuation = 1;     // 自动播放下一节点
                                Story_nodes_state = RevBuffer[1]; // 跳转下一节点
                                NFCdata_Save_clr();
                                Rc522_Read_NTAG216_Full((Story_nodes_state + 4)); // 读取相应节点
                                branches_ALLNumber = RevBuffer[0];                // 保存当前节点分支数量
                            }
                        }
                        else
                        {
#if UART0_DEBUG
                            UART0_send_string("Story_nodes_state: ");
                            uart_send_dec(Story_nodes_state);
                            UART0_send_string("\r\n");
#endif
                        }
                    }
                    else // 如果不是结束音频则直接恢复呼吸灯
                    {
                        LED_mode_select_clr(2);
                    }
                }
                else if (branches_play_flag == 3) // 播放完验证音效，播放第一个主音频
                {
                    branches_play_flag = 1;
                    volume_Play_TF(Story_Number, 1, 0); // 播放节点1音频

                    if (branches_ALLNumber == 0) // 如果没有选项音频
                    {
                        if (RevBuffer[1] == 0) // 如果没有选项音频，也没有下一节点，说明是结束音效
                        {
                            End_play_flag = 1;
#if UART0_DEBUG
                            UART0_send_string("Tip:End play!!\r\n ");
#endif
                        }
                        else
                        {
                            b_Automatic_Continuation = 1;     // 自动播放下一节点
                            Story_nodes_state = RevBuffer[1]; // 跳转下一节点
                            NFCdata_Save_clr();
                            Rc522_Read_NTAG216_Full((Story_nodes_state + 4)); // 读取相应节点
                            branches_ALLNumber = RevBuffer[0];                // 保存当前节点分支数量
                        }
                    }
                    else
                    {
#if UART0_DEBUG
                        UART0_send_string("Story_nodes_state: ");
                        uart_send_dec(Story_nodes_state);
                        UART0_send_string("\r\n");
#endif
                    }
                }
                else if (End_play_flag == 2) //
                {
                    End_play_flag = 0;
                    // ucKeyValue = 0xB1; // 结束音效播放完毕，重新开始
                    // 恢复呼吸灯
                    // volume_Play_TF_SYS(0, 0, 0, 8);
                    LED_mode_select_clr(2); // 呼吸灯
                    Story_Number = 0;       // 故事编号清0
                    branches_play_flag = 0; // 播放状态清0
                    NFCdata_Save_clr();     // 清除读取的数据
                    Data_clear();
                    KEY_NFC_flag = 1;
                }
            }
            else
            {
                Busy_time = 5000; // 防止溢出
            }
        }
        else
        {
            Busy_time++;
        }
    }
    else
    {
        Busy_time = 0;
    }
}
// 引脚初始化
 void GPIO_Init(void)
{
    ALL_GPIO_QUASI_MODE; // 全部设置为准双向

    // 2.4G
    // RC522 SPI引脚配置
    P23_PUSHPULL_MODE; // RST   复位引脚 - 推挽输出
    P11_PUSHPULL_MODE; // SS    片选引脚 - 推挽输出
    P12_PUSHPULL_MODE; // CLK   时钟引脚 - 推挽输出
    P21_QUASI_MODE;    // MISO  数据输入 - 准双向（内部上拉）
    ENABLE_P21_PULLUP;
    P35_PUSHPULL_MODE; // MOSI  数据输出 - 推挽输出
    MF522_RST = 1;
    MF522_SCK = 1;
    MF522_SO = 1;
    MF522_SI = 1;
    MF522_NSS1 = 1;

    // 语音通信 uart通信
    P04_INPUT_MODE;    // BUSY
    P06_PUSHPULL_MODE; // TX
    P07_INPUT_MODE;    // RX 
    P06 = 1;

    // 按键
    P17_INPUT_MODE;    // 编码器  外部中断
    P30_INPUT_MODE;    // 编码器  外部中断
    ENABLE_P34_PULLUP; // 旋钮上的按键
    P34_QUASI_MODE;
    ENABLE_P03_PULLUP; // NFC轻触按键
    P03_QUASI_MODE;
    P24_PUSHPULL_MODE; // LED_KEY推挽输出
    P24 = 0;

    // 电池管理
    P25_INPUT_MODE;
    ENABLE_P25_PULLUP;
    P14_INPUT_MODE;
    ENABLE_P14_PULLUP;

    // 低电量监控
    P05_INPUT_MODE;

    // RGB灯
    P01 = 0;
    P00 = 0;
    P10 = 0;
    P01_PUSHPULL_MODE;
    P00_PUSHPULL_MODE;
    P10_PUSHPULL_MODE;
    P01 = 0;
    P00 = 0;
    P10 = 0;
}
/// @brief 主函数初始化
void main_Init(void)
{
    MODIFY_HIRC(HIRC_24); /* 修改HIRC为24MHz，以适应UART波特率功能 */
    // Timer2_Delay(24000000,128,50,1000);//等待稳定
    GPIO_Init();
    Enable_UART0_VCOM_printf_24M_9600();
    Timer_Interrupt_Enable(TIMER0);
    Timer_Interrupt_Enable(TIMER1);
    INT_Intrerupt_Init();
    ADC_Init();
    key_init();
    LED_Init();
    RC522_Init();
    ENABLE_GLOBAL_INTERRUPT; // 使能全局中断,但会使你的延时函数出现误差
    // Rc522_Test();            // NFC测试
    // Timer2_Delay(24000000,128,50,1000);//等待稳定
    // volume_output(01);       // 切换只DAC输出
}
/// @brief 主函数
void main(void)
{
    main_Init();
    Rc522_Test();            // NFC测试
    bit_Run_flag = 0;        // 关机状态
    volume_output(01);       // 切换只DAC输出
    volume_control_cnt = 10; // 开机音量最大
    low_battery_flag = 0;    // 初始化低电量标志
    LED_mode_select_clr(1);  // 灭灯
    // Timer2_Delay(24000000, 128, 200, 1000);
    // volume_control(5);
    // uart_send_hex(0x31);
    // uart_send_hex(0x32);
    while (1)
    {
        Encoder_Process();          // 编码器处理
        key_scan();                 // 按键扫描
        utime_1s_function();        // 休眠
        KEY_function();             // 按键功能
        volume_control_funtion();   // 音量控制音效
        busy_end_clr();             // busy结束清0
        LED_function();             // 灯功能
        LED_MODE_Select();          // 灯的模式选择
        LED_PWM_Duty_RUN_program(); // PWM呼吸灯控制
        TIPLED_MODE_Select();
    }
}
