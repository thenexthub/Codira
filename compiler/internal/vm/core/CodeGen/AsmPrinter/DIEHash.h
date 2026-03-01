//===-- toolchain/CodeGen/DIEHash.h - Dwarf Hashing Framework -------*- C++ -*--===//
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
//
// This file contains support for DWARF4 hashing of DIEs.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CODEGEN_ASMPRINTER_DIEHASH_H
#define LLVM_LIB_CODEGEN_ASMPRINTER_DIEHASH_H

#include "vm/core/ADT/DenseMap.h"
#include "vm/core/CodeGen/DIE.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/MD5.h"

namespace vm::core {

class AsmPrinter;

/// An object containing the capability of hashing and adding hash
/// attributes onto a DIE.
class DIEHash {
  // Collection of all attributes used in hashing a particular DIE.
  struct DIEAttrs {
#define HANDLE_DIE_HASH_ATTR(NAME) DIEValue NAME;
#include "DIEHashAttributes.def"
  };

public:
  DIEHash(AsmPrinter *A = nullptr, DwarfCompileUnit *CU = nullptr)
      : AP(A), CU(CU) {}

  /// Computes the CU signature.
  uint64_t computeCUSignature(StringRef DWOName, const DIE &Die);

  /// Computes the type signature.
  LLVM_ABI_FOR_TEST uint64_t computeTypeSignature(const DIE &Die);

  // Helper routines to process parts of a DIE.
private:
  /// Adds the parent context of \param Parent to the hash.
  void addParentContext(const DIE &Parent);

  /// Adds the attributes of \param Die to the hash.
  void addAttributes(const DIE &Die);

  /// Computes the full DWARF4 7.27 hash of the DIE.
  void computeHash(const DIE &Die);

  // Routines that add DIEValues to the hash.
public:
  /// Adds \param Value to the hash.
  void update(uint8_t Value) { Hash.update(Value); }

  /// Encodes and adds \param Value to the hash as a ULEB128.
  void addULEB128(uint64_t Value);

  /// Encodes and adds \param Value to the hash as a SLEB128.
  void addSLEB128(int64_t Value);

  void hashRawTypeReference(const DIE &Entry);

private:
  /// Adds \param Str to the hash and includes a NULL byte.
  void addString(StringRef Str);

  /// Collects the attributes of DIE \param Die into the \param Attrs
  /// structure.
  void collectAttributes(const DIE &Die, DIEAttrs &Attrs);

  /// Hashes the attributes in \param Attrs in order.
  void hashAttributes(const DIEAttrs &Attrs, dwarf::Tag Tag);

  /// Hashes the data in a block like DIEValue, e.g. DW_FORM_block or
  /// DW_FORM_exprloc.
  void hashBlockData(const DIE::const_value_range &Values);

  /// Hashes the contents pointed to in the .debug_loc section.
  void hashLocList(const DIELocList &LocList);

  /// Hashes an individual attribute.
  void hashAttribute(const DIEValue &Value, dwarf::Tag Tag);

  /// Hashes an attribute that refers to another DIE.
  void hashDIEEntry(dwarf::Attribute Attribute, dwarf::Tag Tag,
                    const DIE &Entry);

  /// Hashes a reference to a named type in such a way that is
  /// independent of whether that type is described by a declaration or a
  /// definition.
  void hashShallowTypeReference(dwarf::Attribute Attribute, const DIE &Entry,
                                StringRef Name);

  /// Hashes a reference to a previously referenced type DIE.
  void hashRepeatedTypeReference(dwarf::Attribute Attribute,
                                 unsigned DieNumber);

  void hashNestedType(const DIE &Die, StringRef Name);

private:
  MD5 Hash;
  AsmPrinter *AP;
  DwarfCompileUnit *CU;
  DenseMap<const DIE *, unsigned> Numbering;
};
}

#endif
