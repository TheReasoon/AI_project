/*************************************************************************
	> File Name: uart.c
	> Author: Zero_Chen
	> Mail: wufazhucel@163.com
	> Created Time: 2021年03月16日 星期二 16时47分06秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "uart_box.h"
#include "crc16_box.h"
#include "ctrl_box.h"
#include "uart_iot.h"


/*******************************************************************  
*name：          UART0_Open  
*function：      open uart fd
*param：         fd    
                port    uart num(ttyS0,ttyS1,ttyS2)  
*out：			true for 1，error for 0  
*******************************************************************/
int UART0_Open(int fd,char*port)
{
    fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);
    if (fd<0)
    {
        perror("Can't Open Serial Port");
        return(-1);
    }
    //Restore the serial port to blocking state
    if(fcntl(fd, F_SETFL, 0) < 0)
    {
        printf("fcntl failed!\n");
        return(-1);
    }
    else
    {
        printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
    }
	#if 0
    //Test whether it is a terminal device 
    if(0 == isatty(STDIN_FILENO))
    {
        printf("standard input is not a terminal device\n");
        return(-1);
    }
    else
    {
        printf("isatty success!\n");
    }
	#endif
    printf("fd->open=%d\n",fd);
    return fd;
}
/*******************************************************************  
*name：          UART0_Close  
*function：      close uart and return fd
*in：       		fd          file discription
               	port        uart num(ttyS0,ttyS1,ttyS2)  
*out：			void  
*******************************************************************/
 
void UART0_Close(int fd)
{
    close(fd);
}

/*******************************************************************  
*Name: 					UART0_ Set
*Function: 				set serial port data bit, stop bit and check bit
*Entry parameter: 		FD serial port file descriptor
*Speed 					serial port speed
*flow_ctrl 				data flow in Ctrl
*databits				The value of data bits is 7 or 8
*Stopbits 				1 or 2
*parity					The value of parity validity type is n, e, O, S
*out : 					1 for correct return and 0 for error return
*******************************************************************/
int UART0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
    int i;
    //int status;
    int speed_arr[] = {B115200, B19200, B9600, B4800, B2400, B1200, B300};
    int name_arr[] = {115200, 19200, 9600, 4800, 2400, 1200, 300};
             
    struct termios options;    
       
    /* tcgetattr(fd,&options)Get the parameters related to the object FD points to, and save them in theoptions,This function can also test whether the configuration is correct,
	Whether the serial port is available. If the call is successful, the return value of the function is 0. If the call fails, the return value of the function is 1*/
    if(tcgetattr(fd,&options) != 0)
    {
        perror("SetupSerial 1");
        return(-1);
    }

    //Setting input baud rate and output baud rate of serial port
    for ( i= 0; i < sizeof(speed_arr) / sizeof(int); i++)
    {
        if(speed == name_arr[i])
        {
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
        }
    }         
       
    //Modify control mode to ensure that the program does not occupy serial port
    options.c_cflag |= CLOCAL;
    //Modify the control mode to read the input data from the serial port
    options.c_cflag |= CREAD;

    //Set data flow control
    switch(flow_ctrl)
    {
        case 0 ://No flow control
			options.c_cflag &= ~CRTSCTS;
			break;

        case 1 ://Using hardware flow control
			options.c_cflag |= CRTSCTS;
			break;
        case 2 ://Using software flow control
			options.c_cflag |= IXON | IXOFF | IXANY;
			break;
    }
    //set data bit
    //Shield other flag bits
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
        case 5:
			options.c_cflag |= CS5;
			break;
        case 6:
			options.c_cflag |= CS6;
			break;
        case 7:
			options.c_cflag |= CS7;
			break;
        case 8:
			options.c_cflag |= CS8;
			break;
        default:
			fprintf(stderr,"Unsupported data size\n");
			return (-1);
    }
    //Set check bit
    switch (parity)
    {
        case 'n':
        case 'N': //No parity bit
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~ (INPCK|BRKINT |INLCR|ICRNL|IGNCR|IXON);
			options.c_oflag &= ~(ONLCR|OCRNL);
			break;
        case 'o':
        case 'O'://Set to odd parity
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
			break;
        case 'e':
        case 'E'://Set to parity
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
			 break;
        case 's':
        case 'S': //Set to space
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
        default:
			fprintf(stderr,"Unsupported parity\n");
			return (-1);
    }
    //Set stop bit
    switch (stopbits)
    {
        case 1:
			options.c_cflag &= ~CSTOPB; break;
        case 2:
         	options.c_cflag |= CSTOPB; break;
        default:
        	fprintf(stderr,"Unsupported stop bits\n");
			return (-1);
    }
       
    //Modify the output mode and output the original data
    options.c_oflag &= ~OPOST;

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    //options.c_lflag &= ~(ISIG | ICANON);

    //Set the waiting time and minimum receive characters
    options.c_cc[VTIME] = 1; //Read a character and wait 1*(1/10)s
    options.c_cc[VMIN] = 100; // The minimum number of characters to read is 1

    //In case of data overflow, receive the data but do not read it. Refresh the received data but do not read it
    tcflush(fd,TCIFLUSH);

    //Activate configuration (set modified term data to serial port
    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
        perror("com set error!\n");
        return (-1);
    }
    return (0);
}
/*******************************************************************
*Name: 					UART0_ Init()
*Function: 				serial port initialization
*Entry parameter: 		FD file descriptor
*Speed					serial port speed
*flow_ CTRL				data flow control
*Data bits				the value of data bits is 7 or 8
*Stopbits				stop bits are 1 or 2
*Parity					validity type is n, e, O, S
*Out : 					1 for correct return and 0 for error return
*******************************************************************/
int UART0_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
   // int err;
    //Setting serial data frame format
    if (UART0_Set(fd,speed,flow_ctrl,databits,stopbits,parity) == -1)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

/*******************************************************************
*Name:				UART0_ Recv
*Function:			receiving serial data
*Entry parameter:	FD file descriptor
* rcv_ Buf			the data in the receiving serial port is stored in RCV_ In buf buffer
*data_ Len			the length of a frame of data
*******************************************************************/
int UART0_Recv(int fd, UCHAR *rcv_buf,int data_len)
{
    int len,fs_sel;
    fd_set fs_read;
       
    struct timeval time;
       
    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);
       
    time.tv_sec = 10;
    time.tv_usec = 0;
       
    //Multi channel communication of serial port with select function
    fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);
    printf("fs_sel = %d\n",fs_sel);
    if(fs_sel)
    {
        len = read(fd,rcv_buf,data_len);
		#if 1
		printf("uart recv hex data is:\n");
		for(int i = 0; i < len ;i++)
			{
				printf("0x%x ",rcv_buf[i]);			
			}
		printf("\n");
		#endif
        return len;
    }
    else
    {
		printf("receive timeout!\n");
        return -1;
    }
}
/********************************************************************
*name：            	UART0_ Send
*function：        	send data
*in       			fd
*send_ buf     		Store serial port to send data
*data_ len      	The number of data in a frame
*out：    			True returns 1 and error returns 0
*******************************************************************/
int UART0_Send(int fd, UCHAR *send_buf,int data_len)
{
	int i = 0;
    int len = 0;

    len = write(fd,send_buf,data_len);
    if (len == data_len )
    {
		printf("send_buf is:\n");
		for(i = 0; i < data_len; i ++)
		{
			printf("%#X ",send_buf[i]);
		}
		printf("\n");
		for(i = 0; i < data_len; i ++)
		{
			printf("%c ",send_buf[i]);
		}
		printf("\n");
        return len;
    }
    else
    {
        tcflush(fd,TCOFLUSH);
        return -1;
    }
}

int * LG_UART_ReadThread(void * arg)
{
	int i,len;
	int fd = uart_init();

	UCHAR calc_crc16_data[20];
 	UCHAR rcv_buf[128] = {};
	USHORT crcValue;
	USHORT crcValue_L;
	USHORT crcValue_H;

	pthread_t newthid;
	newthid = pthread_self();
	printf("this is a new thread, thread ID = %d\n", newthid);
	
	memset(rcv_buf,0x00,arrsize(rcv_buf));
	
	while(1)
		{
			usleep(100000);
	        len = UART0_Recv(fd, rcv_buf,arrsize(rcv_buf));
			printf("len = :%d\n",len);

	    	if(len > 0)
	    	{
	        	rcv_buf[len] = '\0';
				memset(calc_crc16_data,0x00,arrsize(calc_crc16_data));
				for(i = 0; i < len-2; i ++)
				{
					calc_crc16_data[i] = rcv_buf[i+1];
				}
				
				if(	LG_IOT_HEAD == rcv_buf[0] && LG_IOT_END == rcv_buf[len-1] &&\
					DEV_TYPE_AICAM_LO == rcv_buf[2] && DEV_TYPE_AICAM_HI == rcv_buf[3] )
				{
					printf("Head and tail type detection passed. Start data analysis!\n");
					//calc crc16
					crcValue = getModbusCRC16((UCHAR*)calc_crc16_data,len-4);
					crcValue_L = (crcValue&0xFF00) >> 8;
					crcValue_H = crcValue&0x00FF;

					if(crcValue_L == calc_crc16_data[len-4] && crcValue_H == calc_crc16_data[len-3])
					{
						switch(calc_crc16_data[3])
						{
							case AICAM_DATA_TYPE_CAMSTATE:
								UART0_Send(fd,CAM_STATE_OK,arrsize(CAM_STATE_OK));
								break;
							
							case AICAM_DATA_TYPE_PRODUCTION_INFO:
								UART0_Send(fd,REP_PRODUCTION_INFO,arrsize(REP_PRODUCTION_INFO));
								break;
							
							case AICAM_DATA_TYPE_PM:
								if(0x00 == calc_crc16_data[4])
									{
										//close CAM:
										UART0_Send(fd,ACK_BUF,arrsize(ACK_BUF));
									}
								else if(0x01 == calc_crc16_data[4])
									{
										//sleep CAM:
										UART0_Send(fd,ACK_BUF,arrsize(ACK_BUF));
									}
								else if(0x02 == calc_crc16_data[4])
									{
										//CAM awake:
										UART0_Send(fd,ACK_BUF,arrsize(ACK_BUF));
									}
								break;
								
							case AICAM_DATA_TYPE_USER_ACTION:
								UART0_Send(fd,USER_POWERON,arrsize(USER_POWERON));
								break;
							
							default:
								printf("Unknow CMD!\n");
								break;
						}

					}
					else
					{
						printf("CRC16 check failed!\n");
					}
				}
				else
				{
					printf("It's not CAM cmd data,Don't need to analysis!\n");
				}

			}	 
			else	
			{
				printf("receive timeout!\n");
			}
			memset(rcv_buf,0x00,arrsize(rcv_buf));

		}

	pthread_exit(0);
}

