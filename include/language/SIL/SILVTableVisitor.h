//===--- SILVTableVisitor.h - Class vtable visitor --------------*- C++ -*-===//
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
// This file defines the SILVTableVisitor class, which is used to generate and
// perform lookups in class method vtables.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_SILVTABLEVISITOR_H
#define LANGUAGE_SIL_SILVTABLEVISITOR_H

#include "language/AST/Decl.h"
#include "language/AST/Types.h"

namespace language {

/// A CRTP class for visiting virtually-dispatched methods of a class.
///
/// You must override these two methods in your subclass:
///
/// - addMethod(SILDeclRef):
///   introduce a new vtable entry
///
/// - addMethodOverride(SILDeclRef baseRef, SILDeclRef derivedRef):
///   update vtable entry for baseRef to call derivedRef
///
/// - addPlaceholder(MissingMemberDecl *);
///   introduce an entry for a method that could not be deserialized
///
template <class T> class SILVTableVisitor {
  T &asDerived() { return *static_cast<T*>(this); }

  void maybeAddMethod(FuncDecl *fd) {
    assert(!fd->hasClangNode());

    SILDeclRef constant(fd, SILDeclRef::Kind::Func);
    maybeAddEntry(constant);

    maybeAddAutoDiffDerivativeMethods(constant);
  }

  void maybeAddConstructor(ConstructorDecl *cd) {
    assert(!cd->hasClangNode());

    // The allocating entry point is what is used for dynamic dispatch.
    // The initializing entry point for designated initializers is only
    // necessary for super.init chaining, which is sufficiently constrained
    // to never need dynamic dispatch.
    SILDeclRef constant(cd, SILDeclRef::Kind::Allocator);
    maybeAddEntry(constant);

    maybeAddAutoDiffDerivativeMethods(constant);
  }

  void maybeAddAccessors(AbstractStorageDecl *asd) {
    asd->visitOpaqueAccessors([&](AccessorDecl *accessor) {
      maybeAddMethod(accessor);
    });
  }

  void maybeAddEntry(SILDeclRef declRef) {
    // Introduce a new entry if required.
    if (declRef.requiresNewVTableEntry())
      asDerived().addMethod(declRef);

    // Update any existing entries that it overrides.
    auto nextRef = declRef;
    while ((nextRef = nextRef.getNextOverriddenVTableEntry())) {
      auto baseRef = nextRef.getOverriddenVTableEntry();

      // If A.f() is overridden by B.f() which is overridden by
      // C.f(), it's possible that C.f() is not visible from C.
      // In this case, we pretend that B.f() is the least derived
      // method with a vtable entry in the override chain.
      //
      // This works because we detect the possibility of this
      // happening when we emit B.f() and do two things:
      // - B.f() always gets a new vtable entry, even if it is
      //   ABI compatible with A.f()
      // - The vtable thunk for the override of A.f() in B does a
      //   vtable dispatch to the implementation of B.f() for the
      //   concrete subclass, so a subclass of B only needs to
      //   replace the vtable entry for B.f(); a call to A.f()
      //   will correctly dispatch to the implementation of B.f()
      //   in the subclass.
      auto *UseDC = declRef.getDecl()->getDeclContext();
      if (!baseRef.getDecl()->isAccessibleFrom(
            UseDC,
            /*forConformance=*/false,
            /*allowUsableFromInline=*/true))
        break;

      asDerived().addMethodOverride(baseRef, declRef);
      nextRef = baseRef;
    }
  }

  void maybeAddMember(Decl *member) {
    if (!member->isAvailableDuringLowering())
      return;

    if (isa<AccessorDecl>(member))
      /* handled as part of its storage */;
    else if (auto *fd = dyn_cast<FuncDecl>(member))
      maybeAddMethod(fd);
    else if (auto *cd = dyn_cast<ConstructorDecl>(member))
      maybeAddConstructor(cd);
    else if (auto *asd = dyn_cast<AbstractStorageDecl>(member))
      maybeAddAccessors(asd);
    else if (auto *placeholder = dyn_cast<MissingMemberDecl>(member))
      asDerived().addPlaceholder(placeholder);
  }

  void maybeAddAutoDiffDerivativeMethods(SILDeclRef constant) {
    auto *D = constant.getDecl();
    for (auto *diffAttr : D->getAttrs().getAttributes<DifferentiableAttr>()) {
      maybeAddEntry(constant.asAutoDiffDerivativeFunction(
          AutoDiffDerivativeFunctionIdentifier::get(
              AutoDiffDerivativeFunctionKind::JVP,
              diffAttr->getParameterIndices(),
              diffAttr->getDerivativeGenericSignature(),
              D->getASTContext())));
      maybeAddEntry(constant.asAutoDiffDerivativeFunction(
          AutoDiffDerivativeFunctionIdentifier::get(
              AutoDiffDerivativeFunctionKind::VJP,
              diffAttr->getParameterIndices(),
              diffAttr->getDerivativeGenericSignature(),
              D->getASTContext())));
    }
  }

protected:
  void addVTableEntries(ClassDecl *theClass) {
    // Imported classes do not have a vtable.
    if (!theClass->hasKnownCodiraImplementation())
      return;

    for (Decl *member : theClass->getABIMembers()) {
      maybeAddMember(member);
    }
  }
};

}

#endif
