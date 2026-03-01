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
#ifndef PANDA_RUNTIME_CORE_CORE_CLASS_LINKER_EXTENSION_H_
#define PANDA_RUNTIME_CORE_CORE_CLASS_LINKER_EXTENSION_H_

#include "runtime/include/class_linker-inl.h"
#include "runtime/include/class_linker_extension.h"

namespace ark {

class CoreClassLinkerExtension : public ClassLinkerExtension {
public:
    CoreClassLinkerExtension() : ClassLinkerExtension(panda_file::SourceLang::PANDA_ASSEMBLY) {}

    ~CoreClassLinkerExtension() override;

    bool InitializeArrayClass(Class *arrayClass, Class *componentClass) override;

    bool InitializeUnionClass(Class *unionClass, Span<Class *> constituentClasses) override;

    void InitializePrimitiveClass(Class *primitiveClass) override;

    void InitializeSyntheticClass(Class *synClass) override;

    size_t GetClassVTableSize(ClassRoot root) override;

    size_t GetClassIMTSize(ClassRoot root) override;

    size_t GetClassSize(ClassRoot root) override;

    size_t GetArrayClassVTableSize() override;

    size_t GetArrayClassIMTSize() override;

    size_t GetArrayClassSize() override;

    Class *CreateClass(const uint8_t *descriptor, size_t vtableSize, size_t imtSize, size_t size) override;

    void FreeClass(Class *klass) override;

    void FillStringClass(Class *strCls, ClassRoot flag);

    const uint8_t *GetStringClassDescriptor(ClassRoot flag);

    bool InitializeStringClass();

    bool InitializeClass([[maybe_unused]] Class *klass) override
    {
        return true;
    }

    const void *GetNativeEntryPointFor([[maybe_unused]] Method *method) const override
    {
        return reinterpret_cast<const void *>(intrinsics::UnknownIntrinsic);
    }

    bool CanThrowException([[maybe_unused]] const Method *method) const override
    {
        return true;
    }

    ClassLinkerErrorHandler *GetErrorHandler() override
    {
        return &errorHandler_;
    };

    ClassLinkerContext *GetCommonContext(Span<Class *> classes) override;

    NO_COPY_SEMANTIC(CoreClassLinkerExtension);
    NO_MOVE_SEMANTIC(CoreClassLinkerExtension);

private:
    bool InitializeImpl(bool compressedStringEnabled) override;
    void InitializeClassRoots(const LanguageContext &ctx);

    class ErrorHandler : public ClassLinkerErrorHandler {
    public:
        void OnError(ClassLinker::Error error, const PandaString &message) override;
    };

    ErrorHandler errorHandler_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_CORE_CORE_CLASS_LINKER_EXTENSION_H_
