/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

#include "hart/config.h"
#include "hart/base/std.h"
#include <vector>

namespace hart {
namespace objectfactory {
    typedef void* serialisedobj_t;

    template< typename t_ty >
    struct allocsize_t {
        uint8_t pad[sizeof(t_ty)];
    };

    struct SerialiseParams {
        void* user;
    };

    template< typename t_ty, typename t_ty2 = t_ty::MarshallType >
    struct typehelper_t {
        typedef t_ty objType;
        typedef t_ty2 serialiserType;

        static void* mallocType() {
            return new allocsize_t<t_ty>();
        }
        static void freeType(void* ptr) {
            delete ((allocsize_t<t_ty>*)d);
        }
        static void constructType(void* ptr) {
            new ((t_ty*)ptr) t_ty();
        }
        static void destructType(void* ptr) {
            ((t_ty*)ptr)->~t_ty();   
        }
        static bool deserialiseType(void const* src, void* dst, SerialiseParams const& params) {
            t_ty* type_ptr = reinterpret_cast<t_ty*>(dst);
            serialiserType* src_t = static_cast<serialiserType*>(src);
            return type_ptr->deserialiseObject(src_t, params);
        }
        static bool serialiseType(void const* src, void** dst_ptr, SerialiseParams const& params) {
            t_ty* type_ptr = reinterpret_cast<t_ty*>(type_ptr_raw);
            serialiserType** dst_ptr_t = static_cast<serialiserType**>(dst_ptr);
            return type_ptr->serialiseObject(dst_ptr_t, params);
        }
        static bool linkType(void* type_ptr_raw) {
            t_ty* type_ptr = reinterpret_cast<t_ty*>(type_ptr_raw);
            return type_ptr->linkObject();
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    typedef void* (*ObjectMallocProc)();
    typedef void* (*ObjectConstructProc)(void* in_place);
    typedef void (*ObjectDestructProc)(void* obj_ptr);
    typedef void (*ObjectFreeProc)(void* obj_ptr);
    typedef bool (*ObjectDeserialiseProc)(void const* src, void* dst, SerialiseParams const& p);
    typedef bool (*ObjectSerialiseProc)(void** dst, void const* src, SerialiseParams const& p);
    typedef bool (*ObjectLinkProc)(void*);

    struct ObjectDefinition {
        uint32_t                typecc = 0;
        hstd::string            objectName;
        size_t                  typeSize = 0;
        ObjectMallocProc        objMalloc = nullptr;
        ObjectFreeProc          objFree = nullptr;
        ObjectConstructProc     construct = nullptr;
        ObjectDestructProc      destruct = nullptr;
        ObjectDeserialiseProc   deserialise = nullptr;
        ObjectSerialiseProc     serialise = nullptr;
        ObjectLinkProc          link = nullptr;
        void*                   user = nullptr;
    };

#define HART_OBJECT_TYPE(name, typecc, serialiser_type) \
    private:\
    static hobjfact::ObjectDefinition typeDef; \
    public: \
    typedef serialiser_type MarshallType; \
    static hobjfact::ObjectDefinition const& getObjectDefinition() { return typeDef; } \
    bool deserialiseObject(MarshallType const*, hobjfact::SerialiseParams const&); \
    bool serialiseObject(MarshallType**, hobjfact::SerialiseParams const&) const; \
    bool linkObject(); \
    private: 


#define HART_OBJECT_TYPE_DECL(name, type) \
    static hobjfact::ObjectDefinition type::typeDef = { \
        type::getTypeCC(), \
        type::getTypeNameStatic(), \
        sizeof(type),\
        hobjfact::typehelper_t::mallocType, \
        hobjfact::typehelper_t::freeType, \
        hobjfact::typehelper_t::constructType, \
        hobjfact::typehelper_t::destructType, \
        hobjfact::typehelper_t::deserialiseType, \
        hobjfact::typehelper_t::serialiseType, \
        hobjfact::typehelper_t::linkType, \
        nullptr, \
    };


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
    
const ObjectDefinition*        getObjectDefinition(uint32_t typecc);
void*                          deserialiseObject(void const* data, size_t len, uint32_t* out_typecc);
bool                           objectFactoryRegistar(ObjectDefinition const& obj_def, void* user);

}
}

namespace hobjfact = hart::objectfactory;
