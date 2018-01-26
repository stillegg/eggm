#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "eggm.h"



struct eggm_schedule_data_t g_eggm_week_schedule[7]; //0 = sunday, 7 = thursday
struct eggm_schedule_data_t g_eggm_cur_schedule;
int g_is_static_start_sch_run=0 ;
int g_is_static_end_sch_run=0 ;


pthread_mutex_t schedule_mutex = PTHREAD_MUTEX_INITIALIZER;



void schedule_data_lock() { pthread_mutex_lock(&schedule_mutex); }
void schedule_data_unlock() { pthread_mutex_unlock(&schedule_mutex); }

void get_schedule_data_by_wday(struct eggm_schedule_data_t *s_data, int wday)
{
	schedule_data_lock();
	s_data->is_enable   =  g_eggm_week_schedule[wday].is_enable; 
	s_data->start_hour  =  g_eggm_week_schedule[wday].start_hour; 
	s_data->start_min   =  g_eggm_week_schedule[wday].start_min; 
	s_data->end_min     =  g_eggm_week_schedule[wday].end_min;
	s_data->end_hour    =  g_eggm_week_schedule[wday].end_hour;
	// is_data->end_min     =  g_eggm_week_schedule[wday].end_min;
	schedule_data_unlock();
}

void get_cur_schedule(struct eggm_schedule_data_t *s_data)
{
	schedule_data_lock();
	s_data->is_enable   =  g_eggm_cur_schedule.is_enable;
	s_data->start_hour  =  g_eggm_cur_schedule.start_hour;
	s_data->start_min   =  g_eggm_cur_schedule.start_min;
	s_data->end_hour    =  g_eggm_cur_schedule.end_hour;
	s_data->end_min     =  g_eggm_cur_schedule.end_min;
	schedule_data_unlock();
}

void set_cur_schedule(struct eggm_schedule_data_t *s_data)
{
	schedule_data_lock();
	g_eggm_cur_schedule.is_enable	= s_data->is_enable;
	g_eggm_cur_schedule.start_hour	= s_data->start_hour;
	g_eggm_cur_schedule.start_min	= s_data->start_min;
	g_eggm_cur_schedule.end_hour	= s_data->end_hour;
	g_eggm_cur_schedule.end_min		= s_data->end_min;
	schedule_data_unlock();

	eggm_log("schedule set start %d/%d end %d/%d\n",
		g_eggm_cur_schedule.start_hour,
		g_eggm_cur_schedule.start_min,
		g_eggm_cur_schedule.end_hour,
		g_eggm_cur_schedule.end_min);
}

void set_static_cur_start_schedule(struct eggm_schedule_data_t *s_data)
{
	schedule_data_lock();
	g_eggm_cur_schedule.is_enable	= s_data->is_enable;
	g_eggm_cur_schedule.start_hour	= s_data->start_hour;
	g_eggm_cur_schedule.start_min	= s_data->start_min;
	g_is_static_start_sch_run = 1;
	schedule_data_unlock();
	
	
	eggm_log("%s/%d  => %d:%d", __func__,__LINE__, 
		s_data->start_hour, s_data->start_min);
}

void set_static_cur_end_schedule(struct eggm_schedule_data_t *s_data)
{
	schedule_data_lock();
	g_eggm_cur_schedule.is_enable	= s_data->is_enable;
	g_eggm_cur_schedule.end_hour	= s_data->end_hour;
	g_eggm_cur_schedule.end_min		= s_data->end_min;
	g_is_static_end_sch_run = 1;
	schedule_data_unlock();

	eggm_log("%s/%d  => %d:%d", __func__,__LINE__, 
		s_data->end_hour, s_data->end_min);
}

int get_is_static_cur_start_schedule()
{
	int val;
	schedule_data_lock();
	val = g_is_static_start_sch_run;
	schedule_data_unlock();
	return val;
}

int get_is_static_cur_end_schedule()
{
	int val;
	schedule_data_lock();
	val = g_is_static_end_sch_run;
	schedule_data_unlock();
	return val;
}

void clear_static_cur_start_schedule()
{
	schedule_data_lock();
	g_is_static_start_sch_run = 0;
	schedule_data_unlock();
}

void clear_static_cur_end_schedule()
{
	schedule_data_lock();
	g_is_static_end_sch_run = 0;
	schedule_data_unlock();
}
void set_schedule_data_by_wday(struct eggm_schedule_data_t *s_data, int wday)
{
	schedule_data_lock();
	g_eggm_week_schedule[wday].is_enable    = s_data->is_enable;
	g_eggm_week_schedule[wday].start_hour   = s_data->start_hour;
	g_eggm_week_schedule[wday].start_min    = s_data->start_min;
	g_eggm_week_schedule[wday].end_hour     = s_data->end_hour;
	g_eggm_week_schedule[wday].end_min      = s_data->end_min;
	schedule_data_unlock();
}

void update_cur_start_sch(int wday)
{
	struct eggm_schedule_data_t wsch;
	struct eggm_schedule_data_t cur_sch;
	get_schedule_data_by_wday( &wsch, wday);
	get_cur_schedule( &cur_sch);
	cur_sch.start_hour = wsch.start_hour;
	cur_sch.start_min = wsch.start_min;
	set_cur_schedule( &cur_sch);

	eggm_log( "%s/%d cur_sch_start => %d:%d (%d day)", __func__,__LINE__, 
		cur_sch.start_hour, cur_sch.start_min, wday);
}

void update_cur_end_sch(int wday)
{
	struct eggm_schedule_data_t wsch;
	struct eggm_schedule_data_t cur_sch;
	get_schedule_data_by_wday( &wsch, wday);
	get_cur_schedule( &cur_sch);
	cur_sch.end_hour = wsch.end_hour;
	cur_sch.end_min = wsch.end_min;
	set_cur_schedule( &cur_sch);
	eggm_log("%s/%d cur_sch_end => %d:%d (%d day)", __func__,__LINE__, 
		cur_sch.end_hour, cur_sch.end_min, wday);
}

void *schedule_mgr_task()
{
	struct tm tm_now;
	int ret = 0;
	int last_update_hour = 0;
	int last_log_min = 0;
	int i;
	int cur_sch_day = -1;
	struct eggm_pw_data_t pw;

	struct eggm_schedule_data_t sch[7]; //0 = sunday, 7 = thursday
	// init
	// init time struct

	// 01. read config file
	ret = eggm_io_read_sch_conf(sch); 
	{
		eggm_log("read config file ========\n");
		for(i=0;i<7;i++)
		{
		#if 0
			eggm_log("[%d]-%03s sch start %d:%d, end %d:%d\n",
				i, sch[i].is_enable? "EN":"dis",
				sch[i].start_hour, sch[i].start_min,
				sch[i].end_hour, sch[i].end_min);
	   #endif
			set_schedule_data_by_wday(&sch[i],i);
		}
	}
	// 02. 
	eggm_log("\nschedule check\n");
	for(i=0;i<7;i++)
	{
		struct eggm_schedule_data_t cur_sch;	
		get_schedule_data_by_wday( &cur_sch, i);
		eggm_log("sch [%d:%s] start %d:%d, end %d:%d\n", i, cur_sch.is_enable? "enable":"disable",
				cur_sch.start_hour, cur_sch.start_min, cur_sch.end_hour, cur_sch.end_min);
	}

	while(1)
	{
		struct eggm_schedule_data_t cur_sch;
		
		//01. get time
		ret = get_cur_time(&tm_now);

		
		// get wday data (day change)
		if( cur_sch_day != tm_now.tm_wday)  
		{
			eggm_log("---- day chang!!!\n");
			if( (g_is_static_start_sch_run ==0) && (g_is_static_end_sch_run ==0) )
			{
				eggm_log("change sch[1] old ( %02d:%02d ~ %02d:%02d )",
					cur_sch.start_hour, cur_sch.start_min, cur_sch.end_hour, cur_sch.end_min);
				get_schedule_data_by_wday( &cur_sch, tm_now.tm_wday);
				cur_sch_day = tm_now.tm_wday;
				set_cur_schedule( &cur_sch);
				
				eggm_log("change sch[2] new ( %02d:%02d ~ %02d:%02d )",
					cur_sch.start_hour, cur_sch.start_min, cur_sch.end_hour, cur_sch.end_min);
			}
			else 
			{
				if( cur_sch.start_hour >= 24)
				{
					cur_sch.start_hour -= 24;
					//cur_sch_day = tm_now.tm_wday;
					set_cur_schedule( &cur_sch);
				}

				if( cur_sch.end_hour >=24 )
				{
					cur_sch.end_hour -= 24;
					//cur_sch_day = tm_now.tm_wday;
					set_cur_schedule( &cur_sch);
				}
			}
		}
		else
		{
			get_cur_schedule( &cur_sch);
		}
		
		//02. update time, per 1 hour
#if 1
		if(0)
#else
		if( ( tm_now.tm_hour > last_update_hour ) 
			|| (tm_now.tm_hour == 0 && last_update_hour == 23 ))
#endif
		{
			last_update_hour = tm_now.tm_hour;
			eggm_log("=== time update =============\n");
			system("echo before time : ");
			system("date");
			update_time();
			system("echo after time : ");
			system("date");
			eggm_log("==============================\n");
		}
#if 0
		//03. print time, per 1 min
		if( ( tm_now.tm_min > last_log_min) 
			|| ( tm_now.tm_min ==0 && last_log_min ==59))
		{
			last_log_min = tm_now.tm_min;
			printf("%04d,%02d,%02d(%d)-%02d:%02d:%02d\n",
				1900 + tm_now.tm_year,
				tm_now.tm_mon,
				tm_now.tm_mday,
				tm_now.tm_wday,
				tm_now.tm_hour,
				tm_now.tm_min,
				tm_now.tm_sec);
			printf("sch [%s] start %d:%d, end %d:%d\n", cur_sch.is_enable? "enable":"disable",
				cur_sch.start_hour, cur_sch.start_min, cur_sch.end_hour, cur_sch.end_min);
		}
#endif
		//03. diff time struct, and change on/off time val
		get_pw_data( &pw);

		// power state run check 
		if( pw.state_auto == 0 ) // is power state off  || music state off
		{
			if( cur_sch.start_hour == tm_now.tm_hour )
			{
				if( cur_sch.start_min == tm_now.tm_min )
				//if( cur_sch.start_min >= tm_now.tm_hour )
				{
					eggm_log("=========== auto on!!===========");
					// power state run check

					// power on
					set_pw_auto_state(1);

					// music run
					set_music_run_state(1);
				
					if( get_is_static_cur_start_schedule() )
					{
						clear_static_cur_start_schedule();
						update_cur_start_sch(tm_now.tm_wday);
					}
				}
			}
		}
		else  // is power state on || music state off
		{
			if( cur_sch.end_hour == tm_now.tm_hour )
			{
				if( cur_sch.end_min == tm_now.tm_min )
					//if( cur_sch.end_min >= tm_now.tm_hour )
				{
					eggm_log("=========== auto off!!===========");

					// power off
					set_pw_auto_state(0);

					// music off
					set_music_run_state(0);
					
					if( get_is_static_cur_end_schedule() )
					{
						clear_static_cur_end_schedule();		
						update_cur_end_sch(tm_now.tm_wday);
					}
				}
			}
		}

		usleep(500000);
	}
	return NULL;

}
