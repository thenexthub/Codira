//===-- ClangUtil.cpp -----------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// A collection of helper methods and data structures for manipulating clang
// types and decls.
//===----------------------------------------------------------------------===//

#include "Plugins/ExpressionParser/Clang/ClangUtil.h"
#include "Plugins/TypeSystem/Clang/TypeSystemClang.h"

using namespace clang;
using namespace lldb_private;

bool ClangUtil::IsClangType(const CompilerType &ct) {
  // Invalid types are never Clang types.
  if (!ct)
    return false;

  if (!ct.GetTypeSystem<TypeSystemClang>())
    return false;

  if (!ct.GetOpaqueQualType())
    return false;

  return true;
}

clang::Decl *ClangUtil::GetDecl(const CompilerDecl &decl) {
  assert(llvm::isa<TypeSystemClang>(decl.GetTypeSystem()));
  return static_cast<clang::Decl *>(decl.GetOpaqueDecl());
}

QualType ClangUtil::GetQualType(const CompilerType &ct) {
  // Make sure we have a clang type before making a clang::QualType
  if (!IsClangType(ct))
    return QualType();

  return QualType::getFromOpaquePtr(ct.GetOpaqueQualType());
}

QualType ClangUtil::GetCanonicalQualType(const CompilerType &ct) {
  if (!IsClangType(ct))
    return QualType();

  return GetQualType(ct).getCanonicalType();
}

CompilerType ClangUtil::RemoveFastQualifiers(const CompilerType &ct) {
  if (!IsClangType(ct))
    return ct;

  QualType qual_type(GetQualType(ct));
  qual_type.removeLocalFastQualifiers();
  return CompilerType(ct.GetTypeSystem(), qual_type.getAsOpaquePtr());
}

clang::TagDecl *ClangUtil::GetAsTagDecl(const CompilerType &type) {
  clang::QualType qual_type = ClangUtil::GetCanonicalQualType(type);
  if (qual_type.isNull())
    return nullptr;

  return qual_type->getAsTagDecl();
}

std::string ClangUtil::DumpDecl(const clang::Decl *d) {
  if (!d)
    return "nullptr";

  std::string result;
  llvm::raw_string_ostream stream(result);
  bool deserialize = false;
  d->dump(stream, deserialize);

  return result;
}

std::string ClangUtil::ToString(const clang::Type *t) {
  return clang::QualType(t, 0).getAsString();
}

std::string ClangUtil::ToString(const CompilerType &c) {
  return ClangUtil::GetQualType(c).getAsString();
}
