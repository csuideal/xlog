#include"eros_log.h"
#include"stdlib.h"
#include <glog/logging.h>
#include <gflags/gflags.h>


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
	google::SetLogDestination(google::INFO, "./test.log_");
    google::SetLogDestination(google::WARNING, "./test.log.warn_");
    google::SetLogDestination(google::ERROR, "./test.log.error_");
	FLAGS_logbufsecs = 0;
#else
	
	//eros_log_init(argv[0],"./test");
	ErosLog log(argv[0],"./log/test");
	
#endif 
	 //FLAGS_stderrthreshold=google::INFO;
    //FLAGS_colorlogtostderr=true;
    //FLAGS_log_backtrace_at="";
	//log_backtrace_at
	FLAGS_alsologtostderr=1;
	FLAGS_logtostderr=0;
	FLAGS_max_log_size = 1;
	//FLAGS_log_prefix=0;
	int i = 0;
	while(1)
    {	
		i++;
        LOG(INFO)<<"LOG_IF(INFO,i=true)  google::COUNTER="<<google::COUNTER<<"  i="<<i<<"sleep:"<<sleep_time;
		VLOG(3)<<"vlog3";
		VLOG(2)<<"vlog2";
		VLOG(1)<<"vlog1";
		VLOG(0)<<"vlog0";
        LOG(WARNING)<<"LOG_IF(INFO,i=true)  google::COUNTER="<<google::COUNTER<<"  i="<<i<<"sleep:"<<sleep_time;
#if 0
        LOG(ERROR)<<"LOG_IF(INFO,i=true)  google::COUNTER="<<google::COUNTER<<"  i="<<i<<"sleep:"<<sleep_time;
        LOG_IF(INFO,i==100)<<"LOG_IF(INFO,i==100)  google::COUNTER="<<google::COUNTER<<"  i="<<i<<"sleep:"<<sleep_time;
        LOG_EVERY_N(INFO,10)<<"LOG_EVERY_N(INFO,10)  google::COUNTER="<<google::COUNTER<<"  i="<<i<<"sleep:"<<sleep_time;
        LOG_IF_EVERY_N(WARNING,(i>50),10)<<"LOG_IF_EVERY_N(INFO,(i>50),10)  google::COUNTER="<<google::COUNTER<<"  i="<<i;
        LOG_FIRST_N(ERROR,5)<<"LOG_FIRST_N(INFO,5)  google::COUNTER="<<google::COUNTER<<"  i="<<i<<"sleep:"<<sleep_time;
#endif 
		sleep(sleep_time);
    }
    google::ShutdownGoogleLogging();
}
