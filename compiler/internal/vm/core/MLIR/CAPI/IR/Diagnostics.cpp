//===- Diagnostics.cpp - C Interface for MLIR Diagnostics -----------------===//
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

#include "mlir-c/Diagnostics.h"
#include "mlir/CAPI/Diagnostics.h"
#include "mlir/CAPI/IR.h"
#include "mlir/CAPI/Support.h"
#include "mlir/CAPI/Utils.h"
#include "mlir/IR/Diagnostics.h"

using namespace mlir;

void mlirDiagnosticPrint(MlirDiagnostic diagnostic, MlirStringCallback callback,
                         void *userData) {
  detail::CallbackOstream stream(callback, userData);
  unwrap(diagnostic).print(stream);
}

MlirLocation mlirDiagnosticGetLocation(MlirDiagnostic diagnostic) {
  return wrap(unwrap(diagnostic).getLocation());
}

MlirDiagnosticSeverity mlirDiagnosticGetSeverity(MlirDiagnostic diagnostic) {
  switch (unwrap(diagnostic).getSeverity()) {
  case mlir::DiagnosticSeverity::Error:
    return MlirDiagnosticError;
  case mlir::DiagnosticSeverity::Warning:
    return MlirDiagnosticWarning;
  case mlir::DiagnosticSeverity::Note:
    return MlirDiagnosticNote;
  case mlir::DiagnosticSeverity::Remark:
    return MlirDiagnosticRemark;
  }
  llvm_unreachable("unhandled diagnostic severity");
}

// Notes are stored in a vector, so note iterator range is a pair of
// random access iterators, for which it is cheap to compute the size.
intptr_t mlirDiagnosticGetNumNotes(MlirDiagnostic diagnostic) {
  return static_cast<intptr_t>(toolchain::size(unwrap(diagnostic).getNotes()));
}

// Notes are stored in a vector, so the iterator is a random access iterator,
// cheap to advance multiple steps at a time.
MlirDiagnostic mlirDiagnosticGetNote(MlirDiagnostic diagnostic, intptr_t pos) {
  return wrap(*std::next(unwrap(diagnostic).getNotes().begin(), pos));
}

static void deleteUserDataNoop(void *userData) {}

MlirDiagnosticHandlerID mlirContextAttachDiagnosticHandler(
    MlirContext context, MlirDiagnosticHandler handler, void *userData,
    void (*deleteUserData)(void *)) {
  assert(handler && "unexpected null diagnostic handler");
  if (deleteUserData == nullptr)
    deleteUserData = deleteUserDataNoop;
  DiagnosticEngine::HandlerID id =
      unwrap(context)->getDiagEngine().registerHandler(
          [handler,
           ownedUserData = std::unique_ptr<void, decltype(deleteUserData)>(
               userData, deleteUserData)](Diagnostic &diagnostic) {
            return unwrap(handler(wrap(diagnostic), ownedUserData.get()));
          });
  return static_cast<MlirDiagnosticHandlerID>(id);
}

void mlirContextDetachDiagnosticHandler(MlirContext context,
                                        MlirDiagnosticHandlerID id) {
  unwrap(context)->getDiagEngine().eraseHandler(
      static_cast<DiagnosticEngine::HandlerID>(id));
}

void mlirEmitError(MlirLocation location, const char *message) {
  emitError(unwrap(location)) << message;
}
