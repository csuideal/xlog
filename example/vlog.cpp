#include"eros_log.h"
#include"stdlib.h"
#include <glog/logging.h>
#include <gflags/gflags.h>
#include<string>
using namespace std;

using namespace google;
int v_log()
{    
	int i = 0;
	string data(1024,'a');
	while(i<2)
    {	
		i++;
		VLOG(0)<<"vlog0";
		VLOG(1)<<"vlog1";
		VLOG(2)<<"vlog2";
		VLOG(3)<<"vlog3";
    }
	return 0;
}
