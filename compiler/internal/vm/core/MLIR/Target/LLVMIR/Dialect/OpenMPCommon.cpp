//===- OpenMPCommon.cpp - Utils for translating MLIR dialect to LLVM IR----===//
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
// This file defines general utilities for MLIR Dialect translations to LLVM IR.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVMIR/Dialect/OpenMPCommon.h"

toolchain::Constant *
mlir::LLVM::createSourceLocStrFromLocation(Location loc,
                                           toolchain::OpenMPIRBuilder &builder,
                                           StringRef name, uint32_t &strLen) {
  if (auto fileLoc = dyn_cast<FileLineColLoc>(loc)) {
    StringRef fileName = fileLoc.getFilename();
    unsigned lineNo = fileLoc.getLine();
    unsigned colNo = fileLoc.getColumn();
    return builder.getOrCreateSrcLocStr(name, fileName, lineNo, colNo, strLen);
  }
  std::string locStr;
  toolchain::raw_string_ostream locOS(locStr);
  locOS << loc;
  return builder.getOrCreateSrcLocStr(locStr, strLen);
}

toolchain::Constant *
mlir::LLVM::createMappingInformation(Location loc,
                                     toolchain::OpenMPIRBuilder &builder) {
  uint32_t strLen;
  if (auto nameLoc = dyn_cast<NameLoc>(loc)) {
    StringRef name = nameLoc.getName();
    return createSourceLocStrFromLocation(nameLoc.getChildLoc(), builder, name,
                                          strLen);
  }
  return createSourceLocStrFromLocation(loc, builder, "unknown", strLen);
}
