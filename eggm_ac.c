#include<stdio.h>
#include<pthread.h>

#include"eggm.h"
#include"eggm_ac.h"


struct eggm_ac_data_t g_eggm_ac_data;
int g_ac_machine_state;


pthread_mutex_t ac_mutex = PTHREAD_MUTEX_INITIALIZER;



void ac_data_lock() { pthread_mutex_lock(&ac_mutex); }
void ac_data_unlock() { pthread_mutex_unlock(&ac_mutex); }

void init_eggm_ac_data()
{
	g_ac_machine_state = 0;
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

void *ac_mgr_task()
{
	while(1)
	{
		sleep(1);
	}
	return NULL;
}

