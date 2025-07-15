//===--- InitializeDistActorIdentity.h - dist actor ID init for SILGen ----===//
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

#include "Cleanup.h"

namespace language {

namespace Lowering {

/// A clean-up designed to emit an initialization of a distributed actor's
/// identity upon successful initialization of the actor's system.
struct InitializeDistActorIdentity : Cleanup {
private:
  ConstructorDecl *ctor;
  ManagedValue actorSelf;
  VarDecl *systemVar;
public:
  InitializeDistActorIdentity(ConstructorDecl *ctor, ManagedValue actorSelf);

  void emit(SILGenFunction &SGF, CleanupLocation loc,
            ForUnwind_t forUnwind) override;

  void dump(SILGenFunction &) const override;

  VarDecl* getSystemVar() const { return systemVar; }
};

} // end Lowering namespace
} // end language namespace
