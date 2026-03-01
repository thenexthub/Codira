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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_CLASS_LINKER_EXTENSION_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_CLASS_LINKER_EXTENSION_H_

#include <cstddef>

#include <libarkfile/include/source_lang_enum.h>

#include "libarkbase/macros.h"
#include "runtime/class_linker_context.h"
#include "runtime/include/class_linker_extension.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/class_root.h"
#include "runtime/include/mem/panda_string.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_platform_types.h"

namespace ark {

class Class;
class Method;
class ObjectHeader;

}  // namespace ark

namespace ark::ets {

class EtsRuntimeLinker;

class EtsClassLinkerExtension : public ClassLinkerExtension {
public:
    EtsClassLinkerExtension() : ClassLinkerExtension(panda_file::SourceLang::ETS) {}

    ~EtsClassLinkerExtension() override;

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

    void InitializeClassRoots();

    Class *CreateStringSubClass(const uint8_t *descriptor, Class *stringClass, ClassRoot type);

    const uint8_t *GetStringClassDescriptor(ClassRoot strCls);

    bool InitializeStringClass(Class *classClass);

    bool InitializeClass(Class *klass) override;

    bool InitializeClass(Class *klass, ClassLinkerErrorHandler *handler) override;

    bool IsMethodNativeApi(const Method *method) const override;

    const void *GetNativeEntryPointFor(Method *method) const override;

    bool CanThrowException([[maybe_unused]] const Method *method) const override;

    bool IsNecessarySwitchThreadState(const Method *method) const override;

    bool CanNativeMethodUseObjects(const Method *method) const override;

    ClassLinkerErrorHandler *GetErrorHandler() override
    {
        return &errorHandler_;
    };

    Class *FromClassObject(ark::ObjectHeader *obj) override;
    size_t GetClassObjectSizeFromClassSize(uint32_t size) override;

    ClassLinkerContext *CreateApplicationClassLinkerContext(const PandaVector<PandaString> &path) override;

    void InitializeBuiltinClasses();
    void InitializeBuiltinSpecialClasses();

    EtsPlatformTypes const *GetPlatformTypes()
    {
        return plaformTypes_.get();
    }

    static EtsClassLinkerExtension *FromCoreType(ClassLinkerExtension *ext)
    {
        ASSERT(ext->GetLanguage() == panda_file::SourceLang::ETS);
        return static_cast<EtsClassLinkerExtension *>(ext);
    }

    LanguageContext GetLanguageContext() const
    {
        return langCtx_;
    }

    void InitializeFinish();

    ClassLinkerContext *GetCommonContext(Span<Class *> classes) override;
    static ClassLinkerContext *GetParentContext(ClassLinkerContext *ctx);

    std::optional<Span<Method>> GenerateProxyClassMethods(Class *proxyKlass, Span<Method *> rawMethods) override;

    std::optional<PandaVector<Method *>> BuildProxyClassMethodsSpan(ITable itable) override;

    NO_COPY_SEMANTIC(EtsClassLinkerExtension);
    NO_MOVE_SEMANTIC(EtsClassLinkerExtension);

private:
    bool InitializeImpl(bool compressedStringEnabled) override;

    Class *InitializeClass(ObjectHeader *objectHeader, const uint8_t *descriptor, size_t vtableSize, size_t imtSize,
                           size_t size);

    Class *CacheClass(std::string_view descriptor, bool forceInit = false);
    template <typename F>
    Class *CacheClass(std::string_view descriptor, F const &setup, bool forceInit = false);

    class ErrorHandler : public ClassLinkerErrorHandler {
    public:
        void OnError(ClassLinker::Error error, const PandaString &message) override;
    };

    ErrorHandler errorHandler_;
    LanguageContext langCtx_ {nullptr};
    mem::HeapManager *heapManager_ {nullptr};

    PandaUniquePtr<EtsPlatformTypes> plaformTypes_ {nullptr};
};

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_CLASS_LINKER_EXTENSION_H_
