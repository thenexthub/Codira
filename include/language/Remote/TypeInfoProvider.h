//===--- TypeInfoProvider.h - Abstract access to type info ------*- C++ -*-===//
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
//
//  This file declares an abstract interface for reading type layout info.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_REMOTE_TYPEINFOPROVIDER_H
#define LANGUAGE_REMOTE_TYPEINFOPROVIDER_H

#include <stdint.h>
namespace language {
namespace reflection {
class TypeInfo;
}
namespace remote {

/// An abstract interface for providing external type layout information.
struct TypeInfoProvider {
  using IdType = void *;

  virtual ~TypeInfoProvider() = default;

  /// Attempt to read type information about (Clang)imported types that are not
  /// represented in the metadata. LLDB can read this information from debug
  /// info, for example.
  virtual const reflection::TypeInfo *
  getTypeInfo(toolchain::StringRef mangledName) = 0;

  /// A key that can be used to identify the type info provider (for example, 
  /// for caching purposes).
  virtual IdType getId() {
    // Default implementation is the instance's ID.
    return (void *) this;
  }
};

} // namespace remote
} // namespace language
#endif
