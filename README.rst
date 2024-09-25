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
  slightly better fitness level, but still not great. A program that prints one
  character would be a better fitness level still, and so on until the best fitness
  level, which would be a brainfuck program that prints exactly the target string.

*Brainfuck Intern hard at work producing the "hello world" program that is shown
at the end of this README*

.. image:: images/bfintern.gif

Supported Platforms
===================

Linux & Windows only. Tested on Debian and Windows 10.

See `latest release <https://github.com/eriknyquist/BrainfuckIntern/releases/latest>`_
for 32 and 64 bit Windows binaries.

Usage
=====

Run the ``bfintern`` executable from a terminal (or Command Prompt in Windows)
with no arguments to see usage information:

::

    $> bfintern

    Brainfuck Intern (version 2.0)

    Erik Nyquist <eknyquist@gmail.com>)

    Uses a genetic algorithm to mutate strings of random Brainfuck characters
    until they match a set of user-provided test cases.

    Press Ctrl-C at any time to display the current best Brainfuck program
    and terminate.

    Usage: ./build/bfintern [OPTIONS] <TESTCASE> [<TESTCASE>, ...]

    POSITIONAL ARGUMENTS:

    One or more test cases, where each test case consists of two ASCII
    strings separated by a colon ':'. The first string (left side of the
    colon) will be passed as input to the Brainfuck program under test,
    and the second string (right side of the colon) is the output that
    must be produced by the Brainfuck program in order for the test case
    to pass.

    A test case may also contain a single ASCII string with no colon. In
    this case, the provided string will be considered the expected output,
    and no input will passed to the Brainfuck program under test.

    OPTIONS:

    -e <elitism>       Defines how many of the best Brainfuck programs are
                       selected from the population for each cycle of the
                       evolution process, for breeding & mutation (0.0 to
                       1.0, e.g. 0.25 means the top 25% will be selected).
                       Default is 0.33.

    -c <crossover>     Defines how many of the selected 'elite' Brainfuck
                       programs will be 'bred' (randomly mixed/combined)
                       with other randomly selected programs (0.0 to 1.0,
                       e.g. 0.75 means 75% of elite programs will be bred).
                       Default is 0.33.

    -m <mutation>      Defines how many of the selected 'elite' Brainfuck
                       programs will be randomly mutated (0.0 to 1.0, e.g.
                       0.44 means 44% of elite programs will be mutated.
                       Default is 0.33.

    -s <size>          Defines the number of Brainfuck programs in the
                       population (integer). Default is 1024.

    -l <size>          Defines the maximum size in bytes of each
                       generated Brainfuck program in the population.
                       Default is 4096.

    -o <num>           Once evolution has produced a correct Brainfuck
                       program (passes all test cases), continue evolving
                       for an additional <num> generations to attempt to
                       shorten the Brainfuck program by removing unnecessary
                       characters (-1 to optimise infinitely). Default is 1000.

    -r <seed>          Fixed seed value to seed random number generation.
                       The current time (seconds) is used by default.

    -q                 Do not show evolution progress by printing status and
                       fittest BF program at each improved generation (default
                       behaviour). Instead, only print the fittest Brainfuck
                       program on termination (Ctrl-C).

    -h                 Show this text and exit.

    EXAMPLES:

    Produce a program that prints "Hello, world!":

        ./build/bfintern "Hello, world!"

    Produce a program that prints "Hello, world!", with custom seed,
    mutation and elitism values provided:

        ./build/bfintern -r 123456 -m 0.75 -e 0.1 "Hello, world!"

    Produce a program that prints "true" when input is "1", and prints
    "false" when the input is "0":

        ./build/bfintern "0:false" "1:true"

Hello, world! by Brainfuck Intern
=================================

Brainfuck intern created this beautiful Brainfuck program after a minute or so
of smashing random bits of garbage together. This program prints "Hello, world!"

::

    ->+->++++++[+++++++++++++++++++++++++++++++++++++++++++>+++++++++++++++++++
    +++[+++++++++++<][]>++++-+++++++++.+++++++++++++++++++++++++++++.-++++++++.
    .+++.+++++++>+++++++++++-+.---------+----.-<+[.--+---[----.++-+-++.------.-
    ----[---.-+<]]>+-++--+>--++++.-+>]+->]

Personal testimony from Xzibit
==============================

.. image:: images/x.jpg
