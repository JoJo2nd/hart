/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#include "hart/core/objectfactory.h"
#include "hart/base/std.h"
#include "hart/base/debug.h"
#include "flatbuffers/flatbuffers.h"

namespace hart {
namespace objectfactory {

typedef std::unordered_map<uint32_t, ObjectDefinition> ObjectDefinitionTable;

static bool                  doneGlobalInit = false;
static ObjectDefinitionTable objectDefTable;

ObjectDefinition const* getObjectDefinition(uint32_t typecc) {
  ObjectDefinition const& definition = objectDefTable[typecc];
  return &definition;
}

bool objectFactoryRegister(ObjectDefinition const& definition, void* user) {
  if (!definition.typecc) {
    return false;
  }
  hdbassert(objectDefTable.find(definition.typecc) == objectDefTable.end(),
            "Registering type 0x%08X twice with the object factory", definition.typecc);
  objectDefTable[definition.typecc] = definition;
  objectDefTable[definition.typecc].user = user;
  return true;
}

void* deserialiseObject(void const* data, size_t len, SerialiseParams* params, uint32_t* out_typecc) {
  hdbassert(params, "params must not be null");
  // Get typecc from data (see BufferHasIdentifier)
  uint32_t data_typecc = *((uint32_t*)((char const*)(data) + sizeof(flatbuffers::uoffset_t)));
  // read data
  ObjectDefinition const& definition = objectDefTable[data_typecc];
  if (!definition.typecc) {
    hdbfatal("Unknown typecc 0x%08X", data_typecc);
    return nullptr;
  }

  params->user = definition.user;
  // alloc & construct
  void* vobj = definition.objMalloc();
  definition.construct(vobj);
  // deseralise
  definition.deserialise(data, vobj, *params);
  // update type info
  *out_typecc = data_typecc;
  // return new object
  return vobj;
}
}
}
