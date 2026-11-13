/*
 * @Author:  cray 485562844@QQ.com
 * @Date: 2024-12-27 09:05:48
 * @LastEditTime: 2025-10-14 14:22:00
 * @座右铭: Learning makes me confident
 */
#include "MS51_16K.H"
#include "RC522.H"

/***********************
 * MCU的SPI口为：
 * SS-P15、MISO-P01、MOSI-P00、CLK-P10、RST-P11、
 * RC522的SPI标识：
 * SDA-SS、、SCK-CLK、MISO、MOSI
 ***********************/
// #define MAXRLEN 18
// // #define MF522_IRQ P13   //中断口
// #define MF522_RST P23
// #define MF522_SO P21
// #define MF522_SI P35
// #define MF522_SCK P12
// #define MF522_NSS P11    //SDA

//  #define MAXRLEN 18
//  #define MF522_RST P23
//  #define MF522_SCK P12
//  #define MF522_SO P21
//  #define MF522_SI P35
//  #define MF522_NSS1 P11
//   #define MF522_NSS P11

/////////////////////////////////////////////////////////////////////
// 功    能：寻卡
// 参数说明: req_code[IN]:寻卡方式
//                 0x52 = 寻感应区内所有符合14443A标准的卡
//                 0x26 = 寻未进入休眠状态的卡
//           pTagType[OUT]：卡片类型代码
//                 0x4400 = Mifare_UltraLight
//                 0x0400 = Mifare_One(S50)
//                 0x0200 = Mifare_One(S70)
//                 0x0800 = Mifare_Pro(X)
//                 0x4403 = Mifare_DESFire
//                 0x0x8f00 =
// 返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdRequest(unsigned char req_code, unsigned char *pTagType)
{
    char status;                          // 用于存储函数执行状态的变量
    unsigned int unLen;                   // 用于存储接收数据长度的变量
    unsigned char ucComMF522Buf[MAXRLEN]; // 用于存储通信数据的缓冲区

    // 清除Status2Reg寄存器中的特定位，以确保正确的通信状态
    ClearBitMask(Status2Reg, 0x08);

    // 设置BitFramingReg寄存器，配置通信的位帧
    WriteRawRC(BitFramingReg, 0x07);

    // 设置TxControlReg寄存器，启用发射器
    SetBitMask(TxControlReg, 0x03);

    // 将请求代码写入通信缓冲区的首位
    ucComMF522Buf[0] = req_code; //

    // 调用PcdComMF522函数执行通信操作，发送请求代码，并等待接收卡的响应
    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);
#if RC522_DEBUG
    if (status == MI_OK)
    {
        // printf("PcdComMF522() succeed.\r\n");
        //  printf("\r\n");
        //      UART0_send_string("unLen:");
        //          uart_send_hex(unLen);
        //          printf("\r\n");
        //      *pTagType = ucComMF522Buf[0];
        // *(pTagType + 1) = ucComMF522Buf[1];
    }
    else
    {
        // status = MI_ERR;
        // printf("Error:PcdComMF522 failed.\r\n");
    }

#endif
    // 检查通信状态和接收的数据长度，如果成功且长度符合预期，则解析卡类型信息
    if ((status == MI_OK) && (unLen == 0x10))
    {
        *pTagType = ucComMF522Buf[0];
        *(pTagType + 1) = ucComMF522Buf[1];
    }
    else
    { // 如果通信失败或数据长度不符合预期，则设置状态为错误
        status = MI_ERR;
    }

    // 返回函数执行状态
    return status;
}

/////////////////////////////////////////////////////////////////////
// 功    能：防冲撞
// 参数说明: pSnr[OUT]:卡片序列号，4字节
// 返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdAnticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i, snr_check = 0;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg, 0x08);
    WriteRawRC(BitFramingReg, 0x00);
    ClearBitMask(CollReg, 0x80);

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);

    if (status == MI_OK)
    {
        for (i = 0; i < 4; i++)
        {
            *(pSnr + i) = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i])
        {
            status = MI_ERR;
        }
    }

    SetBitMask(CollReg, 0x80);
    return status;
}

/////////////////////////////////////////////////////////////////////
// 功    能：选定卡片
// 参数说明: pSnr[IN]:卡片序列号，4字节
// 返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdSelect(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i = 0; i < 4; i++)
    {
        ucComMF522Buf[i + 2] = *(pSnr + i);
        ucComMF522Buf[6] ^= *(pSnr + i);
    }
    CalulateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);

    ClearBitMask(Status2Reg, 0x08);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);

    if ((status == MI_OK) && (unLen == 0x18))
    {
        status = MI_OK;
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
// 功    能：验证卡片密码
// 参数说明: auth_mode[IN]: 密码验证模式
//                  0x60 = 验证A密钥
//                  0x61 = 验证B密钥
//           addr[IN]：块地址
//           pKey[IN]：密码
//           pSnr[IN]：卡片序列号，4字节
// 返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdAuthState(unsigned char auth_mode, unsigned char addr, unsigned char *pKey, unsigned char *pSnr)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i + 2] = *(pKey + i);
    }
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i + 8] = *(pSnr + i);
    }
    //   memcpy(&ucComMF522Buf[2], pKey, 6);
    //   memcpy(&ucComMF522Buf[8], pSnr, 4);

    status = PcdComMF522(PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
// 功    能：读取M1卡一块数据
// 参数说明: addr[IN]：块地址
//           pData[OUT]：读出的数据，16字节
// 返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdRead(unsigned char addr, unsigned char *pData)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if ((status == MI_OK) && (unLen == 0x90))
    //   {   memcpy(pData, ucComMF522Buf, 16);   }
    {
        for (i = 0; i < 16; i++)
        {
            *(pData + i) = ucComMF522Buf[i];
        }
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
// 功    能：写数据到M1卡一块
// 参数说明: addr[IN]：块地址
//           pData[IN]：写入的数据，16字节
// 返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdWrite(unsigned char addr, unsigned char *pData)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        // memcpy(ucComMF522Buf, pData, 16);
        for (i = 0; i < 16; i++)
        {
            ucComMF522Buf[i] = *(pData + i);
        }
        CalulateCRC(ucComMF522Buf, 16, &ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, &unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
// 功    能：命令卡片进入休眠状态
// 返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdHalt(void)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
// 用MF522计算CRC16函数
/////////////////////////////////////////////////////////////////////
void CalulateCRC(unsigned char *pIndata, unsigned char len, unsigned char *pOutData)
{
    unsigned char i, n;
    ClearBitMask(DivIrqReg, 0x04);
    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);
    for (i = 0; i < len; i++)
    {
        WriteRawRC(FIFODataReg, *(pIndata + i));
    }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x04));
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

/////////////////////////////////////////////////////////////////////
// 功    能：复位RC522
// 返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdReset(void)
{
    MF522_RST = 1;
    _nop_();
    MF522_RST = 0;
    _nop_();
    MF522_RST = 1;
    _nop_();
    WriteRawRC(CommandReg, PCD_RESETPHASE);
    _nop_();

    WriteRawRC(ModeReg, 0x3D); // 和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL, 30);
    WriteRawRC(TReloadRegH, 0);
    WriteRawRC(TModeReg, 0x8D);
    WriteRawRC(TPrescalerReg, 0x3E);
    WriteRawRC(TxAutoReg, 0x40);
    return MI_OK;
}
//////////////////////////////////////////////////////////////////////
// 设置RC632的工作方式
//////////////////////////////////////////////////////////////////////
char M500PcdConfigISOType(unsigned char type)
{
    if (type == 'A') // ISO14443_A
    {
        ClearBitMask(Status2Reg, 0x08);

        /*     WriteRawRC(CommandReg,0x20);    //as default
              WriteRawRC(ComIEnReg,0x80);     //as default
              WriteRawRC(DivlEnReg,0x0);      //as default
              WriteRawRC(ComIrqReg,0x04);     //as default
              WriteRawRC(DivIrqReg,0x0);      //as default
              WriteRawRC(Status2Reg,0x0);//80    //trun off temperature sensor
              WriteRawRC(WaterLevelReg,0x08); //as default
              WriteRawRC(ControlReg,0x20);    //as default
              WriteRawRC(CollReg,0x80);    //as default
       */
        WriteRawRC(ModeReg, 0x3D); // 3F
        /*	   WriteRawRC(TxModeReg,0x0);      //as default???
               WriteRawRC(RxModeReg,0x0);      //as default???
               WriteRawRC(TxControlReg,0x80);  //as default???

               WriteRawRC(TxSelReg,0x10);      //as default???
           */
        WriteRawRC(RxSelReg, 0x86); // 84
        //      WriteRawRC(RxThresholdReg,0x84);//as default
        //      WriteRawRC(DemodReg,0x4D);      //as default

        //      WriteRawRC(ModWidthReg,0x13);//26
        WriteRawRC(RFCfgReg, 0x7F); // 4F
        /*   WriteRawRC(GsNReg,0x88);        //as default???
           WriteRawRC(CWGsCfgReg,0x20);    //as default???
           WriteRawRC(ModGsCfgReg,0x20);   //as default???
    */
        WriteRawRC(TReloadRegL, 30); // tmoLength);// TReloadVal = 'h6a =tmoLength(dec)
        WriteRawRC(TReloadRegH, 0);
        WriteRawRC(TModeReg, 0x8D);
        WriteRawRC(TPrescalerReg, 0x3E);

        //     PcdSetTmo(106);
        Timer1_Delay(24000000, 10, 1000);
        PcdAntennaOn();
    }
    else
    {
        return -1;
    }

    return MI_OK;
}
/////////////////////////////////////////////////////////////////////
// 功    能：读RC632寄存器
// 参数说明：Address[IN]:寄存器地址
// 返    回：读出的值
/////////////////////////////////////////////////////////////////////
unsigned char ReadRawRC(unsigned char Address)
{
    unsigned char i, ucAddr;
    unsigned char ucResult = 0;

    // 初始化SPI通信的时钟和片选信号
    MF522_SCK = 0;
    MF522_NSS = 0;
    // 准备读取操作的地址，通过左移地址并设置读取标志位
    ucAddr = ((Address << 1) & 0x7E) | 0x80;

    // 发送地址到MF522，通过SPI接口
    for (i = 8; i > 0; i--)
    {
        // 根据地址的当前位设置SPI数据线
        MF522_SI = ((ucAddr & 0x80) == 0x80);
        // 拉高时钟线，发送数据
        MF522_SCK = 1;
        // 地址左移，准备下一位
        ucAddr <<= 1;
        // 拉低时钟线，为下一次发送做准备
        MF522_SCK = 0;
    }

    // 从MF522接收数据，通过SPI接口
    for (i = 8; i > 0; i--)
    {
        // 拉高时钟线，接收数据
        MF522_SCK = 1;
        // 结果左移，准备存储接收到的数据
        ucResult <<= 1;
        // 根据SPI数据线的状态设置结果的当前位
        ucResult |= (bit)MF522_SO;
        // 拉低时钟线，为下一次接收做准备
        MF522_SCK = 0;
    }

    // 结束SPI通信，拉高片选和时钟信号
    MF522_NSS = 1;
    MF522_SCK = 1;
    // 返回接收到的数据
    return ucResult;
}

/////////////////////////////////////////////////////////////////////
// 功    能：写RC632寄存器
// 参数说明：Address[IN]:寄存器地址
//           value[IN]:写入的值
/////////////////////////////////////////////////////////////////////
void WriteRawRC(unsigned char Address, unsigned char value)
{
    unsigned char i, ucAddr;

    MF522_SCK = 0;
    MF522_NSS = 0;
    ucAddr = ((Address << 1) & 0x7E);

    for (i = 8; i > 0; i--)
    {
        MF522_SI = ((ucAddr & 0x80) == 0x80);
        MF522_SCK = 1;
        ucAddr <<= 1;
        MF522_SCK = 0;
    }

    for (i = 8; i > 0; i--)
    {
        MF522_SI = ((value & 0x80) == 0x80);
        MF522_SCK = 1;
        value <<= 1;
        MF522_SCK = 0;
    }
    MF522_NSS = 1;
    MF522_SCK = 1;
}

/////////////////////////////////////////////////////////////////////
// 功    能：置RC522寄存器位
// 参数说明：reg[IN]:寄存器地址
//           mask[IN]:置位值
/////////////////////////////////////////////////////////////////////
void SetBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp | mask); // set bit mask
}

/////////////////////////////////////////////////////////////////////
// 功    能：清RC522寄存器位
// 参数说明：reg[IN]:寄存器地址
//           mask[IN]:清位值
/////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char reg, unsigned char mask)
{
    // 初始化一个临时变量tmp为0，用于存储从寄存器中读取的数据。
    char tmp = 0x0;

    // 从指定的寄存器中读取当前值。
    tmp = ReadRawRC(reg);

    // 使用与运算来清除掩码指定的位，并将结果写回寄存器。
    WriteRawRC(reg, tmp & ~mask); // clear bit mask
}

/////////////////////////////////////////////////////////////////////
// 功    能：通过RC522和ISO14443卡通讯
// 参数说明：Command[IN]:RC522命令字
//           pInData[IN]:通过RC522发送到卡片的数据
//           InLenByte[IN]:发送数据的字节长度
//           pOutData[OUT]:接收到的卡片返回数据
//           *pOutLenBit[OUT]:返回数据的位长度
/////////////////////////////////////////////////////////////////////
/**
 * @brief 与MF522通信函数
 *
 * 本函数用于与MF522芯片进行通信，执行不同的命令，如认证和数据传输。
 * 它负责设置通信参数，发送数据，并接收响应数据。
 *
 * @param Command 命令字，指定要执行的操作，如PCD_AUTHENT认证、PCD_TRANSCEIVE数据传输。
 * @param pInData 指向输入数据的指针，包含要发送到MF522的数据。通过RC522发送到卡片的数据
 * @param InLenByte 输入数据的长度，单位为字节。
 * @param pOutData 指向输出数据缓冲区的指针，用于存储从MF522接收到的数据。接收到的卡片返回数据
 * @param pOutLenBit 指向输出数据长度的指针，单位为位，由函数填写。
 * @return char 函数执行状态，可能的返回值包括MI_OK（成功）、MI_ERR（错误）、MI_NOTAGERR（未检测到标签）等。
 */
char PcdComMF522(unsigned char Command,
                 unsigned char *pInData,
                 unsigned char InLenByte,
                 unsigned char *pOutData,
                 unsigned int *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    // 根据不同的命令设置中断使能和等待的中断源
    switch (Command)
    {
    case PCD_AUTHENT:
        irqEn = 0x12;
        waitFor = 0x10;
        break;
    case PCD_TRANSCEIVE:
        irqEn = 0x77;
        waitFor = 0x30;
        break;
    default:
        break;
    }

    // 配置通信中断使能寄存器
    WriteRawRC(ComIEnReg, irqEn | 0x80);
    // 清除通信中断状态寄存器的中断状态
    ClearBitMask(ComIrqReg, 0x80);
    // 设置命令寄存器为PCD_IDLE状态，准备发送命令
    WriteRawRC(CommandReg, PCD_IDLE);
    // 清空FIFO缓冲区
    SetBitMask(FIFOLevelReg, 0x80);

    // 将输入数据写入FIFO缓冲区
    for (i = 0; i < InLenByte; i++)
    {
        WriteRawRC(FIFODataReg, pInData[i]);
    }
    // 发送命令到MF522
    WriteRawRC(CommandReg, Command);

    // 如果是数据传输命令，设置位帧寄存器
    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg, 0x80);
    }

    // 等待中断或超时，以确定命令执行完成
    // i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
    i = 2000;
    do
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitFor));
    // 命令执行完成后，清除位帧寄存器的设置
    ClearBitMask(BitFramingReg, 0x80);

    // 如果未超时，检查错误并处理接收到的数据
    if (i != 0)
    {
        if (!(ReadRawRC(ErrorReg) & 0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }
            if (Command == PCD_TRANSCEIVE)
            {
                n = ReadRawRC(FIFOLevelReg);
                lastBits = ReadRawRC(ControlReg) & 0x07;
                // 计算接收到的数据长度
                if (lastBits)
                {
                    *pOutLenBit = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n * 8;
                }
                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAXRLEN)
                {
                    n = MAXRLEN;
                }
                // 读取接收到的数据
                for (i = 0; i < n; i++)
                {
                    pOutData[i] = ReadRawRC(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }
    }
    else
    {
#if RC522_DEBUG
        //  printf("time out error \r\n");
#endif
    }
    // 停止定时器
    SetBitMask(ControlReg, 0x80); // stop timer now
    // 设置命令寄存器为PCD_IDLE状态，结束命令执行
    WriteRawRC(CommandReg, PCD_IDLE);
    return status;
}

/////////////////////////////////////////////////////////////////////
// 开启天线
// 每次启动或关闭天险发射之间应至少有1ms的间隔
/////////////////////////////////////////////////////////////////////
void PcdAntennaOn()
{
    unsigned char i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}

/////////////////////////////////////////////////////////////////////
// 关闭天线
/////////////////////////////////////////////////////////////////////
void PcdAntennaOff()
{
    ClearBitMask(TxControlReg, 0x03);
}

/////////////////////////////////////////////////////////////////////
// 功    能：扣款和充值
// 参数说明: dd_mode[IN]：命令字
//                0xC0 = 扣款
//                0xC1 = 充值
//           addr[IN]：钱包地址
//           pValue[IN]：4字节增(减)值，低位在前
// 返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdValue(unsigned char dd_mode, unsigned char addr, unsigned char *pValue)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = dd_mode;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        memcpy(ucComMF522Buf, pValue, 4);
        //       for (i=0; i<16; i++)
        //       {    ucComMF522Buf[i] = *(pValue+i);   }
        CalulateCRC(ucComMF522Buf, 4, &ucComMF522Buf[4]);
        unLen = 0;
        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 6, ucComMF522Buf, &unLen);
        if (status != MI_ERR)
        {
            status = MI_OK;
        }
    }

    if (status == MI_OK)
    {
        ucComMF522Buf[0] = PICC_TRANSFER;
        ucComMF522Buf[1] = addr;
        CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////
// 功    能：备份钱包
// 参数说明: sourceaddr[IN]：源地址
//           goaladdr[IN]：目标地址
// 返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdBakValue(unsigned char sourceaddr, unsigned char goaladdr)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_RESTORE;
    ucComMF522Buf[1] = sourceaddr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        ucComMF522Buf[0] = 0;
        ucComMF522Buf[1] = 0;
        ucComMF522Buf[2] = 0;
        ucComMF522Buf[3] = 0;
        CalulateCRC(ucComMF522Buf, 4, &ucComMF522Buf[4]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 6, ucComMF522Buf, &unLen);
        if (status != MI_ERR)
        {
            status = MI_OK;
        }
    }

    if (status != MI_OK)
    {
        return MI_ERR;
    }

    ucComMF522Buf[0] = PICC_TRANSFER;
    ucComMF522Buf[1] = goaladdr;

    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    return status;
}

// 功能：向NTAG216写入数据（正确版本，符合NTAG216规格书）
// 参数: pageAddr - 要写入的页地址 (0x02-0xE6)
//       pData - 要写入的数据，4字节
// 返回值: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdWrite_NTAG216(unsigned char pageAddr, unsigned char *pData)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    unsigned char i;

#if RC522_DEBUG
    UART0_send_string("Writing to NTAG216 page ");
    uart_send_hex(pageAddr);
    UART0_send_string(": ");
    for (i = 0; i < 4; i++)
    {
        uart_send_hex(pData[i]);
        UART0_send_string(" ");
    }
    UART0_send_string("\r\n");
#endif

    // 检查页地址范围 (NTAG216: 0x02-0xE6)
    if (pageAddr < 0x02 || pageAddr > 0xE6)
    {
#if RC522_DEBUG
        UART0_send_string("Error: Page address out of range (0x02-0xE6)\r\n");
#endif
        return MI_ERR;
    }

    // 构造WRITE命令帧
    ucComMF522Buf[0] = 0xA2;     // WRITE命令
    ucComMF522Buf[1] = pageAddr; // 页地址
    ucComMF522Buf[2] = pData[0]; // 数据字节0
    ucComMF522Buf[3] = pData[1]; // 数据字节1
    ucComMF522Buf[4] = pData[2]; // 数据字节2
    ucComMF522Buf[5] = pData[3]; // 数据字节3

    // 计算CRC
    CalulateCRC(ucComMF522Buf, 6, &ucComMF522Buf[6]);

    // 发送WRITE命令
    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 8, ucComMF522Buf, &unLen);

    // 检查响应
    if ((status == MI_OK) && (unLen == 4))
    {
        // 检查ACK (低4位为0x0A)
        if ((ucComMF522Buf[0] & 0x0F) == 0x0A)
        {
#if RC522_DEBUG
            UART0_send_string("Write successful to page ");
            uart_send_hex(pageAddr);
            UART0_send_string("\r\n");
#endif
            return MI_OK;
        }
        else
        {
#if RC522_DEBUG
            UART0_send_string("Write NACK received for page ");
            uart_send_hex(pageAddr);
            UART0_send_string(": ");
            uart_send_hex(ucComMF522Buf[0]);
            UART0_send_string("\r\n");
#endif
            return MI_ERR;
        }
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Write failed for page ");
        uart_send_hex(pageAddr);
        UART0_send_string(", status: ");
        uart_send_hex(status);
        UART0_send_string(", response length: ");
        uart_send_hex(unLen);
        UART0_send_string("\r\n");
#endif
        return MI_ERR;
    }
}

/////////////////////////////////////////////////////////////////////
// 功能：向NTAG216写入16字节数据（模拟块写入）
// 参数: startPageAddr - 起始页地址（必须是4的倍数或有效起始页）
//       pData - 要写入的数据，16字节
// 返回值: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdWrite_NTAG216_Block16(unsigned char startPageAddr, unsigned char *pData)
{
    char status;
    unsigned char i;
    unsigned char pageData[4];

#if RC522_DEBUG
    UART0_send_string("Writing 16 bytes to NTAG216 starting at page ");
    uart_send_hex(startPageAddr);
    UART0_send_string("\r\n");
#endif

    // 分4次写入，每次写入4字节到连续的页
    for (i = 0; i < 4; i++)
    {
        // 准备4字节数据
        pageData[0] = pData[i * 4 + 0];
        pageData[1] = pData[i * 4 + 1];
        pageData[2] = pData[i * 4 + 2];
        pageData[3] = pData[i * 4 + 3];

        // 写入当前页
        status = PcdWrite_NTAG216(startPageAddr + i, pageData);
        if (status != MI_OK)
        {
#if RC522_DEBUG
            UART0_send_string("Failed to write page ");
            uart_send_hex(startPageAddr + i);
            UART0_send_string("\r\n");
#endif
            return status;
        }
    }

#if RC522_DEBUG
    UART0_send_string("16-byte block write completed successfully\r\n");
#endif

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
// 功能：NTAG216密码验证（改进版）
// 参数: pPassword - 4字节密码
//       pPack - 2字节PACK确认码输出（可为NULL）
// 返回值: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
  char PcdNTAG216_Authenticate_Enhanced(unsigned char *pPassword, unsigned char *pPack)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    unsigned char i;
    uint8_t NFC_password[4] = {0x31, 0x35, 0x39, 0x30};

#if RC522_DEBUG
    UART0_send_string("NTAG216 Password Authentication...\r\n");
    UART0_send_string("Password: ");
    for (i = 0; i < 4; i++)
    {
        uart_send_hex(NFC_password[i]);
        UART0_send_string(" ");
    }
    UART0_send_string("\r\n");
   
#endif

    // 构造PWD_AUTH命令帧
    ucComMF522Buf[0] = 0x1B;         // PWD_AUTH命令
    ucComMF522Buf[1] = NFC_password[0]; // 密码字节0
    ucComMF522Buf[2] = NFC_password[1]; // 密码字节1
    ucComMF522Buf[3] = NFC_password[2]; // 密码字节2
    ucComMF522Buf[4] = NFC_password[3]; // 密码字节3

    // 计算CRC
    CalulateCRC(ucComMF522Buf, 5, &ucComMF522Buf[5]);

    // 执行密码验证命令
    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 7, ucComMF522Buf, &unLen);

    if (status == MI_OK)
    {
        pPack[0] = ucComMF522Buf[0];
        pPack[1] = ucComMF522Buf[1];
#if RC522_DEBUG

        UART0_send_string("Password authentication successful!\r\n");
        UART0_send_string("Pack: ");
        uart_send_hex(pPack[0]);
        uart_send_hex(pPack[1]);
        UART0_send_string("\r\n");
#endif
        return MI_OK;
    }
    else
    {
#if RC522_DEBUG
        UART0_send_string("Error: Incorrect password !!");
#endif

        return MI_ERR;
    }
}