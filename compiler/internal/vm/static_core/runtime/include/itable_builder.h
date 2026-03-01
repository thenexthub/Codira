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
#ifndef PANDA_RUNTIME_ITABLE_BUILDER_H_
#define PANDA_RUNTIME_ITABLE_BUILDER_H_

#include "libarkbase/macros.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/mem/panda_smart_pointers.h"

namespace ark {

class ClassLinker;

class ITableBuilder {
public:
    ITableBuilder() = default;

    [[nodiscard]] virtual bool Build(ClassLinker *classLinker, Class *base, Span<Class *> classInterfaces,
                                     bool isInterface) = 0;

    [[nodiscard]] virtual bool Resolve(Class *klass) = 0;

    virtual void UpdateClass(Class *klass) = 0;

    virtual void DumpITable(Class *klass) = 0;

    virtual ITable GetITable() const = 0;

    virtual ~ITableBuilder() = default;

    NO_COPY_SEMANTIC(ITableBuilder);
    NO_MOVE_SEMANTIC(ITableBuilder);
};

class DummyITableBuilder final : public ITableBuilder {
public:
    bool Build([[maybe_unused]] ClassLinker *classLinker, [[maybe_unused]] Class *base,
               [[maybe_unused]] Span<Class *> classInterfaces, [[maybe_unused]] bool isInterface) override
    {
        return true;
    }

    bool Resolve([[maybe_unused]] Class *klass) override
    {
        return true;
    }

    void UpdateClass([[maybe_unused]] Class *klass) override {}

    void DumpITable([[maybe_unused]] Class *klass) override {}

    ITable GetITable() const override
    {
        return ITable();
    }
};

}  // namespace ark

#endif  // PANDA_RUNTIME_ITABLE_BUILDER_H_
