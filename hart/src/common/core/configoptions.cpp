/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#include "hart/core/configoptions.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include <unordered_map>
#include <string>
#include <memory>

namespace hart {
namespace configoptions {

enum class ValueType {
    Uint, 
    Int, 
    Float,
    Bool,
    String,
    None
};

struct ConfigValue {
    ValueType type = ValueType::None;
    union {
        uint32_t uintData;
        int32_t  intData;
        float    floatData;
        bool     boolData;
    };
    std::string  stringData;
};

std::unordered_map<std::string, ConfigValue> cvarTable;

void loadCVars(const char* script_text, size_t text_len) {
    using namespace rapidjson;
    //
    // JSON is such over kill for this, need to look at INI files and command line overrides
    Document config_doc;
    config_doc.Parse(script_text);

    for (Document::MemberIterator member = config_doc.MemberBegin(); member != config_doc.MemberEnd(); ++member) {
        ConfigValue newVar;
        if (member->value.IsBool()) {
            newVar.type = ValueType::Bool;
            newVar.boolData = member->value.GetBool();
        } else if (member->value.IsUint()) {
            newVar.type = ValueType::Uint;
            newVar.uintData = member->value.GetUint();
        } else if (member->value.IsInt()) {
            newVar.type = ValueType::Int;
            newVar.floatData = member->value.GetInt();
        } else if(member->value.IsDouble()) {
            newVar.type = ValueType::Float;
            newVar.floatData = float(member->value.GetDouble());
        } else if(member->value.IsString()) {
            newVar.type = ValueType::String;
            newVar.stringData = member->value.GetString();
        } else 
            continue;
        cvarTable[member->name.GetString()] = newVar;
    }
}

uint32_t getCVarUint(const char* key, uint32_t defval)  {
    ConfigValue const& v = cvarTable[key];
    if (v.type == ValueType::Uint) {
        return v.uintData;
    }
    return defval;
}

int32_t getCVarInt(const char* key, int32_t defval) {
    ConfigValue const& v = cvarTable[key];
    if(v.type == ValueType::Int) {
        return v.intData;
    }
    return defval;
}

float getCVarFloat(const char* key, float defval) {
    ConfigValue const& v = cvarTable[key];
    if(v.type == ValueType::Float) {
        return v.floatData;
    }
    return defval;
}

bool getCVarBool(const char* key, bool defval) {
    ConfigValue const& v = cvarTable[key];
    if(v.type == ValueType::Bool) {
        return v.boolData;
    }
    return defval;
}

const char* getCVarStr(const char* key, const char* defval) {
    ConfigValue const& v = cvarTable[key];
    if(v.type == ValueType::String) {
        return v.stringData.c_str();
    }
    return defval;
}
}
}
