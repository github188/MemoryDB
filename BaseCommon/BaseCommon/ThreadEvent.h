#ifndef _HIK_EVENT_H_ 
#define _HIK_EVENT_H_
#include "BaseCommon.h"
#ifdef __WINDOWS__
	typedef void*  event_handle;
#elif __LINUX__
#	define event_handle event_t*
#include <pthread.h>
struct  event_t
{
	bool state;
	bool manual_reset;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};
#endif


//返回值：NULL 出错
event_handle BaseCommon_Export event_create(bool manual_reset, bool init_state);

//返回值：0 等到事件，-1出错
int BaseCommon_Export event_wait(event_handle hevent);

//返回值：0 等到事件，1 超时，-1出错
int BaseCommon_Export event_timedwait(event_handle hevent, long milliseconds);

//返回值：0 成功，-1出错
int BaseCommon_Export event_set(event_handle hevent);

//返回值：0 成功，-1出错
int BaseCommon_Export event_reset(event_handle hevent);

//返回值：无
void BaseCommon_Export event_destroy(event_handle &hevent);

#endif

////////////
