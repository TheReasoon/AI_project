/*************************************************************************
	> File Name: crc16.c
	> Author: Zero_Chen
	> Mail: wufazhucel@163.com
	> Created Time: 2021年02月25日 星期四 11时54分00秒
 ************************************************************************/
#include <stdio.h>
#include "crc16_box.h"


USHORT getModbusCRC16(UCHAR *_pBuf, USHORT _usLen)
{
    USHORT CRCValue = 0xFFFF;           //初始化CRC变量各位为1
    UCHAR i,j;

    for(i=0;i<_usLen;++i)
    {
        CRCValue  ^= *(_pBuf+i);                    //当前数据异或CRC低字节
        for(j=0;j<8;++j)                            //一个字节重复右移8次
        {
            if((CRCValue & 0x01) == 0x01)           //判断右移前最低位是否为1
            {
                 CRCValue = (CRCValue >> 1)^0xA001; //如果为1则右移并异或表达式
            }else 
            {
                CRCValue >>= 1;                     //否则直接右移一位
            }           
        }
    } 
    return CRCValue;            
} 

//*pucFrame 为待校验数据首地址，usLen为待校验数据长度。返回值为校验结果。
USHORT usMBCRC16( UCHAR * pucFrame, USHORT usLen )
{
    UCHAR ucCRCHi = 0xFF;
    UCHAR ucCRCLo = 0xFF;
    int iIndex;
    while( usLen-- )
    {
        iIndex = ucCRCLo ^ *( pucFrame++ );
        ucCRCLo = ( UCHAR )( ucCRCHi ^ aucCRCHi[iIndex] );
        ucCRCHi = aucCRCLo[iIndex];
    }
    return ( USHORT )( ucCRCHi << 8 | ucCRCLo );
}

USHORT Calc_CRC16(int cmd_len, int cmd_id, int cmd, int value_L, int value_H)
{
	UCHAR data[10] = {0};
	USHORT crcValue;

	data[0] = cmd_len;
	data[1] = cmd_id;
	data[2] = cmd;
	data[3] = value_H;
	data[4] = value_L;
	crcValue = getModbusCRC16((UCHAR*)data,cmd_len-2);

	return crcValue;
	
}




