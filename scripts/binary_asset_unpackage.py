import argparse
import zipfile
import os
import hashlib
import json
from os.path import join, realpath, splitext, relpath, isfile
import dropbox

parser = argparse.ArgumentParser(prog='binary asset unpackager',description='Script to download and unpackage binary data for a git repro')
parser.add_argument('-r','--root', help='Directory to install too.')

def main():

    args = parser.parse_args()
    root = args.root
    print "Grabbing updated files for %s"%(root)

    manifest = {}
    local = {}

    with open('dropbox.oauth2') as f:
        oauth2_token = f.read();

    dbx = dropbox.Dropbox(oauth2_token)

    with open(join(root, 'binary.manifest'), 'rb') as f:
        manifest = json.loads(f.read())
    if isfile(join(root, 'local.manifest')):
        with open(join(root, 'local.manifest'), 'rb') as f:
            local = json.loads(f.read())

    files_to_dl = []
    files_to_del = []
    for k, v in manifest.iteritems():
        if not k in local or local[k]['sha1'] != v['sha1']:
            files_to_dl += [{'path':'/'+v['sha1']+'.zip', 'dest':k}]

    for k, v in local.iteritems():
        if not k in manifest:
            files_to_del += {'dest':k}

    for f in files_to_del:
        try:
            os.remove(join(root, f['dest']))
        except:
            pass

    db_file_list = []
    for f in read_db_directory(dbx, ''):
        db_file_list += [f]

    # There is an easy improvement here. We know all the files to download already
    # begin a dropbox session and download in parallel. Would speed up a pull greatly.
    for f in files_to_dl:
        full_path = join(root, f['dest'])
        if isfile(full_path):
            os.remove(full_path)
        if not f['path'].lower() in db_file_list:
            print 'Can\'t locate remote data package!'
            return
        src_path = f['path'].lower()
        print "Downloading package %s"%(src_path)
        dbx.files_download_to_file('remote.zip', src_path)

        print "Extracting package from %s"%(src_path)
        with zipfile.ZipFile('remote.zip', 'r') as zip_pkg:
            zip_pkg.extractall()
        # The extract creats a local 'file', move to the dest
        os.rename('file', full_path)

    if isfile('remote.zip'):
        os.remove('remote.zip')

    # update the local manifest
    with open('./../local.manifest', 'wb') as f:
        f.write(json.dumps(manifest, indent=2))


#    try:
#        with open('packaged.sha1.remote', 'rb') as f:
#            remote_txt = f.read()
#    except:
#        remote_txt = 'blah!'
#
#    try:
#        with open('packaged.sha1.local', 'rb') as f:
#            local_txt = f.read()    
#    except:
#        local_txt = 'blah?'
#
#    if remote_txt == local_txt:
#        print "Nothing to do."
#        return
#
#    print "local doesn't match remote. Grabbing latest package."
#
#    with open('scripts/dropbox.oauth2', 'rb') as f:
#        oauth2_token = f.read();
#
#    dbx = dropbox.Dropbox(oauth2_token)
#
#    src_path = '/packaged_'+remote_txt+'.zip'
#
#    print "listing packages..."
#    folder_list = []
#    for f in read_db_directory(dbx, ''):
#        print f
#        folder_list += [f]
#
#    if not src_path.lower() in folder_list:
#        print "Can't locate remote data package!"
#        return
#    
#    print "Downloading package %s"%(src_path)
#    dbx.files_download_to_file('packaged.remote.zip', src_path)
#
#    print "Extracting package from %s"%(src_path)
#    with zipfile.ZipFile('packaged.remote.zip', 'r') as zip_pkg:
#        zip_pkg.extractall()
#
#    os.remove('packaged.remote.zip')

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
