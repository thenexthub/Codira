//===- SymbolicFile.cpp - Interface that only provides symbols ------------===//
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
// This file defines a file format independent SymbolicFile class.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Object/SymbolicFile.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/BinaryFormat/Magic.h"
#include "vm/core/Object/COFFImportFile.h"
#include "vm/core/Object/Error.h"
#include "vm/core/Object/IRObjectFile.h"
#include "vm/core/Object/ObjectFile.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/ErrorHandling.h"
#include <memory>

using namespace vm::core;
using namespace object;

namespace vm::core {
class LLVMContext;
}

SymbolicFile::SymbolicFile(unsigned int Type, MemoryBufferRef Source)
    : Binary(Type, Source) {}

SymbolicFile::~SymbolicFile() = default;

Expected<std::unique_ptr<SymbolicFile>>
SymbolicFile::createSymbolicFile(MemoryBufferRef Object, file_magic Type,
                                 LLVMContext *Context, bool InitContent) {
  StringRef Data = Object.getBuffer();
  if (Type == file_magic::unknown)
    Type = identify_magic(Data);

  if (!isSymbolicFile(Type, Context))
    return errorCodeToError(object_error::invalid_file_type);

  switch (Type) {
  case file_magic::bitcode:
    // Context is guaranteed to be non-null here, because bitcode magic only
    // indicates a symbolic file when Context is non-null.
    return IRObjectFile::create(Object, *Context);
  case file_magic::elf:
  case file_magic::elf_executable:
  case file_magic::elf_shared_object:
  case file_magic::elf_core:
  case file_magic::goff_object:
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
  case file_magic::pecoff_executable:
  case file_magic::xcoff_object_32:
  case file_magic::xcoff_object_64:
  case file_magic::wasm_object:
  case file_magic::dxcontainer_object:
    return ObjectFile::createObjectFile(Object, Type, InitContent);
  case file_magic::coff_import_library:
    return std::unique_ptr<SymbolicFile>(new COFFImportFile(Object));
  case file_magic::elf_relocatable:
  case file_magic::macho_object:
  case file_magic::coff_object: {
    Expected<std::unique_ptr<ObjectFile>> Obj =
        ObjectFile::createObjectFile(Object, Type, InitContent);
    if (!Obj || !Context)
      return std::move(Obj);

    Expected<MemoryBufferRef> BCData =
        IRObjectFile::findBitcodeInObject(*Obj->get());
    if (!BCData) {
      consumeError(BCData.takeError());
      return std::move(Obj);
    }

    return IRObjectFile::create(
        MemoryBufferRef(BCData->getBuffer(), Object.getBufferIdentifier()),
        *Context);
  }
  default:
    llvm_unreachable("Unexpected Binary File Type");
  }
}

bool SymbolicFile::isSymbolicFile(file_magic Type, const LLVMContext *Context) {
  switch (Type) {
  case file_magic::bitcode:
    return Context != nullptr;
  case file_magic::elf:
  case file_magic::elf_executable:
  case file_magic::elf_shared_object:
  case file_magic::elf_core:
  case file_magic::goff_object:
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
  case file_magic::pecoff_executable:
  case file_magic::xcoff_object_32:
  case file_magic::xcoff_object_64:
  case file_magic::wasm_object:
  case file_magic::coff_import_library:
  case file_magic::elf_relocatable:
  case file_magic::macho_object:
  case file_magic::coff_object:
  case file_magic::dxcontainer_object:
    return true;
  default:
    return false;
  }
}
