from random import (choice, random, randint)
from subprocess import (Popen, PIPE, STDOUT)
import os
import sys
import signal
import time

import Brainfuck

binFile = 'testBin.out'
max_run_secs = 0.05
ptr = 0

MOST_UNFIT = 0xffffffff

def repeat(string, limit=16):
    ret = ""
    for i in range(randint(0, limit)):
        ret += string
    return ret

def randStmt():
    global ptr

    op = choice('LL-+<>.')
    if op == 'L':
        return '[' + randStmt() + ']'
    elif op == '-':
        return repeat('-')
    elif op == '+':
        return repeat('+')
    elif op == '>':
        ret = repeat('>')
        ptr += len(ret)
        return ret
    elif op == '<':
        if ptr == 0:
            return ""

        return repeat('<', limit=ptr)
    else:
        return '.'

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
        gene = self.gene
        pick = choice('mcCherg')
        #pick = choice('cCerg')

        if pick == 'm':
            gene = list(gene)
            # Pick a random character and move it to a new random location
            old = getRandIndex(gene)
            save = gene[old]
            del(gene[old])
            new = getRandIndex(gene)

            gene = ''.join(gene[:new] + [save] + gene[new:])

        elif pick == 'c':
            # Randomly copy a character
            ix = getRandIndex(gene)
            gene = gene[:ix] + gene[ix] + gene[ix:]

        elif pick == 'C':
            # Randomly add some more characters to the gene
            ix = getRandIndex(gene)
            gene = gene[:ix] + randStmt() + gene[ix:]

        elif pick == 'h':
            gene = list(gene)
            # Change a random character
            ix = getRandIndex(gene)
            char = choice(".>-+")
            gene[ix] = char
            gene = ''.join(gene)

        elif pick == 'e':
            # Snip a few characters off the beginning/end of the gene
            size = randint(1, int(len(gene) / 2))
            if randint(0, 1) == 0:
                gene = gene[:-size]
            else:
                gene = gene[size:]

        elif pick == 'r':
            # Randomly remove a character
            ix = getRandIndex(gene)
            gene = gene[:ix - 1] + gene[ix + 1:]

        elif pick == 'g':
            # Add some random garbage code
            ix = getRandIndex(gene)
            gene = gene[:ix] + garbage() + gene[ix:]

        if len(gene) < 2:
            return self.getRandom()

        return Chromosome(gene)

    def __updateFitness(self, gene):
        try:
            out = Brainfuck.interpret(gene, time_limit=max_run_secs,
                                      buffer_stdout=True)
        except (ValueError, IndexError):
            return MOST_UNFIT

        if out == None or len(out) < 1:
            return MOST_UNFIT

        self.output = out

        if len(out) != len(self.target):
            ret = abs(len(out) - len(self.target)) * 10000000
        else:
            ret = 0
            for i in range(len(out)):
                diff = (len(out) - i) * abs(ord(out[i]) - ord(self.target[i]))
                ret += diff * diff

            if ret == 0:
                return 0

            ret += len(gene)

        return ret

    @staticmethod
    def getRandom():
        P = ""

        if random() < 0.2:
            while len(P) < 2:
                P = program()
        else:
            P = garbage()

        return Chromosome(P)
