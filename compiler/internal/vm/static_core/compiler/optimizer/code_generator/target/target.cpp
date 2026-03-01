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

/*
   Compiler -> Pass(Codegen)
        Encoder = Fabric->getAsm

Define arch-specific interfaces
*/

#include "operands.h"
#include "encode.h"
#include "compiler/optimizer/code_generator/callconv.h"
#include "registers_description.h"

#ifdef PANDA_COMPILER_TARGET_X86_64
#include "amd64/target.h"
#endif

#ifdef PANDA_COMPILER_TARGET_AARCH32
#include "aarch32/target.h"
#endif

#ifdef PANDA_COMPILER_TARGET_AARCH64
#include "aarch64/target.h"
#endif

#include "asm_printer.h"

#include "frame_info.h"

namespace ark::compiler {
bool BackendSupport(Arch arch)
{
    switch (arch) {
#ifdef PANDA_COMPILER_TARGET_AARCH32
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case Arch::AARCH32: {
            return true;
        }
#endif
#ifdef PANDA_COMPILER_TARGET_AARCH64
        case Arch::AARCH64: {
            return true;
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        case Arch::X86_64: {
            return true;
        }
#endif
        default:
            return false;
    }
}

Encoder *Encoder::Create([[maybe_unused]] ArenaAllocator *arenaAllocator, [[maybe_unused]] Arch arch,
                         [[maybe_unused]] bool printAsm, [[maybe_unused]] bool jsNumberCast)
{
    switch (arch) {
#ifdef PANDA_COMPILER_TARGET_AARCH32
        case Arch::AARCH32: {
            aarch32::Aarch32Encoder *enc = arenaAllocator->New<aarch32::Aarch32Encoder>(arenaAllocator);
            ASSERT(enc != nullptr);
            enc->SetIsJsNumberCast(jsNumberCast);
            if (printAsm) {
                return arenaAllocator->New<aarch32::Aarch32Assembly>(arenaAllocator, enc);
            }
            return enc;
        }
#endif
#ifdef PANDA_COMPILER_TARGET_AARCH64
        case Arch::AARCH64: {
            aarch64::Aarch64Encoder *enc = arenaAllocator->New<aarch64::Aarch64Encoder>(arenaAllocator);
            ASSERT(enc != nullptr);
            enc->SetIsJsNumberCast(jsNumberCast);
            if (printAsm) {
                return arenaAllocator->New<aarch64::Aarch64Assembly>(arenaAllocator, enc);
            }
            return enc;
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        case Arch::X86_64: {
            amd64::Amd64Encoder *enc =
                arenaAllocator->New<amd64::Amd64Encoder>(arenaAllocator, Arch::X86_64, jsNumberCast);
            ASSERT(enc != nullptr);
            enc->SetIsJsNumberCast(jsNumberCast);
            if (printAsm) {
                return arenaAllocator->New<amd64::Amd64Assembly>(arenaAllocator, enc);
            }
            return enc;
        }
#endif
        default:
            return nullptr;
    }
}

RegistersDescription *RegistersDescription::Create([[maybe_unused]] ArenaAllocator *arenaAllocator,
                                                   [[maybe_unused]] Arch arch)
{
    switch (arch) {
#ifdef PANDA_COMPILER_TARGET_AARCH32
        case Arch::AARCH32: {
            return arenaAllocator->New<aarch32::Aarch32RegisterDescription>(arenaAllocator);
        }
#endif

#ifdef PANDA_COMPILER_TARGET_AARCH64
        case Arch::AARCH64: {
            return arenaAllocator->New<aarch64::Aarch64RegisterDescription>(arenaAllocator);
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        case Arch::X86_64: {
            return arenaAllocator->New<amd64::Amd64RegisterDescription>(arenaAllocator);
        }
#endif
        default:
            return nullptr;
    }
}

CallingConvention *CallingConvention::Create([[maybe_unused]] ArenaAllocator *arenaAllocator,
                                             [[maybe_unused]] Encoder *enc,
                                             [[maybe_unused]] RegistersDescription *descr, [[maybe_unused]] Arch arch,
                                             bool isPandaAbi, bool isOsr, bool isDyn, [[maybe_unused]] bool printAsm,
                                             bool isOptIrtoc)
{
    [[maybe_unused]] auto mode = CallConvMode::Panda(isPandaAbi) | CallConvMode::Osr(isOsr) |
                                 CallConvMode::DynCall(isDyn) | CallConvMode::OptIrtoc(isOptIrtoc);
    switch (arch) {
#ifdef PANDA_COMPILER_TARGET_AARCH32
        case Arch::AARCH32: {
            if (printAsm) {
                using PrinterType =
                    PrinterCallingConvention<aarch32::Aarch32CallingConvention, aarch32::Aarch32Assembly>;
                return arenaAllocator->New<PrinterType>(arenaAllocator,
                                                        reinterpret_cast<aarch32::Aarch32Assembly *>(enc), descr, mode);
            }
            return arenaAllocator->New<aarch32::Aarch32CallingConvention>(arenaAllocator, enc, descr, mode);
        }
#endif
#ifdef PANDA_COMPILER_TARGET_AARCH64
        case Arch::AARCH64: {
            if (printAsm) {
                using PrinterType =
                    PrinterCallingConvention<aarch64::Aarch64CallingConvention, aarch64::Aarch64Assembly>;
                return arenaAllocator->New<PrinterType>(arenaAllocator,
                                                        reinterpret_cast<aarch64::Aarch64Assembly *>(enc), descr, mode);
            }
            return arenaAllocator->New<aarch64::Aarch64CallingConvention>(arenaAllocator, enc, descr, mode);
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        case Arch::X86_64: {
            if (printAsm) {
                using PrinterType = PrinterCallingConvention<amd64::Amd64CallingConvention, amd64::Amd64Assembly>;
                return arenaAllocator->New<PrinterType>(arenaAllocator, reinterpret_cast<amd64::Amd64Assembly *>(enc),
                                                        descr, mode);
            }
            return arenaAllocator->New<amd64::Amd64CallingConvention>(arenaAllocator, enc, descr, mode);
        }
#endif
        default:
            return nullptr;
    }
}
}  // namespace ark::compiler
