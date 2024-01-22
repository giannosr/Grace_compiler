#!/usr/bin/python
from sys import argv
from os  import system, getcwd

flags = {
    'O': '',
    'i': '',
    'f': '',
}

for arg in argv[1:]:
    if arg[0] == '-': flags[arg[1]] = arg
    else:                input_file = arg

# can be called from any directory
cmd = f"cd {__file__[:-6]}; ./grc {flags['O']}"

if   flags['i'] != '': pass
elif flags['f'] != '': cmd += ' | llc'
else: # input file can be in any directory and output file will be in the callers directory
    name = getcwd() + '/' + input_file.split('/')[-1].split('.')[0]
    if input_file[0] != '/': input_file = getcwd() + '/' + input_file
    cmd += f" < {input_file} > {name}.ll; clang -S {name}.ll -o {name}.s; clang -Wall -o {name} {name}.s libgrc/libgrc.a"

# perserve the exit code
exit(system(cmd) >> 8)
