//===-- DILEval.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_VALUEOBJECT_DILEVAL_H
#define LLDB_VALUEOBJECT_DILEVAL_H

#include "lldb/ValueObject/DILAST.h"
#include "lldb/ValueObject/DILParser.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include <memory>
#include <vector>

namespace lldb_private::dil {

/// Given the name of an identifier (variable name, member name, type name,
/// etc.), find the ValueObject for that name (if it exists), excluding global
/// variables, and create and return an IdentifierInfo object containing all
/// the relevant information about that object (for DIL parsing and
/// evaluating).
lldb::ValueObjectSP LookupIdentifier(llvm::StringRef name_ref,
                                     std::shared_ptr<StackFrame> frame_sp,
                                     lldb::DynamicValueType use_dynamic);

/// Given the name of an identifier, check to see if it matches the name of a
/// global variable. If so, find the ValueObject for that global variable, and
/// create and return an IdentifierInfo object containing all the relevant
/// informatin about it.
lldb::ValueObjectSP LookupGlobalIdentifier(llvm::StringRef name_ref,
                                           std::shared_ptr<StackFrame> frame_sp,
                                           lldb::TargetSP target_sp,
                                           lldb::DynamicValueType use_dynamic);

class Interpreter : Visitor {
public:
  Interpreter(lldb::TargetSP target, llvm::StringRef expr,
              std::shared_ptr<StackFrame> frame_sp,
              lldb::DynamicValueType use_dynamic, bool use_synthetic,
              bool fragile_ivar, bool check_ptr_vs_member);

  /// Evaluate an ASTNode.
  /// \returns A non-null lldb::ValueObjectSP or an Error.
  llvm::Expected<lldb::ValueObjectSP> Evaluate(const ASTNode &node);

private:
  /// Evaluate an ASTNode. If the result is a reference, it is also
  /// dereferenced using ValueObject::Dereference.
  /// \returns A non-null lldb::ValueObjectSP or an Error.
  llvm::Expected<lldb::ValueObjectSP>
  EvaluateAndDereference(const ASTNode &node);
  llvm::Expected<lldb::ValueObjectSP>
  Visit(const IdentifierNode &node) override;
  llvm::Expected<lldb::ValueObjectSP> Visit(const MemberOfNode &node) override;
  llvm::Expected<lldb::ValueObjectSP> Visit(const UnaryOpNode &node) override;
  llvm::Expected<lldb::ValueObjectSP>
  Visit(const ArraySubscriptNode &node) override;
  llvm::Expected<lldb::ValueObjectSP>
  Visit(const BitFieldExtractionNode &node) override;
  llvm::Expected<lldb::ValueObjectSP>
  Visit(const IntegerLiteralNode &node) override;
  llvm::Expected<lldb::ValueObjectSP>
  Visit(const FloatLiteralNode &node) override;
  llvm::Expected<lldb::ValueObjectSP>
  Visit(const BooleanLiteralNode &node) override;
  llvm::Expected<lldb::ValueObjectSP> Visit(const CastNode &node) override;

  /// Perform usual unary conversions on a value. At the moment this
  /// includes array-to-pointer and integral promotion for eligible types.
  llvm::Expected<lldb::ValueObjectSP>
  UnaryConversion(lldb::ValueObjectSP valobj, uint32_t location);
  llvm::Expected<CompilerType>
  PickIntegerType(lldb::TypeSystemSP type_system,
                  std::shared_ptr<ExecutionContextScope> ctx,
                  const IntegerLiteralNode &literal);

  /// A helper function for VerifyCastType (below). This performs
  /// arithmetic-specific checks. It should only be called if the target_type
  /// is a scalar type.
  llvm::Expected<CastKind> VerifyArithmeticCast(CompilerType source_type,
                                                CompilerType target_type,
                                                int location);

  /// As a preparation for type casting, compare the requested 'target' type
  /// of the cast with the type of the operand to be cast. If the cast is
  /// allowed, return the appropriate CastKind for the cast; otherwise return
  /// an error.
  llvm::Expected<CastKind> VerifyCastType(lldb::ValueObjectSP operand,
                                          CompilerType source_type,
                                          CompilerType target_type,
                                          int location);

  // Used by the interpreter to create objects, perform casts, etc.
  lldb::TargetSP m_target;
  llvm::StringRef m_expr;
  lldb::ValueObjectSP m_scope;
  std::shared_ptr<StackFrame> m_exe_ctx_scope;
  lldb::DynamicValueType m_use_dynamic;
  bool m_use_synthetic;
  bool m_fragile_ivar;
  bool m_check_ptr_vs_member;
};

} // namespace lldb_private::dil

#endif // LLDB_VALUEOBJECT_DILEVAL_H
