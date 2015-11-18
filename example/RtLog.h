#ifndef _MACRO_LOG_H_
#define _MACRO_LOG_H_
#include <gflags/gflags.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include<string>
#include<sstream>
#include "RtRwLock.h"
using namespace std;
using namespace google;

#define F(v) FLAGS_##v

const int LL_TRACE=0;
const int LL_DEBUG=1;
const int LL_INFO=2;
const int LL_WARN=3;
const int LL_ERROR=4;

void SetUid(uint64_t log_id = 0);
uint64_t GetUid();
bool LogInit() ;
bool LogInit(const char* logdir, const char* p_modulename) ;
bool LogClose();


/* 每个线程的buffer size*/
#define   _LOG_BUFFSIZE  40*1024
/* log 文件字符串路径最大长度*/
#define	  _LOG_PATH_LEN  250
/* 日志对应的模块名*/
#define   _LOG_MODULE_LEN 32

/**
*	LogWriter  日志类
*/
class LogWriter 
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
			m_level=0;
		}
		~LogWriter(){
			Destroy();
		}
		bool Init(const  char *filelocation, int level=0,bool append = true, bool issync = false);
		bool Log(int level,const char *logformat,...);
		bool Destroy();
	private:
		int PreMakeStr(int level);
		const char* LevelStr(int level);
		uint64_t FileNode();
		bool Write( char *_pbuffer, int len);
	private:
		FILE* fp;
		bool m_issync;
		bool m_isappend;
		int  m_level;
		char m_filelocation[_LOG_PATH_LEN];
		RWLock m_lock;
		static __thread char m_buffer[_LOG_BUFFSIZE];
		uint64_t m_node;
};

class Logger {
public:
    Logger(LogWriter* writer, const int level,
			const char *file,
			int line, const char* func)
            : m_file(file), m_line(line),m_func(func),m_level(level),m_writer(writer) {}

    virtual ~Logger();

    template <class T>
    Logger &operator <<(const T &t)
    {
        m_buffer << t;
        return *this;
    }

private:
    std::ostringstream m_buffer;
    int m_level;
	const char* m_file;
	const char* m_func;
    int m_line;
	LogWriter* m_writer;

}; // class Logger

extern LogWriter ERROR_W;
extern LogWriter WARN_W;
extern LogWriter INFO_W;

#define TLog  Logger(&INFO_W, LL_TRACE,__FILE__,__LINE__,__FUNCTION__)
#define DLog  Logger(&INFO_W, LL_DEBUG,__FILE__,__LINE__,__FUNCTION__)
#define ILog  Logger(&INFO_W, LL_INFO,__FILE__,__LINE__,__FUNCTION__)
#define WLog  Logger(&WARN_W, LL_WARN,__FILE__,__LINE__,__FUNCTION__)
#define ELog  Logger(&ERROR_W, LL_ERROR,__FILE__,__LINE__,__FUNCTION__)

#endif
