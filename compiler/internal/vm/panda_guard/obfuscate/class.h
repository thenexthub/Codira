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

#ifndef PANDA_GUARD_OBFUSCATE_CLASS_H
#define PANDA_GUARD_OBFUSCATE_CLASS_H

#include "entity.h"
#include "method.h"
#include "module_record.h"

namespace panda::guard {

class Class final : public Entity, public IExtractNames {
public:
    Class(Program *program, const std::string &constructorIdx) : Entity(program), constructor_(program, constructorIdx)
    {
    }

    void Build() override;

    void WriteNameCache(const std::string &filePath) override;

    /**
     * Traverse all method instructions
     * @param callback instruction callback
     */
    void EnumerateMethodIns(const std::function<InsTraver> &callback);

    /**
     * For Each Function In Class
     * 1. Class.constructor
     * 2. Class.methods(LiteralArray)
     * 3. Class.outerMethods(defined by definemethod)
     */
    void EnumerateFunctions(const std::function<FunctionTraver> &callback);

    void ExtractNames(std::set<std::string> &strings) const override;

protected:
    void Update() override;

    void WriteFileCache(const std::string &filePath) override;

    void WritePropertyCache() override;

private:
    void CreateMethods(const pandasm::LiteralArray &literalArray);

    void CreateMethod(const pandasm::LiteralArray &literalArray, size_t index, bool isStatic);

    void UpdateLiteralArrayIdx();

public:
    ModuleRecord *moduleRecord_ = nullptr;
    std::optional<Node*> node_ = std::nullopt;
    Function constructor_;
    std::string literalArrayIdx_;
    /*
     * Method and OuterMethod Example Explanation:
     * class A {
     *  foo() {}
     *  get v() {}
     * }
     * bytecode:
     *  literalArray: main_738 {...[tag_value:5, string:"foo",...]}
     *
     *  defineclasswithbuffer 0x1 main.#~A=#A, main_738
     *  lda.str v
     *  definemethod main.#~A>#v
     *  definegettersetterbyvalue
     *
     *  foo: Method, in literalArray
     *  get v: OuterMethod, not in literalArray
     */
    // The defineclasswithbuffer instruction associates methods in literalArray
    std::vector<std::shared_ptr<Method>> methods_ {};
    // The external method defined by the definemethod instruction
    std::vector<std::shared_ptr<OuterMethod>> outerMethods_ {};
    /* if is callruntime instruction, special processing is required when parsing the literalArray because the end of
     * literalArray of callruntime instruction is not a static method but running information */
    bool callRunTimeInst_ = false;
    bool component_ = false;  // is UI component class
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_CLASS_H
