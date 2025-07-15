//===--- SILBuiltinVisitor.h ------------------------------------*- C++ -*-===//
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
///
/// This file contains SILBuiltinVisitor, a visitor for visiting all possible
/// builtins and toolchain intrinsics able to be used by BuiltinInst.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_SILBUILTINVISITOR_H
#define LANGUAGE_SIL_SILBUILTINVISITOR_H

#include "language/SIL/SILInstruction.h"
#include <type_traits>

namespace language {

template <typename ImplClass, typename ValueRetTy = void>
class SILBuiltinVisitor {
public:
  ImplClass &asImpl() { return static_cast<ImplClass &>(*this); }

  /// Perform any required pre-processing before visiting.
  ///
  /// Sub-classes can override this method to provide custom pre-processing.
  void beforeVisit(BuiltinInst *BI) {}

  ValueRetTy visit(BuiltinInst *BI) {
    asImpl().beforeVisit(BI);

    if (auto BuiltinKind = BI->getBuiltinKind()) {
      switch (BuiltinKind.value()) {
      // BUILTIN_TYPE_CHECKER_OPERATION does not live past the type checker.
#define BUILTIN_TYPE_CHECKER_OPERATION(ID, NAME)                               \
  case BuiltinValueKind::ID:                                                   \
    toolchain_unreachable("Unexpected type checker operation seen in SIL!");

#define BUILTIN(ID, NAME, ATTRS)                                               \
  case BuiltinValueKind::ID:                                                   \
    return asImpl().visit##ID(BI, ATTRS);
#include "language/AST/Builtins.def"
      case BuiltinValueKind::None:
        toolchain_unreachable("None case");
      }
      toolchain_unreachable("Not all cases handled?!");
    }

    if (auto IntrinsicID = BI->getIntrinsicID()) {
      return asImpl().visitLLVMIntrinsic(BI, IntrinsicID.value());
    }
    toolchain_unreachable("Not all cases handled?!");
  }

  ValueRetTy visitLLVMIntrinsic(BuiltinInst *BI, toolchain::Intrinsic::ID ID) {
    return ValueRetTy();
  }

  ValueRetTy visitBuiltinValueKind(BuiltinInst *BI, BuiltinValueKind Kind,
                                   StringRef Attrs) {
    return ValueRetTy();
  }

#define BUILTIN(ID, NAME, ATTRS)                                               \
  ValueRetTy visit##ID(BuiltinInst *BI, StringRef) {                           \
    return asImpl().visitBuiltinValueKind(BI, BuiltinValueKind::ID, ATTRS);    \
  }
#include "language/AST/Builtins.def"
};

} // end language namespace

#endif
