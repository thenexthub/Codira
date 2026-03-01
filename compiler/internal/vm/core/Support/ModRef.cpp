//===--- ModRef.cpp - Memory effect modeling --------------------*- C++ -*-===//
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
//  This file implements ModRef and MemoryEffects misc functions.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/ModRef.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/StringExtras.h"

using namespace vm::core;

raw_ostream &toolchain::operator<<(raw_ostream &OS, ModRefInfo MR) {
  switch (MR) {
  case ModRefInfo::NoModRef:
    OS << "NoModRef";
    break;
  case ModRefInfo::Ref:
    OS << "Ref";
    break;
  case ModRefInfo::Mod:
    OS << "Mod";
    break;
  case ModRefInfo::ModRef:
    OS << "ModRef";
    break;
  }
  return OS;
}

raw_ostream &toolchain::operator<<(raw_ostream &OS, MemoryEffects ME) {
  interleaveComma(MemoryEffects::locations(), OS, [&](IRMemLocation Loc) {
    switch (Loc) {
    case IRMemLocation::ArgMem:
      OS << "ArgMem: ";
      break;
    case IRMemLocation::InaccessibleMem:
      OS << "InaccessibleMem: ";
      break;
    case IRMemLocation::ErrnoMem:
      OS << "ErrnoMem: ";
      break;
    case IRMemLocation::Other:
      OS << "Other: ";
      break;
    case IRMemLocation::TargetMem0:
      OS << "TargetMem0: ";
      break;
    case IRMemLocation::TargetMem1:
      OS << "TargetMem1: ";
      break;
    }
    OS << ME.getModRef(Loc);
  });
  return OS;
}

raw_ostream &toolchain::operator<<(raw_ostream &OS, CaptureComponents CC) {
  if (capturesNothing(CC)) {
    OS << "none";
    return OS;
  }

  ListSeparator LS;
  if (capturesAddressIsNullOnly(CC))
    OS << LS << "address_is_null";
  else if (capturesAddress(CC))
    OS << LS << "address";
  if (capturesReadProvenanceOnly(CC))
    OS << LS << "read_provenance";
  if (capturesFullProvenance(CC))
    OS << LS << "provenance";

  return OS;
}

raw_ostream &toolchain::operator<<(raw_ostream &OS, CaptureInfo CI) {
  ListSeparator LS;
  CaptureComponents Other = CI.getOtherComponents();
  CaptureComponents Ret = CI.getRetComponents();

  OS << "captures(";
  if (!capturesNothing(Other) || Other == Ret)
    OS << LS << Other;
  if (Other != Ret)
    OS << LS << "ret: " << Ret;
  OS << ")";
  return OS;
}
