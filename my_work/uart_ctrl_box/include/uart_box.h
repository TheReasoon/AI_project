/*************************************************************************
	> File Name: uart.h
	> Author: Zero_Chen
	> Mail: wufazhucel@163.com
	> Created Time: 2021年03月16日 星期二 16时46分59秒
 ************************************************************************/

#ifndef  _UART_H
#define  _UART_H
 
  
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>     
#include<sys/stat.h>       
#include<fcntl.h>
#include<termios.h>
#include<errno.h> 
#include<string.h>    
     

#define FALSE  0    
#define TRUE   1

typedef unsigned char UCHAR;
typedef unsigned char *      LPTSTR;
#define CMD_HEAD	        	0x9B
#define CMD_END	        		0x9D
#define CMD_ID                 	0x03

#define POSITION_CMD_LEN	   	0x05
#define HEIGHT_CMD_LEN	    	0x07


//                命令号
/*---------------------------------------*/
#define READ_KEY_VALUE_CMD               0x11//读取键值
#define DISPLAY_CHAR_CMD                 0x12//显示字符

#define KEY_SET_HEIGHT                   0x80

#define KEY_NONE_DATA                    0x00

#define KEY_UP_DATA                      0x01
#define KEY_DN_DATA                      0x02
#define KEY1_DATA                        0x04
#define KEY2_DATA                        0x08
#define KEY3_DATA                        0x10
#define KEYM_DATA                        0x20
#define KEYA_DATA                        0x40
#define KEYM3_DATA                       0x12

#define KEY_RST_DATA                     0x30


//LPTSTR StringToHex(const char* lpSrc, char chTag);
int UART0_Open(int fd,char*port);
void UART0_Close(int fd) ; 
int UART0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
int UART0_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity) ;
int UART0_Recv(int fd, UCHAR *rcv_buf,int data_len);
int UART0_Send(int fd, UCHAR *send_buf,int data_len);
 
#endif


