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
#include<string>
#include <sys/syscall.h>
#define gettid() syscall(__NR_gettid)
using namespace std;

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


bool LogInit(const char* p_modulename, const char* p_logdir)
{
    char _location_str[_LOG_PATH_LEN];  

    snprintf(_location_str, _LOG_PATH_LEN, "%s/%s.access", p_logdir, p_modulename);
    INFO_W.Init(_location_str,"INFO");
    
    snprintf(_location_str, _LOG_PATH_LEN, "%s/%s.error", p_logdir, p_modulename);    
    WARN_W.Init(_location_str,"WARN");
    ERROR_W.Init(_location_str,"ERROR");
    google::InitGoogleLogging("");
    google::base::SetLogger(google::GLOG_INFO, &INFO_W);
    google::base::SetLogger(google::GLOG_WARNING, &WARN_W);
    google::base::SetLogger(google::GLOG_ERROR, &ERROR_W);
    google::base::SetLogger(google::GLOG_FATAL, &ERROR_W);
	FLAGS_log_prefix=0;
    return true;
}

bool LogClose(){
    google::ShutdownGoogleLogging(); 
    ERROR_W.Destroy();
    WARN_W.Destroy();
    INFO_W.Destroy();    
    return true;
}

bool LogWriter::Init(const  char *filelocation,const char* level, bool append, bool issync)
{
    m_isappend = append; 
    m_issync = issync; 
	m_pre_level= level;
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

int LogWriter::PreMakeStr()
{
    time_t now;
    now = time(&now);;
    struct tm vtm; 
    localtime_r(&now, &vtm);
    
    return snprintf(m_buffer, _LOG_BUFFSIZE, "%s: %04d-%02d-%02d %02d:%02d:%02d [%d:%lx]", 
        m_pre_level.c_str(),
        vtm.tm_year+1900, 
        vtm.tm_mon + 1, vtm.tm_mday, vtm.tm_hour, 
        vtm.tm_min, vtm.tm_sec, g_pid, g_uuid);
}

bool LogWriter::Log( const char* logformat,...)
{
    int _size;
    int prestrlen = 0;

    char * star = m_buffer;
    prestrlen = PreMakeStr();
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
            Init(m_filelocation,m_pre_level.c_str(), m_isappend, m_issync);
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

void LogWriter::Flush()
{
    if( fp  )
    {   
        fflush(fp);
    }   
}

uint32_t LogWriter::LogSize()
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

void LogWriter::Write(bool  should_flush ,
                 time_t /* timestamp */,
                 const char* message,
                 int length){
    int prestrlen = 0;
    char * star = m_buffer;
    prestrlen = PreMakeStr();
    star += prestrlen;

    //glog limit length <=30000  
    int copy_len = min(_LOG_BUFFSIZE - prestrlen,length);
    memcpy(star, message, copy_len);
	

    if(NULL == fp){
        fprintf(stderr, "xxxxxxxxxx %d:%s\n", g_pid, m_buffer);
    }else
        Write(m_buffer, prestrlen + copy_len);
		if( should_flush  ){
			fflush(fp);
		}
    return;
}

