#include<stdio.h>
#include"eggm.h"

#include<time.h>
#include<sys/time.h>


#if 0
void printPacket(char *packet)
{
    int i;
    for( i=0 ; i<200 ; i++)
    {
	if ( i % 12 == 0)
	    printf("\n");
	printf(" %02X", packet[i]);
    }
	printf("\n");
}
#endif
int make_eggm_checksum( struct eggm_msg_t *eggm_msg)
{
	int i;
	char checksumBuff[EGGM_MSG_SIZE];
	unsigned int checksum = 0;
	memset(&checksumBuff, 0, EGGM_MSG_SIZE);
	memcpy(&checksumBuff, &eggm_msg, EGGM_MSG_SIZE);
	for(i=0;i < EGGM_MSG_SIZE - 4 ; i++)
	{
		checksum += checksumBuff[i];
	}

//	printf("checksum = 0x%08X\n", checksum);
	eggm_msg->checksum[0] = ( checksum & 0xFF000000 ) >> 24;
	eggm_msg->checksum[1] =  ( checksum & 0x00FF0000 ) >> 16;
	eggm_msg->checksum[2] =  ( checksum & 0x0000FF00 ) >> 8;
	eggm_msg->checksum[3] =  ( checksum & 0x000000FF ) ;
//	printf("checksum[0] = 0x%02X\n", eggm_msg->checksum[0]);
//	printf("checksum[1] = 0x%02X\n", eggm_msg->checksum[1]);
//	printf("checksum[2] = 0x%02X\n", eggm_msg->checksum[2]);
//	printf("checksum[3] = 0x%02X\n", eggm_msg->checksum[3]);
}

int make_eggm_sr_checksum( struct eggm_sr_msg_t *eggm_msg)
{
	int i;
	char checksumBuff[EGGM_SR_MSG_SIZE];
	unsigned int checksum = 0;
	memset(checksumBuff, 0, EGGM_SR_MSG_SIZE);
	memcpy(checksumBuff, eggm_msg, EGGM_SR_MSG_SIZE);
	for(i=0;i < EGGM_SR_MSG_SIZE - 2 ; i++)
	{
		checksum += checksumBuff[i];
	}

	//printf("checksum = 0x%08X\n", checksum);
	eggm_msg->checksum[0] =  ( checksum  >> 8) & 0xFF;
	eggm_msg->checksum[1] =  checksum & 0xFF ;
//	printf("checksum[0] = 0x%02X\n", eggm_msg->checksum[0]);
//	printf("checksum[1] = 0x%02X\n", eggm_msg->checksum[1]);
//	printf("checksum[2] = 0x%02X\n", eggm_msg->checksum[2]);
//	printf("checksum[3] = 0x%02X\n", eggm_msg->checksum[3]);
}

void print_string_to_hex(char* str)
{
	int i;
	printf("%s : ", str); 
	for( i=0 ; i<MAX_NAME_LEN; i++)
	{
		printf(" %02X", str[i]);
	}
	printf("\n");
}


void print_eggm_msg( struct eggm_msg_t eggm_msg)
{
	int i;
	printf(" ---------------------------\n");
	printf(" |   EGGM MSG              |\n");
	printf(" ---------------------------\n");
	printf(" key1 : 0x%02X, key2 : 0x%02X\n", eggm_msg.key1, eggm_msg.key2);
	printf(" src_ : 0x%02X, dst  : 0x%02X\n", eggm_msg.src_id, eggm_msg.dst_id);
	for(i=0;i< EGGM_MSG_DATA_SIZE;i++)
	{
		printf(" %02X",eggm_msg.data[i]);
		if((i+1)%10 == 0)
			printf("\n");
	}
	printf(" checksum : 0x %02X %02X %02X %02X\n",
			eggm_msg.checksum[0],
			eggm_msg.checksum[1],
			eggm_msg.checksum[2],
			eggm_msg.checksum[3]);
	printf("----------------------------\n");
}

void print_eggm_sr_msg(int is_recv, struct eggm_sr_msg_t eggm_msg)
{
#ifdef DEBUG_PW
	int i;
	printf(" ---------------------------\n");
	printf(" | [%04s]EGGM MSG          |\n", is_recv? "RECV":"SEND");
	printf(" ---------------------------\n");
	printf(" key1 : 0x%02X, key2 : 0x%02X\n", eggm_msg.key1, eggm_msg.key2);
	for(i=0;i< EGGM_SR_MSG_DATA_SIZE;i++)
	{
		printf(" %02X",eggm_msg.data[i]);
	}
	printf(" checksum : 0x %02X %02X\n",
			eggm_msg.checksum[0],
			eggm_msg.checksum[1]);
	printf("----------------------------\n");
#endif
}
#define STR_TIME_FORMAT      "%y-%m-%d/%H:%M:%S"

void eggm_log(char *str)
{
	struct tm tm_now;
	struct timespec tspec;
	char buff[32];
	char cmd[1000];
	// get time for clock
	if ( clock_gettime(CLOCK_REALTIME, & tspec) == -1)
	{
		printf("log err\n");
		return;
	}

	// timespec to tm
	localtime_r((time_t*)&tspec.tv_sec, &tm_now);

	if(strftime(buff, 32, STR_TIME_FORMAT, &tm_now)==0)
	{
		printf("log err\n");
		return;
	}

	sprintf(cmd, "echo \"[%s] %s\" >> .eggm.log", buff, str);
	system(cmd);
}

#if 0
void makeEggmMsg(char msg_type, int src_id, int dst_id, int msg_id, char *data, int data_len, struct eggm_msg_t *msg)
{
    memset( msg, 0, sizeof(struct eggm_msg_t));

    msg->msg_type = msg_type;
    msg->src_machine_id = src_id;
    msg->dst_machine_id = dst_id;
    msg->msg_id = msg_id;
    memcpy( &msg->data, data, data_len);
}

void makeSendPacket ( struct eggm_msg_t send_msg, char* sendPacket )
{
    memset( sendPacket , 0, EGGM_MSG_DATA_SIZE );

    sendPacket[0] = 0x20;
    sendPacket[1] = 0x17;
    sendPacket[2] = 0x33;
    sendPacket[3] = send_msg.msg_type;

    //send_msg.src_machine_id 
    sendPacket[4] = (char)((send_msg.src_machine_id>>24) & 0xFF) ;
    sendPacket[5] = (char)((send_msg.src_machine_id>>16) & 0xFF);
    sendPacket[6] = (char)((send_msg.src_machine_id>>8) & 0xFF);
    sendPacket[7] = (char)(send_msg.src_machine_id & 0xFF);

    //send_msg.dst_machine_id
    sendPacket[8] = (char)((send_msg.dst_machine_id >> 24) & 0xFF);
    sendPacket[9] = (char)((send_msg.dst_machine_id >> 16) & 0xFF);
    sendPacket[10] = (char)((send_msg.dst_machine_id >> 8) & 0xFF);
    sendPacket[11] = (char)(send_msg.dst_machine_id & 0xFF);

    //send_data.msg_id
    sendPacket[12] = (char)((send_msg.msg_id >> 24) & 0xFF);
    sendPacket[13] = (char)((send_msg.msg_id >> 16) & 0xFF);
    sendPacket[14] = (char)((send_msg.msg_id >> 8) & 0xFF);
    sendPacket[15] = (char)(send_msg.msg_id & 0xFF);


    //data
    memcpy( &sendPacket[ EGGM_MSG_HEADER_SIZE ], &send_msg.data, EGGM_MSG_DATA_SIZE );

}

void parseRecvPacket ( char *recvPacket, struct eggm_msg_t *recv_msg)
{
    int i;
    char str[EGGM_MSG_DATA_SIZE];
    recv_msg->msg_type = recvPacket[3];

    recv_msg->src_machine_id =
	(int)recvPacket[4] << 24
	| (int)recvPacket[5] << 16
	| (int)recvPacket[6] << 8
	| (int)recvPacket[7] ;

    recv_msg->dst_machine_id =
	(int)recvPacket[8] << 24
	| (int)recvPacket[9] << 16
	| (int)recvPacket[10] << 8
	| (int)recvPacket[11] ;

    recv_msg->msg_id =
	(int)recvPacket[12] << 24
	| (int)recvPacket[13] << 16
	| (int)recvPacket[14] << 8
	| (int)recvPacket[15] ;

    for(i=0;i<EGGM_MSG_DATA_SIZE;i++)
    {
	str[i] = recvPacket[ EGGM_MSG_HEADER_SIZE + i];
    }
    memcpy( &recv_msg->data, &recvPacket[ EGGM_MSG_HEADER_SIZE ], EGGM_MSG_DATA_SIZE);
}
#endif
