/**
 * Brainfuck Intern
 *
 * Copyright 2018 Erik Nyquist <eknyquist@gmail.com>
 * (See LICENSE)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#include "evolution.h"
#include "common.h"
#include "bf_utils.h"

#define ORG_MAX_LEN      (4096)
#define ORG_MIN_LEN      (12)
#define MAX_HRSIZE_LEN   (12)
#define MUTATE_STR_SIZE  (64)
#define ORG_MAX_OUTPUT   (256)
#define MOST_UNFIT       (1024 * 1024 * 1024)

static int optimise_gens;
static int org_max_len;

/* Possible mutations for organisms during evolution */
typedef enum {
    /* Pick a random character and move it to a new random location */
    MUTATE_MOVE = 0,

    /* Randomly copy a character */
    MUTATE_COPY,

    /* Randomly add a character */
    MUTATE_ADD_CHAR,

    /* Randomly add some more characters */
    MUTATE_ADD_STR,

    /* Change a random character */
    MUTATE_CHANGE,

    /* Snip a few characters off the beginning/end */
    MUTATE_REMOVE_RANDOM,

    /* Randomly remove a random number of characters */
    MUTATE_REMOVE_BLOCK,

    NUM_MUTATIONS
} mutation_e;

/* A randomly evolved brainfuck program */
typedef struct organism {
    int program_len;
    int output_len;
    char output[ORG_MAX_OUTPUT];
    char program[];
} organism_t;

static char *target_output;
static int target_output_len;

static void sighandler(int signo)
{
    if (signo == SIGINT) {
        bfi_log("\nQuitting");
        evolution_stop();
    }
}

static uint32_t unfit(organism_t *org)
{
    org->output[0] = 0;
    org->output_len = 0;
    return MOST_UNFIT;
}

/**
 * Run BF program through interpreter and assess fitness level
 *
 * @param organism  organism to assess. 'program' and 'program_len' must be set.
 */
static uint32_t assess(void *data)
{
    uint32_t fitness;
    organism_t *org = (organism_t *)data;

    if (org->program_len < ORG_MIN_LEN) {
        return unfit(org);
    }

    int len = bf_interpret(org->program, org->output, ORG_MAX_OUTPUT);
    if (len < 0) {
        return unfit(org);
    }

    org->output[len] = 0;
    org->output_len = strlen(org->output);
    if ((len < 0) || (org->output_len <= 0)) {
        return unfit(org);
    }

    fitness = 0;

    /* Add a penalty for every character too many/too few that the BF
     * program generates */
    if (org->output_len != target_output_len) {
        int diff = abs(org->output_len - target_output_len);
        fitness += (diff * 1000000);
    }

    /* Add a penalty for each character in the output generated by BF program
     * that differs from the character at the same index in the desired
     * output string */
    for (int i = 0; target_output[i] && org->output[i]; i++) {
        if (org->output[i] != target_output[i]) {
            fitness += ((abs(target_output[i] - org->output[i])) * 10000);
            fitness += (target_output_len - i) * 1000;
        }
    }

    if ((optimise_gens >= 0) && (fitness == 0)) {
        fitness = org->program_len;

        if (optimise_gens > 0) {
            optimise_gens = (optimise_gens == 1) ? -1: optimise_gens - 1;
        }
    }

    return fitness;
}

static void breed(void *parent1, void *parent2, void *child1, void *child2)
{
    /* Cast pointers */
    organism_t *p1 = (organism_t *)parent1;
    organism_t *p2 = (organism_t *)parent2;
    organism_t *c1 = (organism_t *)child1;
    organism_t *c2 = (organism_t *)child2;

    /* Split each parent randomly between the 1st and 3rd quarter */
    int p1i = randrange(p1->program_len / 4, (p1->program_len / 4) * 3);
    int p2i = randrange(p2->program_len / 4, (p2->program_len / 4) * 3);

    if (((p1i + p2i) >= org_max_len) ||
        (((org_max_len - p1i) + (org_max_len)) >= org_max_len)) {
        p1i = p1->program_len / 2;
        p2i = p2->program_len / 2;
    }
    /* Copy 1st half of p1 to 1st half of c1 */
    memcpy(c1->program, p1->program, p1i);

    /* Copy 2nd half of p2 to 2nd half of c1 */
    memcpy(c1->program + p1i, p2->program + p2i, p2->program_len - p2i);

    c1->program[p1i + (p2->program_len - p2i)] = 0;
    c1->program_len = p1i + (p2->program_len - p2i);

    /* Copy 1st half of p2 to 1st half of c2 */
    memcpy(c2->program, p2->program, p2i);

    /* Copy 2nd half of p1 to 2nd half of c2 */
    memcpy(c2->program + p2i, p1->program + p1i, p1->program_len - p1i);

    c2->program[p2i + (p1->program_len - p1i)] = 0;
    c2->program_len = p2i + (p1->program_len - p1i);
}

/**
 * Insert substring into organism program string
 *
 * @param org   organism that holds the program string
 * @param sub   the substring to insert
 * @param size  substring size
 * @param i     index in program string to insert after
 */
static int insert_substring(organism_t *org, char *sub, int size, int i)
{
    char buf[ORG_MAX_LEN];

    if (i >= org->program_len)
        return -1;

    if ((org->program_len + size + 1) > org_max_len)
        return -1;

    /* Save chunk after index */
    memcpy(buf, org->program + i, org->program_len - i);

    /* Insert substring */
    memcpy(org->program + i, sub, size);

    /* Copy chunk back after substring */
    memcpy(org->program + i + size, buf, org->program_len - i);

    org->program_len += size;
    org->program[org->program_len] = 0;

    return 0;
}

/**
 * Remove a slice from an organism's program string
 *
 * @param org   organism that holds the program string
 * @param i     first index of slice in program string
 * @param size  number of characters in slice
 */
static void snip_slice(organism_t *org, int i, int size)
{
    if ((size + 1) > org->program_len) {
        // Don't snip if it means removing all the characters
        return;
    }

    /* Move remaining chunk back to cover slice */
    memcpy(org->program + i, org->program + i + size ,
        org->program_len - (i + size));

    org->program_len -= size;
    org->program[org->program_len] = 0;
}

static int mutate(void *data)
{
    organism_t *org = (organism_t *)data;
    char buf[MUTATE_STR_SIZE];
    int size;
    int quart;
    int upper;
    int len;

    int j;
    int i = randrange(1, org->program_len - 1);
    int m = randrange(0, NUM_MUTATIONS - 1);
    char c;

    switch(m) {
        /* Move a random character to a new location */
        case MUTATE_MOVE:
            j = randrange_except(1, org->program_len - 1, i);
            c = org->program[i - 1];

            snip_slice(org, i - 1, 1);

            if (insert_substring(org, &c, 1, j - 1) < 0) {
                // Not enough space to insert
                return 0;
            }

        break;

        /* Randomly copy a character */
        case MUTATE_COPY:
            j = randrange_except(1, org->program_len - 1, i);
            c = org->program[i - 1];

            if (insert_substring(org, &c, 1, j - 1) < 0) {
                // Not enough space to insert
                return 0;
            }

        break;

        /* Randomly add a character */
        case MUTATE_ADD_CHAR:
            c = bf_rand_sym();

            if (insert_substring(org, &c, 1, i - 1) < 0) {
                // Not enough space to insert
                return 0;
            }

        break;

        /* Randomly add some more characters */
        case MUTATE_ADD_STR:
            size = bf_rand_syms(buf, 1, MUTATE_STR_SIZE);

            if (insert_substring(org, buf, size, i - 1) < 0) {
                // Not enough space to insert
                return 0;
            }

        break;

        /* Change a random character */
        case MUTATE_CHANGE:
            org->program[i - 1] = bf_rand_sym();

        break;

        /* Snip characters off the beginning/end */
        case MUTATE_REMOVE_RANDOM:
            len = randrange(1, org->program_len / 4);

            for (j = 0; j < len; j++) {
                i = randrange(0, org->program_len - 1);
                snip_slice(org, i, 1);
            }

        break;

        /* Randomly remove a contigious block of characters */
        case MUTATE_REMOVE_BLOCK:
            quart = org->program_len / 4;

            if ((i + quart) >= org->program_len) {
                upper = org->program_len - 1;
             } else {
                upper = i + quart;
             }

            len = randrange(0, upper - i);
            snip_slice(org, i, len);
        break;
    }

    return 0;
}

static void gen_random(void *data)
{
    organism_t *org = (organism_t *)data;
    org->program_len = bf_rand_syms(org->program, ORG_MIN_LEN, org_max_len / 4);
}

static int on_evolve(void *data, uint32_t fitness, uint32_t generation)
{
    organism_t *org = (organism_t *)data;

    bfi_log("generation=%u, fitness=%u, output=%s", generation, fitness,
            org->output);
    return 0;
}

static void on_finish(void *data, uint32_t fitness, uint32_t generation)
{
    organism_t *org = (organism_t *)data;
    printf("\n\nBest program: %s\n\nOutput: %s\n\n", org->program, org->output);
}

int check(void *data)
{
    organism_t *org = (organism_t *)data;

    return org->program_len < ORG_MIN_LEN;
}

int population_evolve(char *target, int num_items, float crossover,
        float elitism, float mutation, int opt_gens, int max_len)
{
    char err_buf[EVOLUTION_MAX_ERR_STR_LEN];

    if ((max_len < ORG_MIN_LEN) || (max_len > ORG_MAX_LEN)) {
        bfi_log("Max. brainfuck program length must be between %d-%d\n",
                ORG_MIN_LEN, ORG_MAX_LEN);
        return -1;
    }

    optimise_gens = opt_gens;
    evolution_cfg_t cfg;
    evolution_status_e ret;

    target_output = target;
    target_output_len = strlen(target);

    cfg.random = gen_random;
    cfg.breed = breed;
    cfg.assess = assess;
    cfg.mutate = mutate;
    cfg.on_evolve = on_evolve;
    cfg.on_finish = on_finish;
    cfg.check = check;

    cfg.crossover = crossover;
    cfg.elitism = elitism;
    cfg.mutation = mutation;

    cfg.num_items = num_items;
    cfg.item_size = sizeof(organism_t) + max_len;
    org_max_len = max_len;

    if (signal(SIGINT, sighandler) == SIG_ERR) {
        bfi_log("Can't catch SIGINT\n");
        return -1;
    }

    if ((ret = evolution_evolve(&cfg)) != EVOLUTION_STATUS_SUCCESS) {
        evolution_get_err_str(ret, err_buf, sizeof(err_buf));
        bfi_log("Evolution error: %s", err_buf);
        return -1;
    }

    return 0;
}
