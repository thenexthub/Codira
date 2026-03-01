//===- ObjectFile.cpp - File format independent object file ---------------===//
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
// This file defines a file format independent ObjectFile class.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Object/ObjectFile.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/BinaryFormat/Magic.h"
#include "vm/core/Object/Binary.h"
#include "vm/core/Object/COFF.h"
#include "vm/core/Object/DXContainer.h"
#include "vm/core/Object/Error.h"
#include "vm/core/Object/MachO.h"
#include "vm/core/Object/Wasm.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/ErrorOr.h"
#include "vm/core/Support/Format.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/raw_ostream.h"
#include <cstdint>
#include <memory>
#include <system_error>

using namespace vm::core;
using namespace object;

raw_ostream &object::operator<<(raw_ostream &OS, const SectionedAddress &Addr) {
  OS << "SectionedAddress{" << format_hex(Addr.Address, 10);
  if (Addr.SectionIndex != SectionedAddress::UndefSection)
    OS << ", " << Addr.SectionIndex;
  return OS << "}";
}

void ObjectFile::anchor() {}

ObjectFile::ObjectFile(unsigned int Type, MemoryBufferRef Source)
    : SymbolicFile(Type, Source) {}

bool SectionRef::containsSymbol(SymbolRef S) const {
  Expected<section_iterator> SymSec = S.getSection();
  if (!SymSec) {
    // TODO: Actually report errors helpfully.
    consumeError(SymSec.takeError());
    return false;
  }
  return *this == **SymSec;
}

Expected<uint64_t> ObjectFile::getSymbolValue(DataRefImpl Ref) const {
  uint32_t Flags;
  if (Error E = getSymbolFlags(Ref).moveInto(Flags))
    // TODO: Test this error.
    return std::move(E);

  if (Flags & SymbolRef::SF_Undefined)
    return 0;
  if (Flags & SymbolRef::SF_Common)
    return getCommonSymbolSize(Ref);
  return getSymbolValueImpl(Ref);
}

Error ObjectFile::printSymbolName(raw_ostream &OS, DataRefImpl Symb) const {
  Expected<StringRef> Name = getSymbolName(Symb);
  if (!Name)
    return Name.takeError();
  OS << *Name;
  return Error::success();
}

uint32_t ObjectFile::getSymbolAlignment(DataRefImpl DRI) const { return 0; }

bool ObjectFile::isSectionBitcode(DataRefImpl Sec) const {
  Expected<StringRef> NameOrErr = getSectionName(Sec);
  if (NameOrErr)
    return *NameOrErr == ".toolchain.lto";
  consumeError(NameOrErr.takeError());
  return false;
}

bool ObjectFile::isSectionStripped(DataRefImpl Sec) const { return false; }

bool ObjectFile::isBerkeleyText(DataRefImpl Sec) const {
  return isSectionText(Sec);
}

bool ObjectFile::isBerkeleyData(DataRefImpl Sec) const {
  return isSectionData(Sec);
}

bool ObjectFile::isDebugSection(DataRefImpl Sec) const { return false; }

bool ObjectFile::hasDebugInfo() const {
  return any_of(sections(),
                [](SectionRef Sec) { return Sec.isDebugSection(); });
}

Expected<section_iterator>
ObjectFile::getRelocatedSection(DataRefImpl Sec) const {
  return section_iterator(SectionRef(Sec, this));
}

Triple ObjectFile::makeTriple() const {
  Triple TheTriple;
  auto Arch = getArch();
  TheTriple.setArch(Triple::ArchType(Arch));

  auto OS = getOS();
  if (OS != Triple::UnknownOS)
    TheTriple.setOS(OS);

  // For ARM targets, try to use the build attributes to build determine
  // the build target. Target features are also added, but later during
  // disassembly.
  if (Arch == Triple::arm || Arch == Triple::armeb)
    setARMSubArch(TheTriple);

  // TheTriple defaults to ELF, and COFF doesn't have an environment:
  // something we can do here is indicate that it is mach-o.
  if (isMachO()) {
    TheTriple.setObjectFormat(Triple::MachO);
  } else if (isCOFF()) {
    const auto COFFObj = cast<COFFObjectFile>(this);
    if (COFFObj->getArch() == Triple::thumb)
      TheTriple.setTriple("thumbv7-windows");
  } else if (isXCOFF()) {
    // XCOFF implies AIX.
    TheTriple.setOS(Triple::AIX);
    TheTriple.setObjectFormat(Triple::XCOFF);
  } else if (isGOFF()) {
    TheTriple.setOS(Triple::ZOS);
    TheTriple.setObjectFormat(Triple::GOFF);
  } else if (TheTriple.isAMDGPU()) {
    TheTriple.setVendor(Triple::AMD);
  } else if (TheTriple.isNVPTX()) {
    TheTriple.setVendor(Triple::NVIDIA);
  }

  return TheTriple;
}

Expected<std::unique_ptr<ObjectFile>>
ObjectFile::createObjectFile(MemoryBufferRef Object, file_magic Type,
                             bool InitContent) {
  StringRef Data = Object.getBuffer();
  if (Type == file_magic::unknown)
    Type = identify_magic(Data);

  switch (Type) {
  case file_magic::unknown:
  case file_magic::bitcode:
  case file_magic::clang_ast:
  case file_magic::coff_cl_gl_object:
  case file_magic::archive:
  case file_magic::macho_universal_binary:
  case file_magic::windows_resource:
  case file_magic::pdb:
  case file_magic::minidump:
  case file_magic::goff_object:
  case file_magic::cuda_fatbinary:
  case file_magic::offload_binary:
  case file_magic::offload_bundle:
  case file_magic::offload_bundle_compressed:
  case file_magic::spirv_object:
    return errorCodeToError(object_error::invalid_file_type);
  case file_magic::tapi_file:
    return errorCodeToError(object_error::invalid_file_type);
  case file_magic::elf:
  case file_magic::elf_relocatable:
  case file_magic::elf_executable:
  case file_magic::elf_shared_object:
  case file_magic::elf_core:
    return createELFObjectFile(Object, InitContent);
  case file_magic::macho_object:
  case file_magic::macho_executable:
  case file_magic::macho_fixed_virtual_memory_shared_lib:
  case file_magic::macho_core:
  case file_magic::macho_preload_executable:
  case file_magic::macho_dynamically_linked_shared_lib:
  case file_magic::macho_dynamic_linker:
  case file_magic::macho_bundle:
  case file_magic::macho_dynamically_linked_shared_lib_stub:
  case file_magic::macho_dsym_companion:
  case file_magic::macho_kext_bundle:
  case file_magic::macho_file_set:
    return createMachOObjectFile(Object);
  case file_magic::coff_object:
  case file_magic::coff_import_library:
  case file_magic::pecoff_executable:
    return createCOFFObjectFile(Object);
  case file_magic::xcoff_object_32:
    return createXCOFFObjectFile(Object, Binary::ID_XCOFF32);
  case file_magic::xcoff_object_64:
    return createXCOFFObjectFile(Object, Binary::ID_XCOFF64);
  case file_magic::wasm_object:
    return createWasmObjectFile(Object);
  case file_magic::dxcontainer_object:
    return createDXContainerObjectFile(Object);
  }
  llvm_unreachable("Unexpected Object File Type");
}

Expected<OwningBinary<ObjectFile>>
ObjectFile::createObjectFile(StringRef ObjectPath) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
      MemoryBuffer::getFile(ObjectPath);
  if (std::error_code EC = FileOrErr.getError())
    return errorCodeToError(EC);
  std::unique_ptr<MemoryBuffer> Buffer = std::move(FileOrErr.get());

  Expected<std::unique_ptr<ObjectFile>> ObjOrErr =
      createObjectFile(Buffer->getMemBufferRef());
  if (Error Err = ObjOrErr.takeError())
    return std::move(Err);
  std::unique_ptr<ObjectFile> Obj = std::move(ObjOrErr.get());

  return OwningBinary<ObjectFile>(std::move(Obj), std::move(Buffer));
}

bool ObjectFile::isReflectionSectionStrippable(
    toolchain::binaryformat::Swift5ReflectionSectionKind ReflectionSectionKind)
    const {
  using toolchain::binaryformat::Swift5ReflectionSectionKind;
  return ReflectionSectionKind == Swift5ReflectionSectionKind::fieldmd ||
         ReflectionSectionKind == Swift5ReflectionSectionKind::reflstr ||
         ReflectionSectionKind == Swift5ReflectionSectionKind::assocty;
}
