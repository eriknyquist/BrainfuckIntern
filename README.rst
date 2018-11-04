Brainfuck Intern: a very bad brainfuck programmer
-------------------------------------------------

Brainfuck intern takes a target string as input, e.g. "Hello, world!", and uses
an iterative evolution-like process to produce a
`Brainfuck <https://en.wikipedia.org/wiki/Brainfuck>`_ program that prints the
target string.

Brainfuck intern knows nothing about brainfuck syntax (for example, that opening
loop braces need to be matched with the opposite brace). Brainfuck intern has
only the following tools at its disposal;

* A brainfuck interpreter, to execute brainfuck programs and inspect the output
* Knowledge of valid brainfuck characters (but no knowledge of how to use them;
  no syntactic knowledge. Can only generate strings containing randomly chosen
  brainfuck characters)
* An ability to "mutate" brainfuck programs, by randomly changing, adding or
  removing characters
* An ability to "breed" two brainfuck programs to create new programs, by
  randomly combining elements of the two original programs
* An ability to "assess" brainfuck programs and their output for fitness, e.g.
  a brainfuck program that has syntactic errors will not run and the interpreter
  will return an error. This would be the worst possible fitness level. A program
  that runs successfully in the interpreter but produces no output would be a
  slightly higher fitness level, but still not great. A program that prints one
  character would be a higher fitness level, and so on until the highest fitness
  level, which would be a brainfuck program that prints exactly the target string.

Surprisingly, this actually produces working brainfuck programs in a relatively
short time (usually under a minute for strings under 16 chars on my machine).

Supported Platforms
===================

Linux & Windows only. Tested on Debian and Windows 10.

32 and 64 bit Windows binaries are included (``windows_build/i686/bfintern.exe``
and ``windows_build/x86_64/bfintern.exe``).

Usage
=====

Run the ``bfintern`` executable from a terminal (or Command Prompt in Windows)
with no arguments to see usage information:

::

      $> bfintern

        BrainFuck Intern (Copyright 2018 Erik Nyquist <eknyquist@gmail.com>)

        Usage: bfintern [-ecmsov] <output>

        -e <elitism>       Defines how many fit organisms are selected
                           from the population for each cycle of the
                           evolution process (0.0 to 1.0, e.g. 0.25
                           means the top 25% will be selected). Default
                           is 0.75.

        -c <crossover>     Defines how many selected organisms will be
                           bred with other randomly selected organisms
                           (0.0 to 1.0, e.g. 0.75 means 75% of
                           selected organisms will be bred). Default is
                           0.75.

        -m <mutation>      Defines how many selected organisms will be
                           randomly mutated (0.0 to 1.0, e.g. 0.44
                           means 44% of selected organisms will be
                           mutated). Default is 1.00.

        -s <size>          Defines the number of items in the
                           population (integer). Default is 1024.

        -l <size>          Defines the maximum size in bytes of each
                           generated brainfuck program. Default is 2048.

        -o <num>           Once evolution has produced a correct brainfuck
                           program, continue evolving for an additional
                           <num> generations to attempt to shorten the
                           brainfuck program by removing unnecessary bits.
                           0 means optimise infinitely.

        -v                 Show evolution progress by printing status at
                           each new generation. Default is to only print
                           the final brainfuck program.

        -h                 Show this text and exit

Hello, world! by brainfuck intern
=================================

Brainfuck intern created this beautiful Brainfuck program after a minute or so
of smashing random bits of garbage together. This program prints "Hello, world!"

::

    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++-++++++.
    ++++++++++++++++++++++++><+++++.++++>[+-+<>.+>]++[+++>]<+++++++++<+++..+++.
    +++>++++++++++++++++++++++++++++++.------->+<-----.++[+<+++++.--------.+-++
    +.------.--------.>>+++++++++++++--+++++++++++++++++++++.>].>[>--+>]

What I would say if I was Xzibit
================================

.. image:: x.jpg

Building from source (Linux only)
=================================

Dependencies (Linux only)
#########################

* Standard C lib. (glibc)
* C compiler (tested with GCC and clang)

Build (Linux only)
##################

Use the default Makefile target:

::

    make

This will create a directory called ``build`` and an executable called
``build/bfintern``.

