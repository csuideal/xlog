//#include "log.h"
#include <stdio.h>
#include <sys/file.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
//#include "uuid/uuid.h"
#include <sys/stat.h>  
#include <pthread.h>
//#include "glog/logging.h"
#include <glog/logging.h>

using namespace google;

class LogWriter:public google::base::Logger
{
	public:

		LogWriter()
		{
			//fp = stderr;
			fp = NULL;
			node = 0;
			m_filelocation[0] ='\0'; 			
			pthread_mutex_init(&m_mutex, NULL);
		}
		~LogWriter(){
			Destroy();
		}
		bool Init(const char *file_name, const char* suffix);
		bool Init(const char *full_path);
		uint64_t FileNode();
		
		bool Destroy();
		virtual void Write(bool /* should_flush */,
		         time_t /* timestamp */,
		         const char* message,
		         int length);
		virtual void Flush();
		virtual uint32_t LogSize();

	private:
		FILE* fp;
		uint64_t node;
		std::string m_filelocation;		
		pthread_mutex_t m_mutex;
		
};	

static LogWriter info_logger;
static LogWriter warn_logger;
static LogWriter error_logger;
static LogWriter fatal_logger;

bool LogWriter::Init(const char *file_name, const char* suffix)
{
	std::string path = file_name;
	path += ".";
	path += suffix;
	return Init(path.c_str());
}


bool LogWriter::Init(const char *file_full_path)
{	
	if( fp )
	{
		Destroy();
	}		

	m_filelocation = file_full_path;
	if(m_filelocation == ".")
	{
		fp = stderr;
		fprintf(stderr, "now file set and  put to stderr\n");
		return true;
	}
	
	fp = fopen(m_filelocation.c_str(), "a");
	if(fp == NULL)
	{
		fprintf(stderr, "cannot open log file,file location is %s\n", m_filelocation.c_str());
		exit(0);
	}

	node = FileNode();
	//setvbuf (fp, io_cached_buf, _IOLBF, sizeof(io_cached_buf)); //buf set _IONBF  _IOLBF  _IOFBF
	setvbuf (fp,  (char *)NULL, _IOLBF, 0);
	fprintf(stderr, "log file init:%s\n", m_filelocation.c_str());
	return true;
}

void LogWriter::Write(bool should_flush ,
		         time_t /* timestamp */,
		         const char* _pbuffer,
		         int len)
{
#if 0
	if(0 != access(m_filelocation.c_str(), W_OK))
	{	
		pthread_mutex_lock(&m_mutex);
		//锁内校验 access 看是否在等待锁过程中被其他线程loginit了  避免多线程多次close 和init
		if(0 != access(m_filelocation.c_str(), W_OK))
		{
			fprintf(stderr,"pid=%u file re init",getpid());
			Init( m_filelocation.c_str());
		}
		pthread_mutex_unlock(&m_mutex);
	}
#else
	if( node != FileNode() || !node ){

		pthread_mutex_lock(&m_mutex);

		if( node != FileNode() || !node ){
			fprintf(stderr,"pid=%u file re init",getpid());
            Init( m_filelocation.c_str());
		}
		pthread_mutex_unlock(&m_mutex);
	}
#endif 
	if(1 == fwrite(_pbuffer, len, 1, fp)) //only write 1 item
	{
		if(should_flush)
          	fflush(fp);
		//*_pbuffer='\0';
    }
    else 
	{
        int x = errno;
	    fprintf(stderr, "Failed to write to logfile. errno:%s    message:%s", strerror(x), _pbuffer);
	    return;
	}
	return;
}


bool LogWriter::Destroy()
{
	if(fp == NULL)
		return false;
	fflush(fp);
	fclose(fp);
	fp = NULL;
	node = 0;
	
	return true;
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
	pthread_mutex_lock(&m_mutex);
	unsigned long filesize = 0;
	struct stat statbuff;	
	if(stat(m_filelocation.c_str(), &statbuff) == 0 )
	{			
		filesize = statbuff.st_size;
	}
	pthread_mutex_unlock(&m_mutex);
	return filesize;
}

uint64_t LogWriter::FileNode()
{	
	uint64_t node = 0;
	struct stat statbuff;	
	if(stat(m_filelocation.c_str(), &statbuff) == 0 )
	{			
		node = statbuff.st_ino;
	}
	
	printf("node=%u",node);
	return node;
}



bool eros_log_init(const char* argv0,const char* log_file)
{
	google::InitGoogleLogging(argv0);		
	info_logger.Init(log_file,"info");
	warn_logger.Init(log_file,"warnning");
	error_logger.Init(log_file,"error");
	fatal_logger.Init(log_file,"fatal");
	google::base::SetLogger(GLOG_INFO, &info_logger);
	google::base::SetLogger(GLOG_WARNING, &warn_logger);
	google::base::SetLogger(GLOG_ERROR, &error_logger);
	google::base::SetLogger(GLOG_FATAL, &fatal_logger);
	
	return true;
}

bool eros_log_close(){
	google::ShutdownGoogleLogging();	
    info_logger.Destroy();
	warn_logger.Destroy();
	error_logger.Destroy();
	fatal_logger.Destroy();
}




