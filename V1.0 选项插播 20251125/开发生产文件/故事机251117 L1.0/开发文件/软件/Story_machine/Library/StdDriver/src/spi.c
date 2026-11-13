/*
 * @Author:  cray 485562844@QQ.com
 * @Date: 2024-12-27 08:38:15
 * @LastEditTime: 2024-12-27 10:53:00
 * @座右铭: Learning makes me confident
 */
/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2020 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

//***********************************************************************************************************
//  Library File Function: MS51 Access SPI Flash (W25Q16BV)
//***********************************************************************************************************

#include "MS51_16K.h"

/**
 * @brief 写入一个字节到SPI接口
 * 
 * 本函数通过SPI接口写入一个字节的数据。它首先将数据字节加载到SPI数据寄存器，
 * 然后等待传输完成。传输完成后，清除SPI状态寄存器中的SPIF标志。
 * 
 * @param u8SpiWB 要写入的字节数据
 */
void Spi_Write_Byte(unsigned char u8SpiWB)
{
    // 将数据字节加载到SPI数据寄存器
    SPDR = u8SpiWB;
    // 等待传输完成，SPIF标志置位
    while(!(SPSR&0x80));
    // 清除SPI状态寄存器中的SPIF标志
    clr_SPSR_SPIF;
}

/**
 * @brief 通过SPI接口读取一个字节
 * 
 * 本函数通过SPI接口读取一个字节的数据。它首先将一个字节的数据加载到SPI数据寄存器以启动传输，
 * 然后等待传输完成。在传输完成后，从SPI数据寄存器读取接收到的数据字节，并清除SPI状态寄存器中的SPIF标志。
 * 
 * @param u8SpiWB 启动传输的字节数据
 * @return unsigned char 从SPI接口读取的字节数据
 */
unsigned char Spi_Read_Byte(unsigned char u8SpiWB)
{
    unsigned char u8SpiRB;
    // 启动传输的字节数据
    SPDR = u8SpiWB;
    // 等待传输完成，SPIF标志置位
    while(!(SPSR&0x80));
    // 读取接收到的数据字节
    u8SpiRB = SPDR;
    // 清除SPI状态寄存器中的SPIF标志
    clr_SPSR_SPIF;
    // 返回接收到的数据字节
    return u8SpiRB;
}
/*
char status;
unsigned char uid[4];
unsigned char tagType;

// 1. 初始化RC522模块
PcdReset();
M500PcdConfigISOType('A');

// 2. 请求卡片存在
status = PcdRequest(0x26, &tagType);
if (status != MI_OK) {
    printf("No card detected.\r\n");
    return;
}

// 3. 防冲突和选择卡片
status = PcdAnticoll(uid);
if (status != MI_OK) {
    printf("Failed to anticollision.\r\n");
    return;
}

status = PcdSelect(uid);
if (status != MI_OK) {
    printf("Failed to select card.\r\n");
    return;
}

// 4. 认证
unsigned char key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // 默认密钥
status = PcdAuthState(0x60, 0x01, key, uid); // 认证扇区1，使用A密钥
if (status != MI_OK) {
    printf("Authentication failed.\r\n");
    return;
}

// 5. 读写操作
unsigned char data[16];
status = PcdRead(0x01, data); // 读取扇区1的第一个块
if (status == MI_OK) {
    printf("Data read successfully: ");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", data[i]);
    }
    printf("\r\n");
} else {
    printf("Failed to read data.\r\n");
}

// 写入数据
unsigned char writeData[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
status = PcdWrite(0x01, writeData);
if (status == MI_OK) {
    printf("Data written successfully.\r\n");
} else {
    printf("Failed to write data.\r\n");
}

// 6. 结束通信
PcdHalt();

 */