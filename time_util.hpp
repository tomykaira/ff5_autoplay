#ifndef _TIME_UTIL_H_
#define _TIME_UTIL_H_

#include <sys/time.h>

#define DIFF(end, start) (((end).tv_sec - (start).tv_sec)*1000*1000 + (end).tv_usec - (start).tv_usec)

void after(const struct timeval * from, struct timeval * to, int diff);

#endif /* _TIME_UTIL_H_ */
