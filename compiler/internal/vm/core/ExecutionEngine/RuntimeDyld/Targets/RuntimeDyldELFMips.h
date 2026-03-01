//===-- RuntimeDyldELFMips.h ---- ELF/Mips specific code. -------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_EXECUTIONENGINE_RUNTIMEDYLD_TARGETS_RUNTIMEDYLDELFMIPS_H
#define LLVM_LIB_EXECUTIONENGINE_RUNTIMEDYLD_TARGETS_RUNTIMEDYLDELFMIPS_H

#include "../RuntimeDyldELF.h"

#define DEBUG_TYPE "dyld"

namespace vm::core {

class RuntimeDyldELFMips : public RuntimeDyldELF {
public:

  typedef uint64_t TargetPtrT;

  RuntimeDyldELFMips(RuntimeDyld::MemoryManager &MM,
                     JITSymbolResolver &Resolver)
      : RuntimeDyldELF(MM, Resolver) {}

  void resolveRelocation(const RelocationEntry &RE, uint64_t Value) override;

protected:
  void resolveMIPSO32Relocation(const SectionEntry &Section, uint64_t Offset,
                                uint32_t Value, uint32_t Type, int32_t Addend);
  void resolveMIPSN32Relocation(const SectionEntry &Section, uint64_t Offset,
                                uint64_t Value, uint32_t Type, int64_t Addend,
                                uint64_t SymOffset, SID SectionID);
  void resolveMIPSN64Relocation(const SectionEntry &Section, uint64_t Offset,
                                uint64_t Value, uint32_t Type, int64_t Addend,
                                uint64_t SymOffset, SID SectionID);

private:
  /// A object file specific relocation resolver
  /// \param RE The relocation to be resolved
  /// \param Value Target symbol address to apply the relocation action
  uint64_t evaluateRelocation(const RelocationEntry &RE, uint64_t Value,
                              uint64_t Addend);

  /// A object file specific relocation resolver
  /// \param RE The relocation to be resolved
  /// \param Value Target symbol address to apply the relocation action
  void applyRelocation(const RelocationEntry &RE, uint64_t Value);

  int64_t evaluateMIPS32Relocation(const SectionEntry &Section, uint64_t Offset,
                                   uint64_t Value, uint32_t Type);
  int64_t evaluateMIPS64Relocation(const SectionEntry &Section,
                                   uint64_t Offset, uint64_t Value,
                                   uint32_t Type,  int64_t Addend,
                                   uint64_t SymOffset, SID SectionID);

  void applyMIPSRelocation(uint8_t *TargetPtr, int64_t CalculatedValue,
                           uint32_t Type);

};
}

#undef DEBUG_TYPE

#endif
