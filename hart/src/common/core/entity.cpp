/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "hart/core/entity.h"
#include "hart/core/objectfactory.h"
#include "hart/core/resourcemanager.h"
#include "hart/base/freelist.h"

/********************************************************************

 Possible Entity layout:
 In assets:
   + Object Template defining the list of component types to create with default values. Must define the component name (to be matched in a seperate file to a .fbs file? or just an .fbs filepath)
   ++ When built; Each component is a byte blob within this entity container (uint8* components[total_bytes]; uint32 componentOffsets[no_components];)
   + Object Templates are included in object assets, which can override parameters in the template (via "component"{ "property":new_value }).
   + The object asset is merged down to a collection of json files (one per component) and each is then packed into a flatbuffer and merged into an entity flatbuffer (which is mainly a array with offsets)
   + As entities are resources, they can have prerequisites but will probably need a script to gather these automagically.
   ++ idea for script: List componets and properties that are resource links (i.e. uuids). Script walks all object assets, does the merge down phase (so make this shared code) and gathers any uuids properties it finds.
== OR ==
   + Object Template assets defining the list of component types to create with default values. Must define the component name (to be matched in a seperate file to a .fbs file? or just an .fbs filepath)
   ++ When built; Each component is a byte blob within this entity container (uint8* components[total_bytes]; uint32 componentOffsets[no_components];)
   + Object Templates are separate assets, which are referenced by Object assets. These can override parameters in the template (via "component"{ "property":new_value }).
   ++ When built; Each component is a byte blob within this entity container (uint8* components[total_bytes]; uint32 componentOffsets[no_components];) with a reference to the template Object Template asset
   + As entities are resources, they can have prerequisites but will probably need a script to gather these automagically. The Object Template asset is automatically one of these prerequisites
   ++ idea for script: List componets and properties that are resource links (i.e. uuids). Script walks all object assets and object template assets and gathers any uuids properties it finds.
   + At load the component merges the template version of a componet plus the object version of the component. 

Method 1 is a litter simpler to implement in the runtime. Method 2 would reduce the amount of data loaded at runtime...a lot.

code below implements Method 2.

********************************************************************/

namespace hart {
namespace entity {

static hart::Freelist<ComponentSlot, 65536/sizeof(ComponentSlot)> componentHandles;
static uint16_t componentStamp;

HART_OBJECT_TYPE_DECL(EntityTemplate);
HART_OBJECT_TYPE_DECL(Entity);

bool EntityTemplate::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const& params) {
    params.resdata->persistFileData = true;
    uint8_t const* base_add = in_data->componentData()->data();
    auto* componentOffsets = in_data->componentOffsets();
    componentTemplates.resize(componentOffsets->size());
    for (uint32_t i=0, n=componentOffsets->size(); i<n; ++i) {
        uint8_t const* comp_ptr = base_add + (*componentOffsets)[i];
        // Get typecc from data (see BufferHasIdentifier)
        uint32_t data_typecc = *((uint32_t*)(comp_ptr + sizeof(flatbuffers::uoffset_t)));
        componentTemplates[i].typeCC = data_typecc;
        componentTemplates[i].dataPtr = comp_ptr;
    }
	return true;
}

bool Entity::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const& params) {
    templateEntity = hresmgr::tweakGetResource<EntityTemplate>(huuid::fromData(*in_data->entityTemplate()));

    uint8_t const* base_add = in_data->componentData()->data();
    auto* componentOffsets = in_data->componentOffsets();
    components.reserve(componentOffsets->size());
    for (uint32_t i=0, n=componentOffsets->size(); i<n; ++i) {
        uint8_t const* comp_ptr = base_add + (*componentOffsets)[i];
        // Get typecc from data (see BufferHasIdentifier)
        uint32_t data_typecc = *((uint32_t*)(comp_ptr + sizeof(flatbuffers::uoffset_t)));
        uint8_t const* tpl_comp_ptr = templateEntity->getComponentTemplateData(data_typecc);
        // Template must contain component. entities cannot add new components.
        if (tpl_comp_ptr) {
            hobjfact::ObjectDefinition const* def = hobjfact::getObjectDefinition(data_typecc);
            hdbassert(def && def->component, "No type def of typecc %d", data_typecc);
            ComponentSlot* slot = componentHandles.allocate();
            slot->pointer = def->component(def->objMalloc(), comp_ptr, tpl_comp_ptr);
            slot->stamp = componentStamp++;
            slot->typeCC = data_typecc;
            slot->pointer->initialise(this, slot);
            ComponentHandle hdl;
            hdl.slot = slot;
            hdl.stamp = slot->stamp;
            hdl.typeCC = slot->typeCC;
            components.push_back(hdl);
        }
    }
	return true;
}

}
}

