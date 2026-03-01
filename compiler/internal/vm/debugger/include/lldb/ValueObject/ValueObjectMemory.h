//===-- ValueObjectMemory.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_VALUEOBJECT_VALUEOBJECTMEMORY_H
#define LLDB_VALUEOBJECT_VALUEOBJECTMEMORY_H

#include "lldb/Core/Address.h"
#include "lldb/Symbol/CompilerType.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/ValueObject/ValueObject.h"
#include "lldb/lldb-defines.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-forward.h"
#include "llvm/ADT/StringRef.h"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace lldb_private {
class ExecutionContextScope;

/// A ValueObject that represents memory at a given address, viewed as some
/// set lldb type.
class ValueObjectMemory : public ValueObject {
public:
  ~ValueObjectMemory() override;

  static lldb::ValueObjectSP Create(ExecutionContextScope *exe_scope,
                                    llvm::StringRef name,
                                    const Address &address,
                                    lldb::TypeSP &type_sp);

  static lldb::ValueObjectSP Create(ExecutionContextScope *exe_scope,
                                    llvm::StringRef name,
                                    const Address &address,
                                    const CompilerType &ast_type);

  llvm::Expected<uint64_t> GetByteSize() override;

  ConstString GetTypeName() override;

  ConstString GetDisplayTypeName() override;

  llvm::Expected<uint32_t> CalculateNumChildren(uint32_t max) override;

  lldb::ValueType GetValueType() const override;

  bool IsInScope() override;

  lldb::ModuleSP GetModule() override;

protected:
  bool UpdateValue() override;

  CompilerType GetCompilerTypeImpl() override;

  Address m_address; ///< The variable that this value object is based upon
  lldb::TypeSP m_type_sp;
  CompilerType m_compiler_type;

private:
  ValueObjectMemory(ExecutionContextScope *exe_scope,
                    ValueObjectManager &manager, llvm::StringRef name,
                    const Address &address, lldb::TypeSP &type_sp);

  ValueObjectMemory(ExecutionContextScope *exe_scope,
                    ValueObjectManager &manager, llvm::StringRef name,
                    const Address &address, const CompilerType &ast_type);
  // For ValueObject only
  ValueObjectMemory(const ValueObjectMemory &) = delete;
  const ValueObjectMemory &operator=(const ValueObjectMemory &) = delete;
};

} // namespace lldb_private

#endif // LLDB_VALUEOBJECT_VALUEOBJECTMEMORY_H
