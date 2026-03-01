//===-- CompilerDeclContext.cpp -------------------------------------------===//
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
//===----------------------------------------------------------------------===//

#include "lldb/Symbol/CompilerDeclContext.h"
#include "lldb/Symbol/CompilerDecl.h"
#include "lldb/Symbol/TypeSystem.h"
#include <vector>

using namespace lldb_private;

std::vector<CompilerDecl>
CompilerDeclContext::FindDeclByName(ConstString name,
                                    const bool ignore_using_decls) {
  if (IsValid())
    return m_type_system->DeclContextFindDeclByName(m_opaque_decl_ctx, name,
                                                    ignore_using_decls);
  return std::vector<CompilerDecl>();
}

ConstString CompilerDeclContext::GetName() const {
  if (IsValid())
    return m_type_system->DeclContextGetName(m_opaque_decl_ctx);
  return ConstString();
}

ConstString CompilerDeclContext::GetScopeQualifiedName() const {
  if (IsValid())
    return m_type_system->DeclContextGetScopeQualifiedName(m_opaque_decl_ctx);
  return ConstString();
}

bool CompilerDeclContext::IsClassMethod() {
  if (IsValid())
    return m_type_system->DeclContextIsClassMethod(m_opaque_decl_ctx);
  return false;
}

lldb::LanguageType CompilerDeclContext::GetLanguage() {
  if (IsValid())
    return m_type_system->DeclContextGetLanguage(m_opaque_decl_ctx);
  return {};
}

bool CompilerDeclContext::IsContainedInLookup(CompilerDeclContext other) const {
  if (!IsValid())
    return false;

  // If the other context is just the current context, we don't need to go
  // over the type system to know that the lookup is identical.
  if (this == &other)
    return true;

  return m_type_system->DeclContextIsContainedInLookup(m_opaque_decl_ctx,
                                                       other.m_opaque_decl_ctx);
}

std::vector<lldb_private::CompilerContext>
CompilerDeclContext::GetCompilerContext() const {
  if (IsValid())
    return m_type_system->DeclContextGetCompilerContext(m_opaque_decl_ctx);
  return {};
}

bool lldb_private::operator==(const lldb_private::CompilerDeclContext &lhs,
                              const lldb_private::CompilerDeclContext &rhs) {
  return lhs.GetTypeSystem() == rhs.GetTypeSystem() &&
         lhs.GetOpaqueDeclContext() == rhs.GetOpaqueDeclContext();
}

bool lldb_private::operator!=(const lldb_private::CompilerDeclContext &lhs,
                              const lldb_private::CompilerDeclContext &rhs) {
  return lhs.GetTypeSystem() != rhs.GetTypeSystem() ||
         lhs.GetOpaqueDeclContext() != rhs.GetOpaqueDeclContext();
}
