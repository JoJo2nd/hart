import argparse
import zipfile
import os
import hashlib
from os.path import join, realpath, splitext, relpath

# BUF_SIZE is totally arbitrary, change for your app!
BUF_SIZE = 65536  # lets read stuff in 64kb chunks!

parser = argparse.ArgumentParser(prog='binary asset packager',description='Script to package binary data from a git repro into a deploy-able zip')
parser.add_argument('-d','--directory', action='append', help='Source directory to search. Recursive.')
parser.add_argument('-r','--root', help='Directory to make paths relative too.')

asset_types = [
    '.exe',
    '.lib',
    '.dll',
    '.png',
    '.tga',
    '.bin',
]

def main():
    args = parser.parse_args()
    base_path = args.root
    directories = [realpath(x) for x in args.directory]
    print "Gathering files from %s"%(base_path)

    assets = []
    for d in directories:
        for root, dirs, files in os.walk(d):
            assets += [ relpath(realpath(join(root, x)), base_path) for x in files if splitext(x)[1] in asset_types]

    with zipfile.ZipFile('packaged.zip', 'w') as zip_pkg:
        for a in assets:
            print "Adding", a
            zip_pkg.write(realpath(join(base_path, a)), a)

    sha1 = hashlib.sha1()

    with open('packaged.zip', 'rb') as f:
        while True:
            data = f.read(BUF_SIZE)
            if not data:
                break
            sha1.update(data)

    with open('packaged.sha1.local', 'wb') as f:
        f.write("%s"%(sha1.hexdigest()))

    with zipfile.ZipFile('packaged.zip', 'a') as zip_pkg:
        zip_pkg.write('packaged.sha1.local', 'packaged.sha1.local')

    os.remove('packaged.sha1.local')
    os.rename('packaged.zip', 'packaged_%s.zip'%(sha1.hexdigest()))

if __name__ == '__main__':
    main()
