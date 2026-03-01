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
#ifndef PANDA_RUNTIME_VTABLE_BUILDER_INTERFACE_H
#define PANDA_RUNTIME_VTABLE_BUILDER_INTERFACE_H

#include "runtime/include/method.h"

namespace ark {

class ClassLinker;
class ClassLinkerContext;
class ClassLinkerErrorHandler;

struct CopiedMethod {
    CopiedMethod() = default;
    explicit CopiedMethod(Method *cpMethod) : method_(cpMethod) {}

    enum class Status {
        ORDINARY = 0,
        ABSTRACT,
        CONFLICT,
    };

    Method *GetMethod() const
    {
        return method_;
    }

    Status GetStatus() const
    {
        return status_;
    }

    void SetStatus(Status status)
    {
        status_ = status;
    }

private:
    Method *method_ {};
    Status status_ {Status::ORDINARY};
};

class VTableBuilder {
public:
    VTableBuilder() = default;

    [[nodiscard]] virtual bool Build(panda_file::ClassDataAccessor *cda, Class *baseClass, ITable itable,
                                     ClassLinkerContext *ctx) = 0;

    [[nodiscard]] virtual bool Build(Span<Method> methods, Class *baseClass, ITable itable, bool isInterface) = 0;

    [[nodiscard]] virtual bool FilterProxyClassMethods([[maybe_unused]] Span<Method *> input,
                                                       [[maybe_unused]] PandaVector<Method *> *output,
                                                       [[maybe_unused]] Class *baseClass)
    {
        UNREACHABLE();
    }

    virtual void UpdateClass(Class *klass) const = 0;

    virtual size_t GetNumVirtualMethods() const = 0;

    virtual size_t GetVTableSize() const = 0;

    virtual Span<const CopiedMethod> GetCopiedMethods() const = 0;

    virtual ~VTableBuilder() = default;

    NO_COPY_SEMANTIC(VTableBuilder);
    NO_MOVE_SEMANTIC(VTableBuilder);
};

}  // namespace ark

#endif  // PANDA_RUNTIME_VTABLE_BUILDER_INTERFACE_H
