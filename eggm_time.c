#include<stdio.h>
#include<time.h>
#include<sys/time.h>

#define BUFF_SIZE 32
#define STR_TIME_FORMAT      "%y-%m-%d/%H:%M:%S"


#if 0
struct tm {
    int tm_sec;   /* Seconds */
    int tm_min;   /* Minutes */
    int tm_hour;  /* Hour (0--23) */
    int tm_mday;  /* Day of month (1--31) */
    int tm_mon;   /* Month (0--11) */
    int tm_year;  /* Year (calendar year minus 1900) */
    int tm_wday;  /* Weekday (0--6; Sunday = 0) */
    int tm_yday;  /* Day of year (0--365) */
    int tm_isdst; /* 0 if daylight savings time is not in effect) */

};
#endif

int print_cur_time()
{
    struct tm tm_now;
    struct timespec tspec;

    char buff[BUFF_SIZE];

    // get time for clock
    if ( clock_gettime(CLOCK_REALTIME, & tspec) == -1)
    {
	printf(" It is error\n");
    }
    
    // timespec to tm
    localtime_r((time_t*)&tspec.tv_sec, &tm_now);

//    if(strftime(buff, BUFF_SIZE, STR_TIME_FORMAT, &tm_now)==0)
//    {
//	printf("error!!\n");
//    }
    printf("%04d,%02d,%02d(%d)-%02d:%02d:%02d\n",
	    1900 + tm_now.tm_year,
	    tm_now.tm_mon,
	    tm_now.tm_mday,
	    tm_now.tm_wday,
	    tm_now.tm_hour,
	    tm_now.tm_min,
	    tm_now.tm_sec);

	return 1;
}

int update_time()
{
	system("sudo rdate -s time.bora.net");
	return 1;
}

int get_cur_time(struct tm *tm_now)
{
//	struct tm tm_now;
	struct timespec tspec;

	if ( clock_gettime(CLOCK_REALTIME, & tspec) == -1)
	{
		printf(" It is error\n");
		return -1;
	}
    localtime_r((time_t*)&tspec.tv_sec, tm_now);
	return 1;
}

