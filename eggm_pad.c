#include<stdio.h>
#include<string.h>
//socket
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "eggm.h"


#define EGGM_PAD_MSG_TYPE_SYSTEM 0x11
#define EGGM_PAD_MSG_TYPE_MUSIC 0x22

#define EGGM_PAD_MSG_SET_PW 0x51
#define EGGM_PAD_MSG_SET_AC 0x52
#define EGGM_PAD_MSG_SET_MUSIC 0x53
#define EGGM_PAD_MSG_SET_SCHEDULE 0x54
#define EGGM_PAD_MSG_SET_MUSIC 0x55


enum eggm_pad_msg_type
{
	EGGM_PAD_GET_SYSTEM,
	EGGM_PAD_GET_MUSIC_LIST,
	EGGM_PAD_SET_PW,
	EGGM_PAD_SET_CUR_MUSIC,
	EGGM_PAD_SET_CUR_MUSIC_PLAY_BEFORE,
	EGGM_PAD_SET_CUR_MUSIC_PLAY_NEXT,
	EGGM_PAD_SET_CUR_SCH_START,
	EGGM_PAD_SET_CUR_SCH_END,
	EGGM_PAD_SET_MUSIC_LIST
};

void print_pad_msg(int is_send, struct eggm_pad_msg_t  pad_msg)
{
	int i = 0;
	printf("===========================\n");
	printf(" [%s] KEY:0x%02X%02X\n",is_send? "SEND":"RECV", pad_msg.key1, pad_msg.key2);
	printf("===========================");
	for(i=0;i<500;i++)
	{
		if(i%40 == 0)
			printf("\n");
	    printf(" %x", pad_msg.data[i]);
		
	}
	printf("===========================\n");
}


int make_system_info_packet(struct eggm_pad_msg_t *pad_msg)
{
	int i;
	struct eggm_system_t sys_data;
	memset( &sys_data, 0, sizeof(struct eggm_system_t));
	get_pw_data( &sys_data.pw);
	get_ac_data( &sys_data.ac);
	get_cur_music( &sys_data.cur_music);
#if 0
	//just debug
	sys_data.pw.state_physical = 7;
	sys_data.pw.state_auto = 3;
	sys_data.pw.state_real = 5;
	sys_data.ac.state = 9;
	sys_data.ac.front_temp1 = 10;
	sys_data.ac.room_temp2 = 55;
	sys_data.cur_music.run_state = 7;
	sys_data.cur_music.volume = 24;
	sprintf( sys_data.cur_music.listname, "listname it is good?");
	sprintf( sys_data.cur_music.filename, "filename it is SONG!@@@@");
#endif
	for(i=0;i<7;i++)
	{
		get_schedule_data_by_wday( &sys_data.week_sch[i],i);
	}
	get_cur_schedule( &sys_data.cur_sch);
    
	pad_msg->key1 = 0x03;
	pad_msg->key2 = 0x99;
	pad_msg->src_id = EGGM_SERV_ID;
	pad_msg->dst_id = EGGM_PAD_ID;
	pad_msg->type = EGGM_PAD_GET_SYSTEM;
	memcpy( pad_msg->data, &sys_data, sizeof(struct eggm_system_t));
}

int make_music_list_packet(struct eggm_pad_msg_t *pad_msg)
{
	int music_list_cnt = get_music_list_cnt();
	int i;

	pad_msg->key1 = 0x03;
	pad_msg->key2 = 0x99;
	pad_msg->src_id = EGGM_SERV_ID;
	pad_msg->dst_id = EGGM_PAD_ID;
	pad_msg->type = EGGM_PAD_GET_MUSIC_LIST;
	pad_msg->data[0] = (char)music_list_cnt;
	
	for( i = 0; i < music_list_cnt; i++)
	{
		struct eggm_music_list_t music_list;
		get_music_list_by_num(i, &music_list);
		memcpy( &pad_msg->data[i*100 + 1], music_list.listname, MAX_NAME_LEN);
	}
}

void eggm_pad_set_pw_auto(int is_on)
{
	struct eggm_pw_data_t pw;
	get_pw_data(&pw);
	if( pw.state_auto != is_on)
		pw.state_auto = is_on;
	set_pw_data(&pw);
}

void eggm_pad_set_cur_music_play(int is_run)
{
	set_cur_music_run_state(is_run);
}

void eggm_pad_set_music_list_change(int num)
{
	change_cur_music_list(num);
}


void eggm_pad_set_cur_sch_start(char hour, char min)
{
	struct eggm_schedule_data_t cur_sch;
	get_cur_schedule( &cur_sch);
	cur_sch.is_enable = 1;
	cur_sch.start_hour = hour;
	cur_sch.start_min = min;
	set_static_cur_start_schedule( &cur_sch);
}

void eggm_pad_set_cur_sch_end(char hour, char min)
{
	struct eggm_schedule_data_t cur_sch;
	get_cur_schedule( &cur_sch);
	cur_sch.is_enable = 1;
	cur_sch.end_hour = hour;
	cur_sch.end_min = min;
	set_static_cur_end_schedule( &cur_sch);
}


void *pad_mgr_task()
{
    int listenfd = 0, connfd = 0;
    int recv_len, send_len;
    struct sockaddr_in serv_addr;
	int ret = 0;
    char sendBuff[EGGM_PAD_MSG_SIZE];
    char recvBuff[EGGM_PAD_MSG_SIZE];
    struct eggm_pad_msg_t send_pad_msg;
    struct eggm_pad_msg_t recv_pad_msg;
	struct eggm_system_t sys_data;


    memset(&send_pad_msg, 0, sizeof(struct eggm_pad_msg_t));
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0)
	{
		eggm_log(" eggm_pad socket open error!! (port:11001), please restart eggm");
		return;
	}
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(11001);
	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if( ret < 0 )
	{
		eggm_log("eggm_pad: bind err=%d\n",ret);
		return;
	}
	ret = listen(listenfd, 10);
	if( ret < 0 )
	{
		eggm_log("eggm_pad: listen err = %d\n",ret);
		return;
	}

	while(1)
    {
        int n = 0;
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

        if(connfd>0)
        {
            recv_len = read(connfd, recvBuff, EGGM_PAD_MSG_SIZE);
			if (recv_len> 0)
            {
				memset( &sys_data, 0, sizeof(struct eggm_system_t));
				memcpy(&recv_pad_msg, recvBuff, EGGM_PAD_MSG_SIZE);
				memcpy( &sys_data, recv_pad_msg.data, sizeof(struct eggm_system_t));
	//			print_pad_msg( 0, recv_pad_msg );
	
		//		printf("recv_pad_msg.type = %d\n", recv_pad_msg.type);
                //01 header type 을 확인하여 원하는 메세지를 선택 한다
				switch(recv_pad_msg.type)
				{
					case EGGM_PAD_GET_SYSTEM:
						make_system_info_packet(&send_pad_msg); 	
						//				print_pad_msg( 1, send_pad_msg );
						break;
					case EGGM_PAD_GET_MUSIC_LIST:
						make_music_list_packet(&send_pad_msg);
						break;
					case EGGM_PAD_SET_PW:
						eggm_pad_set_pw_auto(recv_pad_msg.data[0]);
						make_system_info_packet(&send_pad_msg); 	
						break;
					case EGGM_PAD_SET_CUR_MUSIC:
						eggm_pad_set_cur_music_play(recv_pad_msg.data[0]);
						make_system_info_packet(&send_pad_msg); 	
						break;
					case EGGM_PAD_SET_CUR_MUSIC_PLAY_BEFORE:
						play_before_music();
						make_system_info_packet(&send_pad_msg); 	
						break;
					case EGGM_PAD_SET_CUR_MUSIC_PLAY_NEXT:
						play_next_music();
						make_system_info_packet(&send_pad_msg); 	
						break;
					case EGGM_PAD_SET_CUR_SCH_START:
		//				printf("change start time\n");
						eggm_pad_set_cur_sch_start(recv_pad_msg.data[0], recv_pad_msg.data[1]);
						make_system_info_packet(&send_pad_msg); 	
						break;
					case EGGM_PAD_SET_CUR_SCH_END:
						eggm_pad_set_cur_sch_end(recv_pad_msg.data[0], recv_pad_msg.data[1]);
						make_system_info_packet(&send_pad_msg); 	
						break;
					case EGGM_PAD_SET_MUSIC_LIST:
						eggm_pad_set_music_list_change(recv_pad_msg.data[0]);
						make_system_info_packet(&send_pad_msg); 	
						break;
				}
						
				//02 response write by type
				memcpy(sendBuff, &send_pad_msg, EGGM_PAD_MSG_SIZE);
				write(connfd, sendBuff, sizeof(sendBuff));
#if 0
				if( recv_pad_msg.type = EGGM_PAD_MSG_TYPE_SYSTEM)
				{
					make_system_info_packet(&send_pad_msg); 	
					//				print_pad_msg( 1, send_pad_msg );
					//02 response write by type
					memcpy(sendBuff, &send_pad_msg, EGGM_PAD_MSG_SIZE);
					write(connfd, sendBuff, sizeof(sendBuff));
				}
				else
				{
				}
#endif
				close(connfd);
            }
        }
    }
    return NULL;
}

