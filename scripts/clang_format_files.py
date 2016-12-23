## clang_format_files
import argparse
import zipfile
import os
import hashlib
import json
from os.path import join, realpath, splitext, relpath
from subprocess import Popen, PIPE
import time


parser = argparse.ArgumentParser(prog='binary asset packager',description='Script to package binary data from a git repro into a deploy-able zip')
parser.add_argument('-d','--directory', action='append', help='Source directory to search. NOT Recursive.')
parser.add_argument('-r','--rdirectory', action='append', help='Source directory to search. Recursive.')
#parser.add_argument('-s','--style', help='filepath of clang format style.')

file_types = [
    '.c',
    '.cpp',
    '.h',
    '.hpp',
    '.cxx',
]

def main():
    args = parser.parse_args()
    directories = [realpath(x) for x in args.directory]
    rdirectories = [realpath(x) for x in args.rdirectory]
    print "Checking files in %s"%(str([realpath(x) for x in directories]+[realpath(x) for x in rdirectories]))


    source_files = []
    for d in directories:
        for root, dirs, files in os.walk(d):
            dirs = []
            source_files += [realpath(join(root, x)) for x in files if splitext(x)[1] in file_types]
    for d in rdirectories:
        for root, dirs, files in os.walk(d):
            source_files += [realpath(join(root, x)) for x in files if splitext(x)[1] in file_types]

	cmd = ['../external/LLVM/clang-format', '-i', '-style=file'] 
	cmd += source_files
	#print "Running command line", cmd
    p = Popen(cmd, stdin=PIPE, stdout=PIPE)
    returncode = p.wait()

if __name__ == '__main__':
    main()
