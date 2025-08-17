//===----- SemaWasm.h ------ Wasm target-specific routines ----*- C++ -*---===//
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
/// \file
/// This file declares semantic analysis functions specific to Wasm.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_SEMAWASM_H
#define LANGUAGE_CORE_SEMA_SEMAWASM_H

#include "language/Core/AST/ASTFwd.h"
#include "language/Core/Sema/SemaBase.h"

namespace language::Core {
class ParsedAttr;
class TargetInfo;

class SemaWasm : public SemaBase {
public:
  SemaWasm(Sema &S);

  bool CheckWebAssemblyBuiltinFunctionCall(const TargetInfo &TI,
                                           unsigned BuiltinID,
                                           CallExpr *TheCall);

  bool BuiltinWasmRefNullExtern(CallExpr *TheCall);
  bool BuiltinWasmRefIsNullExtern(CallExpr *TheCall);
  bool BuiltinWasmRefNullFunc(CallExpr *TheCall);
  bool BuiltinWasmTableGet(CallExpr *TheCall);
  bool BuiltinWasmTableSet(CallExpr *TheCall);
  bool BuiltinWasmTableSize(CallExpr *TheCall);
  bool BuiltinWasmTableGrow(CallExpr *TheCall);
  bool BuiltinWasmTableFill(CallExpr *TheCall);
  bool BuiltinWasmTableCopy(CallExpr *TheCall);
  bool BuiltinWasmTestFunctionPointerSignature(const TargetInfo &TI,
                                               CallExpr *TheCall);

  WebAssemblyImportNameAttr *
  mergeImportNameAttr(Decl *D, const WebAssemblyImportNameAttr &AL);
  WebAssemblyImportModuleAttr *
  mergeImportModuleAttr(Decl *D, const WebAssemblyImportModuleAttr &AL);

  void handleWebAssemblyExportNameAttr(Decl *D, const ParsedAttr &AL);
  void handleWebAssemblyImportModuleAttr(Decl *D, const ParsedAttr &AL);
  void handleWebAssemblyImportNameAttr(Decl *D, const ParsedAttr &AL);
};
} // namespace language::Core

#endif // LANGUAGE_CORE_SEMA_SEMAWASM_H
