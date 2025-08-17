//===-- Optimizer/Builder/Todo.h --------------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_TODO_H
#define LANGUAGE_COMPABILITY_LOWER_TODO_H

#include "language/Compability/Optimizer/Support/FatalError.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/raw_ostream.h"
#include <cstdlib>

// This is throw-away code used to mark areas of the code that have not yet been
// developed.

#undef TODO
// Use TODO_NOLOC if no mlir location is available to indicate the line in
// Fortran source file that requires an unimplemented feature.
#undef TODO_NOLOC

#undef TODOQUOTE
#define TODOQUOTE(X) #X

// Give backtrace only in debug builds.
#undef GEN_TRACE
#ifdef NDEBUG
#define GEN_TRACE false
#else
#define GEN_TRACE true
#endif

#undef TODO_NOLOCDEFN
#define TODO_NOLOCDEFN(ToDoMsg, ToDoFile, ToDoLine, GenTrace)                  \
  do {                                                                         \
    toolchain::report_fatal_error(toolchain::Twine(ToDoFile ":" TODOQUOTE(               \
                                 ToDoLine) ": not yet implemented: ") +        \
                                 toolchain::Twine(ToDoMsg),                         \
                             GenTrace);                                        \
  } while (false)

#define TODO_NOLOC(ToDoMsg) TODO_NOLOCDEFN(ToDoMsg, __FILE__, __LINE__, false)
#define TODO_NOLOC_TRACE(ToDoMsg)                                              \
  TODO_NOLOCDEFN(ToDoMsg, __FILE__, __LINE__, GENTRACE)

#undef TODO_DEFN
#define TODO_DEFN(MlirLoc, ToDoMsg, ToDoFile, ToDoLine, GenTrace)              \
  do {                                                                         \
    fir::emitFatalError(MlirLoc,                                               \
                        toolchain::Twine(ToDoFile ":" TODOQUOTE(                    \
                            ToDoLine) ": not yet implemented: ") +             \
                            toolchain::Twine(ToDoMsg),                              \
                        GenTrace);                                             \
  } while (false)

#define TODO(MlirLoc, ToDoMsg)                                                 \
  TODO_DEFN(MlirLoc, ToDoMsg, __FILE__, __LINE__, false)
#define TODO_TRACE(MlirLoc, ToDoMsg)                                           \
  TODO_DEFN(MlirLoc, ToDoMsg, __FILE__, __LINE__, GEN_TRACE)

#endif // FORTRAN_LOWER_TODO_H
