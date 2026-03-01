//===-- CompilerDecl.cpp --------------------------------------------------===//
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

#include "lldb/Symbol/CompilerDecl.h"
#include "lldb/Symbol/CompilerDeclContext.h"
#include "lldb/Symbol/TypeSystem.h"
#include "lldb/Utility/Scalar.h"

using namespace lldb_private;

ConstString CompilerDecl::GetName() const {
  return m_type_system->DeclGetName(m_opaque_decl);
}

ConstString CompilerDecl::GetMangledName() const {
  return m_type_system->DeclGetMangledName(m_opaque_decl);
}

CompilerDeclContext CompilerDecl::GetDeclContext() const {
  return m_type_system->DeclGetDeclContext(m_opaque_decl);
}

CompilerType CompilerDecl::GetType() const {
  return m_type_system->GetTypeForDecl(m_opaque_decl);
}

CompilerType CompilerDecl::GetFunctionReturnType() const {
  return m_type_system->DeclGetFunctionReturnType(m_opaque_decl);
}

size_t CompilerDecl::GetNumFunctionArguments() const {
  return m_type_system->DeclGetFunctionNumArguments(m_opaque_decl);
}

CompilerType CompilerDecl::GetFunctionArgumentType(size_t arg_idx) const {
  return m_type_system->DeclGetFunctionArgumentType(m_opaque_decl, arg_idx);
}

bool lldb_private::operator==(const lldb_private::CompilerDecl &lhs,
                              const lldb_private::CompilerDecl &rhs) {
  return lhs.GetTypeSystem() == rhs.GetTypeSystem() &&
         lhs.GetOpaqueDecl() == rhs.GetOpaqueDecl();
}

bool lldb_private::operator!=(const lldb_private::CompilerDecl &lhs,
                              const lldb_private::CompilerDecl &rhs) {
  return lhs.GetTypeSystem() != rhs.GetTypeSystem() ||
         lhs.GetOpaqueDecl() != rhs.GetOpaqueDecl();
}

std::vector<lldb_private::CompilerContext>
CompilerDecl::GetCompilerContext() const {
  return m_type_system->DeclGetCompilerContext(m_opaque_decl);
}

Scalar CompilerDecl::GetConstantValue() const {
  return m_type_system->DeclGetConstantValue(m_opaque_decl);
}
