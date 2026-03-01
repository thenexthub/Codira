//===-- RuntimeDyldCOFF.h - Run-time dynamic linker for MC-JIT ---*- C++ -*-==//
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
//
// COFF support for MC-JIT runtime dynamic linker.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_RUNTIME_DYLD_COFF_H
#define LLVM_RUNTIME_DYLD_COFF_H

#include "RuntimeDyldImpl.h"
#include "vm/core/Support/MathExtras.h"

namespace vm::core {

// Common base class for COFF dynamic linker support.
// Concrete subclasses for each target can be found in ./Targets.
class RuntimeDyldCOFF : public RuntimeDyldImpl {

public:
  std::unique_ptr<RuntimeDyld::LoadedObjectInfo>
  loadObject(const object::ObjectFile &Obj) override;
  bool isCompatibleFile(const object::ObjectFile &Obj) const override;

  static std::unique_ptr<RuntimeDyldCOFF>
  create(Triple::ArchType Arch, RuntimeDyld::MemoryManager &MemMgr,
         JITSymbolResolver &Resolver);

protected:
  RuntimeDyldCOFF(RuntimeDyld::MemoryManager &MemMgr,
                  JITSymbolResolver &Resolver, unsigned PointerSize,
                  uint32_t PointerReloc)
      : RuntimeDyldImpl(MemMgr, Resolver), PointerSize(PointerSize),
        PointerReloc(PointerReloc) {
    assert((PointerSize == 4 || PointerSize == 8) && "Unexpected pointer size");
  }

  uint64_t getSymbolOffset(const SymbolRef &Sym);
  uint64_t getDLLImportOffset(unsigned SectionID, StubMap &Stubs,
                              StringRef Name, bool SetSectionIDMinus1 = false);

  static constexpr StringRef getImportSymbolPrefix() { return "__imp_"; }

  bool relocationNeedsDLLImportStub(const RelocationRef &R) const override;

  unsigned sizeAfterAddingDLLImportStub(unsigned Size) const override {
    return alignTo(Size, PointerSize) + PointerSize;
  }

private:
  unsigned PointerSize;
  uint32_t PointerReloc;
};

} // end namespace vm::core

#endif // LLVM_RUNTIME_DYLD_COFF_H
