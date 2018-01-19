#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "eggm.h"

int listenfd = 0, connfd = 0;
#define MY_MACHINE_ID 1


int eggm_serv_run()
//int eggm_serv_on_service()
{
    struct sockaddr_in serv_addr;

    char sendBuff[ EGGM_MSG_BUFFER_SIZE ];
    char recvBuff[ EGGM_MSG_BUFFER_SIZE ];
    int port_num = 11000;
    
    //    time_t ticks;
    printf("Start eggm_serv_on_service() function\n");
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)
    {
	printf("socket err!\n");
	exit(1);
    }
    memset(&serv_addr, '0', sizeof(struct sockaddr_in));
    memset(sendBuff, '0', EGGM_MSG_BUFFER_SIZE);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port_num);
    //serv_addr.sin_port = htons(11000);
    //serv_addr.sin_port = htons(5022);

    printf("bind socket =%d!!!\n",port_num);
    

    while(1)
    {
	if( bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
	{
	    sleep(1);
	    printf("bind err!, retry!!\n");
	}
	else
	    break;
    }
    while(1)
    {
	if ( listen(listenfd, 10) == -1)
	{
	    sleep(1);
	    printf("listenfd err!, retry!!\n");
	}
	else
	    break;
    }

    printf("\n\n\n");
    printf("======================================\n");
    printf("======= eggm server open!!============\n");
    printf("======================================\n");
    while(1)
    {
	int n = 0;
	if(listenfd < 0)
	{
	    printf("close socket\n");
	    break;
	}
	connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
	printf("<accept!!>\n");
	if(connfd>0)
	{
	    struct eggm_msg_t recv_msg;
	    struct eggm_msg_t send_msg;
	    memset(&send_msg, 0, sizeof(struct eggm_msg_t));
	    n = read(connfd, recvBuff, EGGM_MSG_BUFFER_SIZE);
	    {
		//parseRecvPacket ( recvBuff,  &recv_msg );
		printPacket( recvBuff);
		
		printf("[RECV] : \n");
		//printEggmMsg( recv_msg);
	
		memcpy( &send_msg, &recv_msg, sizeof(struct eggm_msg_t));
		//send_msg.msg_type = EGGM_MSG_TYPE_RESPONSE;
		//send_msg.src_machine_id = MY_MACHINE_ID;
		//send_msg.dst_machine_id = 100;

		
		//makeSendPacket( send_msg, sendBuff);
		write(connfd, sendBuff, EGGM_MSG_BUFFER_SIZE);

		printf("write\n");
	    }
	    close(connfd);
	    //sleep(1);
	    usleep(100);
	}
    }


    return 1;
}
int eggm_serv_close()
{
    close(listenfd);
    printf(" close listen socket ok!!\n");
}


int eggm_send_bcast()
{
    return 1;
}

int eggm_get_bcast()
{
    return 1;
}

#if 0
int eggm_serv_run()
{

    return 1;
}
#endif
