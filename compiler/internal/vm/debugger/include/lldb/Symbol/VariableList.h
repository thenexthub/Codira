//===-- VariableList.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SYMBOL_VARIABLELIST_H
#define LLDB_SYMBOL_VARIABLELIST_H

#include "lldb/Symbol/SymbolContext.h"
#include "lldb/Symbol/Variable.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class VariableList {
  typedef std::vector<lldb::VariableSP> collection;

public:
  // Constructors and Destructors
  //  VariableList(const SymbolContext &symbol_context);
  VariableList();
  virtual ~VariableList();

  VariableList(VariableList &&) = default;
  VariableList &operator=(VariableList &&) = default;

  void AddVariable(const lldb::VariableSP &var_sp);

  bool AddVariableIfUnique(const lldb::VariableSP &var_sp);

  void AddVariables(VariableList *variable_list);

  void Clear();

  void Dump(Stream *s, bool show_context) const;

  lldb::VariableSP GetVariableAtIndex(size_t idx) const;

  lldb::VariableSP RemoveVariableAtIndex(size_t idx);

  lldb::VariableSP FindVariable(ConstString name,
                                bool include_static_members = true);

  lldb::VariableSP FindVariable(ConstString name,
                                lldb::ValueType value_type,
                                bool include_static_members = true);

  uint32_t FindVariableIndex(const lldb::VariableSP &var_sp);

  size_t AppendVariablesIfUnique(VariableList &var_list);

  // Returns the actual number of unique variables that were added to the list.
  // "total_matches" will get updated with the actually number of matches that
  // were found regardless of whether they were unique or not to allow for
  // error conditions when nothing is found, versus conditions where any
  // variables that match "regex" were already in "var_list".
  size_t AppendVariablesIfUnique(const RegularExpression &regex,
                                 VariableList &var_list, size_t &total_matches);

  size_t AppendVariablesWithScope(lldb::ValueType type, VariableList &var_list,
                                  bool if_unique = true);

  uint32_t FindIndexForVariable(Variable *variable);

  size_t MemorySize() const;

  size_t GetSize() const;
  bool Empty() const { return m_variables.empty(); }

  typedef collection::iterator iterator;
  typedef collection::const_iterator const_iterator;

  iterator begin() { return m_variables.begin(); }
  iterator end() { return m_variables.end(); }
  const_iterator begin() const { return m_variables.begin(); }
  const_iterator end() const { return m_variables.end(); }

  llvm::ArrayRef<lldb::VariableSP> toArrayRef() {
    return llvm::ArrayRef(m_variables);
  }

protected:
  collection m_variables;

private:
  // For VariableList only
  VariableList(const VariableList &) = delete;
  const VariableList &operator=(const VariableList &) = delete;
};

} // namespace lldb_private

#endif // LLDB_SYMBOL_VARIABLELIST_H
