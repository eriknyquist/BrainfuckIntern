import argparse
import sys

from random import seed
from datetime import (datetime, timedelta)
from Population import Population
from Chromosome import Chromosome

DESC = (
'\nA program that learns to write programs! Using a genetic algorithm that\n'
'models evolution and natural selection, this program "figures out" how to\n'
'write a Brainfuck program that produces a specific string of output'
)

parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter, epilog=(
        '\nExample:\n\n'
        'Evolve a program that prints "hi!"\n\n'
        '  >  python Brainfuck.py "hi!"\n\n'
    ), description=DESC)

req = parser.add_argument_group('Required (exactly one must be defined)')

req.add_argument('-o', dest='output', type=str, default=None, help=(
        'The target output string of a Brainfuck program. Evolution will stop'
        ' when a program is created that can produce this output string'
    ))

req.add_argument('-f', dest='file', type=str, default=None, help=(
        'File containing a previously saved state to be loaded'
    ))

parser.add_argument('-s', dest='size', type=int, default=128, help=(
        'The size (number of genomes) of the population used for each '
        'generation of the evolution process'
    ))

parser.add_argument('-m', dest='mutation', type=float, default=0.5, help=(
        'The mutation factor (0.0 to 1.0). Higher means more mutation'
    ))

parser.add_argument('-c', dest='crossover', type=float, default=0.5, help=(
        'The crossover/breeding factor (0.0 to 1.0). Higher means more breeding'
    ))

parser.add_argument('-e', dest='elitism', type=float, default=0.5, help=(
        'The elitism factor (0.0 to 1.0). Higher means a stricter selection '
        'process, e.g. a factor of 0.8 means only the fittest 20%% of the '
        ' population are selected for breeding & mutation, while a factor of '
        '0.0 selects the entire population'
    ))

parser.add_argument('-O', dest='optimize', action='store_true', default=False,
        help=( 'If true, BrainfuckIntern will attempt to optimise by removing '
            'unnecessary Brainfuck code from its programs. This will result in '
            'shorter, more sane-looking programs, however it can also slow '
            'the evolution process considerably, since there will be less '
            'variety in the "gene pool"'
    ))

args = parser.parse_args()

def print_summary(total, time):
    print 'Tried %s Brainfuck programs, total time %s' % (total,
        str(timedelta(seconds=int(time.total_seconds()))))

def main():

    if args.file and args.output:
        print "Error: can't use -f and -o together"
        sys.exit(1)

    if args.output != None:
        string = None
        Chromosome.target = args.output
    elif args.file != None:
        with open(args.file, 'r') as fh:
            string = fh.read()
    else:
        parser.print_help()
        sys.exit(1)

    start = datetime.now()
    seed(start)

    if args.file == None:
        print "Generating %d random Brainfuck programs..." % args.size
    else:
        print "Reading saved state from %s..." % args.file

    P = Population(Chromosome.getRandom, size=args.size, elitism=args.elitism,
        crossover=args.crossover, mutation=args.mutation, conf=string,
        optimize=args.optimize)

    print "Starting evolution\n"
    try:
        P.evolveFitnessLevel(0, True)
    except KeyboardInterrupt:
        end = datetime.now()
        ans = None

        while ans not in ['y', 'n']:
            ans = raw_input("Save the current state? [y/n]: ").lower()

        if ans == 'y':
            fname = raw_input("Enter filename for saved state: ")
            with open(fname, 'w') as fh:
                fh.write(str(P))

            print 'State saved in file %s' % fname

        print_summary(P.total, end - start)
        return

    print_summary(P.total, datetime.now() - start)

if __name__ == "__main__":
    main()
