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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_H_
#define PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_H_

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_platform_types_defs.h"

namespace ark::ets {

class EtsClass;
class EtsMethod;
class EtsCoroutine;
class EtsClassLinker;
template <typename T>
class EtsTypedObjectArray;

// A set of types defined and used in platform implementation, owned by the VM
// All the definitions are listed in ets_platform_types_defs.h
// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
class PANDA_PUBLIC_API EtsPlatformTypes {
public:
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
// CC-OFFNXT(G.PRE.09) macro expansion
#define T(descr, name) EtsClass *name;
// CC-OFFNXT(G.PRE.09) macro expansion
#define I(descr, mname, msig, name) EtsMethod *name;
// CC-OFFNXT(G.PRE.09) macro expansion
#define S(descr, mname, msig, name) EtsMethod *name;
    ETS_PLATFORM_TYPES_LIST(T, I, S)
#undef T
#undef I
#undef S
    // NOLINTEND(cppcoreguidelines-macro-usage)

    // Arity threshold for functional types
    static constexpr uint32_t CORE_FUNCTION_ARITY_THRESHOLD = 17U;
    static constexpr uint32_t ASCII_CHAR_TABLE_SIZE = 128;

    std::array<EtsClass *, CORE_FUNCTION_ARITY_THRESHOLD> coreFunctions {};
    std::array<EtsClass *, CORE_FUNCTION_ARITY_THRESHOLD> coreFunctionRs {};

    struct Entry {
        size_t slotIndex {};
    };

    /* Internal Caches */
    void CreateAndInitializeCaches();
    void VisitRoots(const GCRootVisitor &visitor) const;
    EtsTypedObjectArray<EtsString> *GetAsciiCacheTable() const
    {
        return asciiCharCache_;
    }
    Entry const *GetTypeEntry(const uint8_t *descriptor) const;

private:
    friend class EtsClassLinkerExtension;
    friend class mem::Allocator;
    friend class EtsPlatformTypesIrtocOffsets;
    // asciiCharCache_ must be allocated in a non-movable heap region; therefore, we should need to handle
    // this pointer in `ark::ets::PandaEtsVM::UpdateVmRefs`.
    mutable EtsTypedObjectArray<EtsString> *asciiCharCache_ {nullptr};
    void PreloadType(EtsClassLinker *linker, EtsClass **slot, std::string_view descriptor);
    PandaUnorderedMap<const uint8_t *, Entry, utf::Mutf8Hash, utf::Mutf8Equal> entryTable_;

    explicit EtsPlatformTypes(EtsCoroutine *coro);

    /**
     * @brief Initialize all classes.
     * This method must be called after construction of `EtsPlatformTypes`
     * to ensure correct initialization of all classes
     */
    void InitializeClasses(EtsCoroutine *coro);
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

// Obtain EtsPlatformTypes pointer cached in the coroutine
ALWAYS_INLINE inline EtsPlatformTypes const *PlatformTypes(EtsCoroutine *coro)
{
    ASSERT(coro != nullptr);
    return coro->GetLocalStorage().Get<EtsCoroutine::DataIdx::ETS_PLATFORM_TYPES_PTR, EtsPlatformTypes *>();
}

// Obtain EtsPlatfromTypes pointer from the VM
EtsPlatformTypes const *PlatformTypes(PandaEtsVM *vm);

// Obtain EtsPlatfromTypes pointer from the current VM
EtsPlatformTypes const *PlatformTypes();

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_H_
