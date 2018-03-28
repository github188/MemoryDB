#include "ThreadEvent.h"

#ifdef __WINDOWS__
#include <Windows.h>
#endif


#ifdef __LINUX__
#include <sys/time.h>
#include <errno.h>
#endif
#include <iostream>


event_handle event_create(bool manual_reset, bool init_state)
{   
#ifdef __WINDOWS__
	HANDLE hevent = CreateEvent(NULL, manual_reset, init_state, NULL);
#else
	event_handle hevent = new(std::nothrow) event_t;
	if (hevent == NULL)
	{
		return NULL;
	}
	hevent->state = init_state;
	hevent->manual_reset = manual_reset;
	if (pthread_mutex_init(&hevent->mutex, NULL))
	{
		delete hevent;
		return NULL;
	}
	if (pthread_cond_init(&hevent->cond, NULL))
	{
		pthread_mutex_destroy(&hevent->mutex);
		delete hevent;
		return NULL;
	}
#endif
	return hevent;
}
int event_wait(event_handle hevent)
{
#ifdef __WINDOWS__
	DWORD ret = WaitForSingleObject(hevent, INFINITE);
	if (ret == WAIT_OBJECT_0)
	{
		return 0;
	}
	return -1;
#else
	if (pthread_mutex_lock(&hevent->mutex))   
	{      
		return -1;   
	}   
	while (!hevent->state)    
	{      
		if (pthread_cond_wait(&hevent->cond, &hevent->mutex))   
		{   
			pthread_mutex_unlock(&(hevent->mutex));
			return -1;   
		}   
	}   
	if (!(hevent->manual_reset))
	{
		hevent->state = false;
	}
	if (pthread_mutex_unlock(&hevent->mutex))   
	{     
		return -1;   
	}  
	return 0;
#endif
}
int event_timedwait(event_handle hevent, long milliseconds)
{
#ifdef __WINDOWS__
	DWORD ret = WaitForSingleObject(hevent, milliseconds);
	if (ret == WAIT_OBJECT_0)
	{
		return 0;
	}
	if (ret == WAIT_TIMEOUT)
	{
		return 1;
	}
	return -1;
#else

	int rc = 0;   
	struct timespec abstime;   
	struct timeval tv;   
	gettimeofday(&tv, NULL);   
	abstime.tv_sec  = tv.tv_sec + milliseconds / 1000;   
	abstime.tv_nsec = tv.tv_usec*1000 + (milliseconds % 1000)*1000000;   
	if (abstime.tv_nsec >= 1000000000)   
	{   
		abstime.tv_nsec -= 1000000000;   
		abstime.tv_sec++;   
	}   

	if (pthread_mutex_lock(&hevent->mutex) != 0)   
	{     
		return -1;   
	}   
	while (!hevent->state)    
	{      
		if (rc = pthread_cond_timedwait(&hevent->cond, &hevent->mutex, &abstime))   
		{   
			if (rc == ETIMEDOUT) break;   
			pthread_mutex_unlock(&hevent->mutex);    
			return -1;   
		}   
	}   
	if (rc == 0 && !hevent->manual_reset)   
	{
		hevent->state = false;
	}
	if (pthread_mutex_unlock(&hevent->mutex) != 0)   
	{      
		return -1;   
	}
	if (rc == ETIMEDOUT)
	{
		//timeout return 1
		return 1;
	}
	//wait event success return 0
	return 0;
#endif
}
int event_set(event_handle hevent)
{
#ifdef __WINDOWS__
	return !SetEvent(hevent);
#else
	if (pthread_mutex_lock(&hevent->mutex) != 0)
	{
		return -1;
	}

	hevent->state = true;

	if (hevent->manual_reset)
	{
		if(pthread_cond_broadcast(&hevent->cond))
		{
			return -1;
		}
	}
	else
	{
		if(pthread_cond_signal(&hevent->cond))
		{
			return -1;
		}
	}

	if (pthread_mutex_unlock(&hevent->mutex) != 0)
	{
		return -1;
	}

	return 0;
#endif
}
int event_reset(event_handle hevent) 
{
#ifdef __WINDOWS__
	//ResetEvent 返回非零表示成功
	if (ResetEvent(hevent))
	{
		return 0;
	} 
	return -1;
#else
	if (pthread_mutex_lock(&hevent->mutex) != 0)
	{
		return -1;
	}

	hevent->state = false;

	if (pthread_mutex_unlock(&hevent->mutex) != 0)
	{      
		return -1;
	}
	return 0;
#endif
}
void event_destroy(event_handle &hevent)
{
	if (hevent==NULL)
		return;
#ifdef __WINDOWS__
	CloseHandle(hevent);
#else
	pthread_cond_destroy(&hevent->cond);
	pthread_mutex_destroy(&hevent->mutex);
	delete hevent;
#endif
	hevent = NULL;
}