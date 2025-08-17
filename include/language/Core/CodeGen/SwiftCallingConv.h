//==-- SwiftCallingConv.h - Swift ABI lowering ------------------*- C++ -*-===//
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
// Defines constants and types related to Swift ABI lowering. The same ABI
// lowering applies to both sync and async functions.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_CODEGEN_SWIFTCALLINGCONV_H
#define LANGUAGE_CORE_CODEGEN_SWIFTCALLINGCONV_H

#include "language/Core/AST/CanonicalType.h"
#include "language/Core/AST/CharUnits.h"
#include "language/Core/AST/Type.h"
#include "toolchain/Support/TrailingObjects.h"
#include <cassert>

namespace toolchain {
  class IntegerType;
  class Type;
  class StructType;
  class VectorType;
}

namespace language::Core {
class FieldDecl;
class ASTRecordLayout;

namespace CodeGen {
class ABIArgInfo;
class CodeGenModule;
class CGFunctionInfo;

namespace swiftcall {

class SwiftAggLowering {
  CodeGenModule &CGM;

  struct StorageEntry {
    CharUnits Begin;
    CharUnits End;
    toolchain::Type *Type;

    CharUnits getWidth() const {
      return End - Begin;
    }
  };
  SmallVector<StorageEntry, 4> Entries;
  bool Finished = false;

public:
  SwiftAggLowering(CodeGenModule &CGM) : CGM(CGM) {}

  void addOpaqueData(CharUnits begin, CharUnits end) {
    addEntry(nullptr, begin, end);
  }

  void addTypedData(QualType type, CharUnits begin);
  void addTypedData(const RecordDecl *record, CharUnits begin);
  void addTypedData(const RecordDecl *record, CharUnits begin,
                    const ASTRecordLayout &layout);
  void addTypedData(toolchain::Type *type, CharUnits begin);
  void addTypedData(toolchain::Type *type, CharUnits begin, CharUnits end);

  void finish();

  /// Does this lowering require passing any data?
  bool empty() const {
    assert(Finished && "didn't finish lowering before calling empty()");
    return Entries.empty();
  }

  /// According to the target Swift ABI, should a value with this lowering
  /// be passed indirectly?
  ///
  /// Note that this decision is based purely on the data layout of the
  /// value and does not consider whether the type is address-only,
  /// must be passed indirectly to match a function abstraction pattern, or
  /// anything else that is expected to be handled by high-level lowering.
  ///
  /// \param asReturnValue - if true, answer whether it should be passed
  ///   indirectly as a return value; if false, answer whether it should be
  ///   passed indirectly as an argument
  bool shouldPassIndirectly(bool asReturnValue) const;

  using EnumerationCallback =
    toolchain::function_ref<void(CharUnits offset, CharUnits end, toolchain::Type *type)>;

  /// Enumerate the expanded components of this type.
  ///
  /// The component types will always be legal vector, floating-point,
  /// integer, or pointer types.
  void enumerateComponents(EnumerationCallback callback) const;

  /// Return the types for a coerce-and-expand operation.
  ///
  /// The first type matches the memory layout of the data that's been
  /// added to this structure, including explicit [N x i8] arrays for any
  /// internal padding.
  ///
  /// The second type removes any internal padding members and, if only
  /// one element remains, is simply that element type.
  std::pair<toolchain::StructType*, toolchain::Type*> getCoerceAndExpandTypes() const;

private:
  void addBitFieldData(const FieldDecl *field, CharUnits begin,
                       uint64_t bitOffset);
  void addLegalTypedData(toolchain::Type *type, CharUnits begin, CharUnits end);
  void addEntry(toolchain::Type *type, CharUnits begin, CharUnits end);
  void splitVectorEntry(unsigned index);
  static bool shouldMergeEntries(const StorageEntry &first,
                                 const StorageEntry &second,
                                 CharUnits chunkSize);
};

/// Should an aggregate which expands to the given type sequence
/// be passed/returned indirectly under swiftcall?
bool shouldPassIndirectly(CodeGenModule &CGM,
                          ArrayRef<toolchain::Type*> types,
                          bool asReturnValue);

/// Return the maximum voluntary integer size for the current target.
CharUnits getMaximumVoluntaryIntegerSize(CodeGenModule &CGM);

/// Return the Swift CC's notion of the natural alignment of a type.
CharUnits getNaturalAlignment(CodeGenModule &CGM, toolchain::Type *type);

/// Is the given integer type "legal" for Swift's perspective on the
/// current platform?
bool isLegalIntegerType(CodeGenModule &CGM, toolchain::IntegerType *type);

/// Is the given vector type "legal" for Swift's perspective on the
/// current platform?
bool isLegalVectorType(CodeGenModule &CGM, CharUnits vectorSize,
                       toolchain::VectorType *vectorTy);
bool isLegalVectorType(CodeGenModule &CGM, CharUnits vectorSize,
                       toolchain::Type *eltTy, unsigned numElts);

/// Minimally split a legal vector type.
std::pair<toolchain::Type*, unsigned>
splitLegalVectorType(CodeGenModule &CGM, CharUnits vectorSize,
                     toolchain::VectorType *vectorTy);

/// Turn a vector type in a sequence of legal component vector types.
///
/// The caller may assume that the sum of the data sizes of the resulting
/// types will equal the data size of the vector type.
void legalizeVectorType(CodeGenModule &CGM, CharUnits vectorSize,
                        toolchain::VectorType *vectorTy,
                        toolchain::SmallVectorImpl<toolchain::Type*> &types);

/// Is the given record type required to be passed and returned indirectly
/// because of language restrictions?
///
/// This considers *only* mandatory indirectness due to language restrictions,
/// such as C++'s non-trivially-copyable types and Objective-C's __weak
/// references.  A record for which this returns true may still be passed
/// indirectly for other reasons, such as being too large to fit in a
/// reasonable number of registers.
bool mustPassRecordIndirectly(CodeGenModule &CGM, const RecordDecl *record);

/// Classify the rules for how to return a particular type.
ABIArgInfo classifyReturnType(CodeGenModule &CGM, CanQualType type);

/// Classify the rules for how to pass a particular type.
ABIArgInfo classifyArgumentType(CodeGenModule &CGM, CanQualType type);

/// Compute the ABI information of a swiftcall function.  This is a
/// private interface for Clang.
void computeABIInfo(CodeGenModule &CGM, CGFunctionInfo &FI);

/// Is swifterror lowered to a register by the target ABI?
bool isSwiftErrorLoweredInRegister(CodeGenModule &CGM);

} // end namespace swiftcall
} // end namespace CodeGen
} // end namespace language::Core

#endif
