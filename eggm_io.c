#include <stdio.h>
#include <string.h>   
#include <sys/types.h>
#include <dirent.h>

#include "eggm.h"

#define _CRT_SECURE_NO_WARNINGS  


int eggm_io_read_music_list(struct eggm_music_list_t *music_list)
{
	DIR *dp, *dp2;
	struct dirent *ep,*ep2;
	int i,p,cnt1 =0, cnt2 = 0;
    char local_list[MAX_MUSIC_LIST][MAX_NAME_LEN];
	dp = opendir(MUSIC_DIR);
	if( dp== NULL)
		return -1;

	while( ep = readdir(dp))
	{
		strncpy(local_list[cnt1], ep->d_name, sizeof(ep->d_name));
		if(local_list[cnt1][0] =='.')
		   memset(local_list[cnt1], 0, MAX_NAME_LEN);
		else
			cnt1++;
	}
	(void) closedir(dp);

	// list cnt = cnt1
	for(i=0;i<cnt1;i++)
	{
	    cnt2 = 0;
		struct eggm_music_list_t ml;
		char path[MAX_PATH_LEN];
		memset(&ml, 0, sizeof(struct eggm_music_list_t));
		memset(path, 0, MAX_PATH_LEN);
		strncpy(ml.listname, local_list[i], sizeof(local_list[i]));
		sprintf(path, "%s/%s", MUSIC_DIR, local_list[i]);
		dp2 = opendir(path);

	    while( ep2 = readdir(dp2))
		{
			if( strstr( ep2->d_name, ".mp3") == NULL)
				continue;
			
			strncpy(ml.filename[cnt2], ep2->d_name, sizeof(ep2->d_name));
			cnt2++;
		}
		(void) closedir(dp2);
		ml.filecnt = cnt2;
		memcpy( &music_list[i] , &ml, sizeof(struct eggm_music_list_t));
	}
	return cnt1;
}

int eggm_io_write_selected_music_list(char *name)
{
#if 0
	FILE *pFile = NULL;
	pFile = fopen( ".eggm.music.conf", "rw" );
	if( pFile != NULL )
	{
		fprintf(pFile, name);
		fclose(pFile);
		return 1;
	}
	return -1;
#else
	char cmd[300];
	sprintf(cmd, "echo %s > .eggm.music.conf", name);
	system(cmd);
#endif
}

int eggm_io_read_selected_music_list(char *name)
{
	FILE *pFile = NULL;
	char *pStr;
	int i;
	pFile = fopen( ".eggm.music.conf", "r" );
	if( pFile != NULL )
	{
		char strTemp[MAX_NAME_LEN];
#if 1
		if( !feof( pFile ) )
		{
			pStr = fgets( strTemp, sizeof(strTemp), pFile );
			if(pStr == NULL)
			{
				return -1;
			}
			else
			{
				for( i=0; i<MAX_NAME_LEN; i++)
				{
					if( strTemp[i] == 0x0A )
					{
						strTemp[i] = 0;
						break;
					}
				}
				memset(name, 0, MAX_NAME_LEN);
				strcpy( name, strTemp);
				fclose(pFile);
				return 1;
			}
		}
#else
		char c;
		int i=0;
		while( 0 < fscanf( pFile, "%c", &c))
		{
			if( c = 0x0A)
				break;
			strTemp[i] = c;
			i++;
		}
		fclose(pFile);
		printf("%s\n", strTemp);
		memset(name, 0, MAX_NAME_LEN);
		strcpy( name, strTemp);

		return 1;
#endif
	}
	return -1;
}


int eggm_io_read_sch_conf(struct eggm_schedule_data_t *sch)
{
	FILE *pFile = NULL;

	pFile = fopen( ".eggm.sch.conf", "rw" );
	if( pFile != NULL )
	{
		char strTemp[255];
		char *pStr, *resultStr;
		int cnt = 0;
		int day = 0;
		char startTimeStr[10];
		char endTimeStr[10];
		while( !feof( pFile ) )
		{
			pStr = fgets( strTemp, sizeof(strTemp), pFile );
			if(pStr == NULL)
			{
				break;
			}

			resultStr = strtok(strTemp, ",");

			//1. check type
			if(strncmp(resultStr, "sch", 3)==0)
			{
				//schsule config
				resultStr = strtok(NULL, ",");
				day = atoi(resultStr);

				//2. check day
				if( day >=0 && day <= 6) 
				{
					resultStr = strtok(NULL, ",");
					
					// 3. check enable
					//if(strncmp(resultStr, "en", 2)==0)
					{
						//3 .check enable
						int is_enable = !strncmp(resultStr, "en", 2);

						//4. start time
						resultStr = strtok(NULL, ",");
						strcpy( startTimeStr, resultStr);

						//5 end time
						resultStr = strtok(NULL, ",");
						strcpy( endTimeStr, resultStr);

						char s_h_str[3];
						char s_m_str[3];
						char e_h_str[3];
						char e_m_str[3];
							
						strncpy( s_h_str, startTimeStr, 2);
						strncpy( s_m_str, startTimeStr+3, 2);

						strncpy( e_h_str, endTimeStr, 2);
						strncpy( e_m_str, endTimeStr+3, 2);

						int start_hour = atoi ( s_h_str ); 
						int start_min = atoi ( s_m_str ); 
						int end_hour = atoi ( e_h_str ); 
						int end_min = atoi ( e_m_str ); 
//					    printf("start %02d:%02d\n", start_hour, start_min);	
//					    printf("end   %02d:%02d\n", end_hour, end_min);
						
						sch[day].is_enable = is_enable;
						sch[day].start_hour = start_hour;
						sch[day].start_min = start_min;
						sch[day].end_hour = end_hour;
						sch[day].end_min = end_min;
						
					}
				}
			}
			cnt++;
		}
		fclose( pFile );
	}
	return 1;
}
