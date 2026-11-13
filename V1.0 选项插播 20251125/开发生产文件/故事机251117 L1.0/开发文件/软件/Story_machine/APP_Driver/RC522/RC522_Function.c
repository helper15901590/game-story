/*
 * @LastEditors: T sir
 * @LastEditTime: 2025-10-14 14:46:37
 */
#include "MS51_16K.H"
#include "RC522.H"
#include "main.h"
#include "Function_Loop.h"
extern unsigned char RevBuffer[30];
unsigned char idata MLastSelectedSnr[4];                                                                                                // 4个字节卡片序列号
unsigned char RevBuffer_Save[30];                                                                                                       // 卡片数据保存
unsigned char u8_Card_data_Save[16] = {0x42, 0x41, 0x4e, 0x44, 0x41, 0x49, 0x0B, 0x1d, 0x05, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00}; // 卡片数据保存
uint8_t u8_Card_Number = 0;
extern bit b_UniversalCard_flag;
extern uint8_t u8_UniversalCard_cnt; // 万能卡计数
unsigned char NFC_pack[2] = {0x32, 0x32};
// 抽卡时将数据、卡片编号、万能卡使能清零

void Save_DATA_clr(void)
{
    unsigned char i;
    for (i = 0; i < 30; i++)
    {
        RevBuffer_Save[i] = 0;
    }
}
void DATA_clr(void)
{
    unsigned char i;
    for (i = 0; i < 30; i++)
    {
        RevBuffer[i] = 0;
    }
}
// 计算卡片编号
void Compute_Card_Number(void)
{
    uint8_t u8_Card_Number_high, u8_Card_Number_low, u8_Card_Number_bai;
    // uint8_t i;
    u8_Card_Number_high = 0;
    u8_Card_Number_low = 0;
    u8_Card_Number_bai = 0;
    // 89-112
    if (RevBuffer_Save[15] == 0x30 || RevBuffer_Save[15] == 0x31) // 30=0  31=1
    {
        if (RevBuffer_Save[17] > 0 || RevBuffer_Save[16] > 0)
        {
            u8_Card_Number_bai = RevBuffer_Save[15] - 0x30;
            u8_Card_Number_low = RevBuffer_Save[17] - 0x30;
            u8_Card_Number_high = RevBuffer_Save[16] - 0x30;
            u8_Card_Number = u8_Card_Number_bai * 100 + u8_Card_Number_high * 10 + u8_Card_Number_low;

#if RC522_DEBUG
            UART0_send_string("\r\n");
            UART0_send_string("1Card Number: ");
            uart_send_dec(u8_Card_Number);
            UART0_send_string("\r\n");
#endif
        }
    }
    else // 1-89
    {
        //         for (i = 0; i < 16; i++)
        //         {
        //             if (RevBuffer_Save[i] == u8_Card_data_Save[i])
        //             {
        //             }
        //             else
        //             {
        // #if RC522_DEBUG
        //                 UART0_send_string("\r\nError: Card Data No Same!! \r\n");
        // #endif
        //                 return;
        //             }
        //         }
        if (RevBuffer_Save[17] > 0 || RevBuffer_Save[16] > 0)
        {
            u8_Card_Number_high = RevBuffer_Save[17] - 0x70;
            u8_Card_Number_low = (RevBuffer_Save[16] & 0xF0) >> 4;
            u8_Card_Number = u8_Card_Number_high * 16 + u8_Card_Number_low;
#if RC522_DEBUG
            UART0_send_string("\r\n");
            UART0_send_string(" 2Card Number: ");
            uart_send_dec(u8_Card_Number);
            UART0_send_string("\r\n");
#endif
        }
        else
        {
            u8_Card_Number = 0;
#if RC522_DEBUG
            UART0_send_string("Error:No Card!! \r\n");
#endif
        }
    }
    // UART0_send_string("Card Number: ");
    Save_DATA_clr();
}
// M1卡的某一块写为如下格式，则该块为钱包，可接收扣款和充值命令
// 4字节金额（低字节在前）＋4字节金额取反＋4字节金额＋1字节块地址＋1字节块地址取反＋1字节块地址＋1字节块地址取反
// unsigned char code data2[4] = {0x12, 0, 0, 0};
// unsigned char code DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
// unsigned char g_ucTempbuf[20];
/*
void iccardcode()
{
    unsigned char cmd;
    unsigned char status;

    cmd = RevBuffer[0];
    switch (cmd)
    {
    case 1: // Halt the card     //终止卡的操作
        status = PcdHalt();
        ; // 功    能：命令卡片进入休眠状态
        RevBuffer[0] = 1;
        RevBuffer[1] = status;
        break;
    case 2: // Request,Anticoll,Select,return CardType(2 bytes)+CardSerialNo(4 bytes)
            // 寻卡，防冲突，选择卡    返回卡类型（2 bytes）+ 卡系列号(4 bytes)
#if RC522_DEBUG
        printf(" Finding Card...\r\n");
#endif
        status = PcdRequest(RevBuffer[1], &RevBuffer[2]);
        if (status != 0) // 错误
        {
            status = PcdRequest(RevBuffer[1], &RevBuffer[2]);
            if (status != 0)
            {
                RevBuffer[0] = 1;
                RevBuffer[1] = status;
#if RC522_DEBUG
                printf(" No Card!! \r\n");
#endif

                break;
            }
        }

        RevBuffer[0] = 3;
        RevBuffer[1] = status;
        break;

    case 3:                                  // 防冲突 读卡的系列号 MLastSelectedSnr
        status = PcdAnticoll(&RevBuffer[2]); // 功    能：防冲撞
        if (status != 0)
        {
            RevBuffer[0] = 1;
            RevBuffer[1] = status;
            break;
        }
        memcpy(MLastSelectedSnr, &RevBuffer[2], 4);
        RevBuffer[0] = 5;
        RevBuffer[1] = status;
        break;
    case 4: // 选择卡 Select Card
        status = PcdSelect(MLastSelectedSnr);
        if (status != MI_OK)
        {
            RevBuffer[0] = 1;
            RevBuffer[1] = status;
            break;
        }
        RevBuffer[0] = 3;
        RevBuffer[1] = status;
        break;
    case 5:                                                                              // Key loading into the MF RC500's EEPROM
        status = PcdAuthState(RevBuffer[1], RevBuffer[3], DefaultKey, MLastSelectedSnr); // 校验卡密码
        RevBuffer[0] = 1;
        RevBuffer[1] = status;
        break;
    case 6:
        RevBuffer[0] = 1;
        RevBuffer[1] = status;
        break;
    case 7:
        RevBuffer[0] = 1;
        RevBuffer[1] = status;
        break;
    case 8: // Read the mifare card
            // 读卡
        status = PcdRead(RevBuffer[1], &RevBuffer[2]);
        if (status == 0)
        {
            RevBuffer[0] = 17;
        }
        else
        {
            RevBuffer[0] = 1;
        }
        RevBuffer[1] = status;
        break;
    case 9: // Write the mifare card
            // 写卡  下载密码
        status = PcdWrite(RevBuffer[1], &RevBuffer[2]);
        RevBuffer[0] = 1;
        RevBuffer[1] = status;
        break;
    case 10:
        PcdValue(RevBuffer[1], RevBuffer[2], &RevBuffer[3]);
        RevBuffer[0] = 1;
        RevBuffer[1] = status;
        break;
    case 12: // 参数设置
        PcdBakValue(RevBuffer[1], RevBuffer[2]);
        RevBuffer[0] = 1; // contact
        RevBuffer[1] = 0;
        break;
    case 0xAA: // 测试spi通信是否正常
#if RC522_DEBUG
        UART0_send_string("SPI testing... \r\n");
#endif
        temp = ReadRawRC(RevBuffer[1]);
#if RC522_DEBUG
        UART0_send_string("Restore Data:");
        uart_send_hex(temp);
        printf("\r\n");
#endif
        break;
    case 0xBB: // spi写测试
        WriteRawRC(RevBuffer[1], RevBuffer[2]);
        break;
    }
}
*/
// 初始化
void RC522_Init(void)
{
    PcdReset();                // 复位RC522
    PcdAntennaOff();           // 关天线
    PcdAntennaOn();            // 开天线
    M500PcdConfigISOType('A'); // 寻卡模式
}
void RC522_Open(void)
{
    MF522_RST = 1; // MF522_RST P13
    // Timer1_Delay(24000000, 10, 1000);
    RC522_Init(); // 初始化
}

void RC522_Off(void)
{
    MF522_RST = 0; // MF522_RST P13
}
// 开机时进行通信测试
void Rc522_Test(void)
{
    unsigned char temp, temp1;
    RC522_Open();
    // 通信测试
    UART0_send_string("\r\nSPI testing... \r\n");
    temp = ReadRawRC(0x00);
    WriteRawRC(0x00, 0x01);
    temp1 = ReadRawRC(0x00);
    if (temp == temp1)
    {
        UART0_send_string("Error: SPI communication failed !!\r\n");
    }
    else
    {
        UART0_send_string("SPI communication successful !! \r\n");
    }
    WriteRawRC(0x00, temp);
}
// 读卡
void Rc522_NTAG213_Code(void)
{
    unsigned char status, i;

// 寻卡
#if RC522_DEBUG
    UART0_send_string("Finding Card...\r\n");
#endif
    DATA_clr();
    Save_DATA_clr();
    RC522_Open(); // 打开NFC
    status = PcdRequest(0x52, &RevBuffer[0]);
    if (status != 0) // 失败再次寻卡
    {
        RC522_Open(); // 打开NFC
        status = PcdRequest(0x52, &RevBuffer[0]);
        if (status != 0) // 二次失败
        {
            RC522_Open(); // 打开NFC
            status = PcdRequest(0x52, &RevBuffer[0]);
            if (status != 0) // 三次失败
            {
#if RC522_DEBUG
                UART0_send_string("Error: No NO Card!! \r\n");
#endif
                return;
            }
        }
    }
    if (status == 0)
    {
#if RC522_DEBUG
        UART0_send_string("Card Tyte: ");
        _nop_();
        uart_send_hex(RevBuffer[0]);
        UART0_send_string("\r\n");

#endif
    }
    // 防冲撞
    DATA_clr();
    status = PcdAnticoll(&RevBuffer[0]); // 功    能：防冲撞
    if (status != 0)
    {
#if RC522_DEBUG
        UART0_send_string("Error:Card Collision occurs!! \r\n");
#endif
    }
    else
    {
        memcpy(MLastSelectedSnr, &RevBuffer[0], 4);
    }
    // 选卡
    status = PcdSelect(MLastSelectedSnr);
    if (status != MI_OK)
    {
#if RC522_DEBUG
        UART0_send_string("Error:Select Card failure!! \r\n");
#endif
    }
    DATA_clr();
    // 读卡
    status = PcdRead(0x00, &RevBuffer[0]);
    if (status == 0)
    {
        // #if RC522_DEBUG
        //         UART0_send_string("Card UID: ");
        //         uart_send_hex(RevBuffer[0]);
        //         uart_send_hex(RevBuffer[1]);
        //         uart_send_hex(RevBuffer[2]);
        //         uart_send_hex(RevBuffer[3]);
        //         uart_send_hex(RevBuffer[4]);
        //         uart_send_hex(RevBuffer[5]);
        //         uart_send_hex(RevBuffer[6]);
        //         uart_send_hex(RevBuffer[7]);
        //         uart_send_hex(RevBuffer[8]);
        //         UART0_send_string("\r\n");
        //         DATA_clr();
        // #endif
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Error:Read failure!! \r\n");
#endif
    }
    status = PcdRead(0x04, &RevBuffer[0]);
    if (status == 0)
    {
#if RC522_DEBUG
        UART0_send_string("Data from Zone 4 to Zone 8:\r\n");
#endif
        for (i = 0; i < 16; i++)
        {
#if RC522_DEBUG
            if (i == 4 || i == 8 || i == 12)
            {
                UART0_send_string("\r\n");
            }

            uart_send_hex(RevBuffer[i]);
#endif
            RevBuffer_Save[i] = RevBuffer[i];
        }
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Error:Read failure!! \r\n");
#endif
    }
    status = PcdRead(0x08, &RevBuffer[16]);
    if (status == 0)
    {

        UART0_send_string("\r\n");
        for (i = 16; i < 20; i++)
        {
#if RC522_DEBUG
            uart_send_hex(RevBuffer[i]);
#endif
            RevBuffer_Save[i] = RevBuffer[i];
        }
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Error:Read failure!! \r\n");
#endif
    }
    DATA_clr();
}

/////////////////////////////////////////////////////////////////////
// 完整的NTAG216读卡函数
// 参数: blockAddr - 要读取的块地址 (0x00-0x86 for NTAG216)
//       pData - 存储读取数据的缓冲区(至少16字节)
// 返回值: MI_OK表示成功，其他值表示失败
/////////////////////////////////////////////////////////////////////
char Rc522_Read_NTAG216_Full(unsigned char blockAddr)
{
    char status;
      unsigned char i, cnt;
    unsigned char tagType[2];
    unsigned char cardSnr[4];

#if RC522_DEBUG
    UART0_send_string("\r\n=== NTAG216 Read Process ===\r\n");
#endif  

    // 初始化
    DATA_clr();
    RC522_Open();

    status = PcdRequest(0x52, &tagType[0]);
    if (status != 0) // 失败再次寻卡
    {
        RC522_Open(); // 打开NFC
        status = PcdRequest(0x52, &tagType[0]);
        if (status != 0) // 二次失败
        {
            RC522_Open(); // 打开NFC
            status = PcdRequest(0x52, &tagType[0]);
            if (status != 0) // 三次失败
            {
#if RC522_DEBUG
                UART0_send_string("Error: No NO Card!! \r\n");
#endif
                return 0;
            }
        }
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Card Type: ");
        uart_send_hex(tagType[0]);
        uart_send_hex(tagType[1]);
        UART0_send_string("\r\n");
#endif
    }

    // 防碰撞
    status = PcdAnticoll(&cardSnr[0]); // 功    能：防冲撞
    if (status != MI_OK)
    {
#if RC522_DEBUG
        UART0_send_string("Error: Card anticollision failed!\r\n");
#endif
        return status;
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Card SNR: ");
        for (i = 0; i < 4; i++)
        {
            uart_send_hex(cardSnr[i]);
            UART0_send_string(" ");
        }
        UART0_send_string("\r\n");
#endif
    }

    // 选择卡片
    status = PcdSelect(cardSnr);
    if (status != MI_OK)
    {
#if RC522_DEBUG
        UART0_send_string("Error: Select card failed!\r\n");
#endif
        return status;
    }
    DATA_clr();

    // 读卡
    status = PcdRead(blockAddr, &RevBuffer[0]);
    
    if (status == 0)
    {
#if RC522_DEBUG
        UART0_send_string(" success:");
        cnt = blockAddr;
#endif
        for (i = 0; i < 16; i++)
        {
#if RC522_DEBUG
            if (i == 0 || i == 4 || i == 8 || i == 12)
            {
                UART0_send_string("\r\n");
                uart_send_hex(cnt);
                UART0_send_string(":");
                cnt = cnt + 1;
            }

            uart_send_hex(RevBuffer[i]);
#endif
            // pData[i] = RevBuffer[i];
        }
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Error:Read failure!! \r\n");
#endif
    }
    //     DATA_clr();
    //     blockAddr = blockAddr + 4;
    //     status = PcdRead(blockAddr, &RevBuffer[0]);
    //     if (status == 0)
    //     {
    //         for (i = 0; i < 16; i++)
    //         {
    // #if RC522_DEBUG
    //             if (i == 0 || i == 4 || i == 8 || i == 12)
    //             {
    //                 UART0_send_string("\r\n");
    //                 uart_send_hex(cnt);
    //                 UART0_send_string(":");
    //                 cnt = cnt + 1;
    //             }

    //             uart_send_hex(RevBuffer[i]);
    // #endif
    //             // pData[i] = RevBuffer[i];
    //         }
    //     }
    //     else
    //     {
    // #if RC522_DEBUG
    //         UART0_send_string("Error:Read failure!! \r\n");
    // #endif
    //     }
    //     DATA_clr();
    //     blockAddr = blockAddr + 4;
    //     status = PcdRead(blockAddr, &RevBuffer[0]);
    //     if (status == 0)
    //     {
    //         for (i = 0; i < 16; i++)
    //         {
    // #if RC522_DEBUG
    //             if (i == 0 || i == 4 || i == 8 || i == 12)
    //             {
    //                 UART0_send_string("\r\n");
    //                 uart_send_hex(cnt);
    //                 UART0_send_string(": ");
    //                 cnt = cnt + 1;
    //             }

    //             uart_send_hex(RevBuffer[i]);
    // #endif
    //             // pData[i] = RevBuffer[i];
    //         }
    //     }
    //     else
    //     {
    // #if RC522_DEBUG
    //         UART0_send_string("Error:Read failure!! \r\n");
    // #endif
    //     }
    //     DATA_clr();

    // // 休眠卡片
    // PcdHalt();
    RC522_Off();
    return status;
}

/////////////////////////////////////////////////////////////////////
// 功能：向NTAG216写入数据（改进版）
// 参数: blockAddr - 要写入的块地址 (0x00-0x86 for NTAG216)
//       pData - 要写入的数据缓冲区(必须16字节)
// 返回值: MI_OK表示成功，其他值表示失败
/////////////////////////////////////////////////////////////////////
char Rc522_Write_NTAG216_Block_Simple(unsigned char blockAddr, unsigned char *pData)
{
    char status;
    unsigned char i, cnt;
    unsigned char tagType[2];
    unsigned char cardSnr[4];

#if RC522_DEBUG
    UART0_send_string("\r\n=== NTAG216 Simple Write ===\r\n");
#endif

    // 初始化
    DATA_clr();
    RC522_Open();

    // 寻卡
    status = PcdRequest(0x52, &tagType[0]);
    if (status != MI_OK)
    {
        RC522_Open();
        status = PcdRequest(0x52, &tagType[0]);
        if (status != MI_OK)
        {
            RC522_Open();
            status = PcdRequest(0x52, &tagType[0]);
            if (status != MI_OK)
            {
#if RC522_DEBUG
                UART0_send_string("Error: No Card Found!! \r\n");
#endif
                RC522_Off();
                return status;
            }
        }
    }

    // 防冲突
    DATA_clr();
    status = PcdAnticoll(&cardSnr[0]);
    if (status != MI_OK)
    {
#if RC522_DEBUG
        UART0_send_string("Error: Card anticollision failed!\r\n");
#endif
        RC522_Off();
        return status;
    }

    // 选择卡片
    status = PcdSelect(cardSnr);
    if (status != MI_OK)
    {
#if RC522_DEBUG
        UART0_send_string("Error: Select card failed!\r\n");
#endif
        RC522_Off();
        return status;
    }

    DATA_clr();

    // 读卡
    status = PcdRead(0, &RevBuffer[0]);
    cnt = 0;
    if (status == 0)
    {
#if RC522_DEBUG
        UART0_send_string(" Read success:");
#endif
        for (i = 0; i < 16; i++)
        {
#if RC522_DEBUG
            if (i == 0 || i == 4 || i == 8 || i == 12)
            {
                UART0_send_string("\r\n");
                uart_send_hex(cnt);
                UART0_send_string(":");
                cnt = cnt + 1;
            }

            uart_send_hex(RevBuffer[i]);
#endif
            // pData[i] = RevBuffer[i];
        }
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Error:Read failure!! \r\n");
#endif
    }
    DATA_clr();

    // 读卡
    status = PcdRead(blockAddr, &RevBuffer[0]);
    cnt = blockAddr;
    if (status == 0)
    {
        for (i = 0; i < 16; i++)
        {
#if RC522_DEBUG
            if (i == 0 || i == 4 || i == 8 || i == 12)
            {
                UART0_send_string("\r\n");
                uart_send_hex(cnt);
                UART0_send_string(":");
                cnt = cnt + 1;
            }

            uart_send_hex(RevBuffer[i]);
#endif
            // pData[i] = RevBuffer[i];
        }
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Error:Read failure!! \r\n");
#endif
    }
    DATA_clr();

    // 直接写入数据（使用NTAG216专用写入函数）
    status = PcdWrite_NTAG216(blockAddr, pData);
    if (status == MI_OK)
    {
#if RC522_DEBUG
        UART0_send_string("Write block ");
        uart_send_hex(blockAddr);
        UART0_send_string(" success!\r\n");
#else
        // 即使不打印调试信息，也稍作延时确保写入完成
        Timer1_Delay(24000000, 10, 1000);
#endif
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Error: Write block ");
        uart_send_hex(blockAddr);
        UART0_send_string(" failed! Status: ");
        uart_send_hex(status);
        UART0_send_string("\r\n");
#endif
    }

    // 读卡
    status = PcdRead(blockAddr, &RevBuffer[0]);
    cnt = blockAddr;
    if (status == 0)
    {
#if RC522_DEBUG
        UART0_send_string(" Read again:");
#endif
        for (i = 0; i < 16; i++)
        {
#if RC522_DEBUG
            if (i == 0 || i == 4 || i == 8 || i == 12)
            {
                UART0_send_string("\r\n");
                uart_send_hex(cnt);
                UART0_send_string(":");
                cnt = cnt + 1;
            }

            uart_send_hex(RevBuffer[i]);
#endif
            // pData[i] = RevBuffer[i];
        }
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Error:Read failure!! \r\n");
#endif
    }
    DATA_clr();

    // RC522_Off();

    return status;
}

/////////////////////////////////////////////////////////////////////
// 功能：NTAG216写入数据块（带密码验证）
// 参数: blockAddr - 要写入的目标地址 (0x00-0xE6 for NTAG216)
//       pData - 要写入的数据缓冲区(16字节)
//       pPassword - 密码指针（如果为NULL则不进行密码验证）
// 返回值: MI_OK表示成功，其他值表示失败
/////////////////////////////////////////////////////////////////////
char Rc522_Write_NTAG216_Block_Simple_With_Auth(unsigned char blockAddr, unsigned char *pData, unsigned char *pPassword)
{
    char status;
    unsigned char i, cnt;
    unsigned char verifyOK = 1;
    unsigned char tagType[2];
    unsigned char pack[2];
    unsigned char cardSnr[4];

#if RC522_DEBUG
    UART0_send_string("\r\n=== NTAG216 Write with Authentication ===\r\n");
#endif

    // 初始化
    DATA_clr();
    RC522_Open();

    // 寻卡
    status = PcdRequest(0x52, &tagType[0]);
    if (status != MI_OK)
    {
        RC522_Open();
        status = PcdRequest(0x52, &tagType[0]);
        if (status != MI_OK)
        {
            RC522_Open();
            status = PcdRequest(0x52, &tagType[0]);
            if (status != MI_OK)
            {
#if RC522_DEBUG
                UART0_send_string("Error: No Card Found!! \r\n");
#endif
                RC522_Off();
                return status;
            }
        }
    }

    // 防冲突
    DATA_clr();
    status = PcdAnticoll(&cardSnr[0]);
    if (status != MI_OK)
    {
#if RC522_DEBUG
        UART0_send_string("Error: Card anticollision failed!\r\n");
#endif
        RC522_Off();
        return status;
    }

    // 选卡
    status = PcdSelect(cardSnr);
    if (status != MI_OK)
    {
#if RC522_DEBUG
        UART0_send_string("Error: Select card failed!\r\n");
#endif
        RC522_Off();
        return status;
    }

    DATA_clr();

    // 读取页0（验证基本通信）
    status = PcdRead(0, &RevBuffer[0]);
    cnt = 0;
    if (status == 0)
    {
#if RC522_DEBUG
        UART0_send_string("Read page 0 success:\r\n");
#endif
        for (i = 0; i < 16; i++)
        {
#if RC522_DEBUG
            if (i == 0 || i == 4 || i == 8 || i == 12)
            {
                UART0_send_string("\r\n");
                uart_send_hex(cnt);
                UART0_send_string(":");
                cnt = cnt + 1;
            }
            uart_send_hex(RevBuffer[i]);
#endif
        }
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Error: Read page 0 failure!! \r\n");
#endif
    }
    DATA_clr();

    // 读取目标地址数据（写入前）
    status = PcdRead(blockAddr, &RevBuffer[0]);
    cnt = blockAddr;
    if (status == 0)
    {
#if RC522_DEBUG
        UART0_send_string("\r\nData before write:\r\n");
        for (i = 0; i < 16; i++)
        {
            if (i == 0 || i == 4 || i == 8 || i == 12)
            {
                UART0_send_string("\r\n");
                uart_send_hex(cnt);
                UART0_send_string(":");
                cnt = cnt + 1;
            }
            uart_send_hex(RevBuffer[i]);
        }
        UART0_send_string("\r\n");
#endif
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Error: Read before write failed!! \r\n");
#endif
    }
    DATA_clr();

    // 如果提供了密码，则进行密码验证
    if (pPassword != NULL)
    {
#if RC522_DEBUG
        UART0_send_string("Performing password authentication...\r\n");
#endif

        status = PcdNTAG216_Authenticate_Enhanced(pPassword, &pack);
        if (status != MI_OK)
        {
#if RC522_DEBUG
            UART0_send_string("Error: Password authentication failed!\r\n");
#endif
            // RC522_Off();
            return status;
        }
#if RC522_DEBUG
        else
        {
            UART0_send_string("Pack:");
            uart_send_hex(pack[0]);
            uart_send_hex(pack[1]);
            UART0_send_string("\r\n");
            UART0_send_string("Password authentication successful!\r\n");
        }
#endif
    }

    // 写入数据，使用NTAG216专用写入函数
    status = PcdWrite_NTAG216(blockAddr, pData);
    if (status == MI_OK)
    {
#if RC522_DEBUG
        UART0_send_string("Write block ");
        uart_send_hex(blockAddr);
        UART0_send_string(" success!\r\n");
#endif
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Error: Write block ");
        uart_send_hex(blockAddr);
        UART0_send_string(" failed! Status: ");
        uart_send_hex(status);
        UART0_send_string("\r\n");
#endif
    }

    // 写入后读取验证
    status = PcdRead(blockAddr, &RevBuffer[0]);
    cnt = blockAddr;
    if (status == 0)
    {
#if RC522_DEBUG
        UART0_send_string("Data after write:\r\n");
        for (i = 0; i < 16; i++)
        {
            if (i == 0 || i == 4 || i == 8 || i == 12)
            {
                UART0_send_string("\r\n");
                uart_send_hex(cnt);
                UART0_send_string(":");
                cnt = cnt + 1;
            }
            uart_send_hex(RevBuffer[i]);
        }
        UART0_send_string("\r\n");
#endif
        // 验证写入的数据是否正确
        for (i = 0; i < 4; i++)
        {
            if (RevBuffer[i] != pData[i])
            {
                verifyOK = 0;
                break;
            }
        }

        if (verifyOK)
        {
#if RC522_DEBUG
            UART0_send_string("Write verification: SUCCESS\r\n");
#endif
            status = MI_OK;
        }
        else
        {
#if RC522_DEBUG
            UART0_send_string("Write verification: FAILED - Data mismatch\r\n");
#endif
            status = MI_ERR;
        }
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Error: Read after write failed!! \r\n");
#endif
    }
    DATA_clr();

    // RC522_Off();

    return status;
}

/////////////////////////////////////////////////////////////////////
// 功能：NTAG216密码验证函数
// 参数: pPassword - 4字节密码指针
// 返回值: MI_OK表示成功，其他值表示失败
/////////////////////////////////////////////////////////////////////
char Rc522_NTAG216_Password_Verify(unsigned char *pPassword)
{
    char status;
    unsigned char pack[2];
    unsigned char tagType[2];
    unsigned char cardSnr[4];
    unsigned char verify[4] = {0x31, 0x32, 0x33, 0x34};
    // unsigned char save_0x31[4];
    // 初始化
    DATA_clr();
    RC522_Open();

    // 寻卡
    status = PcdRequest(0x52, &tagType[0]);
    if (status != MI_OK)
    {
        RC522_Open();
        status = PcdRequest(0x52, &tagType[0]);
        if (status != MI_OK)
        {
            RC522_Open();
            status = PcdRequest(0x52, &tagType[0]);
            if (status != MI_OK)
            {
#if RC522_DEBUG
                UART0_send_string("Error: No Card Found!! \r\n");
#endif
                // RC522_Off();
                return MI_ERR;
            }
        }
    }

    // 防冲突
    status = PcdAnticoll(&cardSnr[0]);
    if (status != MI_OK)
    {
#if RC522_DEBUG
        UART0_send_string("Error: Card anticollision failed!\r\n");
#endif
        // RC522_Off();
        return MI_ERR;
    }

    // 选卡
    status = PcdSelect(cardSnr);
    if (status != MI_OK)
    {
#if RC522_DEBUG
        UART0_send_string("Error: Select card failed!\r\n");
#endif
        // RC522_Off();
        return MI_ERR;
    }

    DATA_clr();

    // 读取页0（验证基本通信）
    PcdRead(0, &RevBuffer[0]);
    DATA_clr();
    // 执行密码验证
    status = PcdNTAG216_Authenticate_Enhanced(pPassword, &pack);
    if (status != MI_OK)
    {
#if RC522_DEBUG
        UART0_send_string("Error: PcdNTAG216_Authenticate_Enhanced!\r\n");
#endif
        return MI_ERR;
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Tip: pack1:");
        uart_send_hex(pack[0]);
        UART0_send_string(" ");
        uart_send_hex(pack[1]);
        UART0_send_string("\r\n");
#endif
        if ((pack[0] == NFC_pack[0]) && (pack[1] == NFC_pack[1]))
        {
            return MI_OK;

            //             pack[0] = 0;
            //             pack[1] = 0;
            //             save_0x31[0] = 0;
            //             save_0x31[1] = 0;
            //             save_0x31[2] = 0;
            //             save_0x31[3] = 0;
            //             PcdRead(0xE1, &RevBuffer[0]);
            //             save_0x31[0] = RevBuffer[0];
            //             save_0x31[1] = RevBuffer[1];
            //             save_0x31[2] = RevBuffer[2];
            //             save_0x31[3] = RevBuffer[3];
            //             DATA_clr();
            //             // return MI_OK;
            //             status = PcdWrite_NTAG216(0xE1, verify);
            //             Timer2_Delay(24000000, 128, 10, 1000);
            //             if (status == MI_OK)
            //             {

            //                 PcdRead(0xE1, &RevBuffer[0]);
            //                 if ((RevBuffer[0] == verify[0]) && (RevBuffer[1] == verify[1]) && (RevBuffer[2] == verify[2]) && (RevBuffer[3] == verify[3]))
            //                 {
            //                     PcdWrite_NTAG216(0xE1, save_0x31);
            //                     save_0x31[0] = 0;
            //                     save_0x31[1] = 0;
            //                     save_0x31[2] = 0;
            //                     save_0x31[3] = 0;
            //                     return MI_OK;
            //                 }
            //                 else
            //                 {
            // #if RC522_DEBUG
            //                     UART0_send_string("Error: 3!\r\n");
            // #endif

            //                     return MI_ERR;
            //                 }
            //             }
            //             else
            //             {
            // #if RC522_DEBUG
            //                 UART0_send_string("Error: 2!\r\n");
            // #endif

            //                 return MI_ERR;
            //             }
        }
        else
        {
#if RC522_DEBUG
            UART0_send_string("Error: 1!\r\n");
#endif

            return MI_ERR;
        }
    }
}
