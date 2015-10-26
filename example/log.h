#ifndef   _MACRO_LogModule
#define   _MACRO_LogModule
#include <glog/logging.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

#define MLOG(l) VLOG(l)<<"["<<__FILE__<<":"<<__LINE__<<"]["<<__FUNCTION__<<"] "
#define ILOG LOG(INFO)<<"["<<__FILE__<<":"<<__LINE__<<"]["<<__FUNCTION__<<"] "
#define WLOG LOG(ERROR)<<"["<<__FILE__<<":"<<__LINE__<<"]["<<__FUNCTION__<<"] "
#define ELOG LOG(ERROR)<<"["<<__FILE__<<":"<<__LINE__<<"]["<<__FUNCTION__<<"] "



/* 每个线程的buffer size*/
#define   _LOG_BUFFSIZE  40*1024
/* log 文件字符串路径最大长度*/
#define	  _LOG_PATH_LEN  250
/* 日志对应的模块名*/
#define   _LOG_MODULE_LEN 32


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
	int UnLock(){
		return pthread_rwlock_unlock(&m_lv);
	}

private:
	pthread_rwlock_t m_lv;
	
};


/**
*	LogWriter  日志类
*/
class LogWriter : public google::base::Logger
{
	public:
		LogWriter()
		{
			//fp = stderr;
			fp = NULL;
			m_issync = false;
			m_isappend = true;
			m_filelocation[0] ='\0';
			m_node = 0;
			//pthread_mutex_init(&m_mutex, NULL);
		}
		~LogWriter(){
			Destroy();
		}
		bool Init(const  char *filelocation, const char* level, bool append = true, bool issync = false);
		bool Log(const char *logformat,...);
		bool Destroy();
	private:
		int PreMakeStr();
		uint64_t FileNode();
		bool Write( char *_pbuffer, int len);
		virtual void Write(bool /* should_flush */,
                 time_t /* timestamp */,
                 const char* message,
                 int length);
        virtual void Flush();
        virtual uint32_t LogSize();
	private:
		FILE* fp;
		bool m_issync;
		bool m_isappend;
		char m_filelocation[_LOG_PATH_LEN];
		RWLock m_lock;
		static __thread char m_buffer[_LOG_BUFFSIZE];
		uint64_t m_node;
		std::string m_pre_level;
};

void SetUid(uint64_t log_id = 0);
uint64_t GetUid();

bool LogInit(const char* p_modulename, const char* p_logdir);
bool LogClose();
#endif
