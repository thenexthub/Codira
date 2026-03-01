//===- TypeID.cpp - MLIR TypeID -------------------------------------------===//
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

#include "mlir/Support/TypeID.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/RWMutex.h"

#include "vm/core/Support/Signals.h"
#include "vm/core/Support/raw_ostream.h"

using namespace mlir;

#define DEBUG_TYPE "typeid"

//===----------------------------------------------------------------------===//
// TypeID Registry
//===----------------------------------------------------------------------===//

namespace {
struct ImplicitTypeIDRegistry {
  /// Lookup or insert a TypeID for the given type name.
  TypeID lookupOrInsert(StringRef typeName) {
    // Perform a heuristic check to see if this type is in an anonymous
    // namespace. String equality is not valid for anonymous types, so we try to
    // abort whenever we see them.
#ifndef NDEBUG
#if defined(_MSC_VER)
    if (typeName.contains("anonymous-namespace")) {
#else
    if (typeName.contains("anonymous namespace")) {
#endif
      std::string errorStr;
      {
        toolchain::raw_string_ostream errorOS(errorStr);
        errorOS << "TypeID::get<" << typeName
                << ">(): Using TypeID on a class with an anonymous "
                   "namespace requires an explicit TypeID definition. The "
                   "implicit fallback uses string name, which does not "
                   "guarantee uniqueness in anonymous contexts. Define an "
                   "explicit TypeID instantiation for this type using "
                   "`MLIR_DECLARE_EXPLICIT_TYPE_ID`/"
                   "`MLIR_DEFINE_EXPLICIT_TYPE_ID` or "
                   "`MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID`.\n";
      }
      toolchain::report_fatal_error(toolchain::StringRef(errorStr));
    }
#endif

    { // Try a read-only lookup first.
      toolchain::sys::SmartScopedReader<true> guard(mutex);
      auto it = typeNameToID.find(typeName);
      if (it != typeNameToID.end())
        return it->second;
    }
    toolchain::sys::SmartScopedWriter<true> guard(mutex);
    auto it = typeNameToID.try_emplace(typeName, TypeID());
    if (it.second)
      it.first->second = typeIDAllocator.allocate();
    return it.first->second;
  }

  /// A mutex that guards access to the registry.
  toolchain::sys::SmartRWMutex<true> mutex;

  /// An allocator used for TypeID objects.
  TypeIDAllocator typeIDAllocator;

  /// A map type name to TypeID.
  DenseMap<StringRef, TypeID> typeNameToID;
};
} // end namespace

LLVM_ALWAYS_EXPORT TypeID
detail::FallbackTypeIDResolver::registerImplicitTypeID(StringRef name) {
  static ImplicitTypeIDRegistry registry;
  return registry.lookupOrInsert(name);
}

//===----------------------------------------------------------------------===//
// Builtin TypeIDs
//===----------------------------------------------------------------------===//

MLIR_DEFINE_EXPLICIT_SELF_OWNING_TYPE_ID(void)
