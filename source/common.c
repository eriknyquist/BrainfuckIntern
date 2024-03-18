/**
 * Brainfuck Intern (Erik Nyquist, 2023)
 * <eknyquist@gmail.com>)
 *
 * Uses a genetic algorithm to generate poorly written Brainfuck programs
 * that match user-provided test cases, by pure brute-force.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>

#include "pcg_variants.h"
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

#define EXACOUNT                (1000ULL * 1000ULL * 1000ULL * 1000ULL * \
                                1000ULL * 1000ULL)


static pcg32_random_t _pcg_rng;


void pcg32_seed(unsigned int seedval)
{
    pcg32_srandom_r(&_pcg_rng, (uint64_t) seedval, 0u);
}

uint32_t pcg32_rand(void)
{
    return pcg32_random_r(&_pcg_rng);
}

/**
 * @see common.h
 */
uint32_t randrange(uint32_t low, uint32_t high)
{
    return (pcg32_rand() % (high + 1u - low)) + low;
}

/**
 * @see common.h
 */
uint32_t randrange_except(uint32_t low, uint32_t high, uint32_t except)
{
    uint32_t ret = randrange(low, high);

    while (ret == except)
        ret = randrange(low, high);

    return ret;
}

/**
 * @see common.h
 */
float randfloat(void)
{
    return ((float)randrange(0u, 10000u)) / 10000.0f; 
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

    for (i = 0; i < NUMNAMES; ++i, mult /= 1024u)
    {
        if (size < mult)
        {
            continue;
        }

        if (mult && (size % mult) == 0u)
        {
            snprintf(buf, bufsize, "%"PRIu64" %s", size / mult, names[i]);
        }
        else
        {
            snprintf(buf, bufsize, "%.2f %s", (float) size / mult, names[i]);
        }

        return;
    }

    buf[0] = '0';
    buf[1] = '\0';
}

/**
 * @see common.h
 */
void hrcount (unsigned int size, char *buf, unsigned int bufsize)
{
    static const char names[NUMNAMES] =
    {
        'E', 'P', 'T', 'G', 'M', 'K', 'B'
    };

    uint64_t mult;
    int i;

    mult = EXACOUNT;

    for (i = 0; i < NUMNAMES; ++i, mult /= 1000u)
    {
        if (size < mult)
        {
            continue;
        }

        if (mult && (size % mult) == 0u)
        {
            snprintf(buf, bufsize, "%"PRIu64"%c", size / mult, names[i]);
        }
        else
        {
            snprintf(buf, bufsize, "%.1f%c", (float) size / mult, names[i]);
        }

        return;
    }

    buf[0] = '0';
    buf[1] = '\0';
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
    if (ret > (bufsize - 1))
    {
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
