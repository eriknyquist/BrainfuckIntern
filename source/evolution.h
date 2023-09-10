/**
 * Brainfuck Intern (Erik Nyquist, 2023)
 * <eknyquist@gmail.com>)
 *
 * Uses a genetic algorithm to generate poorly written Brainfuck programs
 * that match user-provided test cases, by pure brute-force.
 */

#ifndef EVOLUTION_H
#define EVOLUTION_H

#define MAX_TESTCASE_INPUT_SIZE   (128u)
#define MAX_TESTCASE_OUTPUT_SIZE  (128u)

#include <stdbool.h>


/**
 * Holds a single test case for the evolution process
 */
typedef struct
{
    // Input to pass to BF interpreter
    char input[MAX_TESTCASE_INPUT_SIZE];
    size_t input_size;

    // Expected output. If BF interpreter produces this output, the testcase matches.
    char output[MAX_TESTCASE_OUTPUT_SIZE];
    size_t output_size;
} evolution_testcase_t;


/**
 * Output of the evolve_bf_program function
 */
typedef struct
{
    // Total number of BF programs created & executed
    unsigned int num_bf_programs;

    // The final best BF program
    char bf_program[];
} evolution_output_t;


/**
 * Holds all configurable options for evolution
 */
typedef struct
{
   /* Evolution elitism, from 0.0 to 1.0. Specifies how much of the total
    * population will be "selected" for breeding/mutation, e.g. 0.2 means the
    * top 20% of the population will be selected */
    float elitism;

   /* Evolution crossover, from 0.0 to 1.0. Specifies how much many of the
    * selected items will be bred to create new items, e.g. 0.2 means 20% of
    * selected items will be bred */
    float crossover;

   /* Evolution mutation, from 0.0 to 1.0. Specifies how many of the selected
    * items will be mutated, e.g. 0.2 means 20% of selected items will be
    * mutated */
    float mutation;

    /* Number of BF programs in the 'population' */
    unsigned int population_size;

    /* Maximum size (in bytes) of generated BF programs */
    size_t max_program_size;

    /* Number of generations to continue even after a BF program has been produced
     * which passes all test cases. -1 to continue forever. */
    int num_optimization_gens;

    /* If false, print status + fittest BF program each time a new fittest BF
     * program is produced. Otherwise, only print the fittest BF program on termination. */
    bool quiet;
} evolution_config_t;


/**
 * Evolve a BF program that passes all provided test cases. All output is provided
 * via stdout.
 *
 * @param testcases      Pointer to list of test cases that target BF program must pass
 * @param num_testcases  Number of test cases in the list
 * @param config         Pointer to configuration options
 * @param output         Pointer to location to store evolution result
 *
 * @return 0 if successful, -1 if an error occurred
 */
int evolve_bf_program(evolution_testcase_t *testcases, unsigned int num_testcases,
                      evolution_config_t *config, evolution_output_t *output);

#endif // EVOLUTION_H
