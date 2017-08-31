from random import (choice, random, randint)
from subprocess import (Popen, PIPE, STDOUT)
import os
import sys
import signal
import time

from Brainfuck import interpret, BrainfuckSyntaxError, BrainfuckMemoryError

binFile = 'testBin.out'
max_run_secs = 0.05

MOST_UNFIT = 0xffffffff

def repeat(string, limit=16):
    ret = ""
    for i in range(randint(0, limit)):
        ret += string
    return ret

def randStmt():
    ret = ""

    for i in range(1, 3):
        action = choice([
            lambda: '[' + randStmt() + ']',
            lambda: repeat('-'),
            lambda: repeat('+'),
            lambda: repeat('>'),
            lambda: repeat('<'),
            lambda: '.'
        ])

        ret += action()

    return ret

def garbage():
    return ''.join([choice('[]<>-+.') for _ in range(32)])

def getRandIndex(item):
    ix = 0

    if len(item) == 1:
        ix = 0
    else:
        ix = choice(range(len(item)))

    return ix

def program(stmts=20):
    prog = ""
    for i in range(stmts):
        prog += randStmt()

    return prog

class Chromosome:
    target = "hi!"
    optimize = False

    def __init__(self, gene):
        self.gene = gene
        self.output = None
        self.fitness = self.__updateFitness(gene)

    def __str__(self):
        return '%s (%s)' % (self.gene, self.output)

    def mate(self, mate):
        geneLen = len(self.gene)
        mateLen = len(mate.gene)

        child1 = self.gene[:geneLen / 2] + mate.gene[mateLen / 2:]
        child2 = mate.gene[:mateLen / 2] + self.gene[geneLen / 2:]
        return [Chromosome(child1), Chromosome(child2)]

    def mutate(self):

        if len(self.gene) <= 2:
            return self.getRandom()

        def mutate_move(G):
            # Pick a random character and move it to a new random location
            ret = list(G)
            old = getRandIndex(ret)
            save = ret[old]
            del(ret[old])
            new = getRandIndex(ret)

            return ''.join(ret[:new] + [save] + ret[new:])

        def mutate_copy(G):
            # Randomly copy a character
            ix = getRandIndex(G)
            return G[:ix] + G[ix] + G[ix:]

        def mutate_add_char(G):
            # Randomly add a character
            ix = getRandIndex(G)
            return G[:ix] + choice('.><+-') + G[ix:]

        def mutate_add_str(G):
            # Randomly add some more characters to the gene
            ix = getRandIndex(G)
            return G[:ix] + randStmt() + G[ix:]

        def mutate_change(G):
            # Change a random character
            ret = list(G)
            ix = getRandIndex(ret)
            char = choice(".><-+")
            ret[ix] = char
            return ''.join(ret)

        def mutate_snip(G):
            # Snip a few characters off the beginning/end of the gene
            size = randint(1, int(len(G) / 2))
            if randint(0, 1) == 0:
                return G[:-size]

            return G[size:]


        def mutate_remove(G):
            # Randomly remove a character
            ix = getRandIndex(G)
            return G[:ix - 1] + G[ix + 1:]

        def mutate_add_garbage(G):
            # Add some random garbage code
            ix = getRandIndex(G)
            return G[:ix] + garbage() + G[ix:]

        action = choice([
            mutate_move, mutate_copy, mutate_add_char, mutate_add_str,
            mutate_change, mutate_snip, mutate_remove, mutate_add_garbage
        ])

        return Chromosome(action(self.gene))

    def __updateFitness(self, gene):
        try:
            out = interpret(gene, time_limit=max_run_secs,
                                      buffer_stdout=True)
        except (BrainfuckSyntaxError, BrainfuckMemoryError):
            return MOST_UNFIT

        if out == None or len(out) < 1:
            return MOST_UNFIT

        self.output = out

        if len(out) != len(self.target):
            ret = abs(len(out) - len(self.target)) * 10000000
        else:
            ret = 0
            for i in range(len(out)):
                pos = len(out) - i
                ret +=  (pos * pos) * abs(ord(out[i]) - ord(self.target[i]))

            if ret == 0:
                return 0

        if self.optimize:
            ret += len(gene)

        return ret

    @staticmethod
    def getRandom():
        P = ""

        generator = program if random() < 0.5 else garbage
        return Chromosome(generator())
