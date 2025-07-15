//===--- SpecializedEmitter.h - Special emitters for builtin ----*- C++ -*-===//
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
// Interface to the code for specially emitting builtin functions.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_LOWERING_SPECIALIZEDEMITTER_H
#define LANGUAGE_LOWERING_SPECIALIZEDEMITTER_H

#include "language/Basic/Toolchain.h"
#include "language/AST/Identifier.h"
#include "language/AST/Types.h"
#include "language/Basic/Assertions.h"

namespace language {
class Expr;
struct SILDeclRef;
class SILLocation;
class SILModule;
  
namespace Lowering {
class ManagedValue;
class SGFContext;
class SILGenFunction;
class SILGenModule;
class PreparedArguments;

/// Some kind of specialized emitter for a builtin function.
class SpecializedEmitter {
public:
  /// A special function for emitting a call before the arguments
  /// have already been emitted.
  using EarlyEmitter = ManagedValue (SILGenFunction &,
                                     SILLocation,
                                     SubstitutionMap,
                                     PreparedArguments &&args,
                                     SGFContext);

  /// A special function for emitting a call after the arguments
  /// have already been emitted.
  using LateEmitter = ManagedValue (SILGenFunction &,
                                    SILLocation,
                                    SubstitutionMap,
                                    ArrayRef<ManagedValue>,
                                    SGFContext);

  enum class Kind {
    /// This is a builtin function that will be specially handled
    /// downstream, but doesn't require special treatment at the
    /// SILGen level.
    NamedBuiltin,

    /// This is a builtin function that needs to be specially
    /// handled in SILGen and which needs to be given the original
    /// r-value expression.
    EarlyEmitter,

    /// This is a builtin function that needs to be specially
    /// handled in SILGen, but which can be passed normally-emitted
    /// arguments.
    LateEmitter,
  };

private:
  Kind TheKind;
  union {
    EarlyEmitter *TheEarlyEmitter;
    LateEmitter *TheLateEmitter;
    Identifier TheBuiltinName;
  };

public:
  /*implicit*/ SpecializedEmitter(Identifier builtinName)
    : TheKind(Kind::NamedBuiltin), TheBuiltinName(builtinName) {}

  /*implicit*/ SpecializedEmitter(EarlyEmitter *emitter)
    : TheKind(Kind::EarlyEmitter), TheEarlyEmitter(emitter) {}

  /*implicit*/ SpecializedEmitter(LateEmitter *emitter)
    : TheKind(Kind::LateEmitter), TheLateEmitter(emitter) {}

  /// Try to find an appropriate emitter for the given declaration.
  static std::optional<SpecializedEmitter> forDecl(SILGenModule &SGM,
                                                   SILDeclRef decl);

  bool isEarlyEmitter() const { return TheKind == Kind::EarlyEmitter; }
  EarlyEmitter *getEarlyEmitter() const {
    assert(isEarlyEmitter());
    return TheEarlyEmitter;
  }

  bool isLateEmitter() const { return TheKind == Kind::LateEmitter; }
  LateEmitter *getLateEmitter() const {
    assert(isLateEmitter());
    return TheLateEmitter;
  }

  bool isNamedBuiltin() const { return TheKind == Kind::NamedBuiltin; }
  Identifier getBuiltinName() const {
    assert(isNamedBuiltin());
    return TheBuiltinName;
  }
};

} // end namespace Lowering
} // end namespace language

#endif
