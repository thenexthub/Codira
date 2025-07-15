//===--- FileTypes.h - Input & output formats used by the tools -*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_FILETYPES_H
#define LANGUAGE_BASIC_FILETYPES_H

#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/DenseMapInfo.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringRef.h"

namespace language {
namespace file_types {
enum ID : uint8_t {
#define TYPE(NAME, ID, EXTENSION, FLAGS) TY_##ID,
#include "language/Basic/FileTypes.def"
#undef TYPE
  TY_INVALID
};

/// Return the name of the type for \p Id.
StringRef getTypeName(ID Id);

/// Return the extension to use when creating a file of this type,
/// or an empty string if unspecified.
StringRef getExtension(ID Id);

/// Lookup the type to use for the file extension \p Ext.
/// If the extension is empty or is otherwise not recognized, return
/// the invalid type \c TY_INVALID.
ID lookupTypeForExtension(StringRef Ext);

/// Lookup the type to use for the file name \p Filename.
/// If the filename is empty or type cannot be recognoized, return
/// the invalid type \c TY_INVALID.
ID lookupTypeFromFilename(StringRef Filename);

/// Lookup the type to use for the name \p Name.
ID lookupTypeForName(StringRef Name);

/// Returns true if the type represents textual data.
bool isTextual(ID Id);

/// Returns true if the type is produced in the compiler after the LLVM
/// passes.
///
/// For those types the compiler produces multiple output files in multi-
/// threaded compilation.
bool isAfterLLVM(ID Id);

/// Returns true if the type is a file that contributes to the Codira module
/// being compiled.
///
/// These need to be passed to the Codira frontend
bool isPartOfCodiraCompilation(ID Id);

/// Returns true of the type of the output is produced from a diagnostic engine.
bool isProducedFromDiagnostics(ID Id);

static inline void forAllTypes(toolchain::function_ref<void(file_types::ID)> fn) {
  for (uint8_t i = 0; i < static_cast<uint8_t>(TY_INVALID); ++i)
    fn(static_cast<ID>(i));
}

/// Some files are produced by the frontend and read by the driver in order to
/// support incremental compilation. Invoke the passed-in function for every
/// such file type.
static inline void
forEachIncrementalOutputType(toolchain::function_ref<void(file_types::ID)> fn) {
  fn(file_types::TY_CodiraDeps);
}

} // end namespace file_types
} // end namespace language

namespace toolchain {
template <> struct DenseMapInfo<language::file_types::ID> {
  using ID = language::file_types::ID;
  static inline ID getEmptyKey() { return ID::TY_INVALID; }
  static inline ID getTombstoneKey() {
    return static_cast<ID>(ID::TY_INVALID + 1);
  }
  static unsigned getHashValue(ID Val) { return (unsigned)Val * 37U; }
  static bool isEqual(ID LHS, ID RHS) { return LHS == RHS; }
};
} // end namespace toolchain

#endif
