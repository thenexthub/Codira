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

#ifndef PANDA_GUARD_OBFUSCATE_UI_DECORATOR_H
#define PANDA_GUARD_OBFUSCATE_UI_DECORATOR_H

#include "entity.h"
#include "object.h"
#include "property.h"

namespace panda::guard {

enum class UiDecoratorType {
    NONE = 0,
    // create by new
    LINK,
    PROP,
    OBJECT_LINK,
    PROVIDE,  // @State @Watch class object name same with @Provide, so it's all handled by @Provide
    // create by member method
    LOCAL_STORAGE_LINK,
    LOCAL_STORAGE_PROP,
    STORAGE_LINK,
    STORAGE_PROP,
    CONSUME,
    // create by field in params. @Extend will add prefix __TEXT__, inst is definefunc, function obf will handle it
    LOCAL,
    EVENT,  // @BuilderParam is the same as @Event, so it's all handled by @Event
    ONCE,   // @Once must be used together with @Param, so in func_main0 is @Once, constructor is handled by @Param
    PARAM,
    MONITOR,
    ANIMATABLE_EXTEND,
    TRACK,
    TRACE,
    COMPUTED,
    BUILDER,
    CONSUMER,
    PROVIDER,
    EXTERNAL_API,  // @Provide @Watch external api, the inst of this type will attach to @Provide
    REUSABLE_V2
};

class UiDecorator final : public PropertyOptionEntity, public IExtractNames {
public:
    explicit UiDecorator(Program *program, const std::unordered_map<std::string, std::shared_ptr<Object>> &objectTable)
        : PropertyOptionEntity(program), objectTableRef_(objectTable)
    {
    }

    static bool IsUiDecoratorIns(const InstructionInfo &info, Scope scope);

    void ExtractNames(std::set<std::string> &strings) const override;

    void Build() override;

    void AddDefineInsList(const std::vector<InstructionInfo> &instLIst);

    [[nodiscard]] bool IsValidUiDecoratorType() const;

protected:
    void Update() override;

private:
    // handled ins in func_main0
    void HandleInstInFuncMain0();

    // handled ins in function
    void HandleInstInFunction();

    void HandleStObjByNameIns();

    void HandleNewObjRangeIns(const InstructionInfo &info);

    /*
     * e.g.
     * @Monitor("age", "name") --> need obfuscated age and name
     * @Monitor("info.name") --> need obfuscated info and name
     */
    void BuildMonitorDecorator();

    void BuildCreatedByMemberMethodDecorator(const InstructionInfo &info, uint32_t paramIndex);

    void BuildCreatedByMemberFieldDecorator();

    void BuildEventDecorator();

    void UpdateMonitorDecorator();

    [[nodiscard]] bool IsMonitorUiDecoratorType() const;

    void AddDefineInsList(const InstructionInfo &ins);

    /*
     * e.g.
     * @ReusableV2
     * @ComponentV2
     * struct XXX {}
     *
     * idx_001 { 4 [tag_value:5, string:"getReuseId", tag_value:255, null_value:0] }
     * function getReuseId { lda.str XXX }
     * function foo {
     *   createobjectwithbuffer idx_001
     *   sta v2
     *   definefunc getReuseId
     *   definepropertybyname getReuseId, v2
     *   callthis1 v2
     * }
     *
     * 1. find callthi1, then find v2(object idx_001)
     * 2. check object idx_001 contains getReuseId
     * 3. find the function associated with the property getReuseId, and then iterate through its instructions to obtain
     * the decorator
     */
    void BuildReusableV2Decorator();

    [[nodiscard]] bool IsReusableV2UiDecoratorType() const;

    std::shared_ptr<Property> GetObjectOuterProperty(const std::string &literalArrayIdx) const;

    void EnumerateIns(const InstructionInfo &inst);

public:
    InstructionInfo baseInst_ {};  // obtain ui decorator information by this inst

private:
    UiDecoratorType type_ = UiDecoratorType::NONE;

    const std::unordered_map<std::string, std::shared_ptr<Object>> &objectTableRef_;
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_UI_DECORATOR_H
