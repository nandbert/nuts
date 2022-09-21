/** 
 *
 *
 *  \file      time.h
 *  \author    Norbert Stoeffler
 *  \date      2014-05-15
 */
#ifndef __TIME_H__
#define __TIME_H__

#include	"basic.h"

/***************************************
 *  simple timers
 **************************************/

#if defined WIN32
#include	<time.h>

typedef clock_t tTimer;


inline void startTimer(tTimer *pt)
{
	*pt=clock();
}


inline double stopTimer(const tTimer *t)
{
	clock_t now;

	now=clock();
	
	return (now-*t)*1000.0/CLOCKS_PER_SEC;
}

#else
#include	<sys/time.h>

typedef struct timeval tTimer;


inline void startTimer(tTimer *pt)
{
	gettimeofday(pt,NULL);
}


inline double stopTimer(const tTimer *pt)
{
	struct timeval now;

	gettimeofday(&now,NULL);
	return ((now.tv_sec*10000+now.tv_usec/100)-(pt->tv_sec*10000+pt->tv_usec/100))/10.0;
}

#endif

#endif /* __TIME_H__ */
