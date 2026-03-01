//===- IFSStub.cpp --------------------------------------------------------===//
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
//===-----------------------------------------------------------------------===/

#include "vm/core/InterfaceStub/IFSStub.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;
using namespace vm::core::ifs;

IFSStub::IFSStub(IFSStub const &Stub) {
  IfsVersion = Stub.IfsVersion;
  Target = Stub.Target;
  SoName = Stub.SoName;
  NeededLibs = Stub.NeededLibs;
  Symbols = Stub.Symbols;
}

IFSStub::IFSStub(IFSStub &&Stub) {
  IfsVersion = std::move(Stub.IfsVersion);
  Target = std::move(Stub.Target);
  SoName = std::move(Stub.SoName);
  NeededLibs = std::move(Stub.NeededLibs);
  Symbols = std::move(Stub.Symbols);
}

IFSStubTriple::IFSStubTriple(IFSStubTriple const &Stub) : IFSStub() {
  IfsVersion = Stub.IfsVersion;
  Target = Stub.Target;
  SoName = Stub.SoName;
  NeededLibs = Stub.NeededLibs;
  Symbols = Stub.Symbols;
}

IFSStubTriple::IFSStubTriple(IFSStub const &Stub) {
  IfsVersion = Stub.IfsVersion;
  Target = Stub.Target;
  SoName = Stub.SoName;
  NeededLibs = Stub.NeededLibs;
  Symbols = Stub.Symbols;
}

IFSStubTriple::IFSStubTriple(IFSStubTriple &&Stub) {
  IfsVersion = std::move(Stub.IfsVersion);
  Target = std::move(Stub.Target);
  SoName = std::move(Stub.SoName);
  NeededLibs = std::move(Stub.NeededLibs);
  Symbols = std::move(Stub.Symbols);
}

bool IFSTarget::empty() {
  return !Triple && !ObjectFormat && !Arch && !ArchString && !Endianness &&
         !BitWidth;
}

uint8_t ifs::convertIFSBitWidthToELF(IFSBitWidthType BitWidth) {
  switch (BitWidth) {
  case IFSBitWidthType::IFS32:
    return ELF::ELFCLASS32;
  case IFSBitWidthType::IFS64:
    return ELF::ELFCLASS64;
  default:
    llvm_unreachable("unknown bitwidth");
  }
}

uint8_t ifs::convertIFSEndiannessToELF(IFSEndiannessType Endianness) {
  switch (Endianness) {
  case IFSEndiannessType::Little:
    return ELF::ELFDATA2LSB;
  case IFSEndiannessType::Big:
    return ELF::ELFDATA2MSB;
  default:
    llvm_unreachable("unknown endianness");
  }
}

uint8_t ifs::convertIFSSymbolTypeToELF(IFSSymbolType SymbolType) {
  switch (SymbolType) {
  case IFSSymbolType::Object:
    return ELF::STT_OBJECT;
  case IFSSymbolType::Func:
    return ELF::STT_FUNC;
  case IFSSymbolType::TLS:
    return ELF::STT_TLS;
  case IFSSymbolType::NoType:
    return ELF::STT_NOTYPE;
  default:
    llvm_unreachable("unknown symbol type");
  }
}

IFSBitWidthType ifs::convertELFBitWidthToIFS(uint8_t BitWidth) {
  switch (BitWidth) {
  case ELF::ELFCLASS32:
    return IFSBitWidthType::IFS32;
  case ELF::ELFCLASS64:
    return IFSBitWidthType::IFS64;
  default:
    return IFSBitWidthType::Unknown;
  }
}

IFSEndiannessType ifs::convertELFEndiannessToIFS(uint8_t Endianness) {
  switch (Endianness) {
  case ELF::ELFDATA2LSB:
    return IFSEndiannessType::Little;
  case ELF::ELFDATA2MSB:
    return IFSEndiannessType::Big;
  default:
    return IFSEndiannessType::Unknown;
  }
}

IFSSymbolType ifs::convertELFSymbolTypeToIFS(uint8_t SymbolType) {
  SymbolType = SymbolType & 0xf;
  switch (SymbolType) {
  case ELF::STT_OBJECT:
    return IFSSymbolType::Object;
  case ELF::STT_FUNC:
    return IFSSymbolType::Func;
  case ELF::STT_TLS:
    return IFSSymbolType::TLS;
  case ELF::STT_NOTYPE:
    return IFSSymbolType::NoType;
  default:
    return IFSSymbolType::Unknown;
  }
}
