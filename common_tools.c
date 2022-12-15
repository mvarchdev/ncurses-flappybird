#include "common_tools.h"
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>

/// @brief Will return random integer based on min max definitions
/// @param min Minimum number that should return
/// @param max Maximum number that should return
/// @return Random number based on min max definition
unsigned int rand_gen(int min, int max)
{
    return min + (rand() % ((max + 1) - min));
}

/// @brief Sleep for the requested number of milliseconds
/// @param msec how long it should block
/// @return Error code
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do
    {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

/// @brief Get actual time in ms
/// @return Actual time in miliseconds
long long timeInMilliseconds()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}