/**
 * Brainfuck Intern (Erik Nyquist, 2023)
 * <eknyquist@gmail.com>)
 *
 * Uses a genetic algorithm to generate poorly written Brainfuck programs
 * that match user-provided test cases, by pure brute-force.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "portable_getopt.h"
#include "evolution.h"
#include "common.h"

#define VERSION                 ("2.2")

#define DEFAULT_CROSSOVER       (0.5f)
#define DEFAULT_ELITISM         (0.5f)
#define DEFAULT_MUTATION        (1.0f)
#define DEFAULT_POPSIZE         (2048)
#define DEFAULT_MAX_LEN         (4096)
#define DEFAULT_OPTGENS         (1000)


#define MAX_NUM_TESTCASES (128u)

static evolution_testcase_t _testcases[MAX_NUM_TESTCASES];
static unsigned int _num_testcases = 0u;

static unsigned int _seed = 0u;
static bool _seed_provided = false;


void help_text(char *arg0)
{
    printf("\nBrainfuck Intern (version %s)\n\n"
           "Erik Nyquist <eknyquist@gmail.com>)\n\n", VERSION);

    printf("Uses a genetic algorithm to mutate strings of random Brainfuck characters\n"
           "until they match a set of user-provided test cases.\n\n"
           "Press Ctrl-C at any time to display the current best Brainfuck program\n"
           "and terminate.\n\n");

    printf("Usage: %s [OPTIONS] <TESTCASE> [<TESTCASE>, ...]\n\n", arg0);

    printf("POSITIONAL ARGUMENTS:\n\n");
    printf("One or more test cases, where each test case consists of two ASCII\n"
           "strings separated by a colon ':'. The first string (left side of the\n"
           "colon) will be passed as input to the Brainfuck program under test,\n"
           "and the second string (right side of the colon) is the output that\n"
           "must be produced by the Brainfuck program in order for the test case\n"
           "to pass.\n\n"
           "A test case may also contain a single ASCII string with no colon. In\n"
           "this case, the provided string will be considered the expected output,\n"
           "and no input will passed to the Brainfuck program under test.\n\n");

    printf("OPTIONS:\n\n");
    printf("-e <elitism>       Defines how many of the best Brainfuck programs are\n"
           "                   selected from the population for each cycle of the\n"
           "                   evolution process, for breeding & mutation (0.0 to\n"
           "                   1.0, e.g. 0.25 means the top 25%% will be selected).\n"
           "                   Default is %.2f.\n\n", DEFAULT_ELITISM);

    printf("-c <crossover>     Defines how many of the selected 'elite' Brainfuck\n"
           "                   programs will be 'bred' (randomly mixed/combined)\n"
           "                   with other randomly selected programs (0.0 to 1.0,\n"
           "                   e.g. 0.75 means 75%% of elite programs will be bred).\n"
           "                   Default is %.2f.\n\n", DEFAULT_CROSSOVER);

    printf("-m <mutation>      Defines how many of the selected 'elite' Brainfuck\n"
           "                   programs will be randomly mutated (0.0 to 1.0, e.g.\n"
           "                   0.44 means 44%% of elite programs will be mutated.\n"
           "                   Default is %.2f.\n\n",
           DEFAULT_MUTATION);

    printf("-s <size>          Defines the number of Brainfuck programs in the\n"
           "                   population (integer). Default is %d.\n\n",
           DEFAULT_POPSIZE);

    printf("-l <size>          Defines the maximum size in bytes of each\n"
           "                   generated Brainfuck program in the population.\n"
           "                   Default is %d.\n\n", DEFAULT_MAX_LEN);

    printf("-o <num>           Once evolution has produced a correct Brainfuck\n"
           "                   program (passes all test cases), continue evolving\n"
           "                   for an additional <num> generations to attempt to\n"
           "                   shorten the Brainfuck program by removing unnecessary\n"
           "                   characters (-1 to optimise infinitely). Default is %d.\n\n",
           DEFAULT_OPTGENS);

    printf("-r <seed>          Fixed seed value to seed random number generation.\n"
           "                   The current time (seconds) is used by default.\n\n"),

    printf("-q                 Do not show evolution progress by printing status and\n"
           "                   fittest BF program at each improved generation (default\n"
           "                   behaviour). Instead, only print the fittest Brainfuck\n"
           "                   program on termination (Ctrl-C).\n\n");

    printf("-h                 Show this text and exit.\n\n");

    printf("EXAMPLES:\n\n");

    printf("Produce a program that prints \"Hello, world!\":\n\n"
           "    %s \"Hello, world!\"\n\n", arg0);

    printf("Produce a program that prints \"Hello, world!\", with custom seed,\n"
           "mutation and elitism values provided:\n\n"
           "    %s -r 123456 -m 0.75 -e 0.1 \"Hello, world!\"\n\n", arg0);

    printf("Produce a program that prints \"true\" when input is \"1\", and prints\n"
           "\"false\" when the input is \"0\":\n\n"
           "    %s \"0:false\" \"1:true\"\n\n", arg0);
}

// Parse a float from the current 'optarg' string
static int _parse_float(char optchar, float *dest)
{
    char *endptr;

    float val = (float)strtod(optarg, &endptr);
    if (endptr != (optarg + strlen(optarg)))
    {
        bfi_log("Please provide a value between 0.0 and 1.0 for -%c option\n\n",
            optchar);
        return -1;
    }

    if ((val < 0.0) || (val > 1.0))
    {
        bfi_log("Please provide a value between 0.0 and 1.0 for -%c option\n\n",
            optchar);
        return -1;
    }

    *dest = val;
    return 0;
}

// Parse a signed int from the current 'optarg' string
static int _parse_int(char optchar, long int *dest)
{
    char *endptr = NULL;
    long int intval = strtol(optarg, &endptr, 10);
    if (endptr == optarg)
    {
        bfi_log("Invalid value provided for -%c option, expected an integer\n", optchar);
        return -1;
    }

    *dest = intval;
    return 0;
}

static int _load_testcase_from_arg(char *arg)
{
    if (MAX_NUM_TESTCASES <= _num_testcases)
    {
        bfi_log("Max. test cases exceeded");
        return -1;
    }

    evolution_testcase_t *testcase = &_testcases[_num_testcases];

    // Make 1 pass first to see if testcase string contains input AND output
    bool has_input = false;
    for (int i = 0; arg[i]; i++)
    {
        if (':' == arg[i])
        {
            has_input = true;
            break;
        }
    }

    int pos = 0u;
    if (has_input)
    {
        // Copy input data to testcase
        while(':' != arg[pos])
        {
            if (MAX_TESTCASE_INPUT_SIZE <= pos)
            {
                bfi_log("Max testcase input size (%u) exceeded", MAX_TESTCASE_INPUT_SIZE);
                return -1;
            }

            testcase->input[pos] = arg[pos];
            pos++;
        }

        testcase->input_size = pos;
        testcase->input[testcase->input_size] = 0;

        // Skip past colon
        pos++;
        int output_skip = pos;

        // Copy output data to testcase
        while(arg[pos])
        {
            if (MAX_TESTCASE_OUTPUT_SIZE <= (pos - output_skip))
            {
                bfi_log("Max testcase output size (%u) exceeded", MAX_TESTCASE_OUTPUT_SIZE);
                return -1;
            }

            testcase->output[pos - output_skip] = arg[pos];
            pos++;
        }

        testcase->output_size = pos - output_skip;
        testcase->output[testcase->output_size] = 0;
    }
    else
    {
        // Testcase only has output data
        testcase->input_size = 0;

        while(arg[pos])
        {
            if (MAX_TESTCASE_OUTPUT_SIZE <= pos)
            {
                bfi_log("Max testcase output size (%u) exceeded", MAX_TESTCASE_OUTPUT_SIZE);
                return -1;
            }

            testcase->output[pos] = arg[pos];
            pos++;
        }

        testcase->output_size = pos;
        testcase->output[testcase->output_size] = 0;
    }

    _num_testcases++;
    return 0;
}


static int _parse_args(evolution_config_t *cfg, int argc, char *argv[])
{
    char c;

    while ((c = portable_getopt(argc, argv, "hqe:c:m:s:o:l:r:")) != -1)
    {
        switch (c)
        {
            case 'h':
                help_text(argv[0]);
                return -1;

            case 'e':
                if (_parse_float('e', &cfg->elitism) < 0)
                {
                    return -1;
                }
                break;

            case 'c':
                if (_parse_float('c', &cfg->crossover) < 0)
                {
                    return -1;
                }
                break;

            case 'm':
                if (_parse_float('m', &cfg->mutation) < 0)
                {
                    return -1;
                }
                break;

            case 's':
            {
                long int popsize = 0;
                if (_parse_int('s', &popsize) < 0)
                {
                    return -1;
                }

                if (popsize <= 1)
                {
                    bfi_log("Invalid size provided for -s option, must be greater than 1\n");
                    return -1;
                }

                cfg->population_size = (unsigned int) popsize;
                break;
            }

            case 'o':
            {
                long int opt_gens = 0;
                if (_parse_int('o', &opt_gens) < 0)
                {
                    return -1;
                }

                if (opt_gens < -1)
                {
                    bfi_log("Invalid value provided for -o option, must be greater than or equal to -1\n");
                    return -1;
                }

                cfg->num_optimization_gens = (int) opt_gens;
                break;
            }

            case 'l':
            {
                long int max_len = 0;
                if (_parse_int('l', &max_len) < 0)
                {
                    return -1;
                }

                if (max_len <= 1)
                {
                    bfi_log("Invalid value provided for -l option, must be 1 or greater\n");
                    return -1;
                }

                cfg->max_program_size = (size_t) max_len;
                break;
            }

            case 'r':
            {
                long int lseed = 0;
                if (_parse_int('r', &lseed) < 0)
                {
                    return -1;
                }

                if (lseed < 0)
                {
                    bfi_log("Invalid value provided for -r option, must be greater than 0\n");
                    return -1;
                }

                _seed = (unsigned int) lseed;
                _seed_provided = true;
                break;
            }

            case 'q':
                cfg->quiet = true;
                break;

            case '?':
                bfi_log("Unrecognized option '%c'", c);
                return -1;
                break;
        }
    }

    if (argv[optind] == NULL)
    {
        help_text(argv[0]);
        return -1;
    }

    while (argv[optind] != NULL)
    {
        if (_load_testcase_from_arg(argv[optind]) < 0)
        {
            return -1;
        }

        optind++;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    time_t t;

    evolution_config_t config = {DEFAULT_ELITISM, DEFAULT_CROSSOVER, DEFAULT_MUTATION,
                                 DEFAULT_POPSIZE, DEFAULT_MAX_LEN, DEFAULT_OPTGENS, false};

    if (_parse_args(&config, argc, argv) < 0)
    {
        return -1;
    }

    bfi_log("successfully loaded %u test case(s)", _num_testcases);

    unsigned int seedval = (_seed_provided) ? _seed : (unsigned int) time(&t);
    pcg32_seed(seedval);
    bfi_log("random seed: %u", seedval);

    // allocate space for output
    size_t output_size = sizeof(evolution_output_t) + config.max_program_size + 1;
    evolution_output_t *output = malloc(output_size);
    if (NULL == output)
    {
        bfi_log("Failed to allocate %zu bytes", output_size);
        return -1;
    }

    uint64_t start_time = ms_since_epoch();

    // Runs until a BF program with fitness of 0 (best fitness) is produced, or until Ctrl-C
    int ret = evolve_bf_program(_testcases, _num_testcases, &config, output);
    if (0 != ret)
    {
        return ret;
    }

    uint64_t ms_elapsed = ms_since_epoch() - start_time;
    double seconds_elapsed = (((double) ms_elapsed) / 1000.0);
    uint64_t ex_per_sec = (output->num_bf_programs / ms_elapsed) * 1000u;

    printf("\n\nTotal runtime                      : %.2f seconds\n", seconds_elapsed);
    // Print seed again, so it's easy to grab after evolution has finished

    char countbuf[32];
    hrcount(output->num_bf_programs, countbuf, sizeof(countbuf));
    char ratebuf[32];
    hrcount(ex_per_sec, ratebuf, sizeof(ratebuf));

    printf("Total BF programs created/executed : %s (%s per second)\n", countbuf, ratebuf);
    printf("random seed                        : %u\n", seedval);
    printf("Best BF program                    : %s\n\n", output->bf_program);

    free(output);
}
