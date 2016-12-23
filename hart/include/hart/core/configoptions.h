/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"

namespace hart {
namespace configoptions {

bool loadConfigOptions(const char* script_text, size_t text_len);
uint32_t getUint(const char* section, const char* key, uint32_t defval);
int32_t getInt(const char* section, const char* key, int32_t defval);
float getFloat(const char* section, const char* key, float defval);
bool getBool(const char* section, const char* key, bool defval);
const char* getStr(const char* section, const char* key, const char* defval);
}
}

namespace hconfigopt = hart::configoptions;