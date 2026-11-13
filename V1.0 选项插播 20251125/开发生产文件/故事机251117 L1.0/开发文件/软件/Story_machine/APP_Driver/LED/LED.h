/*
 * @LastEditors: T sir
 * @LastEditTime: 2025-11-14 11:27:08
 */
#ifndef __LED_H__
#define __LED_H__
void LED_MODE_Select(void);
void LED_PWM_Duty_RUN_program(void);
void LED_Init(void);
void LED_MODE(uint8_t mode_cnt);
void LED_clr(void);
void LED_mode_set(uint8_t mode);
void LED_mode_select_clr(uint8_t mode);//在单次操作中使用，带初始化
void TIPLED_MODE_Select(void);
void TIPLED_MODE_Select_clr(UINT8 mode);
#endif