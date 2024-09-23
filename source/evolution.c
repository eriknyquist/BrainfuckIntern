/**
 * Brainfuck Intern (Erik Nyquist, 2023)
 * <eknyquist@gmail.com>)
 *
 * Uses a genetic algorithm to generate poorly written Brainfuck programs
 * that match user-provided test cases, by pure brute-force.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "bf_utils.h"
#include "common.h"
#include "evolution.h"

#if WINDOWS
#include <windows.h>
#else
#include <signal.h>
#endif


#define MUTATE_STR_SIZE              (64)

// Number of items involved in a tournament
#define TOURNAMENT_SIZE  (6)

// If a BF program executes more than this many instructions, it will be considered timed out
#define MAX_INSTRUCTIONS_EXEC (100000)

// Size of a single BF program in the population
#define BF_PROG_SIZE_BYTES (sizeof(bf_program_t) + _config.max_program_size + 1u)

#define BF_MIN_PROG_SIZE (12)

// Convert a population index to a pointer to the corresponding BF program
#define BF_PROG_INDEX(i) ((bf_program_t *) (((uint8_t *) _population) + (BF_PROG_SIZE_BYTES * (i))))

// Get a pointer to a BF program by index, in the active population
#define ACTIVE_POP(i) BF_PROG_INDEX(i + (_active_pop_index * _config.population_size))

// Get a pointer to a BF program by index, in the next population
#define NEXT_POP(i)   BF_PROG_INDEX(i + ((!_active_pop_index) * _config.population_size))

#define MINVAL(x, y) (((x) < (y)) ? x : y)


/**
 * Holds the text for a single BF program, + a fitness score (lower is better)
 */
typedef struct
{
    uint32_t fitness;
    size_t program_len;
    char text[];
} bf_program_t;

/**
 * Enumerates possible mutations for organisms during evolution
 */
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

    /* Remove 1 or more contiguous characters from a random location */
    MUTATE_REMOVE_BLOCK,

    /* Remove 1 or more random characters non-contiguous characters from random locations */
    MUTATE_REMOVE_RANDOM,

    /* Swap the positions of two random characters */
    MUTATE_SWAP,

    NUM_MUTATIONS
} mutation_e;


static volatile bool _stopped = false;
static uint32_t _elite_border = 0u;
static uint32_t _active_pop_index = 0u;
static uint32_t _generation = 0u;

static evolution_testcase_t *_testcases = NULL;
static unsigned int _num_testcases = 0u;
static bf_program_t *_population = NULL;
static bf_program_t *_best_item;

static bool _penalize_length = false;
static evolution_config_t _config;


#if WINDOWS
BOOL WINAPI win_sighandler(DWORD type)
{
    if (type == CTRL_C_EVENT) {
#else
static void sighandler(int signo)
{
    if (signo == SIGINT) {
#endif /* WINDOWS */
        _stopped = true;
    }
#if WINDOWS
    return TRUE;
#endif /* WINDOWS*/
}


// qsort compare function to sort BF programs by their fitness value
static int _cmporgs(const void *a, const void *b)
{
    bf_program_t *pa = (bf_program_t *)a;
    bf_program_t *pb = (bf_program_t *)b;

    if (pa->fitness > pb->fitness)
    {
        return 1;
    }
    else if (pa->fitness < pb->fitness)
    {
        return -1;
    }

    return 0;
}

// Uses qsort to sort the entire population based on fitness
static void _sort_active_population(void)
{
    qsort(ACTIVE_POP(0), _config.population_size, BF_PROG_SIZE_BYTES, _cmporgs);
}

// Assess the fitness of a BF program by running all provided test cases.
// Lower scores are better, 0 is a perfect score.
static uint32_t _assess_bf_program(bf_program_t *prog, bool penalize_length)
{
    uint32_t fitness = 0u;

    for (unsigned int i = 0u; i < _num_testcases; i++)
    {
        char output[MAX_TESTCASE_OUTPUT_SIZE];
        int len = bf_interpret(prog->text, _testcases[i].input, _testcases[i].input_size,
                               output, MAX_TESTCASE_OUTPUT_SIZE - 1u, MAX_INSTRUCTIONS_EXEC);

        if (len <= 0)
        {
            fitness += UINT32_MAX / _num_testcases;
            continue;
        }

        // Null-terminate the output
        output[len] = 0;

        /* Add a penalty for every character too many/too few that the BF
         * program generates */
        if (_testcases[i].output_size != len)
        {
            uint32_t diff = (uint32_t) abs(((int) _testcases[i].output_size) - len);
            uint32_t fitness_add = (diff * 1000000u);
            if (fitness <= (UINT32_MAX - fitness_add))
            {
                fitness += fitness_add;
            }
            else
            {
                fitness = UINT32_MAX;
            }
        }

        /* Add a penalty for each character in the output generated by BF program
         * that differs from the character at the same index in the desired
         * output string from the test case */
        int smallest_size = MINVAL(_testcases[i].output_size, len);
        for (int j = 0; j < smallest_size; j++)
        {
            char a = _testcases[i].output[j];
            char b = output[j];
            //uint32_t fitness_add = (uint32_t) (abs(a - b) * ((smallest_size - j) + 1)) * 1000u;
            uint32_t fitness_add = (uint32_t) (abs(a - b) * 1000u);
            if (fitness <= (UINT32_MAX - fitness_add))
            {
                fitness += fitness_add;
            }
            else
            {
                fitness = UINT32_MAX;
            }
        }
    }

    if (penalize_length && (fitness <= (UINT32_MAX - prog->program_len)))
    {
        fitness += prog->program_len;
    }

    return fitness;
}

// Return the fittest of #TOURNAMENT_SIZE randomly selected organisms
static bf_program_t *_tournament(void)
{
    bf_program_t *curr;
    bf_program_t *best;
    uint32_t org;

    org = randrange(0u, _config.population_size - 1u);
    best = curr = ACTIVE_POP(org);

    // Run all but the last tournament match
    uint32_t tournament_size = MINVAL(TOURNAMENT_SIZE, _config.population_size);
    for (uint32_t i = 1u; i <= tournament_size; i++)
    {
        org = randrange(0u, _config.population_size - 1u);
        curr = ACTIVE_POP(org);

        if (curr->fitness < best->fitness)
        {
            best = curr;
        }
    }

    return best;
}

// Create 2 new BF programs by randomly combining slices from 2 existing BF programs
static int _breed(bf_program_t *p1, bf_program_t *p2, bf_program_t *c1,
                  bf_program_t *c2)
{
    /* Split each parent randomly between the 1st and 3rd quarter */
    uint32_t p1i = randrange(p1->program_len / 4u, (p1->program_len / 4u) * 3u);
    uint32_t p2i = randrange(p2->program_len / 4u, (p2->program_len / 4u) * 3u);

    if (((p1i + p2i) >= _config.max_program_size) ||
        (((_config.max_program_size - p1i) + (_config.max_program_size)) >= _config.max_program_size))
    {
        p1i = p1->program_len / 2u;
        p2i = p2->program_len / 2u;
    }
    /* Copy 1st half of p1 to 1st half of c1 */
    memcpy(c1->text, p1->text, p1i);

    /* Copy 2nd half of p2 to 2nd half of c1 */
    memcpy(c1->text + p1i, p2->text + p2i, p2->program_len - p2i);
    c1->program_len = p1i + (p2->program_len - p2i);

    /* Copy 1st half of p2 to 1st half of c2 */
    memcpy(c2->text, p2->text, p2i);

    /* Copy 2nd half of p1 to 2nd half of c2 */
    memcpy(c2->text + p2i, p1->text + p1i, p1->program_len - p1i);
    c2->program_len = p2i + (p1->program_len - p1i);

    // Make sure both new programs are at least BF_MIN_PROG_SIZE
    if (BF_MIN_PROG_SIZE > c1->program_len)
    {
        int added = bf_rand_syms(c1->text + c1->program_len,
                                 BF_MIN_PROG_SIZE - c1->program_len, -1);
        if (added <= 0)
        {
            bfi_log("failed to generate random BF characters");
            return -1;
        }
        c1->program_len += added;
    }

    if (BF_MIN_PROG_SIZE > c2->program_len)
    {
        int added = bf_rand_syms(c2->text + c2->program_len,
                                 BF_MIN_PROG_SIZE - c2->program_len, -1);
        if (added < 0)
        {
            bfi_log("failed to generate random BF characters");
            return -1;
        }
        c2->program_len += added;
    }

    c1->text[c1->program_len] = 0;
    c2->text[c2->program_len] = 0;

    return 0;
}

/**
 * Insert substring into organism program string
 *
 * @param org   organism that holds the program string (BF program object)
 * @param sub   the substring to insert
 * @param size  substring size
 * @param i     index in program string to insert after
 */
static int _insert_substring(bf_program_t *org, char *sub, int size, int i)
{
    if (i >= org->program_len)
    {
        return -1;
    }

    if ((org->program_len + size) > _config.max_program_size)
    {
        return -1;
    }

    /* Move up chunk after substring location */
    memmove(org->text + i + size, org->text + i, org->program_len - i);

    /* Insert substring */
    memcpy(org->text + i, sub, size);

    org->program_len += size;
    org->text[org->program_len] = 0;

    return 0;
}

/**
 * Remove a slice from an organism's program string
 *
 * @param org   organism that holds the program string (BF program object)
 * @param i     first index of slice in program string
 * @param size  number of characters in slice
 */
static void _snip_slice(bf_program_t *org, int i, int size)
{
    if ((i + size) > org->program_len)
    {
        size = org->program_len - i;
    }

    if (0 == size)
    {
        return;
    }

    if ((size + BF_MIN_PROG_SIZE) > org->program_len)
    {
        // Don't snip if it means removing too many characters
        return;
    }

    /* Move remaining chunk back to cover slice */
    memmove(org->text + i, org->text + i + size, org->program_len - (i + size));

    org->program_len -= size;
    org->text[org->program_len] = 0;
}

static int _mutate(bf_program_t *org)
{
    char buf[MUTATE_STR_SIZE];
    int size;
    uint32_t randlen;

    uint32_t j;
    uint32_t i = randrange(1u, org->program_len);
    uint32_t m = randrange(0u, NUM_MUTATIONS - 1u);
    char c;

    switch(m)
    {
        case MUTATE_SWAP:
        {
            /* Pick two random characters and swap their positions */
            uint32_t j = randrange(1, org->program_len);
            char ci = org->text[i - 1];
            org->text[i - 1] = org->text[j - 1];
            org->text[j - 1] = ci;
        }
        break;

        /* Move a random character to a new location */
        case MUTATE_MOVE:
            j = randrange_except(1u, org->program_len, i);
            c = org->text[i - 1];

            _snip_slice(org, i - 1, 1);

            if (_insert_substring(org, &c, 1, j - 1) < 0)
            {
                // Not enough space to insert
                return 0;
            }

        break;

        /* Randomly copy a character */
        case MUTATE_COPY:
            j = randrange_except(1, org->program_len, i);
            c = org->text[i - 1];

            if (_insert_substring(org, &c, 1, j - 1) < 0)
            {
                // Not enough space to insert
                return 0;
            }

        break;

        /* Randomly add a character */
        case MUTATE_ADD_CHAR:
            c = bf_rand_sym();

            if (_insert_substring(org, &c, 1, i - 1) < 0)
            {
                // Not enough space to insert
                return 0;
            }

        break;

        /* Randomly add some more characters */
        case MUTATE_ADD_STR:
        {
            int stringlen = MINVAL(MUTATE_STR_SIZE - 1, (_config.max_program_size - org->program_len) - 1);
            if (0 < stringlen)
            {
                size = bf_rand_syms(buf, 1, stringlen);

                if (_insert_substring(org, buf, size, i - 1) < 0)
                {
                    // Not enough space to insert
                    return 0;
                }
            }
        }
        break;

        /* Change a random character */
        case MUTATE_CHANGE:
            org->text[i - 1] = bf_rand_sym();

        break;

        /* Randomly remove 1 or more contingious characters */
        case MUTATE_REMOVE_BLOCK:
            randlen = randrange(1u, org->program_len / 2u);
            i = randrange(0u, org->program_len - randlen);
            _snip_slice(org, i, randlen);

        break;

        /* Randomly remove 1 or more non-contingous characters from wherever */
        case MUTATE_REMOVE_RANDOM:
            randlen = randrange(1u, org->program_len / 2u);
            for (uint32_t count = 0u; count < randlen; count++)
            {
                i = randrange(1u, org->program_len);
                _snip_slice(org, i - 1, 1);
            }
        break;
    }

    return 0;
}

// Evolve the active population until the next population is full
static int _evolve(void)
{
    uint32_t nextpos = 0;
    uint32_t activepos = 0u;

    // Always copy over the fittest program
    //memcpy(NEXT_POP(nextpos++), ACTIVE_POP(0), BF_PROG_SIZE_BYTES);

    for (; activepos < _elite_border; activepos++)
    {
        bool new_items_added = false;

        if (nextpos >= (_config.population_size - 1u))
        {
            // next population is full, we're done with this loop
            break;
        }

        /* Pick two elite items; one based on fitness within the overall
         * population, and one based on fitness with a smaller randomly-selected group */
        bf_program_t *curr1 = ACTIVE_POP(activepos);
        bf_program_t *curr2 = curr1;

        while (curr1 == curr2)
        {
           curr2 = _tournament();
        }

        // get pointers to 2 empty items from next population
        bf_program_t *next1 = NEXT_POP(nextpos++);
        bf_program_t *next2 = NEXT_POP(nextpos++);

        if ((randfloat() <= _config.crossover) || (0u == activepos))
        {
            if (0 > _breed(curr1, curr2, next1, next2))
            {
                return -1;
            }

            new_items_added = true;
        }
        else
        {
            // Copy the 2 elite organisms as-is without breeding
            memcpy(next1, curr1, BF_PROG_SIZE_BYTES);
            memcpy(next2, curr2, BF_PROG_SIZE_BYTES);
        }

        if (randfloat() <= _config.mutation)
        {
            // Mutate both new organisms
            _mutate(next1);
            _mutate(next2);

            new_items_added = true;
        }

        if (new_items_added)
        {
            next1->fitness = _assess_bf_program(next1, _penalize_length);
            next2->fitness = _assess_bf_program(next2, _penalize_length);
        }
    }

    /* If finished evolution, and there are still items remaining in the active
     * population that we didn't get to, then just copy them over as-is to the
     * next population */
    int next_remaining = _config.population_size - nextpos;
    if (0 < next_remaining)
    {
        int copy_index = activepos;
        int copy_count = next_remaining;
        int active_remaining = _config.population_size - copy_index;

        if (active_remaining < next_remaining)
        {
            copy_count = active_remaining;
        }


        // Mutate organisms before copying
        for (int i = copy_index; i < copy_index + copy_count; i++)
        {
            if (randfloat() <= _config.mutation)
            {
                _mutate(ACTIVE_POP(i));
            }
        }

        memcpy(NEXT_POP(nextpos), ACTIVE_POP(copy_index), BF_PROG_SIZE_BYTES * copy_count);
        nextpos += copy_count;

        /* If we still haven't filled up the next population, then generate some
         * new random BF programs */
        while (nextpos < _config.population_size)
        {
            bf_program_t *prog = NEXT_POP(nextpos++);
            prog->program_len = bf_rand_syms(prog->text, BF_MIN_PROG_SIZE, _config.max_program_size);
            prog->fitness = _assess_bf_program(prog, _penalize_length);
        }
    }

    return 0;
}

/**
 * @see evolution.h
 */
int evolve_bf_program(evolution_testcase_t *testcases, unsigned int num_testcases,
                      evolution_config_t *config, evolution_output_t *output)
{
    if ((NULL == testcases) || (NULL == config) || (NULL == output) || (0u == num_testcases))
    {
        return -1;
    }

    if (2u > config->max_program_size)
    {
        bfi_log("Max. BF program size must be at least 2");
        return -1;
    }

#if WINDOWS
    if (!SetConsoleCtrlHandler(win_sighandler, TRUE)) {
#else
    if (signal(SIGINT, sighandler) == SIG_ERR) {
#endif /* WINDOWS */
        bfi_log("Can't catch Ctrl-C signal\n");
        return -1;
    }

    _testcases = testcases;
    _num_testcases = num_testcases;
    _elite_border = (unsigned int) (((float) config->population_size) * config->elitism);
    _config = *config;

    // Account for null terminator
    _config.max_program_size -= 1u;

    size_t alloc_size = ((config->population_size * BF_PROG_SIZE_BYTES) * 2u) + BF_PROG_SIZE_BYTES;

    char sizebuf[64];
    hrsize(alloc_size, sizebuf, sizeof(sizebuf));
    bfi_log("%s allocated", sizebuf);

    bfi_log("elitism=%.2f, crossover=%.2f, mutation=%.2f",
            _config.elitism, _config.crossover, _config.mutation);
    bfi_log("population_size=%u, max_program_size=%u, optimization_generations=%d",
            _config.population_size, _config.max_program_size,
            _config.num_optimization_gens);

    fflush(stdout);

    _population = malloc(alloc_size);

    if (NULL == _population)
    {
        bfi_log("Failed to allocate memory");
        return -1;
    }

    // Pre-calculate pointer to best item buffer
    _best_item = (bf_program_t *) (((uint8_t *) _population) + (BF_PROG_SIZE_BYTES * config->population_size * 2));
    _best_item->fitness = 0xffffffffu;

    // Generate initial population of completely random BF programs
    for (unsigned int i = 0; i < config->population_size; i++)
    {
        bf_program_t *prog = ACTIVE_POP(i);
        prog->program_len = bf_rand_syms(prog->text, BF_MIN_PROG_SIZE, config->max_program_size);
        prog->fitness = _assess_bf_program(prog, _penalize_length);
    }

    _sort_active_population();


    unsigned int optgen_count = 0u;  // Number of optimization generations we've done
    bool optimizing = false;         // Flag to indicate whether we're in the optimization stage

    while (!_stopped)
    {
        /* Evolve active population to build next population.
         * Function returns when next population is full and ready to
         * be switched in.  */
        if (0 > _evolve())
        {
            break;
        }

        // Switch to next population
        _active_pop_index = !_active_pop_index;

        // Sort new population
        _sort_active_population();

        // See if we have a new fittest item
        if (ACTIVE_POP(0)->fitness < _best_item->fitness)
        {
            memcpy(_best_item, ACTIVE_POP(0), BF_PROG_SIZE_BYTES);

            if (!_config.quiet)
            {
                bfi_log("(stage %d) gen. #%u, fitness %u, %s", ((int) optimizing) + 1, _generation,
                                                               _best_item->fitness, _best_item->text);
                fflush(stdout);
            }
        }

        _generation++;

        if ((0u == _best_item->fitness) && !optimizing)
        {
            // If fitness reached 0, check if we need to do any optimzation passes
            if (0 == _config.num_optimization_gens)
            {
                _stopped = true;
            }
            else
            {
                bfi_log("start optimizing for length");

                _penalize_length = true;
                optimizing = true;

                // Re-assess fitness of all items, now that we are optimizing for length
                for (unsigned int i = 0; i < _config.population_size; i++)
                {
                    bf_program_t *prog = ACTIVE_POP(i);
                    prog->fitness = _assess_bf_program(prog, _penalize_length);
                }

                _best_item->fitness = _assess_bf_program(_best_item, _penalize_length);

                // Re-sort
                _sort_active_population();

                // Re-select best item
                memcpy(_best_item, ACTIVE_POP(0), BF_PROG_SIZE_BYTES);
            }
        }
        else if (optimizing)
        {
            // If in optimization passes, check if we're ready to stop
            if (0 < _config.num_optimization_gens)
            {
                if (++optgen_count >= _config.num_optimization_gens)
                {
                    _stopped = true;
                }
            }
        }
    }

    // populate output
    output->num_bf_programs = _config.population_size * _generation;
    (void) memcpy(output->bf_program, _best_item->text, _best_item->program_len + 1u);

    free(_population);

    return 0;
}
