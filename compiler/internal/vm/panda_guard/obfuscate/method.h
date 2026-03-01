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

#ifndef PANDA_GUARD_OBFUSCATE_METHOD_H
#define PANDA_GUARD_OBFUSCATE_METHOD_H

#include "function.h"

namespace panda::guard {

class Method final : public Function {
public:
    Method(Program *program, const std::string &idx) : Function(program, idx) {}

protected:
    void RefreshNeedUpdate() override;

    void WriteFileCache(const std::string &filePath) override;

    void WritePropertyCache() override;

private:
    void InitNameCacheScope() override;

    void UpdateDefine() const override;

public:
    std::string literalArrayIdx_;
    std::string className_;
    bool static_ = false;
    size_t idxIndex_ = 0;
    size_t nameIndex_ = 0;
};

class OuterMethod final : public Function {
public:
    OuterMethod(Program *program, const std::string &idx) : Function(program, idx) {}

    /**
     * 设置 内容是否应该混淆
     */
    void SetContentNeedUpdate(bool toUpdate);

    std::string GetName() const override;

    std::string GetObfName() const override;

    void Build() override;

protected:
    void Update() override;

    void WriteFileCache(const std::string &filePath) override;

    void WritePropertyCache() override;

    bool IsNameObfuscated() const override;

private:
    void RefreshNeedUpdate() override;

    void UpdateNameDefine();

public:
    std::string className_;
    std::string nameDefine_;
    std::string obfNameDefine_;
    InstructionInfo nameInfo_ {};
};

class PropertyMethod final : public Function {
public:
    PropertyMethod(Program *program, const std::string &idx) : Function(program, idx) {}

    /**
     * Should the setting content be confused
     */
    void SetContentNeedUpdate(bool toUpdate);

private:
    void RefreshNeedUpdate() override;
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_METHOD_H
