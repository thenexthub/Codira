//===-- CompilerDecl.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SYMBOL_COMPILERDECL_H
#define LLDB_SYMBOL_COMPILERDECL_H

#include "lldb/Symbol/CompilerType.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

/// Represents a generic declaration such as a function declaration.
///
/// This class serves as an abstraction for a declaration inside one of the
/// TypeSystems implemented by the language plugins. It does not have any actual
/// logic in it but only stores an opaque pointer and a pointer to the
/// TypeSystem that gives meaning to this opaque pointer. All methods of this
/// class should call their respective method in the TypeSystem interface and
/// pass the opaque pointer along.
///
/// \see lldb_private::TypeSystem
class CompilerDecl {
public:
  // Constructors and Destructors
  CompilerDecl() = default;

  /// Creates a CompilerDecl with the given TypeSystem and opaque pointer.
  ///
  /// This constructor should only be called from the respective TypeSystem
  /// implementation.
  CompilerDecl(TypeSystem *type_system, void *decl)
      : m_type_system(type_system), m_opaque_decl(decl) {}

  // Tests

  explicit operator bool() const { return IsValid(); }

  bool operator<(const CompilerDecl &rhs) const {
    if (m_type_system == rhs.m_type_system)
      return m_opaque_decl < rhs.m_opaque_decl;
    return m_type_system < rhs.m_type_system;
  }

  bool IsValid() const {
    return m_type_system != nullptr && m_opaque_decl != nullptr;
  }

  // Accessors

  TypeSystem *GetTypeSystem() const { return m_type_system; }

  void *GetOpaqueDecl() const { return m_opaque_decl; }

  void SetDecl(TypeSystem *type_system, void *decl) {
    m_type_system = type_system;
    m_opaque_decl = decl;
  }

  void Clear() {
    m_type_system = nullptr;
    m_opaque_decl = nullptr;
  }

  ConstString GetName() const;

  ConstString GetMangledName() const;

  CompilerDeclContext GetDeclContext() const;

  // If this decl has a type, return it.
  CompilerType GetType() const;

  // If this decl represents a function, return the return type
  CompilerType GetFunctionReturnType() const;

  // If this decl represents a function, return the number of arguments for the
  // function
  size_t GetNumFunctionArguments() const;

  // If this decl represents a function, return the argument type given a zero
  // based argument index
  CompilerType GetFunctionArgumentType(size_t arg_idx) const;

  /// Populate a valid compiler context from the current declaration.
  ///
  /// \returns A valid vector of CompilerContext entries that describes
  /// this declaration. The first entry in the vector is the parent of
  /// the subsequent entry, so the topmost entry is the global namespace.
  std::vector<lldb_private::CompilerContext> GetCompilerContext() const;

  // If decl represents a constant value, return it. Otherwise, return an
  // invalid/empty Scalar.
  Scalar GetConstantValue() const;

private:
  TypeSystem *m_type_system = nullptr;
  void *m_opaque_decl = nullptr;
};

bool operator==(const CompilerDecl &lhs, const CompilerDecl &rhs);
bool operator!=(const CompilerDecl &lhs, const CompilerDecl &rhs);

} // namespace lldb_private

#endif // LLDB_SYMBOL_COMPILERDECL_H
