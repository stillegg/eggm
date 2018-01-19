#ifndef __EGGM_H__
#define __EGGM_H__
#define EGGM_MSG_SIZE  50 
#define EGGM_MSG_HEADER_SIZE  4
#define EGGM_MSG_CHECKSUM_SIZE 4
#define EGGM_MSG_DATA_SIZE  (EGGM_MSG_SIZE - ( EGGM_MSG_HEADER_SIZE +  EGGM_MSG_CHECKSUM_SIZE) )


#define EGGM_PAD_MSG_SIZE 10000
#define EGGM_PAD_MSG_HEADER_SIZE  5
#define EGGM_PAD_MSG_CHECKSUM_SIZE 4
#define EGGM_PAD_MSG_DATA_SIZE  \
		(EGGM_PAD_MSG_SIZE - ( EGGM_PAD_MSG_HEADER_SIZE +  EGGM_PAD_MSG_CHECKSUM_SIZE) )

#define EGGM_SR_MSG_SIZE  10 
#define EGGM_SR_MSG_HEADER_SIZE 2 
#define EGGM_SR_MSG_CHECKSUM_SIZE 2
#define EGGM_SR_MSG_DATA_SIZE  (EGGM_SR_MSG_SIZE - (EGGM_SR_MSG_HEADER_SIZE + EGGM_SR_MSG_CHECKSUM_SIZE) )


#define MAX_MUSIC_LIST          20
#define MAX_MUSIC_FILE_IN_LIST  20
#define MAX_NAME_LEN            100

#define MAX_PATH_LEN			100

#define EGGM_SERV_ID	0x01

#define EGGM_PW_ID	0x11
#define EGGM_AC_ID	0x12

#define EGGM_PAD_ID	0x21
#define EGGM_RMT_ID	0x51




//#define DEBUG_PW  
#define DEBUG_AC
#define DEBUG_SERV
#define DEBUG_PAD
#define DEBUG_REMODE

#define MUSIC_DIR "/home/pi/eggm/music"

//special, remote has many id
//example)   0x51 => machine: remote,  id 1 
//	    10ea machine can connect!!??
struct eggm_msg_t
{
    char key1 ; //0xX3   // first bit 0:get, 0xC:set
    char key2 ; //0x99
    char src_id;
    char dst_id;   
    char data[EGGM_MSG_DATA_SIZE]; //20 - 5
    char checksum[EGGM_MSG_CHECKSUM_SIZE];
};


struct eggm_pad_msg_t
{
	char key1 ;
	char key2 ;
    char src_id ;
	char dst_id ;
	char type ; // 0x11 system, 0x22 music list
    char data[EGGM_PAD_MSG_DATA_SIZE]; 
    char checksum[EGGM_PAD_MSG_CHECKSUM_SIZE];
};


struct eggm_sr_msg_t
{
    char key1 ; //0xX3   // first bit 0:get, 0xC:set
    char key2 ; //0x99
    char data[EGGM_SR_MSG_DATA_SIZE]; //20 - 5
    char checksum[EGGM_SR_MSG_CHECKSUM_SIZE];
};
struct eggm_pw_data_t
{
    char state_physical;// physical button, on:1, off:0 // do not chage by server
    char state_auto;	//  set by server on:1, off:0, FF => set port
    char state_real;	//  real power state => folow last commnd
    char state_port;	// check bit  | 0 1 2 3 4 5 6 7 8 |, on:1, off:0
};

struct eggm_ac_data_t
{
    char state;  //0:off, 1:on
    char front_temp1;
    char front_temp2;
    char room_temp1;
    char room_temp2;
};

struct eggm_music_t
{
    char run_state; //0:off, 1:run
    char volume; //0~100 %
	char listname[MAX_NAME_LEN];
    char filename[MAX_NAME_LEN]; 
};

struct eggm_music_list_t
{
	char is_set;
	char filecnt;
	char listname[MAX_NAME_LEN];
	char filename[MAX_MUSIC_FILE_IN_LIST][MAX_NAME_LEN];
};

struct eggm_schedule_data_t
{
	char is_enable;
	char start_hour;
	char start_min;
	char end_hour;
	char end_min;
};

struct eggm_system_t
{
	struct eggm_pw_data_t pw;
	struct eggm_ac_data_t ac;
	struct eggm_music_t cur_music;
	struct eggm_schedule_data_t week_sch[7];
	struct eggm_schedule_data_t cur_sch;
};

#endif
