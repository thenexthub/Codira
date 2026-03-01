//===-- ClangExpressionVariable.cpp ---------------------------------------===//
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

#include "ClangExpressionVariable.h"

#include "lldb/Core/Value.h"
#include "lldb/Target/ExecutionContext.h"
#include "lldb/Target/Process.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/Utility/DataExtractor.h"
#include "lldb/Utility/Stream.h"
#include "lldb/ValueObject/ValueObjectConstResult.h"
#include "clang/AST/ASTContext.h"

using namespace lldb_private;
using namespace clang;

char ClangExpressionVariable::ID;

ClangExpressionVariable::ClangExpressionVariable(
    ExecutionContextScope *exe_scope, lldb::ByteOrder byte_order,
    uint32_t addr_byte_size)
    : m_parser_vars(), m_jit_vars() {
  m_flags = EVNone;
  m_frozen_sp =
      ValueObjectConstResult::Create(exe_scope, byte_order, addr_byte_size);
}

ClangExpressionVariable::ClangExpressionVariable(
    ExecutionContextScope *exe_scope, Value &value, ConstString name,
    uint16_t flags)
    : m_parser_vars(), m_jit_vars() {
  m_flags = flags;
  m_frozen_sp = ValueObjectConstResult::Create(exe_scope, value, name);
}

ClangExpressionVariable::ClangExpressionVariable(
    const lldb::ValueObjectSP &valobj_sp)
    : m_parser_vars(), m_jit_vars() {
  m_flags = EVNone;
  m_frozen_sp = valobj_sp;
}

ClangExpressionVariable::ClangExpressionVariable(
    ExecutionContextScope *exe_scope, ConstString name,
    const TypeFromUser &user_type, lldb::ByteOrder byte_order,
    uint32_t addr_byte_size)
    : m_parser_vars(), m_jit_vars() {
  m_flags = EVNone;
  m_frozen_sp =
      ValueObjectConstResult::Create(exe_scope, byte_order, addr_byte_size);
  SetName(name);
  SetCompilerType(user_type);
}

TypeFromUser ClangExpressionVariable::GetTypeFromUser() {
  TypeFromUser tfu(m_frozen_sp->GetCompilerType());
  return tfu;
}
