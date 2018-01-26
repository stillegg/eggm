#include <stdio.h>
#include <pthread.h>


#include "eggm_music.h"
#include "eggm_io.h"

struct eggm_music_t g_cur_music;
int g_music_list_cnt = 0;
struct eggm_music_list_t g_music_list[MAX_MUSIC_LIST];

int g_play_before = 0;
int g_play_next = 0;

pthread_mutex_t music_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t music_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t music_play_before_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t music_play_next_mutex = PTHREAD_MUTEX_INITIALIZER;



void music_lock() { pthread_mutex_lock(&music_mutex); }
void music_unlock() { pthread_mutex_unlock(&music_mutex); }
void music_list_lock() { pthread_mutex_lock(&music_list_mutex); }
void music_list_unlock() { pthread_mutex_unlock(&music_list_mutex); }


void music_play_before_lock() { pthread_mutex_lock(&music_play_before_mutex); }
void music_play_before_unlock() { pthread_mutex_unlock(&music_play_before_mutex); }
void music_play_next_lock() { pthread_mutex_lock(&music_play_next_mutex); }
void music_play_next_unlock() { pthread_mutex_unlock(&music_play_next_mutex); }

void init_music_data()
{
    memset(&g_cur_music,0, sizeof(struct eggm_music_t));
}

int get_cur_music(struct eggm_music_t *music)
{
    int ret = 1;
    music_lock();
    memset(music, 0, sizeof(struct eggm_music_t));
    memcpy(music, &g_cur_music, sizeof(struct eggm_music_t));
    music_unlock();
    return ret;
}

void set_cur_music(struct eggm_music_t *music)
{
    music_lock();
    g_cur_music.run_state = music->run_state ;
    g_cur_music.volume = music->volume ;
	memset(g_cur_music.listname, 0, MAX_NAME_LEN);
	memset(g_cur_music.filename, 0, MAX_NAME_LEN);
    memcpy(&g_cur_music.listname, &music->listname, MAX_NAME_LEN);
    memcpy(&g_cur_music.filename, &music->filename, MAX_NAME_LEN);
    music_unlock();
}

void set_cur_music_run_state(int is_run)
{
    music_lock();
    g_cur_music.run_state = is_run;
	//printf("%s set %d\n",__func__, is_run);
    music_unlock();
}

int get_music_list_by_num(int num, struct eggm_music_list_t *ml)
{
    music_list_lock();
    memcpy( ml, &g_music_list[num], sizeof( struct eggm_music_list_t));
    music_list_unlock();
}

int get_play_list_num_by_name(char* name)
{
	int i;
    music_list_lock();
	for( i=0; i< g_music_list_cnt; i++)
	{
#if 0
		printf("strcmp ( %s : %s ) = %d\n", name, g_music_list[i].listname, 
				strcmp( name, g_music_list[i].listname));
		printf("read:");
		print_string_to_hex( name);
		printf("comp:");
		print_string_to_hex( g_music_list[i].listname);
#endif
		if( strcmp( name, g_music_list[i].listname) ==0)
		{
    		music_list_unlock();
			return i;
		}
	}
    music_list_unlock();
	return -1;
}

int get_music_list_cnt()
{
	return g_music_list_cnt;
}

void set_music_run_state(int is_on)
{
    music_lock();
    g_cur_music.run_state = is_on;
    music_unlock();
}

void play_before_music()
{
	music_play_before_lock();	
	g_play_before = 1;
	music_play_before_unlock();	
}

void play_next_music()
{
	music_play_next_lock();	
	g_play_next = 1;
	music_play_next_unlock();	
}

void complate_play_before_music()
{
	music_play_before_lock();	
	g_play_before = 0;
	music_play_before_unlock();	
}

void complate_play_next_music()
{
	music_play_next_lock();	
	g_play_next = 0;
	music_play_next_unlock();	
}
int add_music_list(struct eggm_music_list_t ml)
{
    if(g_music_list_cnt >= MAX_MUSIC_LIST)
        return -1;
    music_list_lock();
    memcpy( &g_music_list[g_music_list_cnt], &ml, sizeof(struct eggm_music_list_t));
    g_music_list_cnt++;
    music_list_unlock();

    return 1;
}


int remove_music_list(struct eggm_music_list_t ml)
{   
    int i;
    int find_flag =  0;
    music_list_lock();
    for(i=0;i<(MAX_MUSIC_LIST-1);i++)
    {   
        if(find_flag == 0)
        {   
            if( strncmp( &g_music_list[i].listname, &ml.listname, sizeof(ml.listname)) == 0)
                find_flag =1;
        }
        
        if(find_flag == 1)
        {   
            memcpy( &g_music_list[i], &g_music_list[i+1], sizeof(struct eggm_music_list_t));
            memset( &g_music_list[i+1], 0, sizeof(struct eggm_music_list_t));
        }
    }
    music_list_unlock();
}

void print_music_list()
{
    int i,j;
    struct eggm_music_list_t tmp_ml;
        eggm_log("===============================================\n");
        eggm_log("  music list!\n");
        eggm_log("===============================================\n");
    for( i=0; i< g_music_list_cnt; i++)
    {
        memset( &tmp_ml, 0, sizeof(struct eggm_music_list_t));
        get_music_list_by_num(i, &tmp_ml);
        eggm_log("[%d] %s\n",i, tmp_ml.listname);
        for( j=0; j<tmp_ml.filecnt ;j++)
        {
            eggm_log("    [%d]. %s\n",j, tmp_ml.filename[j]);
        }
        eggm_log("------------------------------------------------\n");
    }
}

void sort_music_list()
{
	int i, j, len, cnt;
	char tmp[MAX_NAME_LEN];
	music_list_lock();
	for( cnt = 0; cnt < g_music_list_cnt; cnt++)
	{
		len = g_music_list[cnt].filecnt;
		for ( i = 0; i < len -1; i++)
		{
			for ( j = 0 ; j < len -1 -i; j++)
			{
				if ( strcmp(g_music_list[cnt].filename[j], g_music_list[cnt].filename[j+1]) > 0)
				{
					strcpy( tmp, g_music_list[cnt].filename[j]);
					strcpy( g_music_list[cnt].filename[j], g_music_list[cnt].filename[j+1]);
					strcpy( g_music_list[cnt].filename[j+1], tmp);
				} 
			}
		}	
	}
	music_list_unlock();
}


void change_cur_music_list(int num)
{
	int is_play_state_run = 0;
	struct eggm_music_t play_music;
	struct eggm_music_list_t play_list;
	memset( &play_list, 0, sizeof(struct eggm_music_list_t));
	memset( &play_music, 0, sizeof(struct eggm_music_t));
	
//	printf("%s/%d num=%d\n", __func__, __LINE__,num);

	get_cur_music( &play_music);
	get_music_list_by_num(num, &play_list);

	if( strcmp ( play_music.listname, play_list.listname) == 0)
	{
		eggm_log("chage playlist but it is cur play list");
		return ;
	}
	memset( &play_music, 0, sizeof(struct eggm_music_t));

	play_music.volume = 50;
	strcpy( play_music.listname, play_list.listname);
	strcpy( play_music.filename, play_list.filename[0]);
    play_music.run_state = is_play_state_run; 
	set_cur_music( &play_music);

	eggm_io_write_selected_music_list(play_music.listname);
}


pthread_t player_tid = 0;
void *player_thread(char *filename)
{
	FILE *pp;
	FILE *outp;
	int ret;

	char line[1000];
	char cmd[300];
	memset(cmd, 0, 300);
	char output_file[] = "/dev/shm/eggm_music_play";
	//char output_file[] = "/run/user/1000/eggm_music_play";
	sprintf( cmd, "omxplayer \"%s\" -I -s -o alsa > %s", filename, output_file);

	eggm_log("[CMD] %s", cmd);
	pp = popen(cmd, "w");
	outp = fopen(output_file, "r");
	if (pp != NULL) {
		while (1) {
			memset(line, 0, 1000);
			usleep(100000);
			//          fgets (line, 10240, outp);
			fgets (line, 1000, outp);
			if( line[0] != 0)
			{
		//		printf("%s\n", line);
				ret = strncmp(line, "have a nice day", 15);
				if( ret == 0)
				{
					eggm_log("--> end this music");
					break;
				}
			}
		}
		eggm_log("--> close music player pipe");
		pclose(pp);
		player_tid = 0;
	}

	


	pthread_exit(0);
}


int music_play(struct eggm_music_t music)
{
    int err;
	char filename[MAX_NAME_LEN];
	if ( (music.filename == NULL) || (music.listname == NULL) )
		return -1;

	sprintf( &filename, "%s/%s/%s", MUSIC_DIR, music.listname, music.filename);
	eggm_log("music_play");
	eggm_log(filename);

	if(player_tid !=0)
		music_stop();

	err = pthread_create(&player_tid, NULL, &player_thread, (void *)filename);

	return 1;
}

void music_stop()
{
	eggm_log("music_stop");
	system("killall -9 omxplayer");
	system("killall -9 omxplayer.bin");
	pthread_cancel(&player_tid);
}

void *music_mgr_task()
{
	//make music list
	struct eggm_music_list_t tmp_music_list[MAX_MUSIC_LIST];
	struct eggm_music_list_t play_list;
	struct eggm_music_t play_music;

	int ret;
	char cur_play_listname[MAX_NAME_LEN];
	int cnt, check_cnt = 0;
	int play_state = 0;
	int play_num = 0;
	int cur_play_num;
	int cur_play_list_num;
	memset(tmp_music_list, 0, sizeof(struct eggm_music_list_t)* MAX_MUSIC_LIST);
	cnt = eggm_io_read_music_list(tmp_music_list);

	int i;

	for(i=0;i<cnt;i++)
	{
		tmp_music_list[i].is_set = 1;
		add_music_list(tmp_music_list[i]);
	}

	print_music_list();
	sort_music_list();
	print_music_list();
	
	// set cur music
    
	memset( &play_list, 0, sizeof(struct eggm_music_list_t));
	memset( &play_music, 0, sizeof(struct eggm_music_t));
	#if 0
	get_music_list_by_num(0, &play_list);
	cur_play_list_num = 0;
	#else
	ret = eggm_io_read_selected_music_list( cur_play_listname );
	if( ret == -1)
	{
		get_music_list_by_num(0, &play_list);
		cur_play_list_num = 0;
	}
	else
	{
	    eggm_log("io read listname = %s\n", 	cur_play_listname );
		cur_play_list_num = get_play_list_num_by_name( cur_play_listname);
		if( cur_play_list_num != -1)
		{
			eggm_log("find num!!=%d\n", cur_play_list_num);
			memset( &play_list, 0, sizeof(struct eggm_music_list_t));
			get_music_list_by_num( cur_play_list_num, &play_list);
			eggm_log("find list =%s\n", play_list.listname);
		}
		else
		{
			get_music_list_by_num(0, &play_list);
			cur_play_list_num = 0;
		}
	}


	#endif
    //just debug play state = 1
//	play_music.run_state = 1;
	play_music.volume = 50;
	strcpy( play_music.listname, play_list.listname);
	strcpy( play_music.filename, play_list.filename[0]);
    cur_play_num = 0; 
	set_cur_music( &play_music);
   	strcpy( cur_play_listname, play_music.listname); 

	while(1)
	{
        int err;
		check_cnt++;

#if 0
		printf(" --------------------\n");
		printf("play_pid=%d [%s] %s (in %s) \n",
				player_tid,
				play_music.run_state? "PLAY":"READY",  
				play_music.filename,
				play_music.listname);
#endif
		check_cnt = 0;
		//get cur music`
		get_cur_music( &play_music);
		if( play_state != play_music.run_state)
		{
			play_state = play_music.run_state;
			if( play_state == 1) // PLAY
			{
				music_play(play_music);
			}
			else // STOP
			{
				music_stop();
			}
		}

		if( strcmp(play_music.listname, cur_play_listname) !=0)
		{
			eggm_log("cur:%s,  now:%s \n",play_music.listname,cur_play_listname);
			cur_play_list_num = get_play_list_num_by_name( play_music.listname );
			eggm_log("next play list num = %d\n",cur_play_list_num);
			if( cur_play_list_num != -1)
			{
				memset( &play_list, 0, sizeof(struct eggm_music_list_t));
				get_music_list_by_num( cur_play_list_num, &play_list);
   				strcpy( cur_play_listname, play_music.listname); 
			}
		}

		if( player_tid == 0)
		{
		    if( play_state == 1)
			{
			    cur_play_num++;
				// next file play
				if(cur_play_num >= play_list.filecnt)
				    cur_play_num = 0;

				memset(play_music.filename, 0, MAX_NAME_LEN);
			    strcpy(play_music.filename, play_list.filename[cur_play_num]);
				music_play(play_music);
				set_cur_music( &play_music);

			}
		}
		
		if( g_play_before && play_state )
		{
			cur_play_num--;
			if(cur_play_num < 0)
				cur_play_num = play_list.filecnt-1;

			music_stop();
			memset(play_music.filename, 0, MAX_NAME_LEN);
			strcpy(play_music.filename, play_list.filename[cur_play_num]);
			music_play(play_music);
			set_cur_music( &play_music);
			complate_play_before_music();
		}
		else if( g_play_next && play_state )
		{
			music_stop();
			
			cur_play_num++;
			if(cur_play_num >= play_list.filecnt)
				cur_play_num = 0;

			memset(play_music.filename, 0, MAX_NAME_LEN);
			strcpy(play_music.filename, play_list.filename[cur_play_num]);
			music_play(play_music);
			set_cur_music( &play_music);
			complate_play_next_music();
		}
		
         		
		
		usleep(10000);
	}
	return NULL;
}
