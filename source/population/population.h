/**
 * Brainfuck Intern
 *
 * Copyright 2018 Erik Nyquist <eknyquist@gmail.com>
 * (See LICENSE)
 */

#ifndef POPULATION_H
#define POPULATION_H

int population_evolve(char *target, int num_items, float crossover,
        float elitism, float mutation, int opt_gens, int max_len);

#endif
