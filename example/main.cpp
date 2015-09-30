#include"stdlib.h"
#include <glog/logging.h>
#include "gflags/gflags.h"
#include<string>
#include <glog/raw_logging.h>
#include"log.h"
using namespace std;

extern int v_log();
#define STR_LOG(l) VLOG(l)

void SplitLogHead(const char* msg, int len, string& ret_data);
DECLARE_string(vmodule);
using namespace google;
int main(int argc,char* argv[])
{    
	//::google::ParseCommandLineFlags(&argc, &argv, true);
	int sleep_time = 1;
	if(argc > 1){
		sleep_time=atoi(argv[1]);
	}
	
   
#if 0
	google::InitGoogleLogging(argv[0]);
	google::SetLogDestination(google::INFO, "./log/test.log_");
    google::SetLogDestination(google::WARNING, "./log/test.log.warn_");
    google::SetLogDestination(google::ERROR, "./log/test.log.error_");
#else

	log_init((LogLevel)1,"test","./log");
	
#endif 
	 //FLAGS_stderrthreshold=google::INFO;
    //FLAGS_colorlogtostderr=true;
    //FLAGS_log_backtrace_at="";
	//log_backtrace_at
	FLAGS_logbufsecs = 5;
	FLAGS_alsologtostderr=1;
	FLAGS_logtostderr=0;
	FLAGS_max_log_size = 1024*1024*10;
	FLAGS_v=6;
	//FLAGS_vmodule="vlog=3,main=2";
	int i = 0;
	//v_log();
		
	string data;
	while(i<3)
    {	
		i++;
        ILOG<<data;;
		MLOG(0)<<data;	
		ILOG<<"vlog0";
		ELOG<<"vlog0";
		log_debug("vlog0");
		//VLOG(1)<<"vlog1";
		//VLOG(2)<<"vlog2";
		//SYSLOG(ERROR)<<"syslog-1";
	//	RAW_LOG(INFO,"%s",data.c_str());
        //LOG(WARNING)<<"LOG_IF(INFO,i=true)  google::COUNTER="<<google::COUNTER<<"  i="<<i<<"sleep:"<<sleep_time;
#if 0
        LOG(ERROR)<<"LOG_IF(INFO,i=true)  google::COUNTER="<<google::COUNTER<<"  i="<<i<<"sleep:"<<sleep_time;
        LOG_IF(INFO,i==100)<<"LOG_IF(INFO,i==100)  google::COUNTER="<<google::COUNTER<<"  i="<<i<<"sleep:"<<sleep_time;
        LOG_EVERY_N(INFO,10)<<"LOG_EVERY_N(INFO,10)  google::COUNTER="<<google::COUNTER<<"  i="<<i<<"sleep:"<<sleep_time;
        LOG_IF_EVERY_N(WARNING,(i>50),10)<<"LOG_IF_EVERY_N(INFO,(i>50),10)  google::COUNTER="<<google::COUNTER<<"  i="<<i;
        LOG_FIRST_N(ERROR,5)<<"LOG_FIRST_N(INFO,5)  google::COUNTER="<<google::COUNTER<<"  i="<<i<<"sleep:"<<sleep_time;
#endif 
	//	sleep(sleep_time);
    }
    google::ShutdownGoogleLogging();
}
