/**
 * Brainfuck Intern
 *
 * Copyright 2018 Erik Nyquist <eknyquist@gmail.com>
 * (See LICENSE)
 */

#ifndef EVOLUTION_H
#define EVOLUTION_H

#include <stdint.h>

typedef struct {
    uint32_t fitness;
    char data[];
} organism_wrapper_t;

#define EVOLUTION_MAX_ERR_STR_LEN  (64)

// Status codes returned by evolution.c
typedef enum {
    // Operation successful
    EVOLUTION_STATUS_SUCCESS = 0,

    // Invalid data passed to function
    EVOLUTION_STATUS_INVALID_DATA,

    // Unable to allocate memory
    EVOLUTION_STATUS_MEMORY_ERROR,

    // User interface function returned a non-zero value
    EVOLUTION_STATUS_USER_ERROR,

    // Any other error
    EVOLUTION_STATUS_ERROR
} evolution_status_e;

// Evolution configuration structure
typedef struct {
    // Size in bytes of a single item
    int item_size;

    // Number of items in the population
    int num_items;

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

   /**
    * Create a randomly-generated item
    *
    * @param  out   pointer to location for generated item
    */
    void (*random)(void *out);

   /**
    * Assess the fitness of an item
    *
    * @param   item  pointer to item to assess
    * @return  fitness value (lower is better; 0 is the most fit)
    */
    uint32_t (*assess)(void *item);

   /**
    * Randomly mutate an item
    *
    * @param   item  pointer to item to mutate
    * @return  0 if mutation was successful. If any nonzero value is returned,
    *          evolution will stop
    */
    int (*mutate)(void *item);

   /**
    * Breed two items ("parent" items) to create two new items ("child" items)
    *
    * @param  parent1  pointer to first parent item
    * @param  parent2  pointer to second parent item
    * @param  child1   pointer to location for first generated child item
    * @param  child2   pointer to location for second generated child item
    */
    void (*breed)(void *parent1, void *parent2, void *child1, void *child2);
    
   /**
    * Called on each item before any breeding or mutation happens. If this
    * function returns 0, then the item will be assumed safe to breed/mutate.
    * Any non-zero value means the item will not be bred or mutated.
    *
    * @param   item  pointer to item to check
    * @return  0 if item is safe to breed/muate.
    */
    int (*check)(void *item);

   /**
    * Called whenever a population evolves a new "fittest" item, that is, an
    * item with a fitness level lower than any item seen in previos generations
    *
    * @param   item        pointer to new fittest item
    * @param   fitness     item fitness value
    * @param   generation  generation number
    * @return  0 if evolution should continue. If any nonzero value is returned,
    *          evolution will stop
    */
    int (*on_evolve)(void *item, uint32_t fitness, uint32_t generation);

   /**
    * Called whenever evolution finishes, either by the evolution_stop function
    * being called or because evolution has produced an item with a fitness of 0
    *
    * @param   item        pointer to the fittest item at the time of finishing
    * @param   fitness     item fitness value
    * @param   generation  generation number
    */
    void (*on_finish)(void *item, uint32_t fitness, uint32_t generation);
} evolution_cfg_t;

/**
 * Starts the evolution process, and returns when the evolution process completes
 * (or when evolution_stop is called)
 *
 * @param evolution_cfg  pointer to a populated evolution configuration
 *                       structure. @see evolution_cfg_t
 * @return EVOLUTION_STATUS_SUCCESS if evolution completed successfully, or
 *         was terminated by a call to evolution_stop
 */
evolution_status_e evolution_start(evolution_cfg_t *evolution_cfg);

/**
 * Stops the evolution process.
 *
 * @return EVOLUTION_STATUS_SUCCESS if evolution was stopped successfully
 */
evolution_status_e evolution_stop(void);

/**
 * Provides a descriptive string that corresponds to the provided error code
 *
 * @param err  the error code
 * @return pointer to error string
 */
const char *evolution_get_err_str(evolution_status_e err);

#endif
