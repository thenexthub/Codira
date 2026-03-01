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

#include "runtime/osr.h"

#include "libarkbase/events/events.h"
#include "libarkfile/shorty_iterator.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/method.h"
#include "runtime/include/stack_walker.h"
#include "code_info/code_info.h"
#include "runtime/include/panda_vm.h"
#include "compiler/optimizer/ir/runtime_interface.h"

namespace ark {

using compiler::CodeInfo;
using compiler::VRegInfo;

static void UnpoisonAsanStack([[maybe_unused]] void *ptr)
{
#ifdef PANDA_ASAN_ON
    uint8_t sp;
    ASAN_UNPOISON_MEMORY_REGION(&sp, reinterpret_cast<uint8_t *>(ptr) - &sp);
#endif  // PANDA_ASAN_ON
}

#if EVENT_OSR_ENTRY_ENABLED
void WriteOsrEventError(Frame *frame, FrameKind kind, uintptr_t loopHeadBc)
{
    events::OsrEntryKind osrKind;
    switch (kind) {
        case FrameKind::INTERPRETER:
            osrKind = events::OsrEntryKind::AFTER_IFRAME;
            break;
        case FrameKind::COMPILER:
            osrKind = events::OsrEntryKind::AFTER_CFRAME;
            break;
        case FrameKind::NONE:
            osrKind = events::OsrEntryKind::TOP_FRAME;
            break;
        default:
            UNREACHABLE();
    }
    EVENT_OSR_ENTRY(std::string(frame->GetMethod()->GetFullName()), loopHeadBc, osrKind, events::OsrEntryResult::ERROR);
}
#endif  // EVENT_OSR_ENTRY_ENABLED

static size_t GetStackParamsSize(const Frame *frame);

bool OsrEntry(uintptr_t loopHeadBc, const void *osrCode)
{
    auto stack = StackWalker::Create(ManagedThread::GetCurrent());
    Frame *frame = stack.GetIFrame();
    LOG(DEBUG, INTEROP) << "OSR entry in method '" << stack.GetMethod()->GetFullName() << "': " << osrCode;
    ASSERT(osrCode != nullptr);
    CodeInfo codeInfo(CodeInfo::GetCodeOriginFromEntryPoint(osrCode));
    auto stackmap = codeInfo.FindOsrStackMap(loopHeadBc);
    if (!stackmap.IsValid()) {
#if EVENT_OSR_ENTRY_ENABLED
        WriteOsrEventError(frame, stack.GetPreviousFrameKind(), loopHeadBc);
#endif  // EVENT_OSR_ENTRY_ENABLED
        return false;
    }

    switch (stack.GetPreviousFrameKind()) {
        case FrameKind::INTERPRETER:
            LOG(DEBUG, INTEROP) << "OSR: after interpreter frame";
            EVENT_OSR_ENTRY(std::string(frame->GetMethod()->GetFullName()), loopHeadBc,
                            events::OsrEntryKind::AFTER_IFRAME, events::OsrEntryResult::SUCCESS);
            OsrEntryAfterIFrame(frame, loopHeadBc, osrCode, codeInfo.GetFrameSize(), GetStackParamsSize(frame));
            break;
        case FrameKind::COMPILER:
            if (frame->IsDynamic() && frame->GetNumActualArgs() < frame->GetMethod()->GetNumArgs()) {
                frame->DisableOsr();
                // We need to adjust slot arguments space from previous compiled frame.
#if EVENT_OSR_ENTRY_ENABLED
                WriteOsrEventError(frame, stack.GetPreviousFrameKind(), loopHeadBc);
#endif  // EVENT_OSR_ENTRY_ENABLED
                LOG(DEBUG, INTEROP) << "OSR: after compiled frame, fail: num_actual_args < num_args";
                return false;
            }
            UnpoisonAsanStack(frame->GetPrevFrame());
            LOG(DEBUG, INTEROP) << "OSR: after compiled frame";
            EVENT_OSR_ENTRY(std::string(frame->GetMethod()->GetFullName()), loopHeadBc,
                            events::OsrEntryKind::AFTER_CFRAME, events::OsrEntryResult::SUCCESS);
            OsrEntryAfterCFrame(frame, loopHeadBc, osrCode, codeInfo.GetFrameSize());
            UNREACHABLE();
            break;
        case FrameKind::NONE:
            LOG(DEBUG, INTEROP) << "OSR: after no frame";
            EVENT_OSR_ENTRY(std::string(frame->GetMethod()->GetFullName()), loopHeadBc, events::OsrEntryKind::TOP_FRAME,
                            events::OsrEntryResult::SUCCESS);
            OsrEntryTopFrame(frame, loopHeadBc, osrCode, codeInfo.GetFrameSize(), GetStackParamsSize(frame));
            break;
        default:
            break;
    }
    return true;
}

static int64_t GetValueFromVregAcc(const Frame *iframe, LanguageContext &ctx, VRegInfo &vreg)
{
    int64_t value = 0;
    if (vreg.IsAccumulator()) {
        value = iframe->GetAcc().GetValue();
    } else if (!vreg.IsSpecialVReg()) {
        ASSERT(vreg.GetVRegType() == VRegInfo::VRegType::VREG);
        value = iframe->GetVReg(vreg.GetIndex()).GetValue();
    } else {
        value = static_cast<int64_t>(ctx.GetOsrEnv(iframe, vreg));
    }
#if defined(PANDA_32_BIT_MANAGED_POINTER) && defined(PANDA_TARGET_64)
    if (vreg.IsObject()) {
        value = static_cast<uint32_t>(value);
    }
#elif defined(PANDA_TARGET_64)
    if (vreg.GetType() == VRegInfo::Type::INT32) {  // NOTE(urandon): Investigate in #26258
        value = static_cast<uint32_t>(value);
    }
#endif
    return value;
}

extern "C" void *PrepareOsrEntry(const Frame *iframe, uintptr_t bcOffset, const void *osrCode, void *cframePtr,
                                 uintptr_t *regBuffer, uintptr_t *fpRegBuffer)
{
    ASSERT(osrCode != nullptr);
    ASSERT(cframePtr != nullptr);
    CodeInfo codeInfo(CodeInfo::GetCodeOriginFromEntryPoint(osrCode));
    CFrame cframe(cframePtr);
    auto stackmap = codeInfo.FindOsrStackMap(bcOffset);

    ASSERT(stackmap.IsValid());
    ASSERT(iframe != nullptr);

    cframe.SetMethod(iframe->GetMethod());
    cframe.SetFrameKind(CFrameLayout::FrameKind::OSR);
    cframe.SetHasFloatRegs(codeInfo.HasFloatRegs());

    auto *thread {ManagedThread::GetCurrent()};
    ASSERT(thread != nullptr);

    auto *vm {thread->GetVM()};

    auto numSlots {[](size_t bytes) -> size_t {
        ASSERT(IsAligned(bytes, ArchTraits<RUNTIME_ARCH>::POINTER_SIZE));
        return bytes / ArchTraits<RUNTIME_ARCH>::POINTER_SIZE;
    }};

    Span paramSlots(reinterpret_cast<uintptr_t *>(cframe.GetStackArgsStart()), numSlots(GetStackParamsSize(iframe)));

    auto ctx {vm->GetLanguageContext()};
    ctx.InitializeOsrCframeSlots(paramSlots);

    auto *internalAllocator = mem::InternalAllocator<>::GetInternalAllocatorFromRuntime();
    ASSERT(internalAllocator != nullptr);
    for (auto vreg : codeInfo.GetVRegList(stackmap, internalAllocator)) {
        if (!vreg.IsLive()) {
            continue;
        }
        int64_t value = GetValueFromVregAcc(iframe, ctx, vreg);
        switch (vreg.GetLocation()) {
            case VRegInfo::Location::SLOT:
                cframe.SetVRegValue(vreg, value, nullptr);
                break;
            case VRegInfo::Location::REGISTER:
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                regBuffer[vreg.GetValue()] = static_cast<uint64_t>(value);
                break;
            case VRegInfo::Location::FP_REGISTER:
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                fpRegBuffer[vreg.GetValue()] = static_cast<uintptr_t>(value);
                break;
            // NOLINTNEXTLINE(bugprone-branch-clone)
            case VRegInfo::Location::CONSTANT:
                break;
            default:
                break;
        }
    }

    thread->SetCurrentFrame(reinterpret_cast<Frame *>(cframePtr));
    thread->SetCurrentFrameIsCompiled(true);

    return bit_cast<void *>(bit_cast<uintptr_t>(osrCode) + stackmap.GetNativePcUnpacked());
}

extern "C" void SetOsrResult(Frame *frame, uint64_t uval, double fval)
{
    ASSERT(frame != nullptr);
    panda_file::ShortyIterator it(frame->GetMethod()->GetShorty());
    using panda_file::Type;
    auto &acc = frame->GetAcc();

    switch ((*it).GetId()) {
        case Type::TypeId::U1:
        case Type::TypeId::I8:
        case Type::TypeId::U8:
        case Type::TypeId::I16:
        case Type::TypeId::U16:
        case Type::TypeId::I32:
        case Type::TypeId::U32:
        case Type::TypeId::I64:
        case Type::TypeId::U64:
            acc.SetValue(uval);
            acc.SetTag(interpreter::StaticVRegisterRef::PRIMITIVE_TYPE);
            break;
        case Type::TypeId::REFERENCE:
            acc.SetValue(uval);
            acc.SetTag(interpreter::StaticVRegisterRef::GC_OBJECT_TYPE);
            break;
        case Type::TypeId::F32:
        case Type::TypeId::F64:
            acc.SetValue(bit_cast<int64_t>(fval));
            acc.SetTag(interpreter::StaticVRegisterRef::PRIMITIVE_TYPE);
            break;
        case Type::TypeId::VOID:
            // Interpreter always restores accumulator from the callee method, even if callee method is void. Thus, we
            // need to reset it here, otherwise it can hold old object, that probably isn't live already.
            acc.SetValue(0);
            acc.SetTag(interpreter::StaticVRegisterRef::PRIMITIVE_TYPE);
            break;
        case Type::TypeId::TAGGED:
            acc.SetValue(uval);
            break;
        case Type::TypeId::INVALID:
            UNREACHABLE();
        default:
            UNREACHABLE();
    }
}

static size_t GetStackParamsSize(const Frame *frame)
{
    constexpr auto SLOT_SIZE {ArchTraits<RUNTIME_ARCH>::POINTER_SIZE};
    auto *method {frame->GetMethod()};
    if (frame->IsDynamic()) {
        auto numArgs {std::max(method->GetNumArgs(), frame->GetNumActualArgs())};
        auto argSize {std::max(sizeof(coretypes::TaggedType), SLOT_SIZE)};
        return RoundUp(numArgs * argSize, 2U * SLOT_SIZE);
    }

    arch::ArgCounter<RUNTIME_ARCH> counter;
    counter.Count<Method *>();
    if (!method->IsStatic()) {
        counter.Count<ObjectHeader *>();
    }
    panda_file::ShortyIterator it(method->GetShorty());
    ++it;  // skip return type
    while (it != panda_file::ShortyIterator()) {
        switch ((*it).GetId()) {
            case panda_file::Type::TypeId::U1:
            case panda_file::Type::TypeId::U8:
            case panda_file::Type::TypeId::I8:
            case panda_file::Type::TypeId::I16:
            case panda_file::Type::TypeId::U16:
            case panda_file::Type::TypeId::I32:
            case panda_file::Type::TypeId::U32:
                counter.Count<int32_t>();
                break;
            case panda_file::Type::TypeId::F32:
                counter.Count<float>();
                break;
            case panda_file::Type::TypeId::F64:
                counter.Count<double>();
                break;
            case panda_file::Type::TypeId::I64:
            case panda_file::Type::TypeId::U64:
                counter.Count<int64_t>();
                break;
            case panda_file::Type::TypeId::REFERENCE:
                counter.Count<ObjectHeader *>();
                break;
            case panda_file::Type::TypeId::TAGGED:
                counter.Count<coretypes::TaggedType>();
                break;
            default:
                UNREACHABLE();
        }
        ++it;
    }
    return RoundUp(counter.GetOnlyStackSize(), 2U * SLOT_SIZE);
}

}  // namespace ark
