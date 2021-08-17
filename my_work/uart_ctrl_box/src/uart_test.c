/*************************************************************************
	> File Name: uart_test.c
	> Author: Zero_Chen
	> Mail: wufazhucel@163.com
	> Created Time: 2021年03月16日 星期二 16时47分59秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "uart_box.h"
#include "crc16_box.h"
#include "ctrl_box.h"
#include "uart_iot.h"




int main(int argc, char **argv)    
{
	
	pthread_t assistthid;
	int status;
	pthread_create(&assistthid,NULL,(void *)LG_UART_ReadThread,NULL);
	pthread_join(assistthid,(void *) &status);
	printf("assistthread's exit is caused %d\n",status);

	return 0;


#if 0
printf("test uart!!!\n");
int fd = uart_init();
//uart_up_down_ctrl(fd,0x01);
uart_change_height(fd,0x02,20);


//uart_up_down_ctrl(fd,0x02);
//uart_send_height(fd,1200)

 	return 0;
#endif

}


