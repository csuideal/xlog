#include "log.h"
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

Log_Writer PUBLIC_W;
Log_Writer WARN_W;
Log_Writer INFO_W;
__thread char Log_Writer::m_buffer[_LOG_BUFFSIZE];
static __thread uint64_t g_uuid ;
static __thread int g_pid = 0;

void set_pid(int pid)
{
    g_pid = pid;  
}

void set_uuid(uint64_t log_id)
{
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

uint64_t get_uuid(){
    return g_uuid;
}


bool log_init(LogLevel l, const char* p_modulename, const char* p_logdir)
{
    char _location_str[_LOG_PATH_LEN];  

    snprintf(_location_str, _LOG_PATH_LEN, "%s/public.log", p_logdir);    
    PUBLIC_W.loginit(l, _location_str);
        
    snprintf(_location_str, _LOG_PATH_LEN, "%s/%s.access", p_logdir, p_modulename);
    INFO_W.loginit(l, _location_str);
    
    snprintf(_location_str, _LOG_PATH_LEN, "%s/%s.error", p_logdir, p_modulename);    
    
    if(l > LL_WARNING)
        WARN_W.loginit(l, _location_str);
    else
        WARN_W.loginit(LL_WARNING, _location_str);

    google::InitGoogleLogging("");
    google::base::SetLogger(google::GLOG_INFO, &INFO_W);
    google::base::SetLogger(google::GLOG_WARNING, &WARN_W);
    google::base::SetLogger(google::GLOG_ERROR, &WARN_W);
    google::base::SetLogger(google::GLOG_FATAL, &WARN_W);
    return true;
}

bool log_close(){
    google::ShutdownGoogleLogging(); 
    PUBLIC_W.logclose();
    WARN_W.logclose();
    INFO_W.logclose();    
    return true;
}

const char* Log_Writer::logLevelToString(LogLevel l) {
    switch ( l ) {
        case LL_DEBUG:
            return "DEBUG";
        case LL_TRACE:
            return "TRACE";
        case LL_NOTICE:
            return "NOTICE";
        case LL_WARNING:
            return "WARN" ;
        case LL_ERROR:
            return "ERROR";
        case LL_PUBLIC:
            return "PUBLIC";        
        default:
            return "UNKNOWN";
    }
}

bool Log_Writer::checklevel(LogLevel l)
{
    if(l >= m_system_level)
        return true;
    else
        return false;
}

bool Log_Writer::loginit(LogLevel l, const  char *filelocation, bool append, bool issync)
{
    MACRO_RET(NULL != fp, false);
    m_system_level = l;
    m_isappend = append; 
    m_issync = issync; 
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
    
    m_node = file_node();
    
    //setvbuf (fp, io_cached_buf, _IOLBF, sizeof(io_cached_buf)); //buf set _IONBF  _IOLBF  _IOFBF
    setvbuf (fp,  (char *)NULL, _IOLBF, 0);
    fprintf(stderr, "%d now all the running-information are going to the file %s\n", g_pid, m_filelocation);
    return true;
}

int Log_Writer::premakestr(char* m_buffer, LogLevel l)
{
    time_t now;
    now = time(&now);;
    struct tm vtm; 
    localtime_r(&now, &vtm);
    
    return snprintf(m_buffer, _LOG_BUFFSIZE, "%s: %04d-%02d-%02d %02d:%02d:%02d [%d:%lx]", 
        logLevelToString(l),
        vtm.tm_year+1900, 
        vtm.tm_mon + 1, vtm.tm_mday, vtm.tm_hour, 
        vtm.tm_min, vtm.tm_sec, g_pid, g_uuid);
}

bool Log_Writer::log(LogLevel l, const char* logformat,...)
{
    MACRO_RET(!checklevel(l), false);
    int _size;
    int prestrlen = 0;

    char * star = m_buffer;
    prestrlen = premakestr(star, l);
    star += prestrlen;

    va_list args;
    va_start(args, logformat);
    _size = vsnprintf(star, _LOG_BUFFSIZE - prestrlen, logformat, args);
    va_end(args);

    if(NULL == fp){
        fprintf(stderr, "xxxxxxxxxx %d:%s\n", g_pid, m_buffer);
    }else
        _write(m_buffer, prestrlen + _size);
    return true;
}

bool Log_Writer::_write(char *_pbuffer, int len)
{
    if( m_node != file_node() || !m_node ){
        m_lock.WLock();		
		if( m_node != file_node() || !m_node ){
            fprintf(stderr, "xxxxxxxxxx %d:log reinit init file %s\n", g_pid, m_filelocation);
			logclose();
            loginit(m_system_level, m_filelocation, m_isappend, m_issync);
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

LogLevel Log_Writer::get_level()
{
    return m_system_level; 
}

bool Log_Writer::logclose()
{
    if(fp == NULL)
        return false;
    fflush(fp);
    fclose(fp);
    fp = NULL;
    m_node = 0;
    return true;
}

uint64_t Log_Writer::file_node()
{	
	uint64_t node = 0;
	struct stat statbuff;	
	if(stat(m_filelocation, &statbuff) == 0 )
	{			
		node = statbuff.st_ino;
	}	
	
	return node;
}

void Log_Writer::Flush()
{
    if( fp  )
    {   
        fflush(fp);
    }   
}

uint32_t Log_Writer::LogSize()
{
    m_lock.RLock();
    unsigned long filesize = 0;
    struct stat statbuff;   
    if(stat(m_filelocation, &statbuff) == 0 ) 
    {    
        filesize = statbuff.st_size;
    }   
    m_lock.UnLock();
    return filesize;
}

void Log_Writer::Write(bool /* should_flush */,
                 time_t /* timestamp */,
                 const char* message,
                 int length){
                 int _size;
    int prestrlen = 0;

    char * star = m_buffer;
    prestrlen = premakestr(star, m_system_level);
    star += prestrlen;

    //glog limit length <=30000  
    int copy_len = _LOG_BUFFSIZE - prestrlen;
	if( length < copy_len  ){
		copy_len =  length;
	}
    
    memcpy(m_buffer, message, copy_len);

    if(NULL == fp){
        fprintf(stderr, "xxxxxxxxxx %d:%s\n", g_pid, m_buffer);
    }else
        _write(m_buffer, prestrlen + copy_len);
    return;
}





