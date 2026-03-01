//===-- SBSymbol.h ----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBSYMBOL_H
#define LLDB_API_SBSYMBOL_H

#include "lldb/API/SBAddress.h"
#include "lldb/API/SBDefines.h"
#include "lldb/API/SBInstructionList.h"
#include "lldb/API/SBTarget.h"

namespace lldb {

class LLDB_API SBSymbol {
public:
  SBSymbol();

  ~SBSymbol();

  SBSymbol(const lldb::SBSymbol &rhs);

  const lldb::SBSymbol &operator=(const lldb::SBSymbol &rhs);

  explicit operator bool() const;

  bool IsValid() const;

  const char *GetName() const;

  const char *GetDisplayName() const;

  const char *GetMangledName() const;

  const char *GetBaseName() const;

  lldb::SBInstructionList GetInstructions(lldb::SBTarget target);

  lldb::SBInstructionList GetInstructions(lldb::SBTarget target,
                                          const char *flavor_string);

  /// Get the start address of this symbol
  ///
  /// \returns
  ///   If the symbol's value is not an address, an invalid SBAddress object
  ///   will be returned. If the symbol's value is an address, a valid SBAddress
  ///   object will be returned.
  SBAddress GetStartAddress();

  /// Get the end address of this symbol
  ///
  /// \returns
  ///   If the symbol's value is not an address, an invalid SBAddress object
  ///   will be returned. If the symbol's value is an address, a valid SBAddress
  ///   object will be returned.
  SBAddress GetEndAddress();

  /// Get the raw value of a symbol.
  ///
  /// This accessor allows direct access to the symbol's value from the symbol
  /// table regardless of what the value is. The value can be a file address or
  /// it can be an integer value that depends on what the symbol's type is. Some
  /// symbol values are not addresses, but absolute values or integer values
  /// that can be mean different things. The GetStartAddress() accessor will
  /// only return a valid SBAddress if the symbol's value is an address, so this
  /// accessor provides a way to access the symbol's value when the value is
  /// not an address.
  ///
  /// \returns
  ///   Returns the raw integer value of a symbol from the symbol table.
  uint64_t GetValue();

  /// Get the size of the symbol.
  ///
  /// This accessor allows direct access to the symbol's size from the symbol
  /// table regardless of what the value is (address or integer value).
  ///
  /// \returns
  ///   Returns the size of a symbol from the symbol table.
  uint64_t GetSize();

  uint32_t GetPrologueByteSize();

  SymbolType GetType();

  /// Get the ID of this symbol, usually the original symbol table index.
  ///
  /// \returns
  ///     LLDB_INVALID_SYMBOL_ID if this object does not contain a valid symbol
  ///     object. Otherwise, Returns a valid symbol ID.
  uint32_t GetID() const;

  bool operator==(const lldb::SBSymbol &rhs) const;

  bool operator!=(const lldb::SBSymbol &rhs) const;

  bool GetDescription(lldb::SBStream &description);

  // Returns true if the symbol is externally visible in the module that it is
  // defined in
  bool IsExternal();

  // Returns true if the symbol was synthetically generated from something
  // other than the actual symbol table itself in the object file.
  bool IsSynthetic();

  /// Returns true if the symbol is a debug symbol.
  bool IsDebug() const;

  /// Get the string representation of a symbol type.
  static const char *GetTypeAsString(lldb::SymbolType symbol_type);

  /// Get the symbol type from a string representation.
  static lldb::SymbolType GetTypeFromString(const char *str);

protected:
  lldb_private::Symbol *get();

  void reset(lldb_private::Symbol *);

private:
  friend class SBAddress;
  friend class SBFrame;
  friend class SBModule;
  friend class SBSymbolContext;

  SBSymbol(lldb_private::Symbol *lldb_object_ptr);

  void SetSymbol(lldb_private::Symbol *lldb_object_ptr);

  lldb_private::Symbol *m_opaque_ptr = nullptr;
};

} // namespace lldb

#endif // LLDB_API_SBSYMBOL_H
