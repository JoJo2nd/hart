import argparse
import zipfile
import os
import hashlib
import urllib2
import urllib
from os.path import join, realpath, splitext, relpath
import dropbox


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
        print "Nothing to do."
        return

    print "local doesn't match remote. Grabbing latest package."

    with open('scripts/dropbox.oauth2', 'rb') as f:
        oauth2_token = f.read();

    dbx = dropbox.Dropbox(oauth2_token)

    src_path = '/packaged_'+remote_txt+'.zip'

    print "listing packages..."
    folder_list = []
    for f in read_db_directory(dbx, ''):
        print f
        folder_list += [f]

    if not src_path.lower() in folder_list:
        print "Can't locate remote data package!"
        return
    
    print "Downloading package %s"%(src_path)
    dbx.files_download_to_file('packaged.remote.zip', src_path)

    print "Extracting package from %s"%(src_path)
    with zipfile.ZipFile('packaged.remote.zip', 'r') as zip_pkg:
        zip_pkg.extractall()

    os.remove('packaged.remote.zip')

def read_db_directory(dbx, path):
    try:
        res = dbx.files_list_folder(path)
        while True:
            for i in res.entries:
                yield i.path_lower
            if res.has_more:
                res = dbx.files_list_folder_continue(res.cursor)
            else:
                break
    except dropbox.exceptions.ApiError as err:
        pass

if __name__ == '__main__':
    main()
