#ifndef RT_RW_LOGK_H_
#define RT_RW_LOGK_H_

inline void TsAddMs(struct timespec *ts, long ms)
{
	int sec=ms/1000;
	ms=ms-sec*1000;

	// perform the addition
	ts->tv_nsec+=ms*1000000;

	// adjust the time
	ts->tv_sec+=ts->tv_nsec/1000000000 + sec;
	ts->tv_nsec=ts->tv_nsec%1000000000;
}

class RWLock
{
public:
	explicit RWLock(const pthread_rwlockattr_t* attr = NULL)
	{
		::pthread_rwlock_init(&m_lv, attr);
	}
	~RWLock() { 
		::pthread_rwlock_destroy(&m_lv);
	}
	int RLock(){
		return pthread_rwlock_rdlock(&m_lv);
	}
	int WLock(){
		return pthread_rwlock_wrlock(&m_lv);

	}
	int TryRLock(uint64_t time_out_ms=0){
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME,&ts);
		TsAddMs(&ts,time_out_ms);
		return pthread_rwlock_timedrdlock(&m_lv, &ts);
	}
	int TryWLock(uint64_t time_out_ms=0){
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME,&ts);
		TsAddMs(&ts,time_out_ms);
		return pthread_rwlock_timedwrlock(&m_lv,&ts);

	}
	int UnLock(){
		return pthread_rwlock_unlock(&m_lv);
	}

private:
	pthread_rwlock_t m_lv;
		
};

#endif 
