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
#ifndef PANDA_RUNTIME_TESTS_CLASS_LINKER_TEST_EXTENSION_H_
#define PANDA_RUNTIME_TESTS_CLASS_LINKER_TEST_EXTENSION_H_

#include "runtime/include/class_linker.h"
#include "runtime/include/class_linker_extension.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/include/panda_vm.h"

namespace ark::test {

class ClassLinkerTestExtension : public ClassLinkerExtension {
public:
    NO_COPY_SEMANTIC(ClassLinkerTestExtension);
    NO_MOVE_SEMANTIC(ClassLinkerTestExtension);

    ClassLinkerTestExtension(ManagedThread *thread, panda_file::SourceLang lang)
        : ClassLinkerExtension(lang), thread_(thread)
    {
    }

    ~ClassLinkerTestExtension() override
    {
        FreeLoadedClasses();
    }

    bool InitializeArrayClass(Class *arrayClass, Class *componentClass) override
    {
        auto *objectClass = GetClassRoot(ClassRoot::OBJECT);
        arrayClass->SetBase(objectClass);
        arrayClass->SetComponentType(componentClass);
        return true;
    }

    void InitializePrimitiveClass([[maybe_unused]] Class *primitiveClass) override {}

    void InitializeSyntheticClass([[maybe_unused]] Class *synClass) override {}

    size_t GetClassVTableSize([[maybe_unused]] ClassRoot root) override
    {
        return 0;
    }

    size_t GetClassIMTSize([[maybe_unused]] ClassRoot root) override
    {
        return 0;
    }

    size_t GetClassSize(ClassRoot root) override
    {
        return Class::ComputeClassSize(GetClassVTableSize(root), GetClassIMTSize(root), 0, 0, 0, 0, 0, 0);
    }

    size_t GetArrayClassVTableSize() override
    {
        return GetClassVTableSize(ClassRoot::OBJECT);
    }

    size_t GetArrayClassIMTSize() override
    {
        return GetClassIMTSize(ClassRoot::OBJECT);
    }

    size_t GetArrayClassSize() override
    {
        return GetClassSize(ClassRoot::OBJECT);
    }

    Class *CreateClass(const uint8_t *descriptor, size_t vtableSize, size_t imtSize, size_t size) override
    {
        auto vm = thread_->GetVM();
        auto allocator = vm->GetGC()->GetObjectAllocator();
        void *ptr = allocator->AllocateNonMovable(coretypes::Class::GetSize(size), DEFAULT_ALIGNMENT, nullptr,
                                                  mem::ObjectAllocatorBase::ObjMemInitPolicy::REQUIRE_INIT);
        auto *res = reinterpret_cast<coretypes::Class *>(ptr);
        res->InitClass(descriptor, vtableSize, imtSize, size);
        vm->GetGC()->InitGCBits(res);
        res->SetClass(GetClassRoot(ClassRoot::CLASS));
        auto *klass = res->GetRuntimeClass();
        klass->SetManagedObject(res);
        klass->SetSourceLang(GetLanguage());
        AddCreatedClass(klass);
        return klass;
    }

    void FreeClass(Class *klass) override
    {
        RemoveCreatedClass(klass);
    }

    bool InitializeClass(Class * /*klass*/) override
    {
        return true;
    }

    const void *GetNativeEntryPointFor(ark::Method * /*method*/) const override
    {
        return nullptr;
    }

    bool CanThrowException(const Method * /* method */) const override
    {
        return true;
    }

    ClassLinkerErrorHandler *GetErrorHandler() override
    {
        return nullptr;
    }

private:
    bool InitializeImpl(bool compressedStringEnabled) override;

    ManagedThread *thread_;
};

}  // namespace ark::test

#endif  // PANDA_RUNTIME_TESTS_CLASS_LINKER_TEST_EXTENSION_H_
