/*************************************************************************
	> File Name: ctrl_box.h
	> Author: Zero_Chen
	> Mail: wufazhucel@163.com
	> Created Time: 2021å¹?5æœ?7æ—?æ˜ŸæœŸä¸€ 10æ—?8åˆ?8ç§? ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>     
#include<sys/stat.h>       
#include<fcntl.h>
#include<termios.h>
#include<errno.h> 
#include<string.h>    


void uart_Change_Height(int cur_height,int position,int height);
void uart_Send_CMD(int fd,int cmd,int lenth);
void uart_Send_HEIGHT(int fd,int cmd,int lenth,int height);
int uart_init();
void uart_up_down_ctrl(int fd,int cmd);
void uart_send_height(int fd,int height);
 