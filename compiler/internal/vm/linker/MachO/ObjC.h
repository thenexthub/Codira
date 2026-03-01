//===- ObjC.h ---------------------------------------------------*- C++ -*-===//
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

#ifndef LLD_MACHO_OBJC_H
#define LLD_MACHO_OBJC_H

#include "llvm/Support/MemoryBuffer.h"

namespace lld::macho {

namespace objc {

namespace symbol_names {
constexpr const char klass[] = "_OBJC_CLASS_$_";
constexpr const char klassPropList[] = "__OBJC_$_CLASS_PROP_LIST_";

constexpr const char metaclass[] = "_OBJC_METACLASS_$_";
constexpr const char ehtype[] = "_OBJC_EHTYPE_$_";
constexpr const char ivar[] = "_OBJC_IVAR_$_";
constexpr const char instanceMethods[] = "__OBJC_$_INSTANCE_METHODS_";
constexpr const char classMethods[] = "__OBJC_$_CLASS_METHODS_";
constexpr const char listProprieties[] = "__OBJC_$_PROP_LIST_";

constexpr const char category[] = "__OBJC_$_CATEGORY_";
constexpr const char categoryInstanceMethods[] =
    "__OBJC_$_CATEGORY_INSTANCE_METHODS_";
constexpr const char categoryClassMethods[] =
    "__OBJC_$_CATEGORY_CLASS_METHODS_";
constexpr const char categoryProtocols[] = "__OBJC_CATEGORY_PROTOCOLS_$_";

constexpr const char swift_objc_category[] = "__CATEGORY_";
constexpr const char swift_objc_klass[] = "_$s";
} // namespace symbol_names

// Check for duplicate method names within related categories / classes.
void checkCategories();
void mergeCategories();

void doCleanup();
} // namespace objc

bool hasObjCSection(llvm::MemoryBufferRef);

} // namespace lld::macho

#endif
