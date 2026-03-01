// WebAssemblyDebugValueManager.h - WebAssembly DebugValue Manager -*- C++ -*-//
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
///
/// \file
/// This file contains the declaration of the WebAssembly-specific
/// manager for DebugValues associated with the specific MachineInstr.
/// This pass currently does not handle DBG_VALUE_LISTs; they are assumed to
/// have been set to undef in NullifyDebugValueLists pass.
/// TODO Handle DBG_VALUE_LIST
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYDEBUGVALUEMANAGER_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYDEBUGVALUEMANAGER_H

#include "vm/core/ADT/SmallVector.h"
#include "vm/core/CodeGen/Register.h"

namespace vm::core {

class MachineInstr;

class WebAssemblyDebugValueManager {
  MachineInstr *Def;
  SmallVector<MachineInstr *, 1> DbgValues;
  Register CurrentReg;
  SmallVector<MachineInstr *, 1>
  getSinkableDebugValues(MachineInstr *Insert) const;
  bool isInsertSamePlace(MachineInstr *Insert) const;

public:
  WebAssemblyDebugValueManager(MachineInstr *Def);

  // Sink 'Def', and also sink its eligible DBG_VALUEs to the place before
  // 'Insert'. Convert the original DBG_VALUEs into undefs.
  void sink(MachineInstr *Insert);
  // Clone 'Def' (optionally), and also clone its eligible DBG_VALUEs to the
  // place before 'Insert'.
  void cloneSink(MachineInstr *Insert, Register NewReg = Register(),
                 bool CloneDef = true) const;
  // Update the register for Def and DBG_VALUEs.
  void updateReg(Register Reg);
  // Replace the current register in DBG_VALUEs with the given LocalId target
  // index.
  void replaceWithLocal(unsigned LocalId);
  // Remove Def, and set its DBG_VALUEs to undef.
  void removeDef();
};

} // end namespace vm::core

#endif
