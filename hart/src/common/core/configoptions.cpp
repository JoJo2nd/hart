/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#include "hart/core/configoptions.h"
#include "hart/base/debug.h"
#include "hart/base/std.h"

namespace hart {
namespace configoptions {

struct IniValue {
  hstd::string value;
};

typedef hstd::unordered_map<hstd::string, IniValue> IniValuePairs;
hstd::unordered_map<hstd::string, IniValuePairs>    iniSectionTable;

bool loadConfigOptions(const char* script_text, size_t text_len) {
  IniValuePairs* current_section = &iniSectionTable[""];
  char const*    tptr = script_text;
  char const*    etptr = tptr + text_len;
  while (tptr < etptr) {
    if (*tptr == '[') {
      // parse section name
      char const* section_begin = tptr + 1;
      while (tptr < etptr && *tptr != ']')
        ++tptr;
      char const* section_end = tptr;
      if (section_begin >= section_end) {
        hdbfatal("Section name is invalid");
        return false;
      }
      current_section = &iniSectionTable[hstd::string(section_begin, section_end)];
      // skip to end of line
      while (tptr < etptr && *tptr != '\n' && *tptr != '\r')
        ++tptr;
    } else if (*tptr == ';') {
      // comment skip
      while (tptr < etptr && *tptr != '\n' && *tptr != '\r')
        ++tptr;
    } else if (!hcrt::isspace(*tptr)) {
      // start key parse
      char const* name_start = tptr;
      while (tptr < etptr && *tptr != '=' && !hcrt::isspace(*tptr))
        ++tptr;
      char const* name_end = tptr;
      // skip any trailing whitespace
      while (tptr < etptr && *tptr != '=')
        ++tptr;
      // run out of buffer mid parse?
      if (tptr == etptr || name_start >= name_end) {
        hdbfatal("name is missing value");
        return false;
      }
      // skip '='
      ++tptr;
      while (tptr < etptr && hcrt::isspace(*tptr))
        ++tptr;
      if (tptr == etptr) {
        hdbfatal("failed parsing value");
        return false;
      }
      char const* value_start = tptr;
      char const* value_end = value_start;
      while (tptr < etptr && *tptr != '\n' && *tptr != '\r') {
        if (!hcrt::isspace(*tptr)) {
          value_end = tptr + 1;
        }
        ++tptr;
      }

      if (value_start == value_end) {
        hdbfatal("Failed to parse value");
        return false;
      }

      hstd::string name(name_start, name_end);
      IniValue&    val = (*current_section)[name];
      val.value = hstd::string(value_start, value_end);
    } else {
      ++tptr;
    }
  }

  return true;
}

uint32_t getUint(const char* section, const char* key, uint32_t defval) {
  IniValue const& v = iniSectionTable[section][key];
  if (!v.value.empty()) {
    return hcrt::strtoul(v.value.c_str(), nullptr, 10);
  }
  return defval;
}

int32_t getInt(const char* section, const char* key, int32_t defval) {
  IniValue const& v = iniSectionTable[section][key];
  if (!v.value.empty()) {
    return hcrt::strtol(v.value.c_str(), nullptr, 10);
  }
  return defval;
}

float getFloat(const char* section, const char* key, float defval) {
  IniValue const& v = iniSectionTable[section][key];
  if (!v.value.empty()) {
    return float(hcrt::strtod(v.value.c_str(), nullptr));
  }
  return defval;
}

bool getBool(const char* section, const char* key, bool defval) {
  IniValue const& v = iniSectionTable[section][key];
  if (!v.value.empty()) {
    return hcrt::stricmp(v.value.c_str(), "false") != 0;
  }
  return defval;
}

const char* getStr(const char* section, const char* key, const char* defval) {
  IniValue const& v = iniSectionTable[section][key];
  if (!v.value.empty()) {
    return v.value.c_str();
  }
  return defval;
}
}
}
