// /*
//  * @LastEditors: 牛马人
//  * @LastEditTime: 2025-03-07 09:26:16
//  */

// #include "MS51_16K.H"
// #include "Key.h"
// #include "RC522.H"
// #include "RC522_Function.h"
// #include "WTV600.h"
// #include "main.h"
// #include "LED.h"


// //extern uint8_t u8_Card_Number;    // 卡片编号，代表插入了第几张卡

// void Function_Loop_MODE(void)
// {
//     // 按键处理
//     if (key_get_value() > 0)
//     {
//         // uart_send_hex(key_get_value());
//         switch (key_get_value())
//         {
//         case 0xC0: // P3短按
// #if UART0_DEBUG
//             UART0_send_string("P3_short");
// #endif
//             break;
//         case 0xC1: // P3长按
// #if UART0_DEBUG
//             UART0_send_string("P3_long");
// #endif
//                        // 播放
//             break;
//         default:
//             break;
//         }
//         key_clr();
//     }
// }



