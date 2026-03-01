//===- MCSymbolGOFF.cpp - GOFF Symbol Representation ----------------------===//
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

#include "vm/core/MC/MCSymbolGOFF.h"
#include "vm/core/BinaryFormat/GOFF.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;

bool MCSymbolGOFF::setSymbolAttribute(MCSymbolAttr Attribute) {
  switch (Attribute) {
  case MCSA_Invalid:
  case MCSA_Cold:
  case MCSA_ELF_TypeIndFunction:
  case MCSA_ELF_TypeTLS:
  case MCSA_ELF_TypeCommon:
  case MCSA_ELF_TypeNoType:
  case MCSA_ELF_TypeGnuUniqueObject:
  case MCSA_LGlobal:
  case MCSA_Extern:
  case MCSA_Exported:
  case MCSA_IndirectSymbol:
  case MCSA_Internal:
  case MCSA_LazyReference:
  case MCSA_Local:
  case MCSA_NoDeadStrip:
  case MCSA_SymbolResolver:
  case MCSA_AltEntry:
  case MCSA_PrivateExtern:
  case MCSA_Protected:
  case MCSA_Reference:
  case MCSA_WeakDefinition:
  case MCSA_WeakDefAutoPrivate:
  case MCSA_WeakAntiDep:
  case MCSA_Memtag:
    return false;

  case MCSA_ELF_TypeFunction:
    setCodeData(GOFF::ESDExecutable::ESD_EXE_CODE);
    break;
  case MCSA_ELF_TypeObject:
    setCodeData(GOFF::ESDExecutable::ESD_EXE_DATA);
    break;
  case MCSA_OSLinkage:
    setLinkage(GOFF::ESDLinkageType::ESD_LT_OS);
    break;
  case MCSA_XPLinkage:
    setLinkage(GOFF::ESDLinkageType::ESD_LT_XPLink);
    break;
  case MCSA_Global:
    setExternal(true);
    break;
  case MCSA_Weak:
  case MCSA_WeakReference:
    setExternal(true);
    setWeak();
    break;
  case MCSA_Hidden:
    setHidden(true);
    break;
  }

  return true;
}
