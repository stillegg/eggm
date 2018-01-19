#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "eggm.h"
struct sockaddr_in serv_addr;

int eggm_client_socket_init(char *serv_ip, int serv_port)
{
    printf("[%s/%d] server => %s : %d\n",__func__,__LINE__, serv_ip, serv_port);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    if(inet_pton(AF_INET, serv_ip, &serv_addr.sin_addr)<=0)
    {
	printf("\n inet_pton error occured\n");
	return 1;
	/* stillegg, 2017.08.17: */
    }
    printf(" socket init done\n");
}



int eggm_client_send_msg(char *data,int  len)
{
    int n;
    struct eggm_msg_t send_msg;
    struct eggm_msg_t recv_msg;
    
    int sockfd = 0;
    
    char send_buff[EGGM_MSG_BUFFER_SIZE];
    char recvBuff[1024];
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
	printf("\n Error : Could not create socket \n");
	return 1;
    }
    
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
	printf("\n Error : Connect Failed \n");
	return 1;
    }

    //makeEggmMsg(EGGM_MSG_TYPE_RESPONSE, 3,4,0X3278, data, len, &send_msg);
    //makeSendPacket( send_msg, send_buff);

    printPacket(send_buff);
    write(sockfd, send_buff, EGGM_MSG_BUFFER_SIZE);

    printf("[SEND]: %s\n ",data);
   
    usleep(100);
    n = read(sockfd, recvBuff, EGGM_MSG_BUFFER_SIZE);
    if (n > 0)
    {
	printPacket(recvBuff);
	//parseRecvPacket( recvBuff,  &recv_msg);
	printf("[RECV]:\n");
	//printEggmMsg(recv_msg);
    }
    else
	printf("read err!!!\n");
    
    
    close(sockfd);
}



