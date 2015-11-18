#include "RtLog.h"
#include <sys/file.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include "uuid/uuid.h"
#include <stdint.h>
#include<math.h>
#include<stdlib.h>
#include<string>
#include <sys/syscall.h>
#define gettid() syscall(__NR_gettid)
using namespace std;

DEFINE_string(rt_log_dir, "./", "log dir");
DEFINE_string(rt_log_pre, "rt_log", "pre file");
DEFINE_int32(rt_log_level, 1, "pre file");

LogWriter ERROR_W;
LogWriter WARN_W;
LogWriter INFO_W;
__thread char LogWriter::m_buffer[_LOG_BUFFSIZE];
static __thread uint64_t g_uuid ;
static __thread int g_pid = 0;

void SetUid(uint64_t log_id)
{
	g_pid = gettid();
    if( log_id == 0 ){
        uuid_t src_uid;    
        uuid_generate( src_uid );
        uint64_t* p_uid = (uint64_t*)src_uid;
        g_uuid = p_uid[0];
    }
    else{
        g_uuid = log_id;
    }     
}

uint64_t GetUid(){
    return g_uuid;
}

bool LogInit(){
	return LogInit(FLAGS_rt_log_dir.c_str(), FLAGS_rt_log_pre.c_str());

}
bool LogInit(const char* p_logdir,const char* p_modulename) 
{
    char _location_str[_LOG_PATH_LEN];  

    snprintf(_location_str, _LOG_PATH_LEN, "%s/%s.access", p_logdir, p_modulename);
    INFO_W.Init(_location_str);
    
    snprintf(_location_str, _LOG_PATH_LEN, "%s/%s.error", p_logdir, p_modulename);    
    WARN_W.Init(_location_str);
    ERROR_W.Init(_location_str);

    return true;
}

bool LogClose(){
    ERROR_W.Destroy();
    WARN_W.Destroy();
    INFO_W.Destroy();    
    return true;
}

bool LogWriter::Init(const  char *filelocation,int level,bool append, bool issync)
{
    m_isappend = append; 
    m_issync = issync; 
	m_level=level;
    if(strlen(filelocation) >= (sizeof(m_filelocation) -1))
    {
        fprintf(stderr, "the path of log file is too long:%d limit:%d\n", strlen(filelocation), sizeof(m_filelocation) -1);
        exit(0);
    }
    //本地存储filelocation  以防止在栈上的非法调用调用
    strncpy(m_filelocation, filelocation, sizeof(m_filelocation));
    m_filelocation[sizeof(m_filelocation) -1] = '\0';

    if('\0' == m_filelocation[0])
    {
        fp = stdout;
        fprintf(stderr, "now all the running-information are going to put to stderr\n");
        return true;
    }

    fp = fopen(m_filelocation, append ? "a":"w");
    if(fp == NULL)
    {
        fprintf(stderr, "%d cannot open log file,file location is %s\n", g_pid, m_filelocation);
        exit(0);
    }
    
    m_node = FileNode();
    
    //setvbuf (fp, io_cached_buf, _IOLBF, sizeof(io_cached_buf)); //buf set _IONBF  _IOLBF  _IOFBF
    setvbuf (fp,  (char *)NULL, _IOLBF, 0);
    fprintf(stderr, "%d now all the running-information are going to the file %s\n", g_pid, m_filelocation);
    return true;
}

const char* LogWriter::LevelStr(int level){
	switch ( level ) { 
		case LL_TRACE:
			return "TRACE";
		case LL_DEBUG:
			return "DEBUG";
		case LL_INFO:
			return "INFO";
		case LL_WARN:
			return "WARN";
		case LL_ERROR:
			return "ERR";
		default:
			return "UNKNOWN";
	}
	return "UNKNOWN";
}

int LogWriter::PreMakeStr(int level)
{
    timeval time;
    ::gettimeofday(&time, 0);
    struct tm vtm; 
    localtime_r(&time.tv_sec, &vtm);
    return snprintf(m_buffer, _LOG_BUFFSIZE, "%s %04d-%02d-%02d %02d:%02d:%02d.%06d |%d:%lx|", 
		LevelStr(level),
        vtm.tm_year+1900, 
        vtm.tm_mon + 1, vtm.tm_mday, vtm.tm_hour, 
        vtm.tm_min, vtm.tm_sec, time.tv_usec,g_pid, g_uuid);
}

bool LogWriter::Log( int level, const char* logformat,...)
{
	if( level < m_level ){
		return true;
	}
    int _size;
    int prestrlen = 0;

    char * star = m_buffer;
    prestrlen = PreMakeStr(level);
    star += prestrlen;

    va_list args;
    va_start(args, logformat);
    _size = vsnprintf(star, _LOG_BUFFSIZE - prestrlen, logformat, args);
    va_end(args);

    if(NULL == fp){
        fprintf(stderr, "xxxxxxxxxx %d:%s\n", g_pid, m_buffer);
    }else
        Write(m_buffer, prestrlen + _size);
    return true;
}

bool LogWriter::Write(char *_pbuffer, int len)
{
    if( m_node != FileNode() || !m_node ){
        m_lock.WLock();		
		if( m_node != FileNode() || !m_node ){
            fprintf(stderr, "xxxxxxxxxx %d:log reinit init file %s\n", g_pid, m_filelocation);
			Destroy();
            Init(m_filelocation,m_level,m_isappend, m_issync);
		}
        m_lock.UnLock();
	}

    m_lock.RLock();
    if(1 == fwrite(_pbuffer, len, 1, fp)) //only write 1 item
    {
        if(m_issync)
            fflush(fp);
        *_pbuffer='\0';
    }
    else 
    {
        int x = errno;
        fprintf(stderr, "Failed to write to logfile. errno:%s    message:%s", strerror(x), _pbuffer);
        //return false;
    }
    m_lock.UnLock();
    return true;
}

bool LogWriter::Destroy()
{
    if(fp == NULL)
        return false;
    fflush(fp);
    fclose(fp);
    fp = NULL;
    m_node = 0;
    return true;
}

uint64_t LogWriter::FileNode()
{	
	uint64_t node = 0;
	struct stat statbuff;	
	if(stat(m_filelocation, &statbuff) == 0 )
	{			
		node = statbuff.st_ino;
	}	
	
	return node;
}

Logger::~Logger()
{
	if( m_level < FLAGS_rt_log_level ){
		return;	
	}
	m_writer->Log(m_level,"%s:%d|%s| %s\n",m_file, m_line, m_func, m_buffer.str().c_str());
}

