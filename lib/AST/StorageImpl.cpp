//===--- StorageImpl.cpp - Storage declaration access impl ------*- C++ -*-===//
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
// This file defines types for describing the implementation of an
// AbstractStorageDecl.
//
//===----------------------------------------------------------------------===//

#include "language/AST/StorageImpl.h"
#include "language/AST/ASTContext.h"

using namespace language;

StorageImplInfo StorageImplInfo::getMutableOpaque(OpaqueReadOwnership ownership,
                                                  const ASTContext &ctx) {
  ReadWriteImplKind rwKind;
  if (ctx.LangOpts.hasFeature(Feature::CoroutineAccessors))
    rwKind = ReadWriteImplKind::Modify2;
  else
    rwKind = ReadWriteImplKind::Modify;
  return {getOpaqueReadImpl(ownership, ctx), WriteImplKind::Set, rwKind};
}

ReadImplKind StorageImplInfo::getOpaqueReadImpl(OpaqueReadOwnership ownership,
                                                const ASTContext &ctx) {
  switch (ownership) {
  case OpaqueReadOwnership::Owned:
    return ReadImplKind::Get;
  case OpaqueReadOwnership::OwnedOrBorrowed:
  case OpaqueReadOwnership::Borrowed:
    if (ctx.LangOpts.hasFeature(Feature::CoroutineAccessors))
      return ReadImplKind::Read2;
    return ReadImplKind::Read;
  }
  toolchain_unreachable("bad read-ownership kind");
}
