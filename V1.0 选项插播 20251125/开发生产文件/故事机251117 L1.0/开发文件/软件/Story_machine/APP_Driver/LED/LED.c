#include "MS51_16K.H"
#include "main.h"
#include "LED.h"
#define LED1_PWM_ON_Duty 75

uint8_t LED_PWM_FREQ_Reg = 0;
uint8_t LED_DUTY_Stepping_Reg = 0;
uint8_t LED1_PWM_DUTY_Reg = 0;
uint8_t LED_PWM_Duty_RUN_program_start = 0;
uint8_t LED_PWM_FREQ = 75;
uint8_t LED_DUTY_Stepping = 0; // 1:0.4s  2:0.6s  3:0.8
uint8_t LED_mode_cnt = 0;

uint32_t LED_Time_cnt = 0;
uint16_t LED_Time_cnt1 = 0;

void LED_clr(void)
{
    LED_PWM_Duty_RUN_program_start = 0;
    LED1_PWM_DUTY_Reg = 0;
    LED_PWM_FREQ_Reg = 0;
    LED_DUTY_Stepping_Reg = 0;
    LED_Time_cnt = 0;
    LED_mode_cnt = 0;
    LED_Time_cnt1 = 0;
}
void LED_Init(void)
{
    LED_clr();
    LED_B = 0; // 灭
    LED_R = 0;
    LED_G = 0;
}
void LED_mode_select_clr(uint8_t mode) // 在单次操作中使用，带初始化
{
    LED_clr();
    LED_mode_cnt = mode;
}
void LED_mode_set(uint8_t mode) // 无变量初始化，可在连续判断中使用
{
    // LED_clr();
    LED_mode_cnt = mode;
}
void LED_PWM_Duty_RUN_program(void)
{
    if (LED_PWM_Duty_RUN_program_start == 1)
    {
        LED_PWM_FREQ_Reg += 1;               //'LED_PWM'频率控制寄存器增加
        if (LED_PWM_FREQ_Reg > LED_PWM_FREQ) //
        {
            LED_PWM_FREQ_Reg = 0;                          // 清除'LED_PWM'频率控制寄存器
            LED_DUTY_Stepping_Reg += 1;                    // 'LED_PWM'步进控制寄存器则增加
            if (LED_DUTY_Stepping_Reg > LED_DUTY_Stepping) // 0 1 2
            {

                LED_DUTY_Stepping_Reg = 0;                // 清除'LED_PWM'步进控制寄存器则增加
                LED1_PWM_DUTY_Reg += 1;                   // 'LED1_PWM'占空比控制寄存器增加
                if (LED1_PWM_DUTY_Reg > LED1_PWM_ON_Duty) //
                {
                    LED_PWM_FREQ_Reg = 0;
                    LED_DUTY_Stepping_Reg = 0;
                    LED1_PWM_DUTY_Reg = 0; // 清除'LED1_PWM'占空比控制寄存器
                    LED_PWM_Duty_RUN_program_start = 0;
                    switch (LED_mode_cnt)
                    {
                    case 2:
                        LED_mode_cnt = 3;
                        break;
                    case 3:
                        LED_mode_cnt = 2;
                        break;
                    default:
                        LED_mode_cnt = 1;
                        break;
                    }
                }
            }
        }
    }
}
void LED_MODE(uint8_t mode_cnt) // 0关闭
{
    LED_Time_cnt = 0;
    LED1_PWM_DUTY_Reg = 0;
    LED_PWM_FREQ_Reg = 0;
    LED_DUTY_Stepping_Reg = 0;
    LED_mode_cnt = mode_cnt;
}
void LED_MODE_Select(void)
{
    switch (LED_mode_cnt)
    {
    case 0:

        break;
    case 1:        // 灯全灭
        LED_B = 0; // 灭
        LED_R = 0;
        LED_G = 0;
        LED_mode_cnt = 0;
        break;
    case 2: // 白色呼吸灯
        if (LED_PWM_FREQ_Reg >= LED1_PWM_DUTY_Reg)
        {
            LED_B = 1;
            LED_R = 1;
            LED_G = 1;
        }
        else
        {
            LED_B = 0;
            LED_R = 0;
            LED_G = 0;
        }
        LED_DUTY_Stepping = 7;
        LED_PWM_Duty_RUN_program_start = 1;
        break;
    case 3: // 呼吸灯

        if (LED_PWM_FREQ_Reg <= LED1_PWM_DUTY_Reg)
        {
            LED_B = 1;
            LED_R = 1;
            LED_G = 1;
        }
        else
        {
            LED_B = 0;
            LED_R = 0;
            LED_G = 0;
        }
        LED_DUTY_Stepping = 7;
        LED_PWM_Duty_RUN_program_start = 1;
        break;
    case 4: // 白灯亮
        LED_B = 1;
        LED_R = 1;
        LED_G = 1;
        break;
    case 5: // 红灯闪两下 接灭
        LED_Time_cnt++;
        if (LED_Time_cnt == 1) // 亮
        {
            LED_B = 0;
            LED_R = 1;
            LED_G = 0;
        }
        if (LED_Time_cnt == 5000) // 灭
        {
            LED_B = 0;
            LED_R = 0;
            LED_G = 0;
        }
        if (LED_Time_cnt == 10000) // 亮
        {
            LED_B = 0;
            LED_R = 1;
            LED_G = 0;
        }
        if (LED_Time_cnt == 15000) // 灭
        {
            LED_B = 0;
            LED_R = 0;
            LED_G = 0;
            LED_Time_cnt = 0;
            LED_mode_cnt = 0;
        }
        break;
    case 6: // 红灯闪两下  接白色呼吸灯
        LED_Time_cnt++;
        if (LED_Time_cnt == 1) // 亮
        {
            LED_B = 0;
            LED_R = 1;
            LED_G = 0;
        }
        if (LED_Time_cnt == 3000) // 灭
        {
            LED_B = 1;
            LED_R = 1;
            LED_G = 1;
        }
        if (LED_Time_cnt == 6000) // 亮
        {
            LED_B = 0;
            LED_R = 1;
            LED_G = 0;
        }
        if (LED_Time_cnt == 9000) // 灭
        {
            LED_B = 1;
            LED_R = 1;
            LED_G = 1;
            LED_Time_cnt = 0;
            LED_mode_cnt = 2;
        }
        break;
    case 7: // 红灯亮
        LED_B = 0;
        LED_R = 1;
        LED_G = 0;
        break;
    case 8: // 绿灯亮
        LED_B = 0;
        LED_R = 0;
        LED_G = 1;
        break;
    case 9: // 蓝灯亮
        LED_B = 1;
        LED_R = 0;
        LED_G = 0;
        break;
    case 10: // 红灯闪两下  接白色呼吸灯
        LED_Time_cnt++;
        if (LED_Time_cnt == 1) // 亮
        {
            LED_B = 1;
            LED_R = 0;
            LED_G = 0;
        }
        if (LED_Time_cnt == 5000) // 灭
        {
            LED_B = 1;
            LED_R = 1;
            LED_G = 1;
        }
        if (LED_Time_cnt == 10000) // 亮
        {
            LED_B = 1;
            LED_R = 0;
            LED_G = 0;
        }
        if (LED_Time_cnt == 15000) // 灭
        {
            LED_B = 1;
            LED_R = 1;
            LED_G = 1;
            LED_Time_cnt = 0;
            LED_mode_cnt = 2;
        }
        break;
    case 11: // 蓝灯灯闪两下  接白灯
        LED_Time_cnt++;
        if (LED_Time_cnt == 1) // 亮
        {
            LED_B = 1;
            LED_R = 0;
            LED_G = 0;
        }
        if (LED_Time_cnt == 5000) // 灭
        {
            LED_B = 1;
            LED_R = 1;
            LED_G = 1;
        }
        if (LED_Time_cnt == 10000) // 亮
        {
            LED_B = 1;
            LED_R = 0;
            LED_G = 0;
        }
        if (LED_Time_cnt == 20000) // 灭
        {
            LED_B = 1;
            LED_R = 1;
            LED_G = 1;
            LED_Time_cnt = 0;
            LED_mode_cnt = 0;
        }
        break;
    case 12: // 蓝灯灯闪两下  接红灯
        LED_Time_cnt++;
        if (LED_Time_cnt == 1) // 亮
        {
            LED_B = 1;
            LED_R = 1;
            LED_G = 1;
        }
        if (LED_Time_cnt == 5000) // 灭
        {
            LED_B = 1;
            LED_R = 1;
            LED_G = 1;
            LED_Time_cnt = 0;
            LED_mode_cnt = 2;
        }
        break;
    default:
        break;
    }
}
UINT8 TIPLED_mode_cnt = 0;
UINT16 TIPLED_Time_cnt = 0;
void TIPLED_MODE_Select(void)
{
    switch (TIPLED_mode_cnt)
    {
    case 0:

        break;
    case 1: // 提示按键灯灭0.25s后亮
        TIPLED_Time_cnt++;
        if (TIPLED_Time_cnt == 1) // 灭
        {
            LED_KEY = 0;
        }
        if (TIPLED_Time_cnt == 5000) // 亮
        {
            LED_KEY = 1;
            TIPLED_Time_cnt = 0;
            TIPLED_mode_cnt = 0;
        }
        break;
    case 2: // 提示按键灯熄灭
        LED_KEY = 0;
        TIPLED_mode_cnt = 0;
        break;
    default:
        break;
    }
}

void TIPLED_MODE_Select_clr(UINT8 mode)
{
    TIPLED_mode_cnt = mode;
    TIPLED_Time_cnt = 0;
}
