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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CALL_PROTO_READER_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CALL_PROTO_READER_H

#include "runtime/include/class_linker-inl.h"
#include "libarkfile/proto_data_accessor-inl.h"

namespace ark::ets::interop::js {

class ProtoReader {
public:
    ALWAYS_INLINE ProtoReader(Method *method, ClassLinker *classLinker, ClassLinkerContext *classLinkerContext)
        : method_(method),
          pf_(method_->GetPandaFile()),
          pda_(*pf_, panda_file::MethodDataAccessor::GetProtoId(*pf_, method_->GetFileId())),
          classLinker_(classLinker),
          classLinkerContext_(classLinkerContext)
    {
        pda_.EnumerateTypes([](panda_file::Type) {});  // preload reftypes span
        Reset();
    }

    ALWAYS_INLINE void Reset()
    {
        it_ = panda_file::ShortyIterator(method_->GetShorty());
        refArgIdx_ = 0;
    }

    ALWAYS_INLINE void Advance()
    {
        if (GetType().IsReference()) {
            refArgIdx_++;
        }
        it_.IncrementWithoutCheck();
    }

    ALWAYS_INLINE panda_file::Type GetType()
    {
        return *it_;
    }

    ALWAYS_INLINE Class *GetClass()
    {
        ASSERT(GetType().IsReference());
        return ResolveProtoClass(refArgIdx_);
    }

    ALWAYS_INLINE Class *GetLoadedClass()
    {
        ASSERT(GetType().IsReference());
        return ResolveLoadedProtoClass(refArgIdx_);
    }

    ALWAYS_INLINE Class *ResolveLoadedProtoClass(uint32_t idx)
    {
        auto klass = classLinker_->GetLoadedClass(*pf_, pda_.GetReferenceType(idx), classLinkerContext_);
        ASSERT(klass != nullptr);
        return klass;
    }

    ALWAYS_INLINE Class *ResolveProtoClass(uint32_t idx)
    {
        return classLinker_->GetClass(*pf_, pda_.GetReferenceType(idx), classLinkerContext_);
    }

    ALWAYS_INLINE Method *GetMethod()
    {
        return method_;
    }

    ALWAYS_INLINE const Method *GetMethod() const
    {
        return method_;
    }

private:
    panda_file::ShortyIterator it_ {};
    uint32_t refArgIdx_ {};

    Method *const method_;
    panda_file::File const *const pf_;
    panda_file::ProtoDataAccessor pda_;

    ClassLinker *const classLinker_;
    ClassLinkerContext *const classLinkerContext_;
};

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CALL_PROTO_READER_H
