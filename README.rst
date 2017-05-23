Brainfuck Intern: A program that learns to write Brainfuck programs
===================================================================

Starting with a bunch of randomly generated nonsense, this program "evolves"
a Brainfuck program to print a specific string of output through a
trial-and-error process that resembles natural selection.

|

I called it "Brainfuck Intern" because its approach to programming is similar
to most humans' approach when we first start programming (often as an
intern...): Change random things in the code until you get lucky and reduce the
number of errors.

.. image:: x.jpg

Using Brainfuck Intern
======================

Just execute ``BrainfuckIntern.py``, and pass the desired output string with
the ``-o`` parameter:
::

    python BrainfuckIntern.py -o 'Hi!'

There are several additional optional parameters for tuning the evolution
process. Run ``python BrainfuckIntern.py --help`` for full details.

Hello, world! by BrainfuckIntern
================================

BrainfuckIntern created this beautiful Brainfuck program after 3 minutes
of smashing random bits of garbage together (180,022 random bits of garbage in
total for this masterpiece). This program prints "Hello, world!"

::

    ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.++
    +++++++++++++++++++++++++++.+++++++.[<]>.+++.<<++++++++++++++++++++++++++++
    ++++++++++++++++.------------.+++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++++++++++++++++++++++.--------.+++.------<<<++++++++++
    +++++++++++++++++++++++>>>.<<<<++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++++++++++++++++++++++++++++++++++++.>.

And here's another, just for fun. This one prints "I'm a programmer!", and took
about 6 minutes for BrainfuckIntern to create:

::

    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.-
    ---------------------------------.+++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++.---------------------------------------------
    --------------------------------.++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++.[>][[+[.]]<]>+++++++>>>>+++++++++++++++++++++++++++
    +++++.<<<<+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++++++++++++++++++++.++.+----.--------.+<++++++++++++++
    ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++<<<<<<<<+++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++++++++++++.+>>>>>>>>+++++++++++++++++++++++.+++++++++
    +++.+++-------++++.<<<<<<<<--------------.<++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++++++++++++++++++++++++++++++-++++++++++++++++++++++++
    +++++++++.>>[.]<<<<+++++++++++++++++++++++++++++++++.
