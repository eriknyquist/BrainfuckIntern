/**
 * Brainfuck Intern (Erik Nyquist, 2023)
 * <eknyquist@gmail.com>)
 *
 * Uses a genetic algorithm to generate poorly written Brainfuck programs
 * that match user-provided test cases, by pure brute-force.
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>


#if defined __linux__
#define WINDOWS 0
#elif defined _WIN32
#define WINDOWS 1
#else
#error "Platform not supported"
#endif

#define MIN_VAL(x, y) (((x) < (y)) ? (x) : (y))
#define MAX_VAL(x, y) (((x) > (y)) ? (x) : (y))


void pcg32_seed(unsigned int seedval);

uint32_t pcg32_rand(void);

/**
 * Get  random number in specific range
 *
 * @param   low   low end of range
 * @param   high  high end of range
 * @return  random number in specified range
 */
uint32_t randrange(uint32_t low, uint32_t high);

/**
 * Get  random number in specific range, except for one specific number
 *
 * @param   low     low end of range
 * @param   high    high end of range
 * @param   except  high end of range
 * @return  random number in specified range
 */
uint32_t randrange_except(uint32_t low, uint32_t high, uint32_t except);

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
 * Convert a counter value to a human-readable string (e.g. 1048576 becomes
 * "1.04M")
 *
 * @param   size     value to be converted, size in bytes
 * @param   buf      pointer to location where output should be written
 * @param   bufsize  maximum number of output characters
 */
void hrcount (unsigned int size, char *buf, unsigned int bufsize);

/**
 * Logs a formatted string
 *
 * @param format   format string that will b e passed to vprintf
 * @param ...      any arguments for the format string
 */
void bfi_log(const char* format, ...);

#endif
