#ifndef   _MACRO_LOG_MOUDLE_H
#define   _MACRO_LOG_MOUDLE_H
#include <stdio.h>

bool eros_log_init(const char* argv0,const char* log_file);
bool eros_log_close();
class ErosLog
{
public:
	ErosLog(const char* argv0, const char* log_file)
		{
			eros_log_init(argv0, log_file);
		}	
	~ErosLog()
		{
			eros_log_close();
		}
};

#endif
