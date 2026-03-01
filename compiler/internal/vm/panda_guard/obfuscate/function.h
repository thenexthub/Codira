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

#ifndef PANDA_GUARD_OBFUSCATE_FUNCTION_H
#define PANDA_GUARD_OBFUSCATE_FUNCTION_H

#include <functional>

#include "compiler/optimizer/ir/graph.h"
#include "mem/arena_allocator.h"

#include "entity.h"
#include "property.h"

namespace panda::guard {

class Node;

using InsTraver = void(InstructionInfo &functionInfo);

enum class FunctionType {
    NONE,
    INSTANCE_FUNCTION,     // '>'
    STATIC_FUNCTION,       // '<'
    CONSTRUCTOR_FUNCTION,  // '='
    NORMAL_FUNCTION,       // '*'
    ENUM_FUNCTION,         // '%'
    NAMESPACE_FUNCTION,    // '&'
};

class Function : public Entity, public IExtractNames {
public:
    Function(Program *program, std::string idx, bool useScope = true)
        : Entity(program), idx_(std::move(idx)), useScope_(useScope)
    {
        this->obfIdx_ = this->idx_;  // obfIdx default equal to idx
    }

    /**
     * Function Init
     * 1. split idx get record, scope, name
     * 2. InitBaseInfo from origin Function
     */
    void Init();

    void Build() override;

    void WriteNameCache(const std::string &filePath) override;

    void ExtractNames(std::set<std::string> &strings) const override;

    void SetExportAndRefreshNeedUpdate(bool isExport) override;

    /**
     * Traverse all instructions of the function
     * @param callback instruction callback
     */
    void EnumerateIns(const std::function<InsTraver> &callback);

    /**
     * Update all reference instruction
     */
    void UpdateReference();

    /**
     * update function annotation reference
     */
    void UpdateAnnotationReference();

    /**
     * To delete the ConsoleLog log, call it after updating other instruction (deleting the log will change the relative
     * order of instruction)
     */
    void RemoveConsoleLog();

    /**
     * when the compact obfuscation option is enabled, uniformly change the line number to 1
     */
    void RemoveLineNumber();

    /**
     * Fill instruction with specified sequence number
     * @param index instruction number
     * @param instInfo instruction to be filled in
     */
    void FillInstInfo(size_t index, InstructionInfo &instInfo);

    /**
     * Retrieve the Graph structure corresponding to the function, which is used to associate instruction calculations
     */
    void GetGraph(compiler::Graph *&outGraph);

    /**
     * Get origin function struct
     * @return origin function struct
     */
    pandasm::Function &GetOriginFunction();

protected:
    void RefreshNeedUpdate() override;

    void Update() override;

    void WriteFileCache(const std::string &filePath) override;

    void WritePropertyCache() override;

    /**
     * Modify the function definition to default to the instruction where the function is defined (definefunc)
     */
    virtual void UpdateDefine() const;

    virtual void InitNameCacheScope();

    [[nodiscard]] std::string GetLines() const;

    [[nodiscard]] virtual bool IsWhiteListOrAnonymousFunction(const std::string &functionIdx) const;

    [[nodiscard]] virtual bool IsNameObfuscated() const;

private:
    void InitBaseInfo();

    void SetFunctionType(char functionTypeCode);

    void UpdateName(const std::shared_ptr<Node> &node);

    void UpdateFunctionTable(const std::shared_ptr<Node> &node) const;

    void BuildPcInsMap(const compiler::Graph *graph);

    void FreeGraph();

public:
    std::optional<Node *> node_ = std::nullopt;
    std::string idx_;
    std::string obfIdx_;
    std::string recordName_;
    std::string rawName_;
    std::string scopeTypeStr_;
    FunctionType type_ = FunctionType::NONE;
    size_t regsNum_ = 0;  // The number of registers requested within the function
    size_t startLine_ = 0;
    size_t endLine_ = 0;
    uint32_t methodPtr_ = 0;  // this field is used for graph analysis
    // own property, Attributes associated with function key:property name
    std::unordered_map<std::string, std::shared_ptr<Property>> propertyTable_ {};
    // Traverse the properties present in the function or parameters
    std::vector<std::shared_ptr<Property>> variableProperties_ {};
    // Object.defineProperty, only when enable ui decorator scene is assigned values
    std::vector<std::shared_ptr<Property>> objectDecoratorProperties_ {};
    std::unordered_map<size_t, size_t> pcInstMap_ {};  // Mapping table between instruction PC and instruction index
    bool useScope_ = true;
    bool nameNeedUpdate_ = true;     // Function name needs to be updated
    bool contentNeedUpdate_ = true;  // Function content (attributes, etc.) needs to be updated
    bool component_ = false;

private:
    bool anonymous_ = false;  // is anonymous function
    compiler::Graph *graph_ = nullptr;
    std::unique_ptr<ArenaAllocator> allocator_ = nullptr;
    std::unique_ptr<ArenaAllocator> localAllocator_ = nullptr;
    std::unique_ptr<compiler::RuntimeInterface> runtimeInterface_ = nullptr;
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_FUNCTION_H
