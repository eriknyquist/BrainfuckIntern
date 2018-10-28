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
#include "evolution.h"
#include "common.h"

#define TOURNAMENT_SIZE  (24)

/* Convert number of items to total size in bytes */
#define I2B(i) (i * (sizeof(organism_wrapper_t) + config->item_size))

/* Convert item index to an item pointer */
#define IBYTES(x) ((organism_wrapper_t *)&(((char *)pop)[I2B((x))]))

/* Helper macro to access an organism in the current population buffer */
#define ACTIVE_POP(i) IBYTES(i + (config->num_items * popbuf))

/* Helper macro to access an organism in the next population buffer */
#define NEXT_POP(i) IBYTES(i + (config->num_items * !popbuf))

#define MINVAL(x, y) (((x) < (y)) ? x : y)

static volatile short stopped;

static short popbuf;
static int nextpos;
static int elite_border;
static organism_wrapper_t *best_item;

static evolution_cfg_t *config;
organism_wrapper_t *pop = NULL;

static organism_wrapper_t *tournament(void)
{
    organism_wrapper_t *curr;
    organism_wrapper_t *best;
    int org;

    org = randrange(0, config->num_items - 1);
    best = ACTIVE_POP(org);

    for (int i = 1; i < TOURNAMENT_SIZE; i++) {
        org = randrange(0, config->num_items - 1);
        curr = ACTIVE_POP(org);

        if (curr->fitness < best->fitness)
            best = curr;
    }

    return best;
}

static int cmporgs(const void *a, const void *b)
{
    return ((organism_wrapper_t *)a)->fitness - ((organism_wrapper_t *)b)->fitness;
}

static void sort_population(organism_wrapper_t *first)
{
    qsort(first, config->num_items, sizeof(organism_wrapper_t) + config->item_size, cmporgs);
}

static evolution_status_e initialize(void)
{
    size_t popsize_bytes;

    /* Allocate memory for evolution process */
    popsize_bytes = (I2B(1) * config->num_items * 2) + I2B(1);

    if ((pop = malloc(popsize_bytes)) == NULL) {
        return EVOLUTION_STATUS_MEMORY_ERROR;
    }

    memset(pop, 0, popsize_bytes);
    popbuf = 0;
    best_item = (organism_wrapper_t *)(((char *)pop) + (popsize_bytes - I2B(1)));

    /* Generate initial population of random organisms */
    for (int i = 0; i < config->num_items; i++) {
        config->random(ACTIVE_POP(i)->data);
        ACTIVE_POP(i)->fitness = config->assess(ACTIVE_POP(i)->data);
    }

    sort_population(ACTIVE_POP(0));
    return EVOLUTION_STATUS_SUCCESS;
}

static void cleanup(void)
{
    free(pop);
    pop = NULL;
}

static evolution_status_e evolve(void)
{
    organism_wrapper_t *p1, *p2, *c1, *c2;
    int remaining;
    int i;

    for (i = 0; i < elite_border; i++) {
        if (nextpos >= (config->num_items - 1)) {
            /* Finish if next population is full */
            break;
        }

        int new = 0;

        /* Pick two elite organisms */
        p1 = ACTIVE_POP(i);
        if (config->check(p1->data) != 0) {
            continue;
        }

        p2 = p1;
        while ((p2 == p1) || (config->check(p2->data) != 0))
            p2 = tournament();

        /* Locations for new organisms in next population */
        c1 = NEXT_POP(nextpos++);
        c2 = NEXT_POP(nextpos++);

        memset(c1, 0, I2B(2));

        if (randfloat() <= config->crossover) {
            /* Breed two elite organisms */
            config->breed(p1->data, p2->data, c1->data, c2->data);
            new++;
        } else {
            /* Copy two elite organisms without breeding */
            memcpy(c1, p1, I2B(1));
            memcpy(c2, p2, I2B(1));
        }

        if (randfloat() <= config->mutation) {
            /* Mutate both new organisms */
            if (config->mutate(c1->data) != 0)
                return EVOLUTION_STATUS_USER_ERROR;

            if (config->mutate(c2->data) != 0)
                return EVOLUTION_STATUS_USER_ERROR;;

            new++;
        }

        if (new > 0) {
            c1->fitness = config->assess(c1->data);
            c2->fitness = config->assess(c2->data);
        }
    }

    remaining = config->num_items - nextpos;
    if (remaining > 0) {
        i = MINVAL(elite_border, config->num_items - remaining);
        memcpy(NEXT_POP(nextpos), ACTIVE_POP(i), I2B((remaining)));
    }

    return EVOLUTION_STATUS_SUCCESS;
}

static int float_inrange(float val)
{
    return (val >= 0.0) && (val <= 1.0);
}

/* Dummy check handler (to avoid always checking if it's NULL */
static int dummy_check(void *data)
{
    (void)data;
    return 0;
}

/* Dummy on_evolve handler (to avoid always checking if it's NULL */
static int dummy_on_evolve(void *item, uint32_t fitness, uint32_t generation)
{
    (void)item;
    (void)fitness;
    (void)generation;
    return 0;
}

/* Dummy on_finish handler (to avoid always checking if it's NULL */
static void dummy_on_finish(void *item, uint32_t fitness, uint32_t generation)
{
    (void)item;
    (void)fitness;
    (void)generation;
}

static evolution_status_e check_config(evolution_cfg_t *cfg)
{

    if ((cfg->random == NULL) || (cfg->assess == NULL)
            || (cfg->mutate == NULL) || (cfg->breed == NULL)) {
        return EVOLUTION_STATUS_INVALID_DATA;
    }

    if ((cfg->item_size <= 0) || (cfg->num_items <= 0)) {
        return EVOLUTION_STATUS_INVALID_DATA;
    }

    if (!float_inrange(cfg->elitism) || !float_inrange(cfg->mutation)
            || !float_inrange(cfg->crossover)) {
        return EVOLUTION_STATUS_INVALID_DATA;
    }

    if (cfg->check == NULL)
    {
        cfg->check = dummy_check;
    }

    if (cfg->on_evolve == NULL)
    {
        cfg->on_evolve = dummy_on_evolve;
    }

    if (cfg->on_finish == NULL)
    {
        cfg->on_finish = dummy_on_finish;
    }

    return EVOLUTION_STATUS_SUCCESS;
}

evolution_status_e evolution_evolve(evolution_cfg_t *evolution_cfg)
{
    evolution_status_e ret = EVOLUTION_STATUS_SUCCESS;
    uint32_t gen;

    if ((ret = check_config(evolution_cfg)) != EVOLUTION_STATUS_SUCCESS) {
        return ret;
    }

    config = evolution_cfg;
    elite_border = (int)((float)config->num_items * config->elitism);

    if ((ret = initialize()) != EVOLUTION_STATUS_SUCCESS) {
        return ret;
    }

    memcpy(best_item, ACTIVE_POP(0), I2B(1));
    gen = 1;

    while (ACTIVE_POP(0)->fitness > 0) {
        if (stopped) {
            stopped = 0;
            break;
        }

        if ((ret = evolve()) != EVOLUTION_STATUS_SUCCESS) {
            break;
        }

        popbuf = !popbuf;
        nextpos = 0;

        sort_population(ACTIVE_POP(0));

        if (ACTIVE_POP(0)->fitness < best_item->fitness) {
            memcpy(best_item, ACTIVE_POP(0), I2B(1));

            if (config->on_evolve(best_item->data, best_item->fitness, gen)
                    != 0) {
                break;
            }
        }

        gen++;
    }

    config->on_finish(best_item->data, best_item->fitness, gen);
    cleanup();
    return ret;
}

evolution_status_e evolution_stop(void)
{
    stopped = 1;
    return EVOLUTION_STATUS_SUCCESS;
}

void evolution_get_err_str(evolution_status_e err, char *buf, size_t size)
{
    const char *msg;

    switch (err) {
        case EVOLUTION_STATUS_SUCCESS:
            msg = "Success";
        break;
        case EVOLUTION_STATUS_INVALID_DATA:
            msg = "Invalid configuration data provided";
        break;
        case EVOLUTION_STATUS_MEMORY_ERROR:
            msg = "Unable to allocate memory";
        break;
        case EVOLUTION_STATUS_USER_ERROR:
            msg = "User handler returned a non-zero value";
        break;
        case EVOLUTION_STATUS_ERROR:
            msg = "Error";
        break;
        default:
            msg = "Unknown error";
    }

    memcpy(buf, msg, size);
}
