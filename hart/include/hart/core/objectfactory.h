/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

#include "hart/config.h"
#include "hart/base/std.h"
#include <vector>

namespace hart {
namespace resourcemanager {
    struct ResourceLoadData;
}
namespace entity {
    class Component;
}
}

namespace hart {
namespace objectfactory {
    typedef void* serialisedobj_t;

    template< typename t_ty >
    struct allocsize_t {
        uint8_t pad[sizeof(t_ty)];
    };

    struct SerialiseParams {
        hart::resourcemanager::ResourceLoadData* resdata = nullptr;
        void* user = nullptr;
    };

    template< typename t_ty, typename t_ty2 = t_ty::MarshallType >
    struct typehelper_t {
        typedef t_ty objType;
        typedef t_ty2 serialiserType;

        static void* mallocType() {
            return new allocsize_t<t_ty>();
        }
        static void freeType(void* ptr) {
            delete ((allocsize_t<t_ty>*)ptr);
        }
        static void constructType(void* ptr) {
            new ((t_ty*)ptr) t_ty();
        }
        static void destructType(void* ptr) {
            ((t_ty*)ptr)->~t_ty();   
        }
        static bool deserialiseType(void const* src, void* dst, SerialiseParams const& params) {
            t_ty* type_ptr = reinterpret_cast<t_ty*>(dst);
            serialiserType const* src_t = flatbuffers::GetRoot<serialiserType>(src);
            return type_ptr->deserialiseObject(src_t, params);
        }
        static entity::Component* constructTypeAsComponent(void* mem) {
            t_ty* r = new (mem) t_ty();
            return static_cast<entity::Component*>(r);
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    typedef void* (*ObjectMallocProc)();
    typedef void (*ObjectFreeProc)(void* obj_ptr);
    typedef void (*ObjectConstructProc)(void* in_place);
    typedef void (*ObjectDestructProc)(void* obj_ptr);
    typedef bool (*ObjectDeserialiseProc)(void const* src, void* dst, SerialiseParams const& p);
    typedef entity::Component* (*ObjectComponentProc)(void const* mem);

    struct ObjectDefinition {
        ObjectDefinition() = default;
        ObjectDefinition(
            uint32_t in_typecc,
            const char* in_objectName,
            size_t in_typeSize,
            ObjectMallocProc in_objMalloc,
            ObjectFreeProc in_objFree,
            ObjectConstructProc in_construct,
            ObjectDestructProc in_destruct,
            ObjectDeserialiseProc in_deserialise,
            ObjectComponentProc in_component,
            void* in_user
            ) 
            : typecc(in_typecc)
            , objectName(in_objectName)
            , typeSize(in_typeSize)
            , objMalloc(in_objMalloc)
            , objFree(in_objFree)
            , construct(in_construct)
            , destruct(in_destruct)
            , deserialise(in_deserialise)
            , component(in_component)
            , user(in_user)
        {

        }

        uint32_t                typecc = 0;
        hstd::string            objectName;
        size_t                  typeSize = 0;
        ObjectMallocProc        objMalloc = nullptr;
        ObjectFreeProc          objFree = nullptr;
        ObjectConstructProc     construct = nullptr;
        ObjectDestructProc      destruct = nullptr;
        ObjectDeserialiseProc   deserialise = nullptr;
        ObjectComponentProc     component = nullptr;
        void*                   user = nullptr;
    };

#define HART_OBJECT_TYPE(typecc, serialiser_type) \
    private:\
    static hobjfact::ObjectDefinition typeDef; \
    public: \
    static uint32_t getTypeCC() { return typecc; } \
    typedef serialiser_type MarshallType; \
    static hobjfact::ObjectDefinition const& getObjectDefinition() { return typeDef; } \
    bool deserialiseObject(MarshallType const*, hobjfact::SerialiseParams const&); \
    private: 


#define HART_OBJECT_TYPE_DECL(type) \
    hobjfact::ObjectDefinition \
        type \
         ::typeDef( \
        type::getTypeCC(), \
        #type, \
        sizeof(type),\
        hobjfact::typehelper_t< type >::mallocType, \
        hobjfact::typehelper_t< type >::freeType, \
        hobjfact::typehelper_t< type >::constructType, \
        hobjfact::typehelper_t< type >::destructType, \
        hobjfact::typehelper_t< type >::deserialiseType, \
        nullptr, /** No component constructor */\
        nullptr \
    )

#define HART_COMPONENT_OBJECT_TYPE_DECL(type) \
    hobjfact::ObjectDefinition \
        type \
         ::typeDef( \
        type::getTypeCC(), \
        #type, \
        sizeof(type),\
        hobjfact::typehelper_t< type >::mallocType, \
        hobjfact::typehelper_t< type >::freeType, \
        hobjfact::typehelper_t< type >::constructType, \
        hobjfact::typehelper_t< type >::destructType, \
        hobjfact::typehelper_t< type >::deserialiseType, \
        hobjfact::typehelper_t< type >::constructTypeAsComponent, \
        nullptr \
    )


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
    
const ObjectDefinition*        getObjectDefinition(uint32_t typecc);
void*                          deserialiseObject(void const* data, size_t len, SerialiseParams* params, uint32_t* out_typecc);
entity::Component*             createComponentObject(void const* data, size_t len, SerialiseParams* params, uint32_t* out_typecc);
bool                           objectFactoryRegister(ObjectDefinition const& obj_def, void* user);

}
}

namespace hobjfact = hart::objectfactory;
