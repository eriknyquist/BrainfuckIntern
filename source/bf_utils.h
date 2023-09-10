/**
 * Brainfuck Intern (Erik Nyquist, 2023)
 * <eknyquist@gmail.com>)
 *
 * Uses a genetic algorithm to generate poorly written Brainfuck programs
 * that match user-provided test cases, by pure brute-force.
 */

#ifndef BF_UTILS_H
#define BF_UTILS_H

/**
 * Interpret a BF program and place the output (if any) in 'output'
 *
 * @param  prog              BF string to interpret
 * @param  input             pointer to input data (may be NULL)
 * @param  input_len         size of input data
 * @param  output            location to store output
 * @param  max_output        maximum number of output characters
 * @param  max_instructions  maximum number of instructions to execute.
 *                           0 for infinite.
 * @return -1 if interpretation failed, or max. number of output characters
 *         exceeded, or max. number of instructions exceeded
 */
int bf_interpret(char *prog, char *input, size_t input_len, char *output, size_t max_output,
                 int max_instructions);

/**
 * Generate a string of randomly-selected BF symbols
 *
 * @param  output    location to store generated BF symbols
 * @param  min_size  minimum number of BF symbols to generate
 * @param  max_size  maximum number of BF symbols to generate
 * @return number of characters generated
 */
int bf_rand_syms(char *output, int min_size, int max_size);

/**
 * Generate a random BF character
 *
 * @return  random BF character
 */
char bf_rand_sym(void);

#endif // BF_UTILS_H
