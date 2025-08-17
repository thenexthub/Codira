//==--- AbstractTypeWriter.h - Abstract serialization for types -----------===//
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

#ifndef LANGUAGE_CORE_AST_ABSTRACTTYPEWRITER_H
#define LANGUAGE_CORE_AST_ABSTRACTTYPEWRITER_H

#include "language/Core/AST/Type.h"
#include "language/Core/AST/AbstractBasicWriter.h"
#include "language/Core/AST/DeclObjC.h"

namespace language::Core {
namespace serialization {

// template <class PropertyWriter>
// class AbstractTypeWriter {
// public:
//   AbstractTypeWriter(PropertyWriter &W);
//   void write(QualType type);
// };
//
// The actual class is auto-generated; see ClangASTPropertiesEmitter.cpp.
#include "language/Core/AST/AbstractTypeWriter.inc"

} // end namespace serialization
} // end namespace language::Core

#endif
