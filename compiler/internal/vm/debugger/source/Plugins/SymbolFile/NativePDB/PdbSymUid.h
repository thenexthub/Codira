//===-- PdbSymUid.h ---------------------------------------------*- C++ -*-===//
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
// A unique identification scheme for Pdb records.
// The scheme is to partition a 64-bit integer into an 8-bit tag field, which
// will contain some value from the PDB_SymType enumeration.  The format of the
// other 48-bits depend on the tag, but must be sufficient to locate the
// corresponding entry in the underlying PDB file quickly.  For example, for
// a compile unit, we use 2 bytes to represent the index, which allows fast
// access to the compile unit's information.
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_NATIVEPDB_PDBSYMUID_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_NATIVEPDB_PDBSYMUID_H

#include "llvm/DebugInfo/CodeView/SymbolRecord.h"
#include "llvm/DebugInfo/PDB/PDBTypes.h"
#include "llvm/Support/Compiler.h"

#include "lldb/Utility/LLDBAssert.h"
#include "lldb/lldb-types.h"

namespace lldb_private {
namespace npdb {

enum class PdbSymUidKind : uint8_t {
  Compiland,
  CompilandSym,
  PublicSym,
  GlobalSym,
  Type,
  FieldListMember
};

struct PdbCompilandId {
  // 0-based index of module in PDB
  uint16_t modi;
};

struct PdbCompilandSymId {
  PdbCompilandSymId() = default;
  PdbCompilandSymId(uint16_t modi, uint32_t offset)
      : modi(modi), offset(offset) {}
  // 0-based index of module in PDB
  uint16_t modi = 0;

  // Offset of symbol's record in module stream.  This is
  // offset by 4 from the CVSymbolArray's notion of offset
  // due to the debug magic at the beginning of the stream.
  uint32_t offset = 0;
};

struct PdbGlobalSymId {
  PdbGlobalSymId() = default;
  PdbGlobalSymId(uint32_t offset, bool is_public)
      : offset(offset), is_public(is_public) {}

  // Offset of symbol's record in globals or publics stream.
  uint32_t offset = 0;

  // True if this symbol is in the public stream, false if it's in the globals
  // stream.
  bool is_public = false;
};

struct PdbTypeSymId {
  PdbTypeSymId() = default;
  PdbTypeSymId(llvm::codeview::TypeIndex index, bool is_ipi = false)
      : index(index), is_ipi(is_ipi) {}

  // The index of the of the type in the TPI or IPI stream.
  llvm::codeview::TypeIndex index;

  // True if this symbol comes from the IPI stream, false if it's from the TPI
  // stream.
  bool is_ipi = false;
};

struct PdbFieldListMemberId {
  // The TypeIndex of the LF_FIELDLIST record.
  llvm::codeview::TypeIndex index;

  // The offset from the beginning of the LF_FIELDLIST record to this record.
  uint16_t offset = 0;
};

class PdbSymUid {
  uint64_t m_repr = 0;

public:
  PdbSymUid() = default;
  PdbSymUid(uint64_t repr) : m_repr(repr) {}
  PdbSymUid(const PdbCompilandId &cid);
  PdbSymUid(const PdbCompilandSymId &csid);
  PdbSymUid(const PdbGlobalSymId &gsid);
  PdbSymUid(const PdbTypeSymId &tsid);
  PdbSymUid(const PdbFieldListMemberId &flmid);

  uint64_t toOpaqueId() const { return m_repr; }

  PdbSymUidKind kind() const;

  PdbCompilandId asCompiland() const;
  PdbCompilandSymId asCompilandSym() const;
  PdbGlobalSymId asGlobalSym() const;
  PdbTypeSymId asTypeSym() const;
  PdbFieldListMemberId asFieldListMember() const;
};

template <typename T> uint64_t toOpaqueUid(const T &cid) {
  return PdbSymUid(cid).toOpaqueId();
}

struct SymbolAndUid {
  llvm::codeview::CVSymbol sym;
  PdbSymUid uid;
};
} // namespace npdb
} // namespace lldb_private

#endif
