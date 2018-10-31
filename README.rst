Brainfuck Intern: a program that learns to write Brainfuck programs
===================================================================

Starting with a bunch of randomly generated nonsense, this program "evolves"
a Brainfuck program to print a specific string of output through a
trial-and-error process that resembles natural selection.

|

I called it "brainfuck intern" because its approach to programming is similar
to most humans' approach when we first start programming (often as an
intern...): Change random things in the code until you get lucky and reduce the
number of errors.

.. image:: x.jpg

Supported Platforms
===================

Linux only. Tested on Debian.

Dependencies
============

- Standard C lib. (glibc)
- C compiler (tested with GCC and clang)

Build
=====

Use the default Makefile target:

::

    make

This will create a directory called ``build`` and an executable called
``build/bfintern``. Run it with no arguments to see usage information:

::

      $> build/bfintern

        BrainFuck Intern (Copyright 2018 Erik Nyquist <eknyquist@gmail.com>)

        Usage: build/bfintern [-ecmsov] <output>

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
