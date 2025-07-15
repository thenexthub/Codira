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

#ifndef LANGUAGE_SYMBOLGRAPHGEN_JSON_H
#define LANGUAGE_SYMBOLGRAPHGEN_JSON_H

#include "toolchain/TargetParser/Triple.h"
#include "toolchain/Support/JSON.h"
#include "toolchain/Support/VersionTuple.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/SubstitutionMap.h"
#include "language/AST/Type.h"

namespace language {
namespace symbolgraphgen {

struct AttributeRAII {
  StringRef Key;
  toolchain::json::OStream &OS;
  AttributeRAII(StringRef Key, toolchain::json::OStream &OS)
  : Key(Key), OS(OS) {
    OS.attributeBegin(Key);
  }

  ~AttributeRAII() {
    OS.attributeEnd();
  }
};

void serialize(const toolchain::VersionTuple &VT, toolchain::json::OStream &OS);
void serialize(const toolchain::Triple &T, toolchain::json::OStream &OS);
void serialize(const ExtensionDecl *Extension, toolchain::json::OStream &OS);
void serialize(const Requirement &Req, toolchain::json::OStream &OS);
void serialize(const language::GenericTypeParamType *Param, toolchain::json::OStream &OS);
void serialize(const ModuleDecl &M, toolchain::json::OStream &OS, toolchain::Triple Target);

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

#endif // LANGUAGE_SYMBOLGRAPHGEN_JSON_H
