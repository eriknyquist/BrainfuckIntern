/**
 * Brainfuck Intern
 *
 * Copyright 2018 Erik Nyquist <eknyquist@gmail.com>
 * (See LICENSE)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "population.h"

#define DEFAULT_CROSSOVER       (0.75f)
#define DEFAULT_ELITISM         (0.75f)
#define DEFAULT_MUTATION        (1.0f)
#define DEFAULT_POPSIZE         (1024)

static char *target_output =    NULL;
static int target_output_len =  0;
static float crossover =        DEFAULT_CROSSOVER;
static float elitism =          DEFAULT_ELITISM;
static float mutation =         DEFAULT_MUTATION;

static long int popsize =       DEFAULT_POPSIZE;

void help_text(char *arg0)
{
    printf("\nBrainfuck Intern\n\n");
    printf("Usage: %s [-ecms] <output>\n\n", arg0);
    printf("-e <elitism>       Defines how many fit organisms are selected\n"
           "                   from the population for each cycle of the\n"
           "                   evolution process (0.0 to 1.0, e.g. 0.25\n"
           "                   means the top 25%% will be selected). Default\n"
           "                   is %.2f.\n\n", DEFAULT_ELITISM);

    printf("-c <crossover>     Defines how many selected organisms will be\n"
           "                   bred with other randomly selected organisms\n"
           "                   (0.0 to 1.0, e.g. 0.75 means 75%% of\n"
           "                   selected organisms will be bred). Default is\n"
           "                   %.2f.\n\n", DEFAULT_CROSSOVER);

    printf("-m <mutation>      Defines how many selected organisms will be\n"
           "                   randomly mutated (0.0 to 1.0, e.g. 0.44\n"
           "                   means 44%% of selected organisms will be\n"
           "                   mutated). Default is %.2f.\n\n",
               DEFAULT_MUTATION);

    printf("-s <size>          Defines the number of organisms in the\n"
           "                   population (integer). Default is %d.\n\n",
               DEFAULT_POPSIZE);

    printf("-h                 Show this text and exit\n\n");
}

static int parse_float(char optchar, float *dest)
{
    char *endptr;

    float val = (float)strtod(optarg, &endptr);
    if (endptr != (optarg + strlen(optarg))) {
        printf("Please provide a value between 0.0 and 1.0 for -%c option\n",
            optchar);
        return -1;
    }

    if ((val < 0.0) || (val > 1.0)) {
        printf("Please provide a value between 0.0 and 1.0 for -%c option\n",
            optchar);
        return -1;
    }

    *dest = val;
    return 0;
}

int parse_args(int argc, char *argv[])
{
    char c;

      while ((c = getopt(argc, argv, "he:c:m:s:")) != -1) {
        switch (c) {
            case 'h':
                help_text(argv[0]);
                return -1;
            break;

            case 'e':
                if (parse_float('e', &elitism) < 0)
                    return -1;

            break;

            case 'c':
                if (parse_float('c', &crossover) < 0)
                    return -1;

            break;

            case 'm':
                if (parse_float('m', &mutation) < 0)
                    return -1;

            break;

            case 's':
                popsize = atol(optarg);
                if (popsize < 0) {
                    printf("Invalid size provided for -s option\n");
                    return -1;
                }
            break;

            case '?':
                return -1;
            break;
        }
    }

    if (argv[optind] == NULL) {
        help_text(argv[0]);
        return -1;
    }

    target_output = argv[optind];
    target_output_len = strlen(target_output);
    return 0;
}

int main(int argc, char *argv[])
{
    time_t t;

    if (parse_args(argc, argv) < 0)
        return -1;

    srand((unsigned int) time(&t));

    return population_evolve(target_output, popsize, crossover, elitism, mutation);
}
