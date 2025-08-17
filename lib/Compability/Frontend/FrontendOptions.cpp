//===- FrontendOptions.cpp ------------------------------------------------===//
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

#include "language/Compability/Frontend/FrontendOptions.h"

using namespace language::Compability::frontend;

bool language::Compability::frontend::isFixedFormSuffix(toolchain::StringRef suffix) {
  // Note: Keep this list in-sync with flang/test/lit.cfg.py
  return suffix == "f77" || suffix == "f" || suffix == "F" || suffix == "ff" ||
         suffix == "for" || suffix == "FOR" || suffix == "fpp" ||
         suffix == "FPP";
}

bool language::Compability::frontend::isFreeFormSuffix(toolchain::StringRef suffix) {
  // Note: Keep this list in-sync with flang/test/lit.cfg.py
  return suffix == "f90" || suffix == "F90" || suffix == "ff90" ||
         suffix == "f95" || suffix == "F95" || suffix == "ff95" ||
         suffix == "f03" || suffix == "F03" || suffix == "f08" ||
         suffix == "F08" || suffix == "f18" || suffix == "F18" ||
         suffix == "cuf" || suffix == "CUF";
}

bool language::Compability::frontend::isToBePreprocessed(toolchain::StringRef suffix) {
  return suffix == "F" || suffix == "FOR" || suffix == "fpp" ||
         suffix == "FPP" || suffix == "F90" || suffix == "F95" ||
         suffix == "F03" || suffix == "F08" || suffix == "F18" ||
         suffix == "CUF";
}

bool language::Compability::frontend::isCUDAFortranSuffix(toolchain::StringRef suffix) {
  return suffix == "cuf" || suffix == "CUF";
}

InputKind FrontendOptions::getInputKindForExtension(toolchain::StringRef extension) {
  if (isFixedFormSuffix(extension) || isFreeFormSuffix(extension)) {
    return Language::Fortran;
  }

  if (extension == "bc" || extension == "ll")
    return Language::LLVM_IR;
  if (extension == "fir" || extension == "mlir")
    return Language::MLIR;

  return Language::Unknown;
}
