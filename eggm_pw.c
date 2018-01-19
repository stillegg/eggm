
#include<stdio.h>
#include<pthread.h>

#include"eggm.h"

// for serial
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


#include <errno.h>
#include <string.h>
#include <sys/types.h>


int g_pw_machine_state = 0;


struct eggm_pw_data_t g_eggm_pw_data;


pthread_mutex_t pw_mutex = PTHREAD_MUTEX_INITIALIZER;


void pw_data_lock() { pthread_mutex_lock(&pw_mutex); }
void pw_data_unlock() { pthread_mutex_unlock(&pw_mutex); }





void init_pw_data()
{
	    memset(&g_eggm_pw_data,0, sizeof(struct eggm_pw_data_t));
}

// get data
int get_pw_data(struct eggm_pw_data_t *pw_data)
{   
    int ret = 0;
    pw_data_lock();
#if 0// just debug
    printf("\n++++++start+++++pw_data+++++++++++\n");
    printf("physical:%d, auto:%d, real:%d, machine=%d\n",
        g_eggm_pw_data.state_physical,
        g_eggm_pw_data.state_auto,
        g_eggm_pw_data.state_real,
        g_pw_machine_state);
    printf("++++++end+++++++pw_data+++++++++++\n\n");
#endif
    
    if( g_pw_machine_state == 1 )
        ret = 1;
    
    memcpy(pw_data, &g_eggm_pw_data, sizeof(struct eggm_pw_data_t));
#if 0// just debug
    printf("physical:%d, auto:%d, real:%d, machine=%d\n",
        pw_data->state_physical,
        pw_data->state_auto,
        pw_data->state_real,
        ret);
    printf("++++++end+++++++pw_data+++++++++++\n\n");
#endif
    pw_data_unlock();
    return ret;
}

void set_pw_data(struct eggm_pw_data_t *pw_data)
{
	char str[200];    
	pw_data_lock();
    g_eggm_pw_data.state_physical = pw_data->state_physical;
    g_eggm_pw_data.state_auto = pw_data->state_auto ;
    g_eggm_pw_data.state_real = pw_data->state_real ;
    g_eggm_pw_data.state_port = pw_data->state_port ;
    pw_data_unlock();
    
	printf("[%s/%d] - pw_data->state_auto = %d\n", __func__,__LINE__,pw_data->state_auto);
	sprintf(str, "set pw state pysic:%d, auto:%d, real:%d",
				pw_data->state_physical, pw_data->state_auto, pw_data->state_real);
	eggm_log(str);
}

void set_pw_auto_state(int is_on)
{
	char str[200];    
    pw_data_lock();
    g_eggm_pw_data.state_auto = is_on ;
    pw_data_unlock();

    printf("[%s/%d] - pw_data->state_auto = %d\n", __func__,__LINE__,is_on);
	sprintf(str, "set PW auto => %d", is_on);
	eggm_log(str);
}

// main task
void *pw_mgr_task()
{
	int ret;
	struct termios newtio;
	int i;
	int fd;
	int tmp_len;
	char sendBuff[EGGM_SR_MSG_SIZE];
	char readBuff[EGGM_SR_MSG_SIZE];
	struct eggm_pw_data_t pw_data;
	struct eggm_pw_data_t pw_data_by_machine;
	struct eggm_sr_msg_t eggm_msg;
	fd=open("/dev/ttyUSB0", O_RDWR | O_NOCTTY );  // 컨트롤 c 로 취소안되게 하기 | O_NOCTTY
	//	assert(fd != -1);
	if(fd == -1)
	{
		printf(" open error!!\n");
		return -1;
	}
	else
	{
		printf("usb serial open OK!!, fd=%d\n",fd);
	}

	newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
	//newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
	newtio.c_iflag    = IGNPAR | ICRNL;
	newtio.c_oflag = 0;
	newtio.c_lflag = ~(ICANON | ECHO | ECHOE | ISIG);

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	//	pthread_t tid;
	//	pthread_create(&tid, NULL, &do_run, NULL);
	int cnt = 0;
	tcflush(fd,TCIOFLUSH);

	while(1)
	{
		memset(&pw_data,0,sizeof(struct eggm_pw_data_t));
		memset(&pw_data_by_machine,0,sizeof(struct eggm_pw_data_t));
		memset(&eggm_msg,0,EGGM_SR_MSG_SIZE);
		memset(sendBuff,0,EGGM_SR_MSG_SIZE);
		memset(readBuff,0,EGGM_SR_MSG_SIZE);

		//01. get pw_data
		get_pw_data(&pw_data);

		//02. recv data
		char tempBuff;
		tmp_len = read(fd, &tempBuff, 1);
		if( tmp_len !=1 )
		{
			//no data skip
			//printf(" read length err = %d\n", tmp_len);
			usleep(500000);
			continue;
		}

		if( tempBuff != 0xC3 )
		{
			printf(" first char = 0x%02X, flush buff\n",tempBuff);
			tcflush(fd,TCIOFLUSH);
			usleep(100000);
			continue;
		}

		char tempBuff2;
		tmp_len = 0;
		tmp_len = read(fd, &tempBuff2, 1);
		if( tmp_len !=1 )
		{
			printf(" read2 length err = %d\n", tmp_len);
			usleep(100000);
			continue;
		}

		if( tempBuff2 != 0x99 )
		{
			printf(" first char = 0x%02X, flush buff\n",tempBuff2);
			tcflush(fd,TCIOFLUSH);
			usleep(100000);
			continue;
		}

		//printf("recv-serial\n");
		//printf("pass check firt 2byte =  0x%02X %02X\n",tempBuff, tempBuff2);
		tmp_len = read(fd, readBuff, EGGM_SR_MSG_SIZE-2);

		//printf("read length = %d\n",tmp_len);
		if(g_pw_machine_state == 0)
			g_pw_machine_state = 1;

		memset(&eggm_msg,0,EGGM_SR_MSG_SIZE);
		eggm_msg.key1 = tempBuff;
		eggm_msg.key2 = tempBuff2;
		for(i=0;i<EGGM_SR_MSG_DATA_SIZE; i++)
		{
			eggm_msg.data[i] = readBuff[i];
		}
		eggm_msg.checksum[0] = readBuff[6];
		eggm_msg.checksum[1] = readBuff[7];
		print_eggm_sr_msg(1, eggm_msg);

		//03. checksum confirm
		int checksumInt = 0;
		checksumInt = tempBuff;
		checksumInt += tempBuff2;
		for(i=0;i<EGGM_SR_MSG_DATA_SIZE; i++)
		{
			checksumInt += readBuff[i];
		}
		char cmpChecksum[2];
		memset(cmpChecksum, 0, 2);
		cmpChecksum[0] = ( checksumInt >> 8 ) & 0xFF;
		cmpChecksum[1] =  checksumInt & 0xFF;


		//if( ( eggm_msg.checksum[0] != cmpChecksum[0] )  
		//  ||  ( eggm_msg.checksum[1] != cmpChecksum[1] ) )
		if( eggm_msg.checksum[1] != cmpChecksum[1] )
		{
			printf("checksum err!! = 0x%02X %02X : 0x%02X %02X (0x%08X) (client:host)\n",
					eggm_msg.checksum[0],
					eggm_msg.checksum[1],
					cmpChecksum[0],
					cmpChecksum[1],
					checksumInt);
			tcflush(fd,TCIOFLUSH);
			usleep(200000);
			continue;
		}

		//04. change data (only, real, physical)
		memcpy(&pw_data_by_machine, eggm_msg.data, sizeof(EGGM_SR_MSG_DATA_SIZE));
		if( pw_data.state_real != pw_data_by_machine.state_real)
		{
			pw_data.state_real = pw_data_by_machine.state_real;
			pw_data.state_auto = pw_data.state_real;
			set_pw_data(&pw_data);
		}
		if( pw_data.state_physical != pw_data_by_machine.state_physical)
		{
			pw_data.state_physical = pw_data_by_machine.state_physical;
			set_pw_data(&pw_data);
		}

		//05. send data
		//eggm_msg.key1 = 0xC2; // debug check header!!
		eggm_msg.key1 = 0xC3;
		eggm_msg.key2 = 0x99;
		memcpy(eggm_msg.data, &pw_data, sizeof(struct eggm_pw_data_t));
		make_eggm_sr_checksum( &eggm_msg);
		memcpy(sendBuff, &eggm_msg, EGGM_SR_MSG_SIZE);
		write(fd, sendBuff, EGGM_SR_MSG_SIZE);
		//  printf("write-serial\n");
		print_eggm_sr_msg(0,eggm_msg);

		usleep(100000);


	}
	return 0;
}
