#include "time_util.hpp"

void after(const struct timeval * from, struct timeval * to, int diff)
{
	int new_usec = from->tv_usec + diff;
	to->tv_usec = new_usec % (1000*1000);
	to->tv_sec  = from->tv_sec + new_usec / (1000*1000);
}
