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

/**
 * @file
 *
 * This file declares the Mangle Compression.
 */

#ifndef CODIRA_MANGLE_COMPRESSION_H
#define CODIRA_MANGLE_COMPRESSION_H

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Codira::Compression {
enum class EntityType {
    NAME,
    PACKAGE,
    ANONYMOUS,
    GENERAL_FUNCTION,
    GENERIC_FUNCTION,
    GENERIC_DATA,
    EXTEND,
    LAMBDA,
    ERROR_ENTITY_TYPE
};

enum class BaseType {
    PRIMITIVE_TYPE,
    ARRAY_TYPE,
    RAWARRAY_TYPE,
    VARRAY_TYPE,
    TUPLE_TYPE,
    CPOINTER_TYPE,
    CSTRING_TYPE,
    GENERIC_TYPE,
    ENUM_TYPE,
    STRUCT_TYPE,
    CLASS_TYPE,
    FUNCTION_TYPE,
    ERROR_BASE_TYPE
};

struct Entity {
    explicit Entity(const std::string& mangledName, EntityType entityTy) : mangledName(mangledName), entityTy(entityTy)
    {}
    std::string mangledName;
    EntityType entityTy;
    virtual ~Entity() = default;
};

struct CODEType {
    explicit CODEType(const std::string& mangledName, BaseType baseTy) : mangledName(mangledName),
        baseTy(baseTy)
    {
        this->mangledName = mangledName;
    }
    virtual ~CODEType() = default;
    std::string mangledName = "";
    BaseType baseTy = BaseType::ERROR_BASE_TYPE;
};

struct FunctionEntity : public Entity {
    explicit FunctionEntity(const std::string& mangledName, EntityType entityTy,
        std::vector<std::unique_ptr<CODEType>>& paramTys, std::vector<std::unique_ptr<CODEType>>& genericTys)
        : Entity(mangledName, entityTy)
    {
            this->paramTys = std::move(paramTys);
            this->genericTys = std::move(genericTys);
    }
    std::vector<std::unique_ptr<CODEType>> paramTys;
    std::vector<std::unique_ptr<CODEType>> genericTys;
};

struct DataEntity : public Entity {
    explicit DataEntity(const std::string& mangledName, EntityType entityTy,
        std::vector<std::unique_ptr<CODEType>>& genericTys)
        : Entity(mangledName, entityTy)
    {
            this->genericTys = std::move(genericTys);
    }
    std::vector<std::unique_ptr<CODEType>> genericTys;
};

struct ExtendEntity : public Entity {
    explicit ExtendEntity(const std::string& mangledName, EntityType entityTy, std::unique_ptr<CODEType> extendTy,
        const std::string& fileId, const std::string& localId) : Entity(mangledName, entityTy), fileId(fileId),
        localId(localId)
    {
        this->extendTy = std::move(extendTy);
    }
    std::unique_ptr<CODEType> extendTy;
    std::string fileId;
    std::string localId;
};

struct CompositeType : public CODEType {
    explicit CompositeType(const std::string& mangledName, BaseType baseTy,
        std::vector<std::unique_ptr<CODEType>>& genericTys, const std::string& pkg, const std::string& name)
        : CODEType(mangledName, baseTy), pkg(pkg), name(name)
    {
            this->genericTys = std::move(genericTys);
    }
    std::vector<std::unique_ptr<CODEType>> genericTys;
    std::string pkg;
    std::string name;
};

struct FunctionType : public CODEType {
    explicit FunctionType(const std::string& mangledName, BaseType baseTy, std::unique_ptr<CODEType> retTy,
        std::vector<std::unique_ptr<CODEType>>& paramTys) : CODEType(mangledName, baseTy)
    {
            this->retTy = std::move(retTy);
            this->paramTys = std::move(paramTys);
    }
    std::unique_ptr<CODEType> retTy;
    std::vector<std::unique_ptr<CODEType>> paramTys;
};

struct TupleType : public CODEType {
    explicit TupleType(const std::string& mangledName, BaseType baseTy,
        std::vector<std::unique_ptr<CODEType>>& elementTys) : CODEType(mangledName, baseTy)
    {
            this->elementTys = std::move(elementTys);
    }
    std::vector<std::unique_ptr<CODEType>> elementTys;
};

/**
 * @brief Check whether the mangled name is variable decl.
 *
 * @param mangled The mangled name.
 * @return bool If yes, true is returned. Otherwise, false is returned.
 */
bool IsVarDeclEncode(std::string& mangled);

/**
 * @brief Check whether the mangled name is default param function.
 *
 * @param mangled The mangled name.
 * @return bool If yes, true is returned. Otherwise, false is returned.
 */
bool IsDefaultParamFuncEncode(std::string& mangled);

/**
 * @brief Get the index at the end of the type.
 *
 * @param mangled The mangled name.
 * @param tys the vector to save pointers of CODEType.
 * @param isCompressed Whether the mangled name has been compressed.
 * @param idx The start index of the mangled name.
 * @return size_t The end index of the mangled name.
 */
size_t ForwardType(std::string& mangled, std::vector<std::unique_ptr<CODEType>>& tys, bool& isCompressed,
    size_t idx = 0);

/**
 * @brief Get the index at the end of the types.
 *
 * @param mangled The mangled name.
 * @param tys the vector to save pointers of CODEType.
 * @param isCompressed Whether the mangled name has been compressed.
 * @param startId The start index of the mangled name.
 * @return size_t The end index of the mangled name.
 */
size_t ForwardTypes(std::string& mangled, std::vector<std::unique_ptr<CODEType>>& tys, bool& isCompressed,
    size_t startId = 0);

/**
 * @brief Get the index at the end of the name.
 *
 * @param mangled The mangled name.
 * @param isCompressed Whether the mangled name has been compressed.
 * @param idx The start index of the mangled name.
 * @return size_t The end index of the mangled name.
 */
size_t ForwardName(std::string& mangled, bool& isCompressed, size_t idx = 0);

/**
 * @brief Get the index at the end of the number.
 *
 * @param mangled The mangled name.
 * @param idx The start index of the mangled name.
 * @return size_t The end index of the mangled name.
 */
size_t ForwardNumber(std::string& mangled, size_t idx = 0);

/**
 * @brief Main entry of Mangler compression.
 *
 * @param mangled The mangled name.
 * @param isType Whether the mangled name is type.
 * @return std::string The mangled name after compression.
 */
std::string CODEMangledCompression(const std::string mangled, bool isType = false);

/**
 * @brief Try parse path of the mangled name to generate entity vector.
 *
 * @param mangled The mangled name.
 * @param rest The mangled name after removing entities string.
 * @param isCompressed Whether the mangled name has been compressed.
 * @return std::vector<std::unique_ptr<Entity>> The entities.
 */
std::vector<std::unique_ptr<Entity>> TryParsePath(std::string& mangled, std::string& rest, bool& isCompressed);

/**
 * @brief Generate variable decl compressed mangled name.
 *
 * @param entities It belongs to prefix path of variable decl.
 * @param mangled The mangled name.
 * @param compressed The compressed mangled name to be modified.
 * @return bool If generate compressed mangled name success, true is returned. Otherwise, false is returned.
 */
bool SpanningVarDeclTree(std::vector<std::unique_ptr<Entity>>& entities, std::string& mangled,
    std::string& compressed);

/**
 * @brief Generate function decl compressed mangled name.
 *
 * @param entities It belongs to prefix path of function decl.
 * @param mangled The mangled name.
 * @param compressed The compressed mangled name to be modified.
 * @return bool If generate compressed mangled name success, true is returned. Otherwise, false is returned.
 */
bool SpanningFuncDeclTree(std::vector<std::unique_ptr<Entity>>& entities, std::string& mangled,
    std::string& compressed);

/**
 * @brief Generate default param function decl compressed mangled name.
 *
 * @param entities It belongs to prefix path of default param function decl.
 * @param mangled The mangled name.
 * @param compressed The compressed mangled name to be modified.
 * @return bool If generate compressed mangled name success, true is returned. Otherwise, false is returned.
 */
bool SpanningDefaultParamFuncDeclTree(std::vector<std::unique_ptr<Entity>>& entities, std::string& mangled,
    std::string& compressed);

/**
 * @brief Generate compressed mangled name via recursion entity.
 *
 * @param entity Recursed entity.
 * @param treeIdMap The map which key is substring of mangled name, value is compressed index.
 * @param mid The treeIdMap size.
 * @param compressed The compressed mangled name.
 */
void RecursionEntity(const std::unique_ptr<Entity>& entity, std::unordered_map<std::string, size_t>& treeIdMap,
    size_t& mid, std::string& compressed);

/**
 * @brief Generate compressed mangled name via recursion type.
 *
 * @param ty Recursed type.
 * @param treeIdMap The map which key is substring of mangled name, value is compressed index.
 * @param mid The treeIdMap size.
 * @param compressed The compressed mangled name.
 */
void RecursionType(const std::unique_ptr<CODEType>& ty, std::unordered_map<std::string, size_t>& treeIdMap, size_t& mid,
    std::string& compressed, bool isReplaced);

/**
 * @brief Helper function for recursive unit, which is used to update treeIdMap and compressed.
 *
 * @param name The string which may be added to treeIdMap.
 * @param treeIdMap The map which may be updated.
 * @param mid The treeIdMap size.
 * @param compressed The compressed mangled name which may be updated.
 * @param isReplaced Whether the string has been traversed.
 * @param isLeaf Whether the string is leaf.
 * @return bool The end index of the mangled name.
 */
bool RecursionHelper(std::string& name, std::unordered_map<std::string, size_t>& treeIdMap, size_t& mid,
    std::string& compressed, bool isReplaced, bool isLeaf = true);
}  // namespace Codira::Compression
#endif // CODIRA_MANGLE_COMPRESSION_H
