/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once
#include "hart/config.h"

namespace hart {
namespace tls {

typedef void (HART_API *KeyDestructor)(void* key_value);

size_t createKey(KeyDestructor destructor);
void   deleteKey(size_t key);
void   setKeyValue(size_t key, void* value);
void*  getKeyValue(size_t key);

#if HART_PLATFORM == HART_PLATFORM_WINDOWS

void threadExit();

#endif

}	
}    

namespace htls = hart::tls;
