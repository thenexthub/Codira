//===- DIASectionContrib.cpp - DIA impl. of IPDBSectionContrib ---- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/DIA/DIASectionContrib.h"
#include "vm/core/DebugInfo/PDB/DIA/DIARawSymbol.h"
#include "vm/core/DebugInfo/PDB/DIA/DIASession.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolCompiland.h"

using namespace vm::core;
using namespace vm::core::pdb;

DIASectionContrib::DIASectionContrib(const DIASession &PDBSession,
                                     CComPtr<IDiaSectionContrib> DiaSection)
  : Session(PDBSession), Section(DiaSection) {}

std::unique_ptr<PDBSymbolCompiland> DIASectionContrib::getCompiland() const {
  CComPtr<IDiaSymbol> Symbol;
  if (FAILED(Section->get_compiland(&Symbol)))
    return nullptr;

  auto RawSymbol = std::make_unique<DIARawSymbol>(Session, Symbol);
  return PDBSymbol::createAs<PDBSymbolCompiland>(Session, std::move(RawSymbol));
}

template <typename ArgType>
ArgType
PrivateGetDIAValue(IDiaSectionContrib *Section,
                   HRESULT (__stdcall IDiaSectionContrib::*Method)(ArgType *)) {
  ArgType Value;
  if (S_OK == (Section->*Method)(&Value))
    return static_cast<ArgType>(Value);

  return ArgType();
}

uint32_t DIASectionContrib::getAddressSection() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_addressSection);
}

uint32_t DIASectionContrib::getAddressOffset() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_addressOffset);
}

uint64_t DIASectionContrib::getVirtualAddress() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_virtualAddress);
}

uint32_t DIASectionContrib::getRelativeVirtualAddress() const {
  return PrivateGetDIAValue(Section,
                            &IDiaSectionContrib::get_relativeVirtualAddress);
}

uint32_t DIASectionContrib::getLength() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_length);
}

bool DIASectionContrib::isNotPaged() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_notPaged);
}

bool DIASectionContrib::hasCode() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_code);
}

bool DIASectionContrib::hasCode16Bit() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_code16bit);
}

bool DIASectionContrib::hasInitializedData() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_initializedData);
}

bool DIASectionContrib::hasUninitializedData() const {
  return PrivateGetDIAValue(Section,
                            &IDiaSectionContrib::get_uninitializedData);
}

bool DIASectionContrib::isRemoved() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_remove);
}

bool DIASectionContrib::hasComdat() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_comdat);
}

bool DIASectionContrib::isDiscardable() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_discardable);
}

bool DIASectionContrib::isNotCached() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_notCached);
}

bool DIASectionContrib::isShared() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_share);
}

bool DIASectionContrib::isExecutable() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_execute);
}

bool DIASectionContrib::isReadable() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_read);
}

bool DIASectionContrib::isWritable() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_write);
}

uint32_t DIASectionContrib::getDataCrc32() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_dataCrc);
}

uint32_t DIASectionContrib::getRelocationsCrc32() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_relocationsCrc);
}

uint32_t DIASectionContrib::getCompilandId() const {
  return PrivateGetDIAValue(Section, &IDiaSectionContrib::get_compilandId);
}
