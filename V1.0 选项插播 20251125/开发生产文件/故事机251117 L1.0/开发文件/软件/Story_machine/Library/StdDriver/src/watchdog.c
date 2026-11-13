/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2020 nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

//***********************************************************************************************************
//  Website: http://www.nuvoton.com
//  E-Mail : MicroC-8bit@nuvoton.com
//***********************************************************************************************************

#include "MS51_16K.H"

/** 
 * @brief 启用看门狗定时器复位配置
 * @details 通过IAP（在应用编程）修改配置寄存器，启用看门狗定时器的复位功能。
 * @note 该函数需要确保在调用时系统处于可编程状态。
 * @return 无
 */
void Enable_WDT_Reset_Config(void)
{
    /**
     * @brief 配置IAP寄存器以执行配置字节编程
     * @details 设置CHPCON的IAPEN位，配置IAP地址、频率和操作码。
     */
    set_CHPCON_IAPEN;
    IAPAL = 0x04;
    IAPAH = 0x00;
    IAPFD = 0x0F;
    IAPCN = BYTE_PROGRAM_CONFIG;

    /**
     * @brief 触发IAP编程操作
     * @details 启用CFUEN并触发IAP执行。
     */
    set_IAPUEN_CFUEN;
    set_IAPTRG_IAPGO;

    /**
     * @brief 等待IAP编程完成
     * @details 持续检查CHPCON的IAPFF位（位6）是否被清除。
     */
    while ((CHPCON & SET_BIT6) == SET_BIT6);

    /**
     * @brief 清除IAP相关配置
     * @details 禁用CFUEN和IAPEN，确保后续操作安全。
     */
    clr_IAPUEN_CFUEN;
    clr_CHPCON_IAPEN;

    /**
     * @brief 触发系统软件复位
     * @details 通过设置CHPCON的SWRST位执行软件复位。
     */
    set_CHPCON_SWRST;
}

/****************************************************************************/
/* WDT CONFIG disable                                                       */
/* warning : this macro is for CONFIG already enable WDT Reset to disable.  */
/* Since erase CONFIG is full page                                          */
/* step 1. storage CONFIG value in RAM                                      */
/* step 2. modify RAM CONFIG4 WDT value to disable                          */
/* step 3. erase CONFIG page                                                */
/* step 4. re writer CONFIG.                                                */
/* **************************************************************************/
/* 
 * 函数名: Disable_WDT_Reset_Config
 * 功能: 禁用看门狗定时器（WDT）的复位功能，通过修改配置寄存器实现。
 * 参数: 无参数。
 * 返回值: 无返回值。
 */
void Disable_WDT_Reset_Config(void)
{
    unsigned char cf0, cf1, cf2, cf3, cf4;

    /* 启用IAP模式，允许读写配置寄存器 */
    set_CHPCON_IAPEN;
    IAPAL = 0x00;
    IAPAH = 0x00;
    IAPCN = BYTE_READ_CONFIG;
    set_IAPTRG_IAPGO;                                  //Storage CONFIG0 data
    cf0 = IAPFD;
    IAPAL = 0x01;
    set_IAPTRG_IAPGO;                                  //Storage CONFIG1 data
    cf1 = IAPFD;
    IAPAL = 0x02;
    set_IAPTRG_IAPGO;                                  //Storage CONFIG2 data
    cf2 = IAPFD;
    IAPAL = 0x03;
    set_IAPTRG_IAPGO;                                  //Storage CONFIG3 data
    cf3 = IAPFD;
    IAPAL = 0x04;
    set_IAPTRG_IAPGO;                                  //Storage CONFIG4 data
    cf4 = IAPFD;

    /* 修改CONFIG4寄存器值以禁用WDT复位功能 */
    cf4 |= 0xF0;                                      //Moidfy Storage CONFIG4 data disable WDT reset

    /* 擦除配置页，准备写入新数据 */
    set_IAPUEN_CFUEN;
    IAPCN = PAGE_ERASE_CONFIG;                        //Erase CONFIG all
    IAPAH = 0x00;
    IAPAL = 0x00;
    IAPFD = 0xFF;
    set_IAPTRG_IAPGO;

    /* 将修改后的配置写回存储器 */
    IAPCN = BYTE_PROGRAM_CONFIG;                    //Write CONFIG
    IAPFD = cf0;
    set_IAPTRG_IAPGO;
    IAPAL = 0x01;
    IAPFD = cf1;
    set_IAPTRG_IAPGO;
    IAPAL = 0x02;
    IAPFD = cf2;
    set_IAPTRG_IAPGO;
    IAPAL = 0x03;
    IAPFD = cf3;
    set_IAPTRG_IAPGO;
    IAPAL = 0x04;
    IAPFD = cf4;
    set_IAPTRG_IAPGO;

    /* 关闭IAP模式并释放资源 */
    set_IAPUEN_CFUEN;
    clr_CHPCON_IAPEN;

    /* 检查并触发复位以应用配置更改 */
    if (WDCON & SET_BIT3)
    {
        clr_WDCON_WDTRF;
        set_CHPCON_SWRST;
    }
}
void WDT_Reset_Init(void)
{
    
    TA=0xAA;TA=0x55;WDCON|=0x07;                //Setting WDT prescale 
    set_WDCON_WDCLR;                            //Clear WDT timer
    while((WDCON|~SET_BIT6)==0xFF);             //confirm WDT clear is ok before into power down mode

    set_WDCON_WDTR;                             //WDT run
}
// 需在代码中实现喂狗函数（示例）
void Feed_Watchdog(void) {
    set_WDCON_WDCLR;  // 重置看门狗计数器（需参考芯片手册确认具体操作）
}