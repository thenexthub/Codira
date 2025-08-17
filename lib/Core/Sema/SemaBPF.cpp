//===------ SemaBPF.cpp ---------- BPF target-specific routines -----------===//
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
//
//  This file implements semantic analysis functions specific to BPF.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Sema/SemaBPF.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/Type.h"
#include "language/Core/Basic/DiagnosticSema.h"
#include "language/Core/Basic/TargetBuiltins.h"
#include "language/Core/Sema/ParsedAttr.h"
#include "language/Core/Sema/Sema.h"
#include "toolchain/ADT/APSInt.h"
#include <optional>

namespace language::Core {

SemaBPF::SemaBPF(Sema &S) : SemaBase(S) {}

static bool isValidPreserveFieldInfoArg(Expr *Arg) {
  if (Arg->getType()->getAsPlaceholderType())
    return false;

  // The first argument needs to be a record field access.
  // If it is an array element access, we delay decision
  // to BPF backend to check whether the access is a
  // field access or not.
  return (Arg->IgnoreParens()->getObjectKind() == OK_BitField ||
          isa<MemberExpr>(Arg->IgnoreParens()) ||
          isa<ArraySubscriptExpr>(Arg->IgnoreParens()));
}

static bool isValidPreserveTypeInfoArg(Expr *Arg) {
  QualType ArgType = Arg->getType();
  if (ArgType->getAsPlaceholderType())
    return false;

  // for TYPE_EXISTENCE/TYPE_MATCH/TYPE_SIZEOF reloc type
  // format:
  //   1. __builtin_preserve_type_info(*(<type> *)0, flag);
  //   2. <type> var;
  //      __builtin_preserve_type_info(var, flag);
  if (!isa<DeclRefExpr>(Arg->IgnoreParens()) &&
      !isa<UnaryOperator>(Arg->IgnoreParens()))
    return false;

  // Typedef type.
  if (ArgType->getAs<TypedefType>())
    return true;

  // Record type or Enum type.
  const Type *Ty = ArgType->getUnqualifiedDesugaredType();
  if (const auto *RT = Ty->getAs<RecordType>()) {
    if (!RT->getOriginalDecl()->getDeclName().isEmpty())
      return true;
  } else if (const auto *ET = Ty->getAs<EnumType>()) {
    if (!ET->getOriginalDecl()->getDeclName().isEmpty())
      return true;
  }

  return false;
}

static bool isValidPreserveEnumValueArg(Expr *Arg) {
  QualType ArgType = Arg->getType();
  if (ArgType->getAsPlaceholderType())
    return false;

  // for ENUM_VALUE_EXISTENCE/ENUM_VALUE reloc type
  // format:
  //   __builtin_preserve_enum_value(*(<enum_type> *)<enum_value>,
  //                                 flag);
  const auto *UO = dyn_cast<UnaryOperator>(Arg->IgnoreParens());
  if (!UO)
    return false;

  const auto *CE = dyn_cast<CStyleCastExpr>(UO->getSubExpr());
  if (!CE)
    return false;
  if (CE->getCastKind() != CK_IntegralToPointer &&
      CE->getCastKind() != CK_NullToPointer)
    return false;

  // The integer must be from an EnumConstantDecl.
  const auto *DR = dyn_cast<DeclRefExpr>(CE->getSubExpr());
  if (!DR)
    return false;

  const EnumConstantDecl *Enumerator =
      dyn_cast<EnumConstantDecl>(DR->getDecl());
  if (!Enumerator)
    return false;

  // The type must be EnumType.
  const Type *Ty = ArgType->getUnqualifiedDesugaredType();
  const auto *ET = Ty->getAs<EnumType>();
  if (!ET)
    return false;

  // The enum value must be supported.
  return toolchain::is_contained(ET->getOriginalDecl()->enumerators(), Enumerator);
}

bool SemaBPF::CheckBPFBuiltinFunctionCall(unsigned BuiltinID,
                                          CallExpr *TheCall) {
  assert((BuiltinID == BPF::BI__builtin_preserve_field_info ||
          BuiltinID == BPF::BI__builtin_btf_type_id ||
          BuiltinID == BPF::BI__builtin_preserve_type_info ||
          BuiltinID == BPF::BI__builtin_preserve_enum_value) &&
         "unexpected BPF builtin");
  ASTContext &Context = getASTContext();
  if (SemaRef.checkArgCount(TheCall, 2))
    return true;

  // The second argument needs to be a constant int
  Expr *Arg = TheCall->getArg(1);
  std::optional<toolchain::APSInt> Value = Arg->getIntegerConstantExpr(Context);
  diag::kind kind;
  if (!Value) {
    if (BuiltinID == BPF::BI__builtin_preserve_field_info)
      kind = diag::err_preserve_field_info_not_const;
    else if (BuiltinID == BPF::BI__builtin_btf_type_id)
      kind = diag::err_btf_type_id_not_const;
    else if (BuiltinID == BPF::BI__builtin_preserve_type_info)
      kind = diag::err_preserve_type_info_not_const;
    else
      kind = diag::err_preserve_enum_value_not_const;
    Diag(Arg->getBeginLoc(), kind) << 2 << Arg->getSourceRange();
    return true;
  }

  // The first argument
  Arg = TheCall->getArg(0);
  bool InvalidArg = false;
  bool ReturnUnsignedInt = true;
  if (BuiltinID == BPF::BI__builtin_preserve_field_info) {
    if (!isValidPreserveFieldInfoArg(Arg)) {
      InvalidArg = true;
      kind = diag::err_preserve_field_info_not_field;
    }
  } else if (BuiltinID == BPF::BI__builtin_preserve_type_info) {
    if (!isValidPreserveTypeInfoArg(Arg)) {
      InvalidArg = true;
      kind = diag::err_preserve_type_info_invalid;
    }
  } else if (BuiltinID == BPF::BI__builtin_preserve_enum_value) {
    if (!isValidPreserveEnumValueArg(Arg)) {
      InvalidArg = true;
      kind = diag::err_preserve_enum_value_invalid;
    }
    ReturnUnsignedInt = false;
  } else if (BuiltinID == BPF::BI__builtin_btf_type_id) {
    ReturnUnsignedInt = false;
  }

  if (InvalidArg) {
    Diag(Arg->getBeginLoc(), kind) << 1 << Arg->getSourceRange();
    return true;
  }

  if (ReturnUnsignedInt)
    TheCall->setType(Context.UnsignedIntTy);
  else
    TheCall->setType(Context.UnsignedLongTy);
  return false;
}

void SemaBPF::handlePreserveAIRecord(RecordDecl *RD) {
  // Add preserve_access_index attribute to all fields and inner records.
  for (auto *D : RD->decls()) {
    if (D->hasAttr<BPFPreserveAccessIndexAttr>())
      continue;

    D->addAttr(BPFPreserveAccessIndexAttr::CreateImplicit(getASTContext()));
    if (auto *Rec = dyn_cast<RecordDecl>(D))
      handlePreserveAIRecord(Rec);
  }
}

void SemaBPF::handlePreserveAccessIndexAttr(Decl *D, const ParsedAttr &AL) {
  auto *Rec = cast<RecordDecl>(D);
  handlePreserveAIRecord(Rec);
  Rec->addAttr(::new (getASTContext())
                   BPFPreserveAccessIndexAttr(getASTContext(), AL));
}

} // namespace language::Core
