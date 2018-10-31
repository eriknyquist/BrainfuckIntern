/**
 * Brainfuck Intern
 *
 * Copyright 2018 Erik Nyquist <eknyquist@gmail.com>
 * (See LICENSE)
 */

#ifndef POPULATION_H
#define POPULATION_H

/**
 * Configure evolution and start the evolution process
 *
 * @param target       evolution target. The goal is to evolve a brainfuck
 *                     program that prints this string
 * @param num_items    number of brainfuck programs in the population
 * @param max_len      maximum length of generated brainfuck programs in bytes
 * @param crossover    crossover (breeding) factor, 0.0-1.0 (see evolution.h)
 *                     breeding
 * @param elitism      elitism factor, 0.0-1.0 (see evolution.h)
 * @param mutation     mutation factor, 0.0-1.0 (see evolution.h)
 * @param opt_gens     Number of additional generations to evolve to optimise
 *                     for shorter brainfuck programs, once the target string
 *                     has been reached. 0 means infinite additional
 *                     generations (in this case, bfintern will never terminate
 *                     unless terminated by ther user with Ctrl-C)
 *
 * @return  0 if evolution completes successfully
 */
int population_evolve(char *target, int num_items, int max_len,
        float crossover, float elitism, float mutation, int opt_gens);

#endif
