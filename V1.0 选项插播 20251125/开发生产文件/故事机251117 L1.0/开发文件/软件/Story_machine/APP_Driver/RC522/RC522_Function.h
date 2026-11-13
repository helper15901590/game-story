/*
 * @LastEditors: T sir
 * @LastEditTime: 2025-09-23 11:12:34
 */
#ifndef __RC522_FUNCTION_H__
#define __RC522_FUNCTION_H__
void RC522_Init(void);
void Rc522_NTAG213_Code(void);
void Save_DATA_clr(void);
void Compute_Card_Number(void);
void Rc522_Test(void);
char Rc522_Read_NTAG216_Full(unsigned char blockAddr);
char Rc522_Write_NTAG216_Block_Simple(unsigned char blockAddr, unsigned char *pData);

char Rc522_Read_Write_NTAG216_Full(unsigned char blockAddr);
char Rc522_Write_NTAG216_Block_Simple_With_Auth(unsigned char blockAddr, unsigned char *pData, unsigned char *pPassword);

void Read_NTAG216_Config(void);
char Rc522_Set_NTAG216_Password(unsigned char *pPassword, unsigned char *pPack, unsigned char auth0Page);
char Rc522_NTAG216_Password_Verify(unsigned char *pPassword);

void RC522_Open(void);
void RC522_Off(void);

#endif
