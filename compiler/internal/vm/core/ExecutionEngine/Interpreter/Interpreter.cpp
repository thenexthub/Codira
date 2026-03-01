//===- Interpreter.cpp - Top-Level LLVM Interpreter Implementation --------===//
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
// This file implements the top-level functionality for the LLVM interpreter.
// This interpreter is designed to be a very simple, portable, inefficient
// interpreter.
//
//===----------------------------------------------------------------------===//

#include "Interpreter.h"
#include "vm/core/CodeGen/IntrinsicLowering.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/Module.h"
#include <cstring>
using namespace vm::core;

namespace {

static struct RegisterInterp {
  RegisterInterp() { Interpreter::Register(); }
} InterpRegistrator;

}

extern "C" void LLVMLinkInInterpreter() { }

/// Create a new interpreter object.
///
ExecutionEngine *Interpreter::create(std::unique_ptr<Module> M,
                                     std::string *ErrStr) {
  // Tell this Module to materialize everything and release the GVMaterializer.
  if (Error Err = M->materializeAll()) {
    std::string Msg;
    handleAllErrors(std::move(Err), [&](ErrorInfoBase &EIB) {
      Msg = EIB.message();
    });
    if (ErrStr)
      *ErrStr = Msg;
    // We got an error, just return 0
    return nullptr;
  }

  return new Interpreter(std::move(M));
}

//===----------------------------------------------------------------------===//
// Interpreter ctor - Initialize stuff
//
Interpreter::Interpreter(std::unique_ptr<Module> M)
    : ExecutionEngine(std::move(M)) {

  memset(&ExitValue.Untyped, 0, sizeof(ExitValue.Untyped));
  // Initialize the "backend"
  initializeExecutionEngine();
  initializeExternalFunctions();
  emitGlobals();

  IL = new IntrinsicLowering(getDataLayout());
}

Interpreter::~Interpreter() {
  delete IL;
}

void Interpreter::runAtExitHandlers () {
  while (!AtExitHandlers.empty()) {
    callFunction(AtExitHandlers.back(), {});
    AtExitHandlers.pop_back();
    run();
  }
}

/// run - Start execution with the specified function and arguments.
///
GenericValue Interpreter::runFunction(Function *F,
                                      ArrayRef<GenericValue> ArgValues) {
  assert (F && "Function *F was null at entry to run()");

  // Try extra hard not to pass extra args to a function that isn't
  // expecting them.  C programmers frequently bend the rules and
  // declare main() with fewer parameters than it actually gets
  // passed, and the interpreter barfs if you pass a function more
  // parameters than it is declared to take. This does not attempt to
  // take into account gratuitous differences in declared types,
  // though.
  const size_t ArgCount = F->getFunctionType()->getNumParams();
  ArrayRef<GenericValue> ActualArgs =
      ArgValues.slice(0, std::min(ArgValues.size(), ArgCount));

  // Set up the function call.
  callFunction(F, ActualArgs);

  // Start executing the function.
  run();

  return ExitValue;
}
