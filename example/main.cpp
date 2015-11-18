#include"stdlib.h"
#include<string>
#include"RtLog.h"
using namespace std;


void SplitLogHead(const char* msg, int len, string& ret_data);
DECLARE_string(vmodule);
using namespace google;
int main(int argc,char* argv[])
{    
	::google::ParseCommandLineFlags(&argc, &argv, true);
	int sleep_time = 1;
	if(argc > 1){
		sleep_time=atoi(argv[1]);
	}
   
	LogInit();
	SetUid();
	string data;
	int i = 0;
	while(i<3)
    {	
		i++;
        ILog<<"123";
		ELog<<"456";	
		WLog<<"vlog0";
   }
	LogClose();
}
