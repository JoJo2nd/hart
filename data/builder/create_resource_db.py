##
## Create resource_db 
##

##struct uuid {
##    highDword : ulong;
##    lowDword : ulong;
##}
##
##table ResourceInfo {
##    friendlyName:string; // friendly name (for debug, could be stripped in release but isn't ATM)
##    filepath:string; // filepath to open and load
##    prerequisites:[uint]; // indices of assets that must be loaded before this asset
##}
##
##table ResourceList {
##    assetUUIDs:[uuid];
##    assetInfos:[ResourceInfo];
##}

import sys
import json
import os.path
import base64
import uuid
from subprocess import Popen, PIPE

if __name__ == '__main__':
    with open(sys.argv[1]) as fin:
        resource_json = json.load(fin)

    asset_index = {}
    final_output = {}
    l = 0;
    final_output['assetUUIDs'] = []
    final_output['assetInfos'] = []
    for k, i in resource_json.iteritems():
        au = uuid.UUID(i['filepath'][0].replace('.bin', ''))
        final_output['assetUUIDs'] += [{'highword3': (au.int & (0xFFFFFFFF << 96) ) >> 96, 'highword2': (au.int & (0xFFFFFFFF << 64) ) >> 64, 'highword1': (au.int & (0xFFFFFFFF << 32) ) >> 32, 'lowword': (au.int & 0xFFFFFFFF)}]
        asset_index[i['filepath'][0]] = l
        l+=1

    for k, v in resource_json.iteritems():
        final_output['assetInfos'] += [{'friendlyName': k, 'filepath': v['filepath'][0], 'prerequisites': [asset_index[x] for x in v['prerequisites']]}]

    with open(sys.argv[1]+'.fbs.src', 'wb') as f:
        f.write(json.dumps(final_output, indent=2))

    cmdline = ['flatc', '-o', os.path.split(sys.argv[1])[0], '-b', sys.argv[3], sys.argv[1]+'.fbs.src']

    p = Popen(cmdline)
    p.wait()

    try:
        os.remove(sys.argv[2])
    except:
        pass
    os.rename(sys.argv[1]+'.fbs.bin', sys.argv[2])
    os.remove(sys.argv[1]+'.fbs.src')
