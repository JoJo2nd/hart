/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"

namespace hart {
namespace configoptions {

void        loadCVars(const char* script_text, size_t text_len);
uint32_t    getCVarUint(const char* key, uint32_t defval);
int32_t     getCVarInt(const char* key, int32_t defval);
float       getCVarFloat(const char* key, float defval);
bool        getCVarBool(const char* key, bool defval);
const char* getCVarStr(const char* key, const char* defval);

}
}

namespace hconfigopt = hart::configoptions;