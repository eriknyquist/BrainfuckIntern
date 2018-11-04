#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>

#include "common.h"

#if WINDOWS
#include <windows.h>
#else
#include <sys/time.h>
#include <time.h>
#endif /* WINDOWS */

#define NUMNAMES (7)

#define EXABYTES                (1024ULL * 1024ULL * 1024ULL * 1024ULL * \
                                1024ULL * 1024ULL)

/**
 * @see common.h
 */
int randrange(int low, int high)
{
    return (rand() % (high + 1 - low)) + low;
}

/**
 * @see common.h
 */
int randrange_except(int low, int high, int except)
{
    int ret = randrange(low, high);

    while (ret == except)
        ret = randrange(low, high);

    return ret;
}

/**
 * @see common.h
 */
float randfloat(void)
{
    return ((float)randrange(0, 10000)) / 10000.0f; 
}

/**
 * @see common.h
 */
void hrsize (size_t size, char *buf, unsigned int bufsize)
{
    static const char *names[NUMNAMES] =
    {
        "EB", "PB", "TB", "GB", "MB", "KB", "B"
    };

    uint64_t mult;
    int i;

    mult = EXABYTES;

    for (i = 0; i < NUMNAMES; ++i, mult /= 1024) {
        if (size < mult)
            continue;

        if (mult && (size % mult) == 0) {
            snprintf(buf, bufsize, "%"PRIu64" %s", size / mult, names[i]);
        } else {
            snprintf(buf, bufsize, "%.2f %s", (float) size / mult, names[i]);
        }

        return;
    }

    strcpy(buf, "0");
}

static void timestamp(char *buf, int bufsize)
{
    int ret;
#if WINDOWS
    SYSTEMTIME tm;

    GetLocalTime(&tm);

    ret = snprintf(buf, bufsize, "%02d-%02d-%02d %02d:%02d:%02d.%03d",
        tm.wDay, tm.wMonth, tm.wYear, tm.wHour, tm.wMinute, tm.wSecond,
        tm.wMilliseconds);
#else
    struct tm *tm;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    ret = snprintf(buf, bufsize, "%02d-%02d-%02d %02d:%02d:%02d.%03d",
        tm->tm_mday, tm->tm_mon + 1, 1900 + tm->tm_year,
        tm->tm_hour, tm->tm_min, tm->tm_sec, (int)((float)tv.tv_usec / 1000.0));
#endif /* WINDOWS */

    /* Null-terminate string if snprintf didn't do it */
    if (ret > (bufsize - 1)) {
        buf[bufsize - 1] = 0;
    }
}

/**
 * @see common.h
 */
void bfi_log(const char* format, ...)
{ 
    va_list args;
    char tm[32];

    timestamp(tm, sizeof(tm));
    va_start(args, format);
    printf("[bfintern %s]: ", tm);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}
