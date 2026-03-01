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

#ifndef PANDA_GUARD_OBFUSCATE_PROGRAM_H
#define PANDA_GUARD_OBFUSCATE_PROGRAM_H

#include "node.h"

namespace panda::guard {

class Program final : public IEntity {
public:
    explicit Program(pandasm::Program *program) : prog_(program) {}

    void Create() override;

    void Obfuscate() override;

private:
    void CreateNode(const pandasm::Record &record);

    void CreateAnnotation(const pandasm::Record &record);

    void EnumerateFunctions(const std::function<FunctionTraver> &callback);

    void RemoveConsoleLog();

    void RemoveLineNumber();

    void UpdateReference();

public:
    pandasm::Program *prog_ = nullptr;
    std::map<std::string, std::shared_ptr<Node>> nodeTable_ {};
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_PROGRAM_H
