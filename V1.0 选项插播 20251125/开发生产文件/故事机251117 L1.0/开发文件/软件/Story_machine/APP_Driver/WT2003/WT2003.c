/*
 * @LastEditors: T sir
 * @LastEditTime: 2025-11-04 09:51:04
 */
#include "MS51_16K.H"

/*
?:00 表示：OK 命令执行；
?:01 表示：命令出错，不执行；
?:02 表示：EMP 无此文件；
?:05 表示：该设备不在线；
*/

// TF卡语音播放，指定 TF 卡根目录下的音频文件名播放(A3),volume_Play(0,0,0,1);播放0001音频
void volume_Play(UINT8 byte1, UINT8 byte2, UINT8 byte3, UINT8 byte4)
{
	UINT8 cnt_sum;
	byte1 = byte1 + 30;
	byte2 = byte2 + 30;
	byte3 = byte3 + 30;
	byte4 = byte4 + 30;
	cnt_sum = byte1 + byte2 + byte3 + byte4 + 170;
	uart_send_byte(0X7E); // 起始位
	uart_send_byte(0X07); // 长度（除去起始与结束，其他的字节和）
	uart_send_byte(0XA3); // 命令码（A1单次播放、A2循环播放、A3多通道播放）
	uart_send_byte(byte1);
	uart_send_byte(byte2);
	uart_send_byte(byte3);
	uart_send_byte(byte4);
	uart_send_byte(cnt_sum); // 校验位（长度到地址相加和）
	uart_send_byte(0XEF);	 // 结束码
}

/*
// 指定 TF 卡文件夹内-文件名播放(A5)
file: 选择文件夹，也是故事编号  范围001-255
byte1: 音频文件名前三位 000-255	 表示节点数
byte2: 音频文件最后一位 0-9 	表示选项音频
volume_Play_TF（1,2,3） 故事1 主音频2 选项音频3
*/
void volume_Play_TF(UINT16 file, UINT8 byte1, UINT8 byte2)
{
	UINT8 cnt_sum, file_tens, file_units, file_hundreds, byte1_tens, byte1_units, byte1_hundreds, byte2_units;
	// 提取file的十位和个位
	file_hundreds = file / 100;
	file_tens = file / 10;	// 十位数字
	file_units = file % 10; // 个位数字
	file_hundreds = file_hundreds + 0x30;
	file_tens = file_tens + 0x30;
	file_units = file_units + 0x30;

	byte1_hundreds = file / 100;
	byte1_tens = byte1 / 10;
	byte1_units = byte1 % 10;
	byte1_hundreds = byte1_hundreds + 0x30;
	byte1_tens = byte1_tens + 0x30;
	byte1_units = byte1_units + 0x30;

	byte2_units = byte2 % 10;
	byte2_units = byte2_units + 0x30;

	cnt_sum = file_hundreds + file_tens + file_units + byte1_tens + byte1_units + byte1_hundreds + byte2_units + 273;
	uart_send_byte(0X7E); // 起始位
	uart_send_byte(0X0C); // 长度（除去起始与结束，其他的字节和）
	uart_send_byte(0XA5); // 命令码（A1单次播放、A2循环播放、A3多通道播放）

	uart_send_byte(0x30); // 发送故事编码 00000-00255
	uart_send_byte(0x30);
	uart_send_byte(file_hundreds);
	uart_send_byte(file_tens);
	uart_send_byte(file_units);

	uart_send_byte(byte1_hundreds); // 发送节点编号 000-255
	uart_send_byte(byte1_tens);
	uart_send_byte(byte1_units);

	uart_send_byte(byte2_units); // 发送选项编号 0-9

	uart_send_byte(cnt_sum); // 校验位（长度到地址相加和）
	uart_send_byte(0XEF);	 // 结束码
}
/*
// 指定 TF 卡文件夹内-文件名中选项音频播放(A5)
file: 选择文件夹，也是故事编号  范围001-255
byte1: 音频文件名前三位 000-255	 表示节点数
byte2: 选项音频对应的插播文件 0-9  对应 A B C D E F G H I  	表示选项音频的插播音频
volume_Play_TF（1,2,3） 故事1 主音频2 选项音频3的插播音频
*/
void volume_Play_TF_option(UINT16 file, UINT8 byte1, UINT8 byte2)
{
	UINT8 cnt_sum, file_tens, file_units, file_hundreds, byte1_tens, byte1_units, byte1_hundreds, byte2_units;
	// 提取file的十位和个位
	file_hundreds = file / 100;
	file_tens = file / 10;	// 十位数字
	file_units = file % 10; // 个位数字
	file_hundreds = file_hundreds + 0x30;
	file_tens = file_tens + 0x30;
	file_units = file_units + 0x30;

	byte1_hundreds = file / 100;
	byte1_tens = byte1 / 10;
	byte1_units = byte1 % 10;
	byte1_hundreds = byte1_hundreds + 0x30;
	byte1_tens = byte1_tens + 0x30;
	byte1_units = byte1_units + 0x30;

	byte2_units = byte2 % 10;
	byte2_units = byte2_units + 0x40;

	cnt_sum = file_hundreds + file_tens + file_units + byte1_tens + byte1_units + byte1_hundreds + byte2_units + 273;
	uart_send_byte(0X7E); // 起始位
	uart_send_byte(0X0C); // 长度（除去起始与结束，其他的字节和）
	uart_send_byte(0XA5); // 命令码（A1单次播放、A2循环播放、A3多通道播放）

	uart_send_byte(0x30); // 发送故事编码 00000-00255
	uart_send_byte(0x30);
	uart_send_byte(file_hundreds);
	uart_send_byte(file_tens);
	uart_send_byte(file_units);

	uart_send_byte(byte1_hundreds); // 发送节点编号 000-255
	uart_send_byte(byte1_tens);
	uart_send_byte(byte1_units);

	uart_send_byte(byte2_units); // 发送选项编号 0-9

	uart_send_byte(cnt_sum); // 校验位（长度到地址相加和）
	uart_send_byte(0XEF);	 // 结束码
}
/*
系统音播放 0001 开机
data1：0-9 data2：0-9 data3：0-9 data4：0-9
volume_Play_TF_SYS(0,0,0,1); SYSTE文件夹内命名为0001的音频播放
*/
void volume_Play_TF_SYS(UINT8 data1, UINT8 data2, UINT8 data3, UINT8 data4)
{
	uint8_t cnt_sum;
	data1 = data1 + 0x30;
	data2 = data2 + 0x30;
	data3 = data3 + 0x30;
	data4 = data4 + 0x30;

	cnt_sum = 0X0C + 0XA5 + 0x53 + 0x59 + 0x53 + 0x54 + 0x45 + data1 + data2 + data3 + data4;
	uart_send_byte(0X7E); // 起始位
	uart_send_byte(0X0C); // 长度（除去起始与结束，其他的字节和）
	uart_send_byte(0XA5); // 命令码（A1单次播放、A2循环播放、A3多通道播放）

	uart_send_byte(0x53); // S
	uart_send_byte(0x59); // Y
	uart_send_byte(0x53); // S
	uart_send_byte(0x54); // T
	uart_send_byte(0x45); // E

	uart_send_byte(data1); //
	uart_send_byte(data2);
	uart_send_byte(data3);
	uart_send_byte(data4); //

	uart_send_byte(cnt_sum); // 校验位（长度到地址相加和）
	uart_send_byte(0XEF);	 // 结束码
}


/*
. 指定 TF 卡文件夹内音频索引播放(A4)
此命令可以指定根目录下的文件夹内的音频按索引序号进行播放。（文件夹名固定 5 字
符） 该项目主要用于插播按键播放
 */
void volume_Play_TF_A4(UINT8 high,UINT8 low)
 {
	uint8_t sum = 10+164+48+48+48+48+48 + high + low;
	uart_send_byte(0X7E); // 起始位
	uart_send_byte(0X0A); // 长度（除去起始与结束，其他的字节和）
	uart_send_byte(0XA4); // 命令码

	uart_send_byte(0X30);
	uart_send_byte(0X30);
	uart_send_byte(0X30);
	uart_send_byte(0X30);
	uart_send_byte(0X30);

	uart_send_byte(high); // 曲目高位
	uart_send_byte(low);  // 曲目低位
	uart_send_byte(sum);  // 校验位（长度到地址相加和）
	uart_send_byte(0XEF);
 }
 
/*暂停放音命令(AA),播放状态下，发送该指令，则暂停播放；暂停状态下，发送该指令，则从暂停处继续播
放音乐。停止状态下发送该指令无效。*/
void volume_pause(void)
{
	uart_send_byte(0X7E); // 起始位
	uart_send_byte(0X03); // 长度（除去起始与结束，其他的字节和）
	uart_send_byte(0XAA); // 命令码
	uart_send_byte(0XAD); // 校验位（长度到地址相加和）
	uart_send_byte(0XEF); // 结束码
}

// 停止播放命令(AB),停止播放,发送该指令，停止播放当前正在播放的音乐。
void volume_stop(void)
{
	uart_send_byte(0X7E); // 起始位
	uart_send_byte(0X03); // 长度（除去起始与结束，其他的字节和）
	uart_send_byte(0XAB); // 命令码
	uart_send_byte(0XAE); // 校验位（长度到地址相加和）
	uart_send_byte(0XEF); // 结束码
}

/*音量控制命令(AE),音量等级共有 32 级，分别为 0～31，其中 0 为静音，1F 级为最大音量。（此命令具有掉
电记忆）*/
void volume_control(unsigned char DATA)
{
	uint8_t sum = 178 + DATA;
	uart_send_byte(0X7E); // 起始位
	uart_send_byte(0X04); // 长度（除去起始与结束，其他的字节和）
	uart_send_byte(0XAE); // 命令码
	uart_send_byte(DATA); // 分别为 0～31，其中 0 为静音，1F 级为最大音量。
	uart_send_byte(sum);  // 校验位（长度到地址相加和）
	uart_send_byte(0XEF); // 结束码
}

/*插播指令(B1)该指令只能在播放状态下插播，停止状态下插播无效，不支持内置固定语音（9F）播放
过程中插播
注：发送本条指令时，会暂停正在播放的曲目，然后执行本条指令所指定的播放曲目，
当播放完后，接着播放原来暂停的曲目。第一次插播命令未播放完时，发第二次插播命令时，
命令无效。要等第一次插播音乐播放完后才可以再一次进行插播。标示字：?00；表示：插
播 FLASH 内指定索引地址；?01；表示：插播 TF 卡内指定索引地址；?02；表示：插播 U
盘内指定索引地址；
volume_insert(0,1)；//插播 第一首索引
*/
void volume_insert(unsigned char high, unsigned char low)
{
	uint8_t sum = 184 + high + low;
	uart_send_byte(0X7E); // 起始位
	uart_send_byte(0X06); // 长度（除去起始与结束，其他的字节和）
	uart_send_byte(0XB1); // 命令码
	uart_send_byte(0X01); // 插播 TF 卡内指定索引地址
	uart_send_byte(high); // 曲目高位
	uart_send_byte(low);  // 曲目低位
	uart_send_byte(sum);  // 校验位（长度到地址相加和）
	uart_send_byte(0XEF);
}

/*音频输出方式切换(B6)
发送该指令后，切换到 SPK 输出或 DAC 输出，此功能具有掉电记忆（芯片默认 SPK 输
出）
参数：00 表示 SPK 输出，01 表示 DAC 输出
注：示例指令是在默认 SPK 输出的基础上切换到 DAC 输出
*/
void volume_output(unsigned char DATA)
{
	uint8_t sum = 186 + DATA;
	uart_send_byte(0X7E); // 起始位
	uart_send_byte(0X04); // 长度（除去起始与结束，其他的字节和）
	uart_send_byte(0XB6); // 命令码
	uart_send_byte(DATA); // 0：SPK 输出；1：DAC 输出
	uart_send_byte(sum);  // 校验位（长度到地址相加和）
	uart_send_byte(0XEF);
}
