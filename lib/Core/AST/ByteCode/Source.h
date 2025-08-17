//===--- Source.h - Source location provider for the VM  --------*- C++ -*-===//
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
// Defines a program which organises and links multiple bytecode functions.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_INTERP_SOURCE_H
#define LANGUAGE_CORE_AST_INTERP_SOURCE_H

#include "PrimType.h"
#include "language/Core/AST/DeclBase.h"
#include "language/Core/AST/Stmt.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/Support/Endian.h"

namespace language::Core {
class Expr;
class SourceLocation;
class SourceRange;
namespace interp {
class Function;

/// Pointer into the code segment.
class CodePtr final {
public:
  CodePtr() = default;

  CodePtr &operator+=(int32_t Offset) {
    Ptr += Offset;
    return *this;
  }

  int32_t operator-(const CodePtr &RHS) const {
    assert(Ptr != nullptr && RHS.Ptr != nullptr && "Invalid code pointer");
    return Ptr - RHS.Ptr;
  }

  CodePtr operator-(size_t RHS) const {
    assert(Ptr != nullptr && "Invalid code pointer");
    return CodePtr(Ptr - RHS);
  }

  bool operator!=(const CodePtr &RHS) const { return Ptr != RHS.Ptr; }
  const std::byte *operator*() const { return Ptr; }
  explicit operator bool() const { return Ptr; }
  bool operator<=(const CodePtr &RHS) const { return Ptr <= RHS.Ptr; }
  bool operator>=(const CodePtr &RHS) const { return Ptr >= RHS.Ptr; }

  /// Reads data and advances the pointer.
  template <typename T> std::enable_if_t<!std::is_pointer<T>::value, T> read() {
    assert(aligned(Ptr));
    using namespace toolchain::support;
    T Value = endian::read<T, toolchain::endianness::native>(Ptr);
    Ptr += align(sizeof(T));
    return Value;
  }

private:
  friend class Function;
  /// Constructor used by Function to generate pointers.
  CodePtr(const std::byte *Ptr) : Ptr(Ptr) {}
  /// Pointer into the code owned by a function.
  const std::byte *Ptr = nullptr;
};

/// Describes the statement/declaration an opcode was generated from.
class SourceInfo final {
public:
  SourceInfo() {}
  SourceInfo(const Stmt *E) : Source(E) {}
  SourceInfo(const Decl *D) : Source(D) {}

  SourceLocation getLoc() const;
  SourceRange getRange() const;

  const Stmt *asStmt() const {
    return dyn_cast_if_present<const Stmt *>(Source);
  }
  const Decl *asDecl() const {
    return dyn_cast_if_present<const Decl *>(Source);
  }
  const Expr *asExpr() const;

  operator bool() const { return !Source.isNull(); }

private:
  toolchain::PointerUnion<const Decl *, const Stmt *> Source;
};

using SourceMap = std::vector<std::pair<unsigned, SourceInfo>>;

/// Interface for classes which map locations to sources.
class SourceMapper {
public:
  virtual ~SourceMapper() {}

  /// Returns source information for a given PC in a function.
  virtual SourceInfo getSource(const Function *F, CodePtr PC) const = 0;

  /// Returns the expression if an opcode belongs to one, null otherwise.
  const Expr *getExpr(const Function *F, CodePtr PC) const;
  /// Returns the location from which an opcode originates.
  SourceLocation getLocation(const Function *F, CodePtr PC) const;
  SourceRange getRange(const Function *F, CodePtr PC) const;
};

} // namespace interp
} // namespace language::Core

#endif
