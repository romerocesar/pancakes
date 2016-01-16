env = Environment()

if ARGUMENTS.get('verb',0):
    env.Append(CCFLAGS='-DVERB')

if ARGUMENTS.get('debug',0):
    env.Append(CCFLAGS=['-g3','-O0'])
elif ARGUMENTS.get('profile',0):
    env.Append(CCFLAGS=['-pg'])
elif ARGUMENTS.get('ndebug',0): 
    env.Append(CCFLAGS=['-O3','-DNDEBUG'])

NoClean('getopt_pp.o')
main=env.Program(target='pancakes',source=['getopt_pp.cpp','pancake.cpp','search.cpp','main.cpp'])
Default(main)
