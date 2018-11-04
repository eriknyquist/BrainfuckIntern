#ifndef COMMON_H
#define COMMON_H

#include "evolution.h"

#if defined __linux__
#define WINDOWS 0
#elif defined _WIN32
#define WINDOWS 1
#else
#error "Platform not supported"
#endif

/**
 * Get  random number in specific range
 *
 * @param   low   low end of range
 * @param   high  high end of range
 * @return  random number in specified range
 */
int randrange(int low, int high);

/**
 * Get  random number in specific range, except for one specific number
 *
 * @param   low     low end of range
 * @param   high    high end of range
 * @param   except  high end of range
 * @return  random number in specified range
 */
int randrange_except(int low, int high, int except);

/**
 * Get random number between 0.0 - 1.0, with 4 digits of precision
 *
 * @return  random number between 0.0 - 1.0
 */
float randfloat(void);

/**
 * Convert a size in bytes to a human-readable string (e.g. 1048576 becomes
 * "1.00MB")
 *
 * @param   size     value to be converted, size in bytes
 * @param   buf      pointer to location where output should be written
 * @param   bufsize  maximum number of output characters
 */
void hrsize (size_t size, char *buf, unsigned int bufsize);

/**
 * Logs a formatted string
 *
 * @param format   format string that will b e passed to vprintf
 * @param ...      any arguments for the format string
 */
void bfi_log(const char* format, ...);

#endif
