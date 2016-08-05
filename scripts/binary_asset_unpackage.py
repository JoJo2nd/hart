import argparse
import zipfile
import os
import hashlib
import urllib2
import urllib
from os.path import join, realpath, splitext, relpath


def main():
    try:
        with open('packaged.sha1.remote', 'rb') as f:
            remote_txt = f.read()
    except:
        remote_txt = 'blah!'

    try:
        with open('packaged.sha1.local', 'rb') as f:
            local_txt = f.read()    
    except:
        local_txt = 'blah?'

    if remote_txt == local_txt:
        return

    with open('packaged.sha1.where', 'rb') as f:
        where = f.read()
    
    print "Downloading package from %s"%(where)
    urllib.urlretrieve(where, 'packaged.remote.zip')

    print "Extracting package from %s"%(where)
    with zipfile.ZipFile('packaged.remote.zip', 'r') as zip_pkg:
        zip_pkg.extractall()

    os.remove('packaged.remote.zip')

if __name__ == '__main__':
    main()
