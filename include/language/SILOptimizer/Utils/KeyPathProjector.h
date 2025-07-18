//===-- KeyPathProjector.h - Project a static key path ----------*- C++ -*-===//
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
///
/// \file
/// Utility class to project a statically known key path
/// expression to a direct property access sequence.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_UTILS_KEYPATHPROJECTOR_H
#define LANGUAGE_SILOPTIMIZER_UTILS_KEYPATHPROJECTOR_H

#include "language/SIL/SILBuilder.h"
#include <memory>

namespace language {

class KeyPathInst;

/// Projects a statically known key path expression to
/// a direct property access.
class KeyPathProjector {
public:
  /// The type of a key path access.
  enum class AccessType {
    /// A get-only access (i.e. language_getAtKeyPath).
    Get,
    
    /// A set-only access (i.e. language_setAtWritableKeyPath).
    Set,
    
    /// A modification (i.e. language_modifyAtWritableKeyPath).
    Modify
  };
  
  /// Creates a key path projector for a key path.
  ///
  /// Returns nullptr if \p keyPath is not a keypath instruction or if there is
  /// any other reason why the optimization cannot be done.
  ///
  /// \param keyPath The key path to project. Must be the result of either
  ///    a keypath instruction or an upcast of a key path instruction.
  /// \param root The address of the object the key path is applied to.
  /// \param loc The location of the key path application.
  /// \param builder The SILBuilder to use.
  static std::unique_ptr<KeyPathProjector>
  create(SILValue keyPath, SILValue root, SILLocation loc, SILBuilder &builder);
  
  /// Extract the literal KeyPathInst underlying a value, or return null if there is none.
  static KeyPathInst *
  getLiteralKeyPath(SILValue keyPath);
  
  /// Projects the key path to an address. Sets up the projection,
  /// invokes the callback, then tears down the projection.
  /// \param accessType The access type of the projected address.
  /// \param callback A callback to invoke with the projected address.
  ///     The projected address is only valid from within \p callback.
  ///     If accessType is Get or Modify, the projected address is an
  ///     initialized address type. If accessType is set, the projected
  ///     address points to uninitialized memory.
  virtual void project(AccessType accessType,
               std::function<void(SILValue addr)> callback) = 0;
    
  virtual ~KeyPathProjector() {};
  
  /// Whether this projection returns a struct.
  virtual bool isStruct() = 0;
protected:
  KeyPathProjector(SILLocation loc, SILBuilder &builder)
      : loc(loc), builder(builder) {}
  
  /// The location of the key path application.
  SILLocation loc;
  
  /// The SILBuilder to use.
  SILBuilder &builder;
};

}  // end namespace language

#endif /* KeyPathProjector_h */
