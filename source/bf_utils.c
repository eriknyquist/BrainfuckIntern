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
#include "common.h"

#define TAPE_SIZE  300000

#define STACK_SIZE 2048

#define index_valid(i) (i >= 0 && i < TAPE_SIZE)

typedef enum {
    BF_INC = 0,
    BF_DEC = 1,
    BF_LSH = 2,
    BF_RSH = 3,
    BF_OUT = 4,
    BF_OPEN = 5,
    BF_CLOSE = 6,
    BF_IN = 7,
    BF_NUM_SYMS = 8
} bf_sym_e;

static int stack[STACK_SIZE];

static unsigned int pos;
static const char *syms = "+-<>.[],";

static int stack_push(int val)
{
    if (pos >= STACK_SIZE)
    {
        return -1;
    }

    stack[pos++] = val;
    return 0;
}

static int stack_pop(void)
{
    if (pos == 0)
    {
        return -1;
    }


    return stack[--pos];
}

static int stack_peek(void)
{
    if (pos == 0)
    {
        return -1;
    }


    return stack[pos - 1];
}

/**
 * @see bf_utils.h
 */
char bf_rand_sym(void)
{
    return syms[rand() % BF_NUM_SYMS];
}

/**
 * @see bf_utils.h
 */
int bf_rand_syms(char *output, int min_size, int max_size)
{
 	int size;

 	if (max_size < 0)
 	{
  		size = min_size;
  	}
 	else
  	{
    	size = randrange(min_size, max_size);
  	}

   	for (int i = 0; i < size; i++)
  	{
    	output[i] = bf_rand_sym();
    }

    output[size] = 0;

    if (size == 0)
    {
        return -1;
    }

    return size;
}

/**
 * Count the number of characters following the first character that are the
 * same as the first character
 *
 * @param s  pointer to string, positioned at the character to check for
 *           duplicates after
 * @return  number of duplicates after first character
 */
int count_dupes_ahead(char *s)
{
    int i;

    for (i = 1; s[i]; i++)
    {
        if (s[i] != *s)
        {
            break;
        }
    }

    return i - 1;
}

/**
 * @see bf_utils.h
 */
int bf_interpret(char *prog, char *input, size_t input_len, char *output, size_t max_output,
                 int max_instructions)
{
    unsigned char tape[TAPE_SIZE];

    int depth = 0;

    /* No. of instructions executed */
    int ep;

    /* Index of current instruction */
    int i;

    /* Index to current instruction in program */
    int p = 0;

    /* Index to current position in output buffer */
    int out = 0;

    /* Index to current position in input buffer */
    int in = 0;

    int dupes;

    memset(tape, 0, TAPE_SIZE);
    pos = 0;

    for(i = 0, ep = 0; prog[i]; i++, ep++)
    {
        if (ep >= max_instructions)
        {
            return -1;
        }

        if (prog[i] == syms[BF_INC])
        {
            if (!index_valid(p))
            {
                return -1;
            }

            dupes = count_dupes_ahead(prog + i);
            tape[p] = (tape[p] + dupes + 1) % 256;
            i += dupes;

        }
        else if (prog[i] == syms[BF_DEC])
        {
            if (!index_valid(p))
            {
                return -1;
            }

            dupes = count_dupes_ahead(prog + i);
            if (tape[p] < (dupes + 1))
            {
                tape[p] = 255 - ((dupes + 1) % 256);
            }
            else
            {
                tape[p] -= (dupes + 1);
            }

            i += dupes;

        }
        else if (prog[i] == syms[BF_LSH])
        {
            dupes = count_dupes_ahead(prog + i);
            p -= (dupes + 1);
            i += dupes;
        }
        else if (prog[i] == syms[BF_RSH])
        {
            dupes = count_dupes_ahead(prog + i);
            p += (dupes + 1);
            i += dupes;
        }
        else if (prog[i] == syms[BF_OUT])
        {
            if (!index_valid(p))
            {
                return -1;
            }

            if (out >= max_output)
            {
                return -1;
            }

            output[out++] = tape[p];

        }
        else if (prog[i] == syms[BF_IN])
        {
            if (!index_valid(p))
            {
                return -1;
            }

            if (in >= input_len)
            {
                return -1;
            }

            tape[p] = input[in++];

        }
        else if (prog[i] == syms[BF_OPEN])
        {
            if (!index_valid(p))
            {
                return -1;
            }

            if (tape[p])
            {
                // Ignore obvious infinite loops
                if (prog[i + 1] == BF_CLOSE)
                {
                    return -1;
                }

                if (stack_push(i) < 0)
                {
                    return -1;
                }

                depth++;

            }
            else
            {
                unsigned int inner_depth = 1;

                while(inner_depth > 0)
                {
                    i++;

                    if (!prog[i])
                    {
                        return -1;
                    }

                    if (prog[i] == syms[BF_CLOSE])
                    {
                        inner_depth--;
                    }
                    else if (prog[i] == syms[BF_OPEN])
                    {
                        inner_depth++;
                    }
                }
            }

        }
        else if (prog[i] == syms[BF_CLOSE])
        {
            if (!index_valid(p))
            {
                return -1;
            }

            if (depth <= 0)
            {
                return -1;
            }

            if (tape[p])
            {
                int start;

                if ((start = stack_peek()) < 0)
                {
                    return -1;
                }

                i = start;
            }
            else
            {
                (void)stack_pop();
                depth--;
            }
        }
    }

    if (depth != 0)
    {
        return -1;
    }

    if (out > 0)
    {
        output[out] = 0;
    }

    return out;
}
