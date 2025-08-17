//===- HLSLBufferLayoutBuilder.h ------------------------------------------===//
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

#include "toolchain/ADT/StringRef.h"
#include "toolchain/IR/DerivedTypes.h"

namespace language::Core {
class RecordType;
class FieldDecl;

namespace CodeGen {
class CodeGenModule;

//===----------------------------------------------------------------------===//
// Implementation of constant buffer layout common between DirectX and
// SPIR/SPIR-V.
//===----------------------------------------------------------------------===//

class HLSLBufferLayoutBuilder {
private:
  CodeGenModule &CGM;
  toolchain::StringRef LayoutTypeName;

public:
  HLSLBufferLayoutBuilder(CodeGenModule &CGM, toolchain::StringRef LayoutTypeName)
      : CGM(CGM), LayoutTypeName(LayoutTypeName) {}

  // Returns LLVM target extension type with the name LayoutTypeName
  // for given structure type and layout data. The first number in
  // the Layout is the size followed by offsets for each struct element.
  toolchain::TargetExtType *
  createLayoutType(const RecordType *StructType,
                   const toolchain::SmallVector<int32_t> *Packoffsets = nullptr);

private:
  bool layoutField(const language::Core::FieldDecl *FD, unsigned &EndOffset,
                   unsigned &FieldOffset, toolchain::Type *&FieldType,
                   int Packoffset = -1);
};

} // namespace CodeGen
} // namespace language::Core
