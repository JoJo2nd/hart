import argparse
import zipfile
import os
import hashlib
import json
from os.path import join, realpath, splitext, relpath
import dropbox
import time

# BUF_SIZE is totally arbitrary, change for your app!
BUF_SIZE = 65536  # lets read stuff in 64kb chunks!

parser = argparse.ArgumentParser(prog='binary asset packager',description='Script to package binary data from a git repro into a deploy-able zip')
parser.add_argument('-d','--directory', action='append', help='Source directory to search. Recursive.')
parser.add_argument('-r','--root', help='Directory to make paths relative too.')
parser.add_argument('--preview', action='store_true')

asset_types = [
    '.exe',
    '.lib',
    '.dll',
    '.png',
    '.tga',
    '.bin',
    '.qm',
    '.lpf',
    '.zip',
    '.piskel',
    '.7z',
]

def getFileSHA1(filepath):
    sha1 = hashlib.sha1()
    with open(filepath, 'rb') as f:
        while True:
            data = f.read(BUF_SIZE)
            if not data:
                break
            sha1.update(data)

    return sha1.hexdigest()

def main():
    args = parser.parse_args()
    base_path = args.root
    directories = [realpath(x) for x in args.directory]
    print "Checking files in %s"%(realpath(base_path))

    manifest = {}

    with open('dropbox.oauth2') as f:
        oauth2_token = f.read();

    dbx = dropbox.Dropbox(oauth2_token)

    assets = []
    for d in directories:
        for root, dirs, files in os.walk(d):
            assets += [ relpath(realpath(join(root, x)), base_path) for x in files if splitext(x)[1] in asset_types]

    for asset in assets:
        manifest[asset] = { 'sha1': getFileSHA1(join(base_path, asset)) }

    with open('./../binary.manifest', 'wb') as f:
        f.write(json.dumps(manifest, indent=2, sort_keys=True))

    remote_files = []
    #use empty string to get root folder
    for f in read_db_directory(dbx, ''):
        remote_files += [f]

    to_upload = [local for local in assets if ('/'+manifest[local]['sha1']+'.zip').lower() not in remote_files]

    if args.preview:
        for u in to_upload:
            print u, "needs uploading."        
    else:
        for u in to_upload:
            zipped_name = manifest[u]['sha1']+'.zip'
            with zipfile.ZipFile(zipped_name, 'w') as zip_pkg:
                print u, "Compressing...",
                zip_pkg.write(realpath(join(base_path, u)), 'file')

            with open(zipped_name, 'rb') as zf:
                data = zf.read()
                print 'Uploading %d bytes' % (os.path.getsize(zipped_name)),
                try:
                    res = dbx.files_upload(
                        data,
                        '/'+zipped_name,
                        mode=dropbox.files.WriteMode.overwrite,
                        mute=True)
                except dropbox.exceptions.ApiError as err:
                    print('*** API error', err)
                    return None
                print ' as ', res.name.encode('utf8'),

            print '...deleting temp data.'
            try:
                os.remove(zipped_name)
            except:
                pass



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
