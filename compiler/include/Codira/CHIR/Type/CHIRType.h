/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef CODIRA_CHIR_CHIRTYPE_H
#define CODIRA_CHIR_CHIRTYPE_H

#include "Codira/AST/Types.h"
#include "Codira/CHIR/AST2CHIR/AST2CHIRNodeMap.h"
#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/Type/Type.h"

#include <mutex>

namespace Codira::CHIR {

class CHIRTypeCache {
public:
    std::unordered_map<AST::Ty*, Type*>& typeMap; // AST::Type -> CHIR::Type
    // cache custom type(interface decl, class decl, struct decl, enum decl), except for extend decl
    AST2CHIRNodeMap<CustomTypeDef> globalNominalCache;
    explicit CHIRTypeCache(std::unordered_map<AST::Ty*, Type*>& typeMap) : typeMap(typeMap)
    {
    }
    explicit CHIRTypeCache(
        std::unordered_map<AST::Ty*, Type*>& typeMap, const AST2CHIRNodeMap<CustomTypeDef>& globalNominalCache)
        : typeMap(typeMap), globalNominalCache(globalNominalCache)
    {
    }
};
// typeTranslate
class CHIRType {
public:
    explicit CHIRType(CHIRBuilder& builder, CHIRTypeCache& typeCache) : builder(builder), chirTypeCache(typeCache)
    {
    }
    ~CHIRType() = default;

    /**
     * @brief Translates an AST type to a CHIR type.
     *
     * @param ty The AST type to be translated.
     * @return The translated CHIR type.
     */
    Type* TranslateType(AST::Ty& ty);
    
    /**
     * @brief Fills the generic argument types.
     *
     * @param ty The AST generics type to be processed.
     */
    void FillGenericArgType(AST::GenericsTy& ty);
    /* Notice that chirTypeCache.globalNominalCache is non-thread-safe, so SetGlobalNominalCache only be invoked
     * serially. Concurrent execution of SetGlobalNominalCache is not advisable.
     */
    void SetGlobalNominalCache(const AST::Decl& decl, CustomTypeDef& def)
    {
        chirTypeCache.globalNominalCache.Set(decl, def);
    }
    Ptr<CustomTypeDef> GetGlobalNominalCache(const AST::Decl& decl) const
    {
        return chirTypeCache.globalNominalCache.Get(decl);
    }
    Ptr<CustomTypeDef> TryGetGlobalNominalCache(const AST::Decl& decl) const
    {
        return chirTypeCache.globalNominalCache.TryGet(decl);
    }
    bool Has(const AST::Decl& decl) const
    {
        return chirTypeCache.globalNominalCache.Has(decl);
    }

    const std::unordered_map<const AST::Node*, CustomTypeDef*>& GetAllTypeDef() const
    {
        return chirTypeCache.globalNominalCache.GetALL();
    }
    std::unordered_map<AST::Ty*, Type*>& GetTypeMap() const
    {
        return chirTypeCache.typeMap;
    }
    const AST2CHIRNodeMap<CustomTypeDef>& GetGlobalNominalCache() const
    {
        return chirTypeCache.globalNominalCache;
    }

private:
    Type* TranslateTupleType(AST::TupleTy& tupleTy);
    Type* TranslateFuncType(const AST::FuncTy& fnTy);
    Type* TranslateStructType(AST::StructTy& structTy);
    Type* TranslateClassType(AST::ClassTy& classTy);
    Type* TranslateInterfaceType(AST::InterfaceTy& interfaceTy);
    Type* TranslateEnumType(AST::EnumTy& enumTy);
    Type* TranslateArrayType(AST::ArrayTy& arrayTy);
    Type* TranslateVArrayType(AST::VArrayTy& varrayTy);
    Type* TranslateCPointerType(AST::PointerTy& pointerTy);
    CHIRBuilder& builder;
    CHIRTypeCache& chirTypeCache;
    // mutex for translateType. TranslateType is recursive, so we use the recursive mutex.
    static std::recursive_mutex chirTypeMtx;
};
} // namespace Codira::CHIR

#endif
