/* A minimal POSIX getopt() implementation in ANSI C
 *
 * This is free and unencumbered software released into the public domain.
 *
 * This implementation supports the convention of resetting the option
 * parser by assigning optind to 0. This resets the internal state
 * appropriately.
 *
 * Ref: http://pubs.opengroup.org/onlinepubs/9699919799/functions/getopt.html
 *
 * (taken from https://github.com/skeeto/getopt/blob/master/getopt.h)
 *
 * Erik Nyquist 2/11/2018: added shift_nonopt_args so that non-option arguments
 * are not required to be at the beginning of the argument list
 */

#ifndef PORTABLE_GETOPT_H
#define PORTABLE_GETOPT_H

extern int optind;
extern int opterr;
extern int optopt;
extern char *optarg;

int portable_getopt(int argc, char *argv[], const char *optstring);

#endif /* PORTABLE_GETOPT_H */
