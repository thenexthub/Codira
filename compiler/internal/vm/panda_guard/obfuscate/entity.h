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

#ifndef PANDA_GUARD_OBFUSCATE_ENTITY_H
#define PANDA_GUARD_OBFUSCATE_ENTITY_H

#include "instruction_info.h"

namespace panda::guard {

enum Scope {
    NONE,
    TOP_LEVEL,
    FUNCTION,
};

enum ParamIndex {
    INDEX_0,
    INDEX_1,
    INDEX_2,
    INDEX_3,
};

class IExtractNames {
public:
    virtual ~IExtractNames() = default;

    /**
     * extract all export names
     * @param strings strings set
     */
    virtual void ExtractNames(std::set<std::string> &strings) const = 0;
};

using FunctionTraver = void(Function &function);

class IEntity {
public:
    virtual ~IEntity() = default;

    /**
     * Create Entity
     */
    virtual void Create() = 0;

    /**
     * Obfuscate Entity
     */
    virtual void Obfuscate() = 0;
};

class Program;

class Entity : public IEntity {
public:
    explicit Entity(Program *program) : program_(program) {}

    Entity(Program *program, const std::string &name) : program_(program), name_(name), obfName_(name) {}

    /**
     * set entity and it's members export
     * @param isExport entity is export
     */
    virtual void SetExportAndRefreshNeedUpdate(bool isExport);

    /**
     * get Entity is export
     * @return isExport
     */
    [[nodiscard]] bool IsExport() const;

    /**
     * Create Entity
     * 1. Build Entity
     * 2. RefreshNeedUpdate
     */
    void Create() final;

    /**
     * Obfuscate Entity
     * 1. needUpdate true->2 false->return
     * 2、Update name
     * @return result code
     */
    void Obfuscate() final;

    /**
     * Write NameCacheInfo to NameCacheEntity
     * 1. Write NameCache to FileCache
     * 2. Write NameCache to PropertyCache
     * @param filePath File name before obfuscation
     */
    virtual void WriteNameCache(const std::string &filePath);

    /**
     * Implement this method when obfuscated is not required but writing nameCache is needed
     */
    virtual void WriteNameCache();

    /**
     * origin name of Entity
     */
    [[nodiscard]] virtual std::string GetName() const;

    /**
     * obf name of Entity
     */
    [[nodiscard]] virtual std::string GetObfName() const;

protected:
    /**
     * Build Entity
     */
    virtual void Build();

    /**
     * Refresh needUpdate by configs
     */
    virtual void RefreshNeedUpdate();

    /**
     * Write obfuscation mapping to the IdentifierCache and MemberMethodCache nodes of the obfuscated files in the
     * NameCache file
     * @param filePath File name before obfuscation
     */
    virtual void WriteFileCache(const std::string &filePath);

    /**
     * Write obfuscation mapping to the PropertyCache nodes of the obfuscated files in the NameCache
     */
    virtual void WritePropertyCache();

    /**
     * Update Entity name
     */
    virtual void Update();

    /**
     * set nameCacheScope
     * 1. SCOPE=TOP_LEVEL nameCacheScope: name
     * 2. SCOPE=FUNCTION nameCacheScope: functionScope#name
     * @param name nameCache name
     */
    void SetNameCacheScope(const std::string &name);

    [[nodiscard]] std::string GetNameCacheScope() const;

    void UpdateLiteralArrayTableIdx(const std::string &originIdx, const std::string &updatedIdx) const;

public:
    Program *program_ = nullptr;
    Scope scope_ = NONE;
    std::string nameCacheScope_;
    std::string name_;
    std::string obfName_;
    bool export_ = false;
    bool needUpdate_ = true;
    bool obfuscated_ = false;
    // define instruction list, when entity define in finally block, define ins maybe occurs multiple times
    std::vector<InstructionInfo> defineInsList_ {};
};

class TopLevelOptionEntity : public Entity {
public:
    static bool NeedUpdate(const Entity &entity);

    static void WritePropertyCache(const Entity &entity);

    explicit TopLevelOptionEntity(Program *program) : Entity(program) {}

    void WritePropertyCache() override;

protected:
    void RefreshNeedUpdate() override;
};

class PropertyOptionEntity : public Entity {
public:
    static bool NeedUpdate(const Entity &entity);

    static void WritePropertyCache(const Entity &entity);

    explicit PropertyOptionEntity(Program *program) : Entity(program) {}

    void WritePropertyCache() override;

protected:
    void RefreshNeedUpdate() override;
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_ENTITY_H
