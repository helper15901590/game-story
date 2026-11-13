/*
 * @LastEditors: T sir
 * @LastEditTime: 2025-11-04 09:51:24
 */
#ifndef __WTV600_H__
#define __WTV600_H__
void volume_Play(UINT8 byte1, UINT8 byte2, UINT8 byte3, UINT8 byte4);
void volume_pause(void);
void volume_stop(void);
void volume_control(unsigned char DATA);
void volume_insert(unsigned char high, unsigned char low);
void volume_output(unsigned char DATA);

void volume_Play_TF(UINT16 file, UINT8 byte1, UINT8 byte2);
void volume_Play_TF_SYS(UINT8 data1, UINT8 data2, UINT8 data3, UINT8 data4);
void volume_Play_TF_A4(UINT8 high,UINT8 low);
void volume_Play_TF_option(UINT16 file, UINT8 byte1, UINT8 byte2);
#endif