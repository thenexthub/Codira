//===-- WebAssemblyUtilities - WebAssembly Utility Functions ---*- C++ -*-====//
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
/// utility functions.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_UTILS_WEBASSEMBLYUTILITIES_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_UTILS_WEBASSEMBLYUTILITIES_H

#include "vm/core/Support/CommandLine.h"

namespace vm::core {

class MachineBasicBlock;
class MachineInstr;
class MachineOperand;
class MCContext;
class MCSymbolWasm;
class TargetRegisterClass;
class WebAssemblyFunctionInfo;
class WebAssemblySubtarget;

namespace WebAssembly {

bool isChild(const MachineInstr &MI, const WebAssemblyFunctionInfo &MFI);
bool mayThrow(const MachineInstr &MI);

// Exception-related function names
extern const char *const ClangCallTerminateFn;
extern const char *const CxaBeginCatchFn;
extern const char *const CxaRethrowFn;
extern const char *const StdTerminateFn;
extern const char *const PersonalityWrapperFn;

/// Returns the operand number of a callee, assuming the argument is a call
/// instruction.
const MachineOperand &getCalleeOp(const MachineInstr &MI);

/// Returns the __indirect_function_table, for use in call_indirect and in
/// function bitcasts.
MCSymbolWasm *
getOrCreateFunctionTableSymbol(MCContext &Ctx,
                               const WebAssemblySubtarget *Subtarget);

/// Returns the __funcref_call_table, for use in funcref calls when lowered to
/// table.set + call_indirect.
MCSymbolWasm *
getOrCreateFuncrefCallTableSymbol(MCContext &Ctx,
                                  const WebAssemblySubtarget *Subtarget);

/// Find a catch instruction from an EH pad. Returns null if no catch
/// instruction found or the catch is in an invalid location.
MachineInstr *findCatch(MachineBasicBlock *EHPad);

/// Returns the appropriate copy opcode for the given register class.
unsigned getCopyOpcodeForRegClass(const TargetRegisterClass *RC);

/// Returns true if multivalue returns of a function can be lowered directly,
/// i.e., not indirectly via a pointer parameter that points to the value in
/// memory.
bool canLowerMultivalueReturn(const WebAssemblySubtarget *Subtarget);

/// Returns true if the function's return value(s) can be lowered directly,
/// i.e., not indirectly via a pointer parameter that points to the value in
/// memory.
bool canLowerReturn(size_t ResultSize, const WebAssemblySubtarget *Subtarget);

} // end namespace WebAssembly

} // end namespace vm::core

#endif
