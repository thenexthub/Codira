/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ets/runtime/ets_platform_types.h"
#include "ets_class_linker_extension.h"
#include "include/thread_scopes.h"
#include "plugins/ets/runtime/ets_class_linker.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/ets_vm.h"

#include "runtime/include/class_linker-inl.h"

namespace ark::ets {

void EtsPlatformTypes::CreateAndInitializeCaches()
{
    auto *charClass = this->coreString;
    // asciiCharCache_ must point to a piece of memory in the non-movable space.
    asciiCharCache_ = EtsTypedObjectArray<EtsString>::Create(charClass, EtsPlatformTypes::ASCII_CHAR_TABLE_SIZE,
                                                             ark::SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    if (UNLIKELY(asciiCharCache_ == nullptr)) {
        LOG(FATAL, RUNTIME) << "Failed to create asciiCharCache";
    }

    for (uint32_t i = 0; i < EtsPlatformTypes::ASCII_CHAR_TABLE_SIZE; ++i) {
        auto *str = EtsString::CreateNewStringFromCharCode(i);
        asciiCharCache_->Set(i, str);
    }
}

void EtsPlatformTypes::VisitRoots(const GCRootVisitor &visitor) const
{
    if (asciiCharCache_ != nullptr) {
        visitor(mem::GCRoot(mem::RootType::ROOT_VM, reinterpret_cast<ObjectHeader **>(&asciiCharCache_)));
    }
}

EtsPlatformTypes const *PlatformTypes(PandaEtsVM *vm)
{
    return vm->GetClassLinker()->GetEtsClassLinkerExtension()->GetPlatformTypes();
}

EtsPlatformTypes const *PlatformTypes()
{
    return PlatformTypes(PandaEtsVM::GetCurrent());
}

class SuppressErrorHandler : public ClassLinkerErrorHandler {
    void OnError([[maybe_unused]] ClassLinker::Error error, [[maybe_unused]] const PandaString &message) override {}
};

static EtsClass *FindType(EtsClassLinker *classLinker, std::string_view descriptor)
{
    SuppressErrorHandler handler;
    auto bootCtx = classLinker->GetEtsClassLinkerExtension()->GetBootContext();
    auto klass = classLinker->GetClass(descriptor.data(), false, bootCtx, &handler);
    if (klass == nullptr) {
        // In some cases (i.e. unit-tests) we allow platform classes to be not found
        LOG(ERROR, RUNTIME) << "Cannot find a platform class " << descriptor;
        return nullptr;
    }
    return klass;
}

static EtsMethod *FindMethod(EtsClass *klass, char const *name, char const *signature, bool isStatic)
{
    if (klass == nullptr) {
        return nullptr;
    }

    EtsMethod *method = isStatic ? klass->GetStaticMethod(name, signature) : klass->GetInstanceMethod(name, signature);
    if (method == nullptr) {
        LOG(ERROR, RUNTIME) << "Method " << name << " is not found in class " << klass->GetDescriptor();
        return nullptr;
    }
    return method;
}

EtsPlatformTypes::Entry const *EtsPlatformTypes::GetTypeEntry(const uint8_t *descriptor) const
{
    if (auto it = entryTable_.find(descriptor); it != entryTable_.end()) {
        return &it->second;
    }
    return nullptr;
}

void EtsPlatformTypes::PreloadType(EtsClassLinker *linker, EtsClass **slot, std::string_view descriptor)
{
    auto cls = FindType(linker, descriptor);
    if (cls == nullptr) {
        return;
    }
    *slot = cls;

    size_t offset = ToUintPtr(slot) - ToUintPtr(this);
    ASSERT(IsAligned(offset, sizeof(uintptr_t)));
    Entry entry = {offset / sizeof(uintptr_t)};
    entryTable_.insert({utf::CStringAsMutf8(cls->GetDescriptor()), entry});
}

static EtsClass *GetTypeEntryClass(EtsPlatformTypes *ptypes, std::string_view descriptor)
{
    auto entry = ptypes->GetTypeEntry(utf::CStringAsMutf8(descriptor.data()));
    if (entry == nullptr) {
        return nullptr;
    }
    return *(reinterpret_cast<EtsClass **>(ptypes) + entry->slotIndex);
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid logic
EtsPlatformTypes::EtsPlatformTypes([[maybe_unused]] EtsCoroutine *coro)
{
    // NOLINTNEXTLINE(google-build-using-namespace)
    using namespace panda_file_items::class_descriptors;

    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    [[maybe_unused]] size_t orderOffset = 0;
    auto const updateOffset = [this, &orderOffset](auto *slot) {
        size_t newOffset = ToUintPtr(slot) - ToUintPtr(this);
        ASSERT_PRINT(newOffset == 0 || (newOffset == orderOffset + sizeof(uintptr_t)),
                     "type preloading should follow the definition order");
        orderOffset = newOffset;
    };
    auto const findType = [this, classLinker, &updateOffset](EtsClass *ark::ets::EtsPlatformTypes::*field,
                                                             std::string_view descriptor) {
        auto slot = &(this->*field);
        PreloadType(classLinker, slot, descriptor);
        updateOffset(slot);
    };
    auto const findMethod = [this, &updateOffset](EtsMethod *ark::ets::EtsPlatformTypes::*field, EtsClass *cls,
                                                  char const *name, char const *signature, bool isStatic) {
        auto slot = &(this->*field);
        *slot = FindMethod(cls, name, signature, isStatic);
        updateOffset(slot);
    };

    // NOLINTBEGIN(cppcoreguidelines-macro-usage)
    // CC-OFFNXT(G.PRE.09) macro expansion
#define T(descr, name) findType(&EtsPlatformTypes::name, descr);
#define M(descr, mname, msig, name, isStatic) \
    /* CC-OFFNXT(G.PRE.09) macro expansion */ \
    findMethod(&EtsPlatformTypes::name, GetTypeEntryClass(this, descr), mname, msig, isStatic);
#define I(descr, mname, msig, name) M(descr, mname, msig, name, false)
#define S(descr, mname, msig, name) M(descr, mname, msig, name, true)
    ETS_PLATFORM_TYPES_LIST(T, I, S)
#undef T
#undef M
#undef I
#undef S
    // NOLINTEND(cppcoreguidelines-macro-usage)

    for (size_t i = 0; i < coreFunctions.size(); ++i) {
        PandaString descr = "Lstd/core/Function" + PandaString(std::to_string(i)) + ";";
        PreloadType(classLinker, &coreFunctions[i], descr);
        updateOffset(&coreFunctions[i]);
    }
    for (size_t i = 0; i < coreFunctionRs.size(); ++i) {
        PandaString descr = "Lstd/core/FunctionR" + PandaString(std::to_string(i)) + ";";
        PreloadType(classLinker, &coreFunctionRs[i], descr);
        updateOffset(&coreFunctionRs[i]);
    }

    if (LIKELY(Runtime::GetOptions().IsUseStringCaches())) {
        CreateAndInitializeCaches();
    }
}

void EtsPlatformTypes::InitializeClasses(EtsCoroutine *coro)
{
    ASSERT(coro != nullptr);
    ScopedManagedCodeThread sj(coro);

    auto *classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    for (auto iter : entryTable_) {
        auto idx = iter.second.slotIndex;
        ASSERT(idx < sizeof(EtsPlatformTypes));
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        EtsClass *klass = *(reinterpret_cast<EtsClass **>(this) + idx);
        if (klass != nullptr && !classLinker->InitializeClass(coro, klass)) {
            LOG(FATAL, RUNTIME) << "Cannot initialize a platform class " << klass->GetDescriptor();
        }
    }
}

}  // namespace ark::ets
