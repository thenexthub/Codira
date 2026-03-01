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
#ifndef PANDA_RUNTIME_IMTABLE_BUILDER_H_
#define PANDA_RUNTIME_IMTABLE_BUILDER_H_

#include "libarkbase/macros.h"
#include "libarkfile/class_data_accessor.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/mem/panda_smart_pointers.h"

namespace ark {

class ClassLinker;

class IMTableBuilder {
public:
    static constexpr uint32_t OVERSIZE_MULTIPLE = 2;

    void Build(const panda_file::ClassDataAccessor *cda, ITable itable);

    void Build(ITable itable, bool isInterface);

    void UpdateClass(Class *klass);

    bool AddMethod(ark::Span<ark::Method *> imtable, uint32_t imtableSize, uint32_t id, Method *method);

    void DumpIMTable(Class *klass);

    size_t GetIMTSize() const
    {
        return imtSize_;
    }

    void SetIMTSize(size_t size)
    {
        imtSize_ = size;
    }

    IMTableBuilder() = default;
    ~IMTableBuilder() = default;

    NO_COPY_SEMANTIC(IMTableBuilder);
    NO_MOVE_SEMANTIC(IMTableBuilder);

private:
    size_t imtSize_ = 0;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_IMTABLE_BUILDER_H_
