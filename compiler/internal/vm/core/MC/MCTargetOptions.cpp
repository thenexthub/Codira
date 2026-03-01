//===- lib/MC/MCTargetOptions.cpp - MC Target Options ---------------------===//
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

#include "vm/core/MC/MCTargetOptions.h"
#include "vm/core/ADT/StringRef.h"

using namespace vm::core;

MCTargetOptions::MCTargetOptions()
    : MCRelaxAll(false), MCNoExecStack(false), MCFatalWarnings(false),
      MCNoWarn(false), MCNoDeprecatedWarn(false), MCNoTypeCheck(false),
      MCSaveTempLabels(false), MCIncrementalLinkerCompatible(false),
      FDPIC(false), ShowMCEncoding(false), ShowMCInst(false), AsmVerbose(false),
      PreserveAsmComments(true), Dwarf64(false),
      EmitDwarfUnwind(EmitDwarfUnwindType::Default),
      MCUseDwarfDirectory(DefaultDwarfDirectory),
      EmitCompactUnwindNonCanonical(false), EmitSFrameUnwind(false),
      PPCUseFullRegisterNames(false) {}

StringRef MCTargetOptions::getABIName() const {
  return ABIName;
}

StringRef MCTargetOptions::getAssemblyLanguage() const {
  return AssemblyLanguage;
}
