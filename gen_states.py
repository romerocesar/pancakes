'''Generate random permutations of a given size'''
import random
from optparse import OptionParser

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('-n', default=17)
    parser.add_option('-N', default=100)
    options, args = parser.parse_args()
    if options.n and options.N:
        # generate N random permutations of size n
        s = range(int(options.n))
        for i in range(int(options.N)):
            random.shuffle(s)
            for x in s:
                print x,
            print
