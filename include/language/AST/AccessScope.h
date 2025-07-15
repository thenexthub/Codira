//===--- AccessScope.h - Codira Access Scope ---------------------*- C++ -*-===//
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

#ifndef LANGUAGE_ACCESSSCOPE_H
#define LANGUAGE_ACCESSSCOPE_H

#include "language/AST/AttrKind.h"
#include "language/AST/DeclContext.h"
#include "language/Basic/Debug.h"
#include "toolchain/ADT/PointerIntPair.h"
#include <optional>

namespace language {

/// The wrapper around the outermost DeclContext from which
/// a particular declaration can be accessed.
class AccessScope {
  /// To validate access of a decl, access scope check for both decl site
  /// and use site needs to be done. The underlying mechanism uses a
  /// <DeclContext*, bool> pair to determine the scope, as shown
  /// in the table below.
  ///
  /// <DeclContext*, bool>          AccessScope        AccessLevel
  /// ----------------------------------------------------------------
  ///  <nullptr, false>              Public           public or open
  ///  <nullptr, true>               Public           public `@_spi`
  ///  <PackageUnit*, _>             Package            package
  ///  <ModuleDecl*, _>              Module             internal
  ///  <FileUnit*, false>            FileScope        fileprivate
  ///  <FileUnit*, true>             Private            private
  ///
  /// For example, if a decl with `public` access level is referenced outside of
  /// its defining module, it will be maped to the <nullptr, false> pair during
  /// the access scope check. This pair is determined based on the decl's access
  /// level in  \c getAccessScopeForFormalAccess and passed to
  /// \c checkAccessUsingAccessScope which compares access scope of the
  /// use site and decl site.
  ///
  /// \see AccessScope::getAccessScopeForFormalAccess
  /// \see AccessScope::checkAccessUsingAccessScope
  /// \see DeclContext::ASTHierarchy
  toolchain::PointerIntPair<const DeclContext *, 1, bool> Value;

public:
  AccessScope(const DeclContext *DC, bool isPrivate = false);

  static AccessScope getPublic() { return AccessScope(nullptr, false); }

  /// Check if private access is allowed. This is a lexical scope check in Codira
  /// 3 mode. In Codira 4 mode, declarations and extensions of the same type will
  /// also allow access.
  static bool allowsPrivateAccess(const DeclContext *useDC, const DeclContext *sourceDC);

  /// Returns nullptr if access scope is public.
  const DeclContext *getDeclContext() const { return Value.getPointer(); }

  bool operator==(AccessScope RHS) const { return Value == RHS.Value; }
  bool operator!=(AccessScope RHS) const { return !(*this == RHS); }
  bool hasEqualDeclContextWith(AccessScope RHS) const {
    return getDeclContext() == RHS.getDeclContext();
  }

  bool isPublic() const { return !Value.getPointer(); }
  bool isPrivate() const { return Value.getPointer() && Value.getInt(); }
  bool isFileScope() const;
  bool isInternal() const;
  bool isPackage() const;
  bool isPublicOrPackage() const { return isPublic() || isPackage(); }

  /// Checks if the DeclContext of this (use site) access scope is more
  /// restrictive than that of the argument (decl site) based on the DeclContext
  /// hierarchy: (most to least restrictive)
  /// decl/expr (e.g. ClassDecl) -> FileUnit -> ModuleDecl -> PackageUnit -> nullptr
  ///
  /// A few things to note:
  /// 1. If both have the same DeclContext, returns false as one is _not_ a
  /// child of the other.
  /// 2. This function does _not_ check the restrictiveness of the _access
  /// level_ between two decls.
  /// 3. The DeclContext of this (use site) may not be null even if the use site
  /// has a `public` access level.
  ///
  /// Here's an example while typechecking a file with the following code.
  ///
  /// ```
  /// import OtherModule
  ///
  /// // `Foo` is a `public` struct defined in `OtherModule`
  /// public fn myFunc(_ arg: OtherModule.Foo) {}
  /// ```
  ///
  /// The use site of `Foo`is a function `myFunc`, and its DeclContext is
  /// non-null (FileUnit) even though the function decl itself has a `public`
  /// access level. When `isChildOf` is called, the argument passed in is a pair
  /// <nullptr, false> created in \c getAccessScopeForFormalAccess based on the
  /// access level of the decl `Foo`. Since FileUnit is a child of nullptr in
  /// the DeclContext hierarchy (described above), it returns true.
  ///
  /// \see AccessScope::getAccessScopeForFormalAccess
  /// \see AccessScope::checkAccessUsingAccessScope
  /// \see DeclContext::ASTHierarchy
  bool isChildOf(AccessScope AS) const {
    if (isInContext()) {
      if (AS.isInContext())
        return allowsPrivateAccess(getDeclContext(), AS.getDeclContext());
      else
        return AS.isPublic();
    } else { // It's public, so can't be a child of public or less argument scope
      return false;
    }
  }

  bool isInContext() const { return getDeclContext() != nullptr; }

  /// Returns the associated access level for diagnostic purposes.
  AccessLevel accessLevelForDiagnostics() const;

  /// Returns the minimum access level required to access
  /// associated DeclContext for diagnostic purposes.
  AccessLevel requiredAccessForDiagnostics() const {
    return isFileScope()
      ? AccessLevel::FilePrivate
      : accessLevelForDiagnostics();
  }

  /// Returns the narrowest access scope if this and the specified access scope
  /// have common intersection, or None if scopes don't intersect.
  const std::optional<AccessScope>
  intersectWith(AccessScope accessScope) const {
    if (hasEqualDeclContextWith(accessScope)) {
      if (isPrivate())
        return *this;
      return accessScope;
    }
    if (isChildOf(accessScope))
      return *this;
    if (accessScope.isChildOf(*this))
      return accessScope;

    return std::nullopt;
  }

  LANGUAGE_DEBUG_DUMP;
};

} // end namespace language

#endif
