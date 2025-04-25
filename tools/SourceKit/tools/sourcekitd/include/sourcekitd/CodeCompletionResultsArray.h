//===--- CodeCompletionResultsArray.h - -------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SOURCEKITD_CODECOMPLETION_RESULTS_ARRAY_H
#define LLVM_SOURCEKITD_CODECOMPLETION_RESULTS_ARRAY_H

#include "sourcekitd/Internal.h"

namespace sourcekitd {

VariantFunctions *getVariantFunctionsForCodeCompletionResultsArray();

class CodeCompletionResultsArrayBuilder {
public:
  CodeCompletionResultsArrayBuilder();
  ~CodeCompletionResultsArrayBuilder();

  void add(SourceKit::UIdent Kind, llvm::StringRef Name,
           llvm::StringRef Description, llvm::StringRef SourceText,
           llvm::StringRef TypeName, std::optional<llvm::StringRef> ModuleName,
           std::optional<llvm::StringRef> DocBrief,
           std::optional<llvm::StringRef> AssocUSRs,
           SourceKit::UIdent SemanticContext, SourceKit::UIdent TypeRelation,
           bool NotRecommended, bool IsSystem, unsigned NumBytesToErase);

  std::unique_ptr<llvm::MemoryBuffer> createBuffer();

private:
  struct Implementation;
  Implementation &Impl;
};

}

#endif
