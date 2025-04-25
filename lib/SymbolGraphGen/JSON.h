//===--- JSON.h - Symbol Graph JSON Helpers -------------------------------===//
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
// Adds Symbol Graph JSON serialization to other types.
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SYMBOLGRAPHGEN_JSON_H
#define SWIFT_SYMBOLGRAPHGEN_JSON_H

#include "llvm/TargetParser/Triple.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/VersionTuple.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/SubstitutionMap.h"
#include "language/AST/Type.h"

namespace language {
namespace symbolgraphgen {

struct AttributeRAII {
  StringRef Key;
  llvm::json::OStream &OS;
  AttributeRAII(StringRef Key, llvm::json::OStream &OS)
  : Key(Key), OS(OS) {
    OS.attributeBegin(Key);
  }

  ~AttributeRAII() {
    OS.attributeEnd();
  }
};

void serialize(const llvm::VersionTuple &VT, llvm::json::OStream &OS);
void serialize(const llvm::Triple &T, llvm::json::OStream &OS);
void serialize(const ExtensionDecl *Extension, llvm::json::OStream &OS);
void serialize(const Requirement &Req, llvm::json::OStream &OS);
void serialize(const swift::GenericTypeParamType *Param, llvm::json::OStream &OS);
void serialize(const ModuleDecl &M, llvm::json::OStream &OS, llvm::Triple Target);

void filterGenericParams(
    ArrayRef<GenericTypeParamType *> GenericParams,
    SmallVectorImpl<const GenericTypeParamType*> &FilteredParams,
    SubstitutionMap SubMap = {});

/// Filter generic requirements on an extension that aren't relevant
/// for documentation.
void filterGenericRequirements(
    ArrayRef<Requirement> Requirements, const ProtocolDecl *Self,
    SmallVectorImpl<Requirement> &FilteredRequirements,
    SubstitutionMap SubMap = {},
    ArrayRef<const GenericTypeParamType *> FilteredParams = {});

/// Filter generic requirements on an extension that aren't relevant
/// for documentation.
void
filterGenericRequirements(const ExtensionDecl *Extension,
                          SmallVectorImpl<Requirement> &FilteredRequirements);

} // end namespace symbolgraphgen
} // end namespace language

#endif // SWIFT_SYMBOLGRAPHGEN_JSON_H
