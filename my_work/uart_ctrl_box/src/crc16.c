/*************************************************************************
	> File Name: crc16.c
	> Author: Zero_Chen
	> Mail: wufazhucel@163.com
	> Created Time: 2021��02��25�� ������ 11ʱ54��00��
 ************************************************************************/
#include <stdio.h>
#include "crc16_box.h"


USHORT getModbusCRC16(UCHAR *_pBuf, USHORT _usLen)
{
    USHORT CRCValue = 0xFFFF;           //��ʼ��CRC������λΪ1
    UCHAR i,j;

    for(i=0;i<_usLen;++i)
    {
        CRCValue  ^= *(_pBuf+i);                    //��ǰ�������CRC���ֽ�
        for(j=0;j<8;++j)                            //һ���ֽ��ظ�����8��
        {
            if((CRCValue & 0x01) == 0x01)           //�ж�����ǰ���λ�Ƿ�Ϊ1
            {
                 CRCValue = (CRCValue >> 1)^0xA001; //���Ϊ1�����Ʋ������ʽ
            }else 
            {
                CRCValue >>= 1;                     //����ֱ������һλ
            }           
        }
    } 
    return CRCValue;            
} 

//*pucFrame Ϊ��У�������׵�ַ��usLenΪ��У�����ݳ��ȡ�����ֵΪУ������
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




