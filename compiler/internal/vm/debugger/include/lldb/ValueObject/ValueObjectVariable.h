//===-- ValueObjectVariable.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_VALUEOBJECT_VALUEOBJECTVARIABLE_H
#define LLDB_VALUEOBJECT_VALUEOBJECTVARIABLE_H

#include "lldb/ValueObject/ValueObject.h"

#include "lldb/Core/Value.h"
#include "lldb/Symbol/CompilerType.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/lldb-defines.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-forward.h"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace lldb_private {
class DataExtractor;
class Declaration;
class Status;
class ExecutionContextScope;
class SymbolContextScope;

/// A ValueObject that contains a root variable that may or may not
/// have children.
class ValueObjectVariable : public ValueObject {
public:
  ~ValueObjectVariable() override;

  static lldb::ValueObjectSP Create(ExecutionContextScope *exe_scope,
                                    const lldb::VariableSP &var_sp);

  llvm::Expected<uint64_t> GetByteSize() override;

  ConstString GetTypeName() override;

  ConstString GetQualifiedTypeName() override;

  ConstString GetDisplayTypeName() override;

  llvm::Expected<uint32_t> CalculateNumChildren(uint32_t max) override;

  lldb::ValueType GetValueType() const override;

  bool IsInScope() override;

  lldb::ModuleSP GetModule() override;

  SymbolContextScope *GetSymbolContextScope() override;

  bool GetDeclaration(Declaration &decl) override;

  const char *GetLocationAsCString() override;

  bool SetValueFromCString(const char *value_str, Status &error) override;

  bool SetData(DataExtractor &data, Status &error) override;

  lldb::VariableSP GetVariable() override { return m_variable_sp; }

protected:
  bool UpdateValue() override;

  void DoUpdateChildrenAddressType(ValueObject &valobj) override;

  CompilerType GetCompilerTypeImpl() override;

  /// The variable that this value object is based upon.
  lldb::VariableSP m_variable_sp;

  /// The value that DWARFExpression resolves this variable to before we patch
  /// it up.
  Value m_resolved_value;

private:
  ValueObjectVariable(ExecutionContextScope *exe_scope,
                      ValueObjectManager &manager,
                      const lldb::VariableSP &var_sp);
  // For ValueObject only
  ValueObjectVariable(const ValueObjectVariable &) = delete;
  const ValueObjectVariable &operator=(const ValueObjectVariable &) = delete;
};

} // namespace lldb_private

#endif // LLDB_VALUEOBJECT_VALUEOBJECTVARIABLE_H
