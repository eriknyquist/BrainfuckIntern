import time
from random import (choice, random, randint)
from Chromosome import Chromosome

class Population:
    """
    A class representing a population for a genetic algorithm simulation.

    A population is simply a sorted collection of chromosomes
    (sorted by fitness) that has a convenience method for evolution.  This
    implementation of a pop uses a tournament selection algorithm for
    selecting parents for crossover during each generation's evolution.

    Note that this object is mutable, and calls to the evolve()
    method will generate a new collection of chromosome objects.
    """

    _tournamentSize = 3

    def __init__(self, getRandom, size=1024, elitism=0.5, crossover=0.5,
                 mutation=0.5, conf=None):
        if conf == None:
            self.elitism = elitism
            self.mutation = mutation
            self.crossover = crossover
            self.size = size
            self.total = 0
            self.gen = 1

            self.pop = [getRandom() for _ in range(size)]
            self.total = self.size
        else:
            self.pop = self.__fromString(conf)

        self.pop.sort(key=lambda x: x.fitness)

    def _tournament(self):
        """
        A helper method used to select a random chromosome from the
        pop using a tournament selection algorithm.
        """
        best = choice(self.pop)
        for i in range(Population._tournamentSize):
            cont = choice(self.pop)
            if (cont.fitness < best.fitness): best = cont

        return best

    def __str__(self):
        ret = '%s;;\n\n\n%f;;\n%f;;\n%f;;\n%d;;\n%d;;\n%d;;\n' % (
            Chromosome.target, self.elitism, self.mutation, self.crossover,
            self.size, self.total, self.gen)

        for g in self.pop:
            ret += '%s;;\n' % g.gene

        return ret

    def __fromString(self, string):
        halves = string.split(';;\n\n\n')
        Chromosome.target = halves[0]

        fields = halves[1].split(';;\n')[:-1]

        self.elitism = float(fields[0])
        self.mutation = float(fields[1])
        self.crossover = float(fields[2])
        self.size = int(fields[3])
        self.total = int(fields[4])
        self.gen = int(fields[5])

        return [Chromosome(x) for x in fields[6:]]

    def evolve(self):
        """
        Method to evolve the population of chromosomes.
        """
        size = len(self.pop)
        idx = int(round(size * self.elitism, -1))
        buf = self.pop[:idx]

        while idx < size:
            if random() <= self.crossover:
                # Breeding: breed one of the 'elite' (high fitness) chromosomes
                # with another randomly-chosen chromosome, to create two new
                # chromosomes for the next generation
                if randint(0, 1):
                    partner = self._tournament()
                else:
                    partner = self.pop[idx]

                newGenes = partner.mate(self.pop[0])
                self.total += 2
            else:
                # No breeding: pick two 'elite' chromosomes to be duplicated in
                # the next generation
                newGenes = [self.pop[idx], self._tournament()]

            if random() <= self.mutation:
                # Mutation: randomly mutate these two chromosomes
                newGenes = [x.mutate() for x in newGenes]
                self.total += 2

            buf.extend(newGenes)
            idx += 2

        self.pop = buf[:size]
        self.pop.sort(key=lambda x: x.fitness)

    def evolveFitnessLevel(self, level=0, verbose=False):
        """
        Method to repeatedly evolve the population until the fittest members
        reach a certain fitness level
        """


        print ("Generation %d (fitness=%d): %s\n"
            % (self.gen, self.pop[0].fitness, self.pop[0]))

        while self.pop[0].fitness > level:
            start = time.time()
            self.evolve()
            end = time.time()

            if verbose:
                print("Generation %d (fitness=%d, time=%.4f): %s\n" %
                    (self.gen, self.pop[0].fitness, end - start,
                    str(self.pop[0])))
            self.gen += 1

        return self.pop[0]
