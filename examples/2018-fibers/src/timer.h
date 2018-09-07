#pragma once

#include <time.h>
#include <sys/time.h>

typedef struct timeval timer;

#define timer_start(timer_name) gettimeofday(&timer_name, NULL)

#define timer_value_milli(timer_name) ({\
					struct timeval __rs_tmp_timer;\
					int __rs_timedif;\
					gettimeofday(&__rs_tmp_timer, NULL);\
					__rs_timedif = __rs_tmp_timer.tv_sec * 1000 + __rs_tmp_timer.tv_usec / 1000;\
					__rs_timedif -= timer_name.tv_sec * 1000 + timer_name.tv_usec / 1000;\
					__rs_timedif;\
				})
