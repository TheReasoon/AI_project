/*************************************************************************
	> File Name: uart_test.c
	> Author: Zero_Chen
	> Mail: wufazhucel@163.com
	> Created Time: 2021年03月16日 星期二 16时47分59秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h> 
#include<unistd.h>     /*Unix func*/    
#include<sys/types.h>     
#include<sys/stat.h>       
#include<fcntl.h>      /*file ctrl*/    
#include<termios.h>    /*PPSIX console*/    
#include<errno.h>      /*error num*/    
#include<string.h>
#include "../include/uart_box.h"
#include "../include/crc16_box.h"

int len;							

char rcv_buf[64];			   
char send_Read_Buffer[16] = {0x9B, 0x05, 0x03, 0x12, 0x3c, 0xE1, 0x9D};
//static int height_temp = 0;
int height_shift = 0;

const unsigned char DISP_NUM_CHAR[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};//0 - 9



//  CMD:(CMD_HEAD,CMD_LEN,CMD_ID,CMD,CRC_L,CRC_H,CMD_END)

 void uart_Send_CMD(int fd,int cmd,int lenth)
{
	
 	USHORT crcValue;
	USHORT crcValue_L;
	USHORT crcValue_H;
	char send_CMD_Buffer[16];
	crcValue = Calc_CRC16(lenth, CMD_ID, cmd,0,0);
	crcValue_L = (crcValue&0xFF00) >> 8;
	crcValue_H = crcValue&0x00FF;
	
	//printf("CRC16 value is: 0x%x\n",crcValue);
	//printf("CRC16 crcValue_L is: 0x%x\n",crcValue_L);
	//printf("CRC16 crcValue_H is: 0x%x\n",crcValue_H);

	send_CMD_Buffer[0] = CMD_HEAD;
	send_CMD_Buffer[1] = lenth;
	send_CMD_Buffer[2] = CMD_ID;
	send_CMD_Buffer[3] = cmd;
	send_CMD_Buffer[4] = crcValue_L;
	send_CMD_Buffer[5] = crcValue_H;
	send_CMD_Buffer[6] = CMD_END;
	UART0_Send(fd,send_CMD_Buffer,lenth+2);
	return;
}
 void uart_Send_HEIGHT(int fd,int cmd,int lenth,int height)
 {
 	USHORT crcValue;
	USHORT crcValue_L;
	USHORT crcValue_H;
 	USHORT hightValue_L;
	USHORT hightValue_H;
	char send_CMD_Buffer[16];
 	hightValue_L = (height&0xFF00) >> 8;
	hightValue_H = (height&0x00FF);
	crcValue = Calc_CRC16(lenth, CMD_ID, cmd,hightValue_L,hightValue_H);
	crcValue_L = (crcValue&0xFF00) >> 8;
	crcValue_H = crcValue&0x00FF;
	
	send_CMD_Buffer[0] = CMD_HEAD;
	send_CMD_Buffer[1] = lenth;
	send_CMD_Buffer[2] = CMD_ID;
	send_CMD_Buffer[3] = cmd;
	send_CMD_Buffer[4] = hightValue_H;
	send_CMD_Buffer[5] = hightValue_L;
	send_CMD_Buffer[6] = crcValue_L;
	send_CMD_Buffer[7] = crcValue_H;
	send_CMD_Buffer[8] = CMD_END;
	
	UART0_Send(fd,send_CMD_Buffer,lenth+2);

	return;
 }
 
int uart_init()
{
	int fd = -1;           //fd
    int err;               //error state
    
    fd = UART0_Open(fd,"/dev/ttySLB2"); //open uart
    //printf("fd= \n",fd);

	do
	{
		err = UART0_Init(fd,9600,0,8,1,'N');
        printf("Set Port Exactly!\n");
        sleep(1);
    }while(-1 == err || -1 == fd);
	return fd;
}

void uart_up_down_ctrl(int fd,int cmd)
{   

	

	int count_up_down = 0;

	switch(cmd)
	{
		case 0x01:
		{
			for(count_up_down = 0; count_up_down <10; count_up_down ++)
			{
				usleep(100000);
				uart_Send_CMD(fd,KEY_UP_DATA,POSITION_CMD_LEN);
			}
			break;
		}
		case 0x02:
		{
			for(count_up_down = 0; count_up_down < 10; count_up_down ++)
			{
				usleep(100000);
				uart_Send_CMD(fd,KEY_DN_DATA,POSITION_CMD_LEN);
			}
			break;
		}
		default:
		{
			for(count_up_down = 0; count_up_down < 10; count_up_down ++)
			{
				usleep(100000);
				uart_Send_CMD(fd,KEY_NONE_DATA,POSITION_CMD_LEN);
			}
			break;
		}
	}
    UART0_Close(fd);
}

void uart_send_height(int fd,int height)
{


	int count_time = 0;
	for(count_time = 0; count_time < 4; count_time ++)
	{
		usleep(100000);
		uart_Send_HEIGHT(fd,KEY_SET_HEIGHT,HEIGHT_CMD_LEN,height);
	}
    UART0_Close(fd);
	return;
}

void uart_change_height(int fd,int position,int height)
{
	uart_send_height(fd,(get_hight(fd)+height));
	return;
}
	
	
int get_hight(int fd)
{
	unsigned char deca = 1;
	unsigned int displayData = 0;
	int i,j;
	int height;
	int READ_COUNT;
	//By send CMD A+ find out the real hight now!
	UART0_Send(fd,send_Read_Buffer,7);
	
    for(READ_COUNT = 0; READ_COUNT < 5; READ_COUNT ++) //read data 3 times
    {
		usleep(50000);
    	printf("READ_COUNT = [%d]\n",READ_COUNT);
        len = UART0_Recv(fd, rcv_buf,sizeof(rcv_buf));
		usleep(50000);
        if(len > 0)
        {
            rcv_buf[len] = '\0';
			printf("rcv_buf is: %s\n",rcv_buf);
			if(rcv_buf[1] == 0x07 && rcv_buf[2] == 0x12)
			{
				if((rcv_buf[4] & 0x80) == 0x80)
					deca = 1;
				else 
					deca = 10;

				rcv_buf[3] &= 0x7F;
				rcv_buf[4] &= 0x7F;
				rcv_buf[5] &= 0x7F;

				for( i = 3; i < 6; i++)
				{
				    displayData *= 10;
				    for( j = 0; j < sizeof(DISP_NUM_CHAR); j++)
				    {
				        if(rcv_buf[i] == DISP_NUM_CHAR[j])
				        {
				            displayData += j;
				            break;
				        }
				    }
				}
				displayData *= deca;
				height = displayData;
				printf("[ Get Height = [%d] \n", displayData);
				displayData = 0;
			}
        }
        else
        {
            printf("cannot receive data\n");
        }
        usleep(50000);
	}
	return height;
} 

