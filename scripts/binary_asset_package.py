import argparse
import zipfile
import os
import hashlib
from os.path import join, realpath, splitext, relpath
import dropbox
import time

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

    with open('dropbox.oauth2') as f:
        oauth2_token = f.read();

    dbx = dropbox.Dropbox(oauth2_token)

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

    final_zip_name = 'packaged_%s.zip'%(sha1.hexdigest());

    remote_files = []
    #use empty string to get root folder
    for f in read_db_directory(dbx, ''):
        print f
        remote_files += [f]

    try:
        os.remove('./../packaged.sha1.remote')
    except:
        pass
    os.rename('packaged.sha1.local', './../packaged.sha1.remote')
    if '/'+final_zip_name.lower() in remote_files:
        print 'Skipping file upload. File already exists'
        try:
            os.remove('packaged.zip')
        except:
            pass
        return

    os.rename('packaged.zip', final_zip_name)

    with open(final_zip_name, 'rb') as zf:
        data = zf.read()
        print('Uploading %s - %d bytes' % (final_zip_name, os.path.getsize(final_zip_name)))
        try:
            res = dbx.files_upload(
                data,
                '/'+final_zip_name,
                mode=dropbox.files.WriteMode.overwrite,
                mute=True)
        except dropbox.exceptions.ApiError as err:
            print('*** API error', err)
            return None
        print('uploaded as', res.name.encode('utf8'))

    os.remove(final_zip_name)

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
