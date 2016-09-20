/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "hart/config.h"
#include "hart/base/uuid.h"
#include "hart/core/objectfactory.h"
#include "hart/fbs/entity_generated.h"
#include "hart/fbs/entitytemplate_generated.h"

namespace hart {
namespace entity {

class Entity;
struct ComponentSlot;

class Component {
    Entity* owner = nullptr;
    ComponentSlot* slot = nullptr;
public:
    virtual ~Component() {}
    void initialise(Entity* in_owner, ComponentSlot* in_slot) {
        owner=in_owner;
        slot=in_slot;
    }

    //hEntity* getOwner() const { return owner; }
};

struct ComponentSlot {
    Component* pointer;
    uint16_t stamp;
    uint32_t typeCC;
};

struct ComponentHandle {
    ComponentSlot* slot; 
    uint32_t typeCC;
    uint16_t stamp;
};


class EntityTemplate {
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('e','t','p','l'), resource::EntityTemplate)
public:
    struct ComponentTemplate {
        uint32_t typeCC;
        uint8_t const* dataPtr;
    };
    hstd::vector<ComponentTemplate> componentTemplates;
    uint8_t const* getComponentTemplateData(uint32_t typecc) {
        for (auto const& i : componentTemplates) {
            if (i.typeCC == typecc) {
                return i.dataPtr;
            }
        }
        return nullptr;
    }
};

class Entity {
    HART_OBJECT_TYPE(HART_MAKE_FOURCC('e','n','t','y'), resource::Entity)
public:
    template < typename t_ty >
    t_ty* getComponent() {
        for (const auto& i : components) {
            if (i.typeCC == t_ty::getTypeCC())
                return static_cast<t_ty*>(i.ptr);
        }
        return nullptr;
    }
    uint32_t getComponentCount() const { return (uint32_t)components.size(); } 
    huuid::uuid_t getEntityID() const { return entityId; }
    const std::vector<ComponentHandle>& getComponents() const { return components; }
#if HART_DEBUG_INFO
    const char* getFriendlyName() const { return friendlyName.c_str(); }
    void setFriendlyName(const char* name) { friendlyName = name; }
#endif

private:
    EntityTemplate* templateEntity;
    huuid::uuid_t entityId;
    hstd::vector<ComponentHandle> components;
#if HART_DEBUG_INFO
    hstd::string friendlyName;
#endif
};

}
}

namespace hety = hart::entity;
