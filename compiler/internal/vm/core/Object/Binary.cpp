//===- Binary.cpp - A generic binary file ---------------------------------===//
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
// This file defines the Binary class.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Object/Binary.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/BinaryFormat/Magic.h"
#include "vm/core/Object/Archive.h"
#include "vm/core/Object/Error.h"
#include "vm/core/Object/MachOUniversal.h"
#include "vm/core/Object/Minidump.h"
#include "vm/core/Object/ObjectFile.h"
#include "vm/core/Object/OffloadBinary.h"
#include "vm/core/Object/TapiUniversal.h"
#include "vm/core/Object/WindowsResource.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/ErrorOr.h"
#include "vm/core/Support/MemoryBuffer.h"
#include <memory>
#include <system_error>

using namespace vm::core;
using namespace object;

Binary::~Binary() = default;

Binary::Binary(unsigned int Type, MemoryBufferRef Source)
    : TypeID(Type), Data(Source) {}

StringRef Binary::getData() const { return Data.getBuffer(); }

StringRef Binary::getFileName() const { return Data.getBufferIdentifier(); }

MemoryBufferRef Binary::getMemoryBufferRef() const { return Data; }

Expected<std::unique_ptr<Binary>> object::createBinary(MemoryBufferRef Buffer,
                                                       LLVMContext *Context,
                                                       bool InitContent) {
  file_magic Type = identify_magic(Buffer.getBuffer());

  switch (Type) {
  case file_magic::archive:
    return Archive::create(Buffer);
  case file_magic::elf:
  case file_magic::elf_relocatable:
  case file_magic::elf_executable:
  case file_magic::elf_shared_object:
  case file_magic::elf_core:
  case file_magic::goff_object:
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
  case file_magic::coff_object:
  case file_magic::coff_import_library:
  case file_magic::pecoff_executable:
  case file_magic::bitcode:
  case file_magic::xcoff_object_32:
  case file_magic::xcoff_object_64:
  case file_magic::wasm_object:
  case file_magic::dxcontainer_object:
    return ObjectFile::createSymbolicFile(Buffer, Type, Context, InitContent);
  case file_magic::macho_universal_binary:
    return MachOUniversalBinary::create(Buffer);
  case file_magic::windows_resource:
    return WindowsResource::createWindowsResource(Buffer);
  case file_magic::pdb:
    // PDB does not support the Binary interface.
    return errorCodeToError(object_error::invalid_file_type);
  case file_magic::unknown:
  case file_magic::clang_ast:
  case file_magic::cuda_fatbinary:
  case file_magic::coff_cl_gl_object:
  case file_magic::offload_bundle:
  case file_magic::offload_bundle_compressed:
  case file_magic::spirv_object:
    // Unrecognized object file format.
    return errorCodeToError(object_error::invalid_file_type);
  case file_magic::offload_binary:
    return OffloadBinary::create(Buffer);
  case file_magic::minidump:
    return MinidumpFile::create(Buffer);
  case file_magic::tapi_file:
    return TapiUniversal::create(Buffer);
  }
  llvm_unreachable("Unexpected Binary File Type");
}

Expected<OwningBinary<Binary>>
object::createBinary(StringRef Path, LLVMContext *Context, bool InitContent) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
      MemoryBuffer::getFileOrSTDIN(Path, /*IsText=*/false,
                                   /*RequiresNullTerminator=*/false);
  if (std::error_code EC = FileOrErr.getError())
    return errorCodeToError(EC);
  std::unique_ptr<MemoryBuffer> &Buffer = FileOrErr.get();

  Expected<std::unique_ptr<Binary>> BinOrErr =
      createBinary(Buffer->getMemBufferRef(), Context, InitContent);
  if (!BinOrErr)
    return BinOrErr.takeError();
  std::unique_ptr<Binary> &Bin = BinOrErr.get();

  return OwningBinary<Binary>(std::move(Bin), std::move(Buffer));
}
