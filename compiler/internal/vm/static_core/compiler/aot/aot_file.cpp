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

#include "aot_file.h"
#include "compiler/optimizer/ir/inst.h"
#include "optimizer/ir/runtime_interface.h"
#include "libarkbase/utils/logger.h"
#include "code_info/code_info.h"
#include "mem/gc/gc_types.h"
#include "libarkbase/trace/trace.h"
#include "entrypoints/entrypoints.h"

// In some targets, runtime library is not linked, so linker will fail while searching CallStaticPltResolver symbol.
// To solve this issue, we define this function as weak.
// NOTE(msherstennikov): find a better way instead of weak function, e.g. make aot_manager library static.
extern "C" void CallStaticPltResolver([[maybe_unused]] void *slot) __attribute__((weak));
extern "C" void CallStaticPltResolver([[maybe_unused]] void *slot) {}

namespace ark::compiler {
static inline Expected<const uint8_t *, std::string> LoadSymbol(const ark::os::library_loader::LibraryHandle &handle,
                                                                const char *name)
{
    auto sym = ark::os::library_loader::ResolveSymbol(handle, name);
    if (!sym) {
        return Unexpected(sym.Error().ToString());
    }
    return reinterpret_cast<uint8_t *>(sym.Value());
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOAD_AOT_SYMBOL(name)                                             \
    /* CC-OFFNXT(G.PRE.02) name part */                                   \
    auto name = LoadSymbol(handle, (#name));                              \
    if (!(name)) {                                                        \
        /* CC-OFFNXT(G.PRE.05) code generation */                         \
        return Unexpected("Cannot load name section: " + (name).Error()); \
    }

Expected<std::unique_ptr<AotFile>, std::string> AotFile::Open(const std::string &fileName, uint32_t gcType,
                                                              bool forDump)
{
    trace::ScopedTrace scopedTrace("Open aot file " + fileName);
    auto handleLoad = ark::os::library_loader::Load(fileName);
    if (!handleLoad) {
        return Unexpected("AOT elf library open failed: " + handleLoad.Error().ToString());
    }
    auto handle = std::move(handleLoad.Value());

    LOAD_AOT_SYMBOL(aot);
    LOAD_AOT_SYMBOL(aot_end);
    LOAD_AOT_SYMBOL(code);
    LOAD_AOT_SYMBOL(code_end);

    if (code_end.Value() < code.Value() || aot_end.Value() <= aot.Value()) {
        return Unexpected(std::string("Invalid symbols"));
    }

    auto aotHeader = reinterpret_cast<const AotHeader *>(aot.Value());
    if (aotHeader->magic != MAGIC) {
        return Unexpected(std::string("Wrong AotHeader magic"));
    }

    if (aotHeader->version != VERSION) {
        return Unexpected(std::string("Wrong AotHeader version"));
    }

    if (!forDump && aotHeader->environmentChecksum != RuntimeInterface::GetEnvironmentChecksum(RUNTIME_ARCH)) {
        return Unexpected(std::string("Compiler environment checksum mismatch"));
    }

    if (!forDump && aotHeader->gcType != gcType) {
        return Unexpected(std::string("Wrong AotHeader gc-type: ") +
                          std::string(mem::GCStringFromType(static_cast<mem::GCType>(aotHeader->gcType))) + " vs " +
                          std::string(mem::GCStringFromType(static_cast<mem::GCType>(gcType))));
    }
    return std::make_unique<AotFile>(std::move(handle), Span(aot.Value(), aot_end.Value() - aot.Value()),
                                     Span(code.Value(), code_end.Value() - code.Value()));
}

void AotFile::InitializeGot(RuntimeInterface *runtime)
{
    size_t minusFirstSlot = static_cast<size_t>(RuntimeInterface::IntrinsicId::COUNT) + 1;
    auto *table = const_cast<uintptr_t *>(
        reinterpret_cast<const uintptr_t *>(code_.data() - minusFirstSlot * PointerSize(RUNTIME_ARCH)));
    ASSERT(BitsToBytesRoundUp(MinimumBitsToStore(static_cast<uint32_t>(AotSlotType::COUNT) - 1)) == 1);
    while (*table != 0) {
        switch (GetByteFrom(*table, 0U)) {
            case AotSlotType::PLT_SLOT:
                table -= 2U;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                table[1] = reinterpret_cast<uintptr_t>(CallStaticPltResolver);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                table[2U] = reinterpret_cast<uintptr_t>(
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    table + 1 - runtime->GetCompiledEntryPointOffset(RUNTIME_ARCH) / sizeof(uintptr_t));
                break;
            case AotSlotType::VTABLE_INDEX:
                table--;       // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                table[1] = 0;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                break;
            case AotSlotType::CLASS_SLOT:
                table -= 2U;    // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                table[1] = 0;   // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                table[2U] = 0;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                break;
            case AotSlotType::STRING_SLOT:
                table--;       // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                table[1] = 0;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                break;
            case AotSlotType::INLINECACHE_SLOT:
                break;
            case AotSlotType::COMMON_SLOT:
                table[0] = 0;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                break;
            default:
                UNREACHABLE();
                break;
        }
        table--;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
}

void AotFile::PatchTable(RuntimeInterface *runtime)
{
    auto *table = const_cast<uintptr_t *>(reinterpret_cast<const uintptr_t *>(
        code_.data() - static_cast<size_t>(RuntimeInterface::IntrinsicId::COUNT) * PointerSize(RUNTIME_ARCH)));
    for (size_t i = 0; i < static_cast<size_t>(RuntimeInterface::IntrinsicId::COUNT); i++) {
        IntrinsicInst inst(Opcode::Intrinsic, static_cast<RuntimeInterface::IntrinsicId>(i));
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        table[i] = runtime->GetIntrinsicAddress(inst.IsRuntimeCall(), SourceLanguage::INVALID,
                                                static_cast<RuntimeInterface::IntrinsicId>(i));
    }
}

AotClass AotPandaFile::GetClass(uint32_t classId) const
{
    auto classes = aotFile_->GetClassHeaders(*header_);
    auto it = std::lower_bound(classes.begin(), classes.end(), classId,
                               [](const auto &a, uintptr_t klassId) { return a.classId < klassId; });
    if (it == classes.end() || it->classId != classId) {
        return {};
    }
    ASSERT(it->methodsCount != 0 && "AOT file shall not contain empty classes");
    return AotClass(aotFile_, &*it);
}

const void *AotClass::FindMethodCodeEntry(size_t index) const
{
    auto methodHeader = FindMethodHeader(index);
    if (methodHeader == nullptr) {
        return nullptr;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return aotFile_->GetMethodCode(methodHeader) + CodeInfo::GetCodeOffset(RUNTIME_ARCH);
}

Span<const uint8_t> AotClass::FindMethodCodeSpan(size_t index) const
{
    auto methodHeader = FindMethodHeader(index);
    if (methodHeader == nullptr) {
        return {};
    }
    auto code = Span(aotFile_->GetMethodCode(methodHeader), methodHeader->codeSize);
    return CodeInfo(code).GetCodeSpan();
}

const MethodHeader *AotClass::FindMethodHeader(size_t index) const
{
    auto bitmap = GetBitmap();
    CHECK_LT(index, bitmap.size());
    if (!bitmap[index]) {
        return nullptr;
    }
    auto methodIndex = bitmap.PopCount(index);
    ASSERT(methodIndex < header_->methodsCount);
    return aotFile_->GetMethodHeader(header_->methodsOffset + methodIndex);
}

BitVectorSpan AotClass::GetBitmap() const
{
    // NOTE(msherstennikov): remove const_cast once BitVector support constant storage
    auto bitmapBase = const_cast<uint32_t *>(reinterpret_cast<const uint32_t *>(aotFile_->GetMethodsBitmap()));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return BitVectorSpan(bitmapBase + header_->methodsBitmapOffset, header_->methodsBitmapSize);
}

}  // namespace ark::compiler
