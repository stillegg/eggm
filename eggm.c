#include <stdio.h>


//socket
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//serial
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "eggm.h"
#include "eggm_music.h"
#include "eggm_pw.h"
#include "eggm_pad.h"
#include "eggm_schedule.h"
//#include <termio.h>


// global data
int g_ac_machine_state = 0;

struct eggm_ac_data_t g_eggm_ac_data;


//struct eggm_schedule_data_t g_eggm_week_schedule[7]; //0 = sunday, 7 = thursday
//struct eggm_schedule_data_t g_eggm_cur_schedule;

// lock!!
pthread_mutex_t ac_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t schedule_mutex = PTHREAD_MUTEX_INITIALIZER;


void ac_data_lock() { pthread_mutex_lock(&ac_mutex); }
void ac_data_unlock() { pthread_mutex_unlock(&ac_mutex); }
//void schedule_data_lock() { pthread_mutex_lock(&schedule_mutex); }
//void schedule_data_unlock() { pthread_mutex_unlock(&schedule_mutex); }

// int function
void init_eggm_data()
{
    memset(&g_eggm_ac_data,0, sizeof(struct eggm_ac_data_t));
}


int get_ac_data(struct eggm_ac_data_t *ac_data)
{
    int ret = -1;
    ac_data_lock();
    if( g_ac_machine_state == 1 )
    {
	memcpy(ac_data, &g_eggm_ac_data, sizeof(struct eggm_ac_data_t));
	ret = 1;
    }
    ac_data_unlock();
    return ret;
}
void set_ac_data(struct eggm_ac_data_t *ac_data)
{
    ac_data_lock();
    g_eggm_ac_data.state = ac_data->state ;
    g_eggm_ac_data.front_temp1 = ac_data->front_temp1 ;
    g_eggm_ac_data.front_temp2 = ac_data->front_temp2 ;
    g_eggm_ac_data.room_temp1 = ac_data->room_temp1 ;
    g_eggm_ac_data.room_temp2 = ac_data->room_temp2 ;
    ac_data_unlock();
}
#if 0
void get_schedule_data_by_wday(struct eggm_schedule_data_t *s_data, int wday)
{
	schedule_data_lock();
  	s_data->is_enable	=  g_eggm_week_schedule[wday].is_enable;
  	s_data->start_hour 	=  g_eggm_week_schedule[wday].start_hour;
  	s_data->start_min	=  g_eggm_week_schedule[wday].start_min;
  	s_data->end_hour 	=  g_eggm_week_schedule[wday].end_hour;
  	s_data->end_min 	=  g_eggm_week_schedule[wday].end_min;
	schedule_data_unlock();
}

void get_cur_schedule(struct eggm_schedule_data_t *s_data)
{
	schedule_data_lock();
  	s_data->is_enable	=  g_eggm_cur_schedule.is_enable;
  	s_data->start_hour 	=  g_eggm_cur_schedule.start_hour;
  	s_data->start_min	=  g_eggm_cur_schedule.start_min;
  	s_data->end_hour 	=  g_eggm_cur_schedule.end_hour;
  	s_data->end_min 	=  g_eggm_cur_schedule.end_min;
	schedule_data_unlock();
}

void set_schedule_data_by_wday(struct eggm_schedule_data_t *s_data, int wday)
{
	schedule_data_lock();
	g_eggm_week_schedule[wday].is_enable	= s_data->is_enable;
  	g_eggm_week_schedule[wday].start_hour 	= s_data->start_hour;
  	g_eggm_week_schedule[wday].start_min	= s_data->start_min;
  	g_eggm_week_schedule[wday].end_hour		= s_data->end_hour;
  	g_eggm_week_schedule[wday].end_min		= s_data->end_min;	
	schedule_data_unlock();
}
#endif

void *ac_mgr_task()
{
	while(1)
	{
		sleep(1);
	}
	return NULL;
}

void *remote_mgr_task()
{
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr;

	char sendBuff[EGGM_MSG_SIZE];
	char recvBuff[EGGM_MSG_SIZE];
	struct eggm_msg_t eggm_msg;
	struct eggm_pw_data_t pw_data;
	time_t ticks;

	memset(&eggm_msg, 0, sizeof(struct eggm_msg_t));
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(11002);
	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	listen(listenfd, 10);

	while(1)
	{
		int n = 0;
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

		if(connfd>0)
		{
			if( read(connfd, recvBuff, EGGM_MSG_SIZE) < 0)
			{
				printf("read error\n");
			}
			else
			{
				int is_auto_on = 0;
				memset(&eggm_msg, 0, EGGM_MSG_SIZE);
				memcpy(&eggm_msg, recvBuff, EGGM_MSG_SIZE);
				print_eggm_msg(eggm_msg);
				//process msg
				// check msg
				if(eggm_msg.key1 == 0xC0)
				{
					is_auto_on = eggm_msg.data[0];

					// 1. get pw data
					get_pw_data(&pw_data);

					// 2. change pw data
					pw_data.state_auto = is_auto_on;

					// 3. write pw data
					set_pw_data(&pw_data);
					memcpy(sendBuff, &eggm_msg, EGGM_MSG_SIZE);
					write(connfd, sendBuff, strlen(sendBuff));
					printf("write : %s\n", sendBuff);

				}
				else if(eggm_msg.key2 == 0x03)
				{
					// 1. get pw data
					get_pw_data(&pw_data);
				
					memcpy(sendBuff, &eggm_msg, EGGM_MSG_SIZE);
					if( pw_data.state_real == 1)
						eggm_msg.data[0] = 1;
				
					write(connfd, sendBuff, strlen(sendBuff));
					printf("write : %s\n", sendBuff);
				}

				close(connfd);
				sleep(1);
			}
		}
	}
	return NULL;
}

#define MON_MSG_MAX_LINE_COUNT	30
#define MON_MSG_MAX_LINE_SIZE	1000
#define MON_MSG_MAX_SIZE 		(MON_MSG_MAX_LINE_COUNT * MON_MSG_MAX_LINE_SIZE)
void *monitoring_task()
{
	char send_buff[MON_MSG_MAX_SIZE];

	// init socket
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(11003);
	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	listen(listenfd, 10);

	while(1)
	{

		int n = 0;
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

		if(connfd>0)
		{
			//get data
			write(connfd, send_buff, MON_MSG_MAX_SIZE);
#if 0
			//view state
			//	system("clear");
			printf(" ========================================\n");
			printf(" | RUN MODE = SERVER			|\n"); 
			printf(" | -------------------------------------|\n");
			printf(" | key info				|\n");
			printf(" |  : q -> quit				|\n");
			printf(" | -------------------------------------|\n");
			printf(" | stat:				|\n");
			printf(" | -------------------------------------|\n");
			printf(" | [ power ] %07s,                   |\n",pw_machine_state? "ONLINE":"OFFLINE"); 
			printf(" |           at:%03s( ps:%03s, rl:%03s)     |\n", pw_data.state_auto? "ON":"OFF", pw_data.state_physical? "ON":"OFF", pw_data.state_real? "ON":"OFF");
			printf(" |           port: 0x%02X	         |\n", pw_data.state_port);
			printf(" | [ A/C   ] %s, stat:%s, temp:%s(%s) |\n", A_C_machine_state? "ONLINE":"OFFLINE", A_C_state? "ON":"OFF", A_C_room_temp, A_C_front_temp);
			printf(" | [ pad   ] %s                         |\n", ctlpad_machine_state? "ONLINE":"OFFLINE");
			printf(" | [ music ] %s, name:%s, volume:%s     |\n", music_run_state? "RUN":"STOP", music_run_name, music_volume);
			printf(" | [ time  ] cur:%s, off time:after %s  |\n", "0","0");
			printf(" |           MON : [%03s] %02d:%02d ~ %02d:%02d              |\n" ,w_sch[1].is_enable ? "EN":"DIS", w_sch[1].start_hour,w_sch[1].start_min, w_sch[1].end_hour, w_sch[1].end_min);
			printf(" |           TUE : [%03s] %02d:%02d ~ %02d:%02d              |\n" ,w_sch[2].is_enable ? "EN":"DIS", w_sch[2].start_hour,w_sch[2].start_min, w_sch[2].end_hour, w_sch[2].end_min);
			printf(" |           WED : [%03s] %02d:%02d ~ %02d:%02d              |\n" ,w_sch[3].is_enable ? "EN":"DIS", w_sch[3].start_hour,w_sch[3].start_min, w_sch[3].end_hour, w_sch[3].end_min);
			printf(" |           THR : [%03s] %02d:%02d ~ %02d:%02d              |\n" ,w_sch[4].is_enable ? "EN":"DIS", w_sch[4].start_hour,w_sch[4].start_min, w_sch[4].end_hour, w_sch[4].end_min);
			printf(" |           TRI : [%03s] %02d:%02d ~ %02d:%02d              |\n" ,w_sch[5].is_enable ? "EN":"DIS", w_sch[5].start_hour,w_sch[5].start_min, w_sch[5].end_hour, w_sch[5].end_min);
			printf(" |           THE : [%03s] %02d:%02d ~ %02d:%02d              |\n" ,w_sch[6].is_enable ? "EN":"DIS", w_sch[6].start_hour,w_sch[6].start_min, w_sch[6].end_hour, w_sch[6].end_min);
			printf(" |           SUN : [%03s] %02d:%02d ~ %02d:%02d              |\n" ,w_sch[0].is_enable ? "EN":"DIS", w_sch[0].start_hour,w_sch[0].start_min, w_sch[0].end_hour, w_sch[0].end_min);
			printf(" ========================================\n");
			print_cur_time();
			sleep(1);
#else
#endif

		}
	}
}

void *eggm_client_thread()
{
	printf("Start %s!\n", __func__); 
	//eggm_client_run();
	return NULL;
}

int main(int argc, char *argv[])
{
	int err;
	int run = 1;
	pthread_t pw_mgr_tid = - 1;
	pthread_t ac_mgr_tid = - 1;
	pthread_t pad_mgr_tid = - 1;
	pthread_t remote_mgr_tid = - 1;
	pthread_t music_mgr_tid = - 1;
	pthread_t schedule_mgr_tid = - 1;
	pthread_t monitoring_tid = - 1;
	
	eggm_log("");
	eggm_log("----------------------");
	eggm_log("--- run start eggm! --");
	eggm_log("----------------------");

	
	

	err = pthread_create(&pw_mgr_tid, NULL, &pw_mgr_task, NULL);
	err = pthread_create(&ac_mgr_tid, NULL, &ac_mgr_task, NULL);
	err = pthread_create(&pad_mgr_tid, NULL, &pad_mgr_task, NULL);
	err = pthread_create(&remote_mgr_tid, NULL, &remote_mgr_task, NULL);
	err = pthread_create(&music_mgr_tid, NULL, &music_mgr_task, NULL);
	err = pthread_create(&schedule_mgr_tid, NULL, &schedule_mgr_task, NULL);
	err = pthread_create(&monitoring_tid, NULL, &monitoring_task, NULL);

	while(run)
	{
#if 0
		char ch;
		count++;
		tcsetattr(0, TCSAFLUSH, &new);
		ch = getchar();
		tcsetattr(0, TCSAFLUSH, &old);

		printf("get key = %c\n", ch);

		system("reset");


		switch(ch)
		{
			case 'q' :
				run = 0;
				//		eggm_serv_close();
				sleep(1);

				break;
		}

#endif
		sleep(1);

	}
	return 1;
}
