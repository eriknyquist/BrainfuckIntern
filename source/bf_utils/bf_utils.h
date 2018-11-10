/**
 * Brainfuck Intern
 *
 * Copyright 2018 Erik Nyquist <eknyquist@gmail.com>
 * (See LICENSE)
 */

#ifndef BF_UTILS_H
#define BF_UTILS_H

/**
 * Interpret a BF program and place the output (if any) in 'output'
 *
 * @param  input             BF string to interpret
 * @param  output            location to store output
 * @param  output_size       maximum number of output characters
 * @param  max_instructions  maximum number of instructions to execute.
 *                           0 for infinite.
 * @return -1 if interpretation failed, or max. number of output characters
 *         exceeded, or max. number of instructions exceeded
 */
int bf_interpret(char *input, char *output, int max_output,
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

#endif
