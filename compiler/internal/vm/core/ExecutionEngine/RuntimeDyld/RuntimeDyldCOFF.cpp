//===-- RuntimeDyldCOFF.cpp - Run-time dynamic linker for MC-JIT -*- C++ -*-==//
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
// Implementation of COFF support for the MC-JIT runtime dynamic linker.
//
//===----------------------------------------------------------------------===//

#include "RuntimeDyldCOFF.h"
#include "Targets/RuntimeDyldCOFFAArch64.h"
#include "Targets/RuntimeDyldCOFFI386.h"
#include "Targets/RuntimeDyldCOFFThumb.h"
#include "Targets/RuntimeDyldCOFFX86_64.h"
#include "vm/core/Object/ObjectFile.h"
#include "vm/core/Support/FormatVariadic.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;
using namespace vm::core::object;

#define DEBUG_TYPE "dyld"

namespace {

class LoadedCOFFObjectInfo final
    : public LoadedObjectInfoHelper<LoadedCOFFObjectInfo,
                                    RuntimeDyld::LoadedObjectInfo> {
public:
  LoadedCOFFObjectInfo(
      RuntimeDyldImpl &RTDyld,
      RuntimeDyld::LoadedObjectInfo::ObjSectionToIDMap ObjSecToIDMap)
      : LoadedObjectInfoHelper(RTDyld, std::move(ObjSecToIDMap)) {}

  OwningBinary<ObjectFile>
  getObjectForDebug(const ObjectFile &Obj) const override {
    return OwningBinary<ObjectFile>();
  }
};
}

namespace vm::core {

std::unique_ptr<RuntimeDyldCOFF>
toolchain::RuntimeDyldCOFF::create(Triple::ArchType Arch,
                              RuntimeDyld::MemoryManager &MemMgr,
                              JITSymbolResolver &Resolver) {
  switch (Arch) {
  default: llvm_unreachable("Unsupported target for RuntimeDyldCOFF.");
  case Triple::x86:
    return std::make_unique<RuntimeDyldCOFFI386>(MemMgr, Resolver);
  case Triple::thumb:
    return std::make_unique<RuntimeDyldCOFFThumb>(MemMgr, Resolver);
  case Triple::x86_64:
    return std::make_unique<RuntimeDyldCOFFX86_64>(MemMgr, Resolver);
  case Triple::aarch64:
    return std::make_unique<RuntimeDyldCOFFAArch64>(MemMgr, Resolver);
  }
}

std::unique_ptr<RuntimeDyld::LoadedObjectInfo>
RuntimeDyldCOFF::loadObject(const object::ObjectFile &O) {
  if (auto ObjSectionToIDOrErr = loadObjectImpl(O)) {
    return std::make_unique<LoadedCOFFObjectInfo>(*this, *ObjSectionToIDOrErr);
  } else {
    HasError = true;
    raw_string_ostream ErrStream(ErrorStr);
    logAllUnhandledErrors(ObjSectionToIDOrErr.takeError(), ErrStream);
    return nullptr;
  }
}

uint64_t RuntimeDyldCOFF::getSymbolOffset(const SymbolRef &Sym) {
  // The value in a relocatable COFF object is the offset.
  return cantFail(Sym.getValue());
}

uint64_t RuntimeDyldCOFF::getDLLImportOffset(unsigned SectionID, StubMap &Stubs,
                                             StringRef Name,
                                             bool SetSectionIDMinus1) {
  LLVM_DEBUG(dbgs() << "Getting DLLImport entry for " << Name << "... ");
  assert(Name.starts_with(getImportSymbolPrefix()) &&
         "Not a DLLImport symbol?");
  RelocationValueRef Reloc;
  Reloc.SymbolName = Name.data();
  auto [It, Inserted] = Stubs.try_emplace(Reloc);
  if (!Inserted) {
    LLVM_DEBUG(dbgs() << format("{0:x8}", It->second) << "\n");
    return It->second;
  }

  assert(SectionID < Sections.size() && "SectionID out of range");
  auto &Sec = Sections[SectionID];
  auto EntryOffset = alignTo(Sec.getStubOffset(), PointerSize);
  Sec.advanceStubOffset(EntryOffset + PointerSize - Sec.getStubOffset());
  It->second = EntryOffset;

  RelocationEntry RE(SectionID, EntryOffset, PointerReloc, 0, false,
                     Log2_64(PointerSize));
  // Hack to tell I386/Thumb resolveRelocation that this isn't section relative.
  if (SetSectionIDMinus1)
    RE.Sections.SectionA = -1;
  addRelocationForSymbol(RE, Name.drop_front(getImportSymbolPrefix().size()));

  LLVM_DEBUG({
    dbgs() << "Creating entry at "
           << formatv("{0:x16} + {1:x8} ( {2:x16} )", Sec.getLoadAddress(),
                      EntryOffset, Sec.getLoadAddress() + EntryOffset)
           << "\n";
  });
  return EntryOffset;
}

bool RuntimeDyldCOFF::isCompatibleFile(const object::ObjectFile &Obj) const {
  return Obj.isCOFF();
}

bool RuntimeDyldCOFF::relocationNeedsDLLImportStub(
    const RelocationRef &R) const {
  object::symbol_iterator Symbol = R.getSymbol();
  Expected<StringRef> TargetNameOrErr = Symbol->getName();
  if (!TargetNameOrErr)
    return false;

  return TargetNameOrErr->starts_with(getImportSymbolPrefix());
}

} // namespace vm::core
