/*
 * @Author:  cray 485562844@QQ.com
 * @Date: 2025-02-06 14:52:25
 * @LastEditTime: 2025-09-08 11:26:36
 * @座右铭: Learning makes me confident
 */

#include "Key.h"
#include "MS51_16K.h"
#define SHORT_PRESS_THRESHOLD 2000 // 短按时间 500=20MS
#define LONG_PRESS_THRESHOLD 40000

#define KEY_1_open 0 // 按键1  短按，长按累加
#define KEY_2_open 1 // 按键2  拨码类型
#define KEY_3_open 1 // 按键3  短按，长按
#define KEY_4_open 0 // 按键4 短按 双击 三击。。， 长按
#define KEY_5_open 1 // 按键5  短按，长按

// //按键
// sbit KEY_L     = P0^4;//左灯板按键 短按 长按
// sbit KEY_R     = P1^3;//右灯板按键 短按 长按
// sbit KEY_S     = P1^7;//主板按键 按与松
#define KEY_3 KEY_Knob // 短按，长按
#define KEY_5 KEY_Inter // 短按，长按
#define KEY_2 KEY_NFC // 拨码类型

UINT8 ucKeyValue; // 按键返回值

// 按键1  短按，长按累加
// UINT8 ucKeyLock1;     // 默认松开，单点按键
// UINT8 ucShortKeyCnt1; // 短按次数计数
// UINT32 uiKeyCnt1;     // 按下时间计时
// UINT16 uiKeylongCnt1; // 长按计时
// UINT8 uiKeylongCnt2;  // 长按连续触发计数器

// 按键2  拨码类型
UINT8 ucKeyLock2;
UINT16 uiKeyCnt2, uiKeyCnt22;

// 按键3  短按，长按
UINT8 ucKeyLock3; // 默认松开，单点按键
UINT8 ucShortKeyCnt3;
UINT32 uiKeyCnt3;

// 按键4 短按 双击 三击。。， 长按
// UINT8 ucKeyLock4 = 0; // 默认松开，拨码类型
// UINT16 uiKeyCnt4 = 0;
// UINT16 uiKey4ReleaseCnt = 0;
// UINT32 uiKey4LongPressCnt = 0;
// UINT8 u_key4short_press_flag = 0; // 第一次短按标识

// 按键5  短按，长按
UINT8 ucKeyLock5; // 默认松开，单点按键
UINT8 ucShortKeyCnt5;
UINT32 uiKeyCnt5;


// 按键6
//  按键初始化，注意：当进入休眠后可能不要初始化按键变量，可能会导致重复触发按键
void key_init(void)
{
#if KEY_1_open
    ucKeyLock1 = 0;
    ucShortKeyCnt1 = 0;
    uiKeyCnt1 = 0;
    uiKeylongCnt1 = 0;
    uiKeylongCnt2 = 0;
#endif
#if KEY_2_open
    if (KEY_2)
    {
        ucKeyLock2 = 1;
    }
    else
    {
        ucKeyLock2 = 0;
    }
    uiKeyCnt22 = 0;
    uiKeyCnt2 = 0;
#endif
#if KEY_3_open
    ucKeyLock3 = 0;
    ucShortKeyCnt3 = 0;
    uiKeyCnt3 = 0;
#endif
#if KEY_4_open
    ucKeyLock4 = 0;
    uiKeyCnt4 = 0;
    uiKey4ReleaseCnt = 0;
    uiKey4LongPressCnt = 0;
    u_key4short_press_flag = 0;
#endif
#if KEY_5_open
    ucKeyLock5 = 0;
    ucShortKeyCnt5 = 0;
    uiKeyCnt5 = 0;
#endif


}
void key_lock_clr(void)
{
}
unsigned char key_get_value(void)
{
    return ucKeyValue;
}
void key_clr(void)
{
    ucKeyValue = 0;
}
void key_scan(void)
{
#if KEY_1_open
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (KEY_1 == 1) // 没有按下
    {
        ucKeyLock1 = 0;    // 清除锁
        uiKeyCnt1 = 0;     // 重置计数器
        uiKeylongCnt1 = 0; // 重置长按计数器
        uiKeylongCnt2 = 0; // 重置长按连续触发计数器
        if (ucShortKeyCnt1 > 0)
        {
            ucKeyValue = 0XA0;
            ucShortKeyCnt1 = 0;
        }
    }
    else if (ucKeyLock1 == 0)
    {
        uiKeyCnt1++;
        if (uiKeyCnt1 == SHORT_PRESS_THRESHOLD) // 单次按下防抖时间，只满足40MS那个点
        {
            ucShortKeyCnt1++;
        }
        if (uiKeyCnt1 >= 12000) // 长按时间阈值
        {
            ucShortKeyCnt1 = 0;
            uiKeylongCnt1++;
            uiKeyCnt1 = 12000;
            if (uiKeylongCnt1 > 5000) // 长按持续时间阈值
            {
                uiKeylongCnt1 = 0;
                ucKeyValue = 0XA1;
                uiKeylongCnt2++;
                if (uiKeylongCnt2 == 15) // 达到15次长按
                {
                    ucKeyLock1 = 1;    // 设置自锁标志
                    uiKeylongCnt2 = 0; // 重置长按连续触发计数器
                }
            }
        }
    }
#endif
#if KEY_2_open
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if ((KEY_2 == 1) && (ucKeyLock2 == 0)) // 松开
    {
        uiKeyCnt2 = 0;
        uiKeyCnt22++;
        if (uiKeyCnt22 >= SHORT_PRESS_THRESHOLD)
        {
            ucKeyValue = 0xB0;
            uiKeyCnt22 = 0;
            ucKeyLock2 = 1;
        }
    }
    else if ((KEY_2 == 0) && (ucKeyLock2 == 1)) // 按下
    {
        uiKeyCnt22 = 0;
        uiKeyCnt2++;
        if (uiKeyCnt2 >= SHORT_PRESS_THRESHOLD)
        {
            ucKeyValue = 0xB1;
            uiKeyCnt2 = 0;
            ucKeyLock2 = 0;
        }
    }
#endif
#if KEY_3_open
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (KEY_3 == 1 ) // 没有按下
    {
        ucKeyLock3 = 0;
        uiKeyCnt3 = 0;

        if (ucShortKeyCnt3 > 0)
        {
            ucKeyValue = 0XC0;
            ucShortKeyCnt3 = 0;
        }
    }
    else if (ucKeyLock3 == 0 )
    {
        uiKeyCnt3++;
        if (uiKeyCnt3 == SHORT_PRESS_THRESHOLD) // 800         //单次按下防抖时间，只满足40MS那个点
        {

            ucShortKeyCnt3++;
        }
        if (uiKeyCnt3 >= LONG_PRESS_THRESHOLD) // 长按1秒
        {
            ucShortKeyCnt3 = 0;
            uiKeyCnt3 = 0;
            ucKeyLock3 = 1;
            ucKeyValue = 0XC1;
        }
    }
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
#if KEY_4_open
    if (KEY_4 == 1) // 松开状态
    {
        ucKeyLock4 = 0;
        uiKey4LongPressCnt = 0; // 重置长按计时
        if (u_key4short_press_flag > 0)
        {
            uiKey4ReleaseCnt++;          // 释放计时
            if (uiKey4ReleaseCnt > 5000) // 0.25s内没有按下则输出结果
            {
                if (u_key4short_press_flag == 1)
                {
                    ucKeyValue = 0xD0; // 设置键值为单击D
                }
                else if (u_key4short_press_flag == 2)
                {
                    ucKeyValue = 0xD2; // 设置键值为双击
                }
                else
                {
                    ucKeyValue = 0xD2; // 设置键值为双击
                }
                uiKey4ReleaseCnt = 0;       // 重置释放计时
                u_key4short_press_flag = 0; // 按键操作完成，清除标识
            }
            else // 又再次按下了
            {
            }
        }
        else
        {
        }
    }
    else if (ucKeyLock4 == 0) // 按下
    {
        uiKey4ReleaseCnt = 0;                            // 重置释放计时
        uiKey4LongPressCnt++;                            // 长按计时
        if (uiKey4LongPressCnt == SHORT_PRESS_THRESHOLD) // 消抖
        {
            u_key4short_press_flag++;
            if (u_key4short_press_flag >= 3) // 三次
            {
                ucKeyValue = 0xD2;          // 设置键值为双击
                uiKey4LongPressCnt = 0;     // 重置长按计时
                ucKeyLock4 = 1;             // 锁定键，防止重复触发
                u_key4short_press_flag = 0; // 清除短按
            }
        }
        if (uiKey4LongPressCnt >= LONG_PRESS_THRESHOLD) // 长按阈值 1.6s
        {
            ucKeyValue = 0xD1;          // 设置键值为长按
            uiKey4LongPressCnt = 0;     // 重置长按计时
            ucKeyLock4 = 1;             // 锁定键，防止重复触发
            u_key4short_press_flag = 0; // 清除短按
        }
    }
#endif
#if KEY_5_open
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (KEY_5 == 1) // 没有按下
    {
        ucKeyLock5 = 0;
        uiKeyCnt5 = 0;

        if (ucShortKeyCnt5 > 0)
        {
            ucKeyValue = 0XE0;
            ucShortKeyCnt5 = 0;
        }
    }
    else if (ucKeyLock5 == 0)
    {
        uiKeyCnt5++;
        if (uiKeyCnt5 == SHORT_PRESS_THRESHOLD) // 800         //单次按下防抖时间，只满足40MS那个点
        {
            ucShortKeyCnt5++;
        }
        if (uiKeyCnt5 >= LONG_PRESS_THRESHOLD) // 长按1秒
        {
            ucShortKeyCnt5 = 0;
            uiKeyCnt5 = 0;
            ucKeyLock5 = 1;
            ucKeyValue = 0XE1;
        }
    }
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

}
