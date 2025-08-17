//===-- Optimizer/Support/InternalNames.h -----------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_INTERNALNAMES_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_INTERNALNAMES_H

#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringRef.h"
#include <cstdint>
#include <optional>

namespace fir {

static constexpr toolchain::StringRef kNameSeparator = ".";
static constexpr toolchain::StringRef kBoundsSeparator = ".b.";
static constexpr toolchain::StringRef kComponentSeparator = ".c.";
static constexpr toolchain::StringRef kComponentInitSeparator = ".di.";
static constexpr toolchain::StringRef kDataPtrInitSeparator = ".dp.";
static constexpr toolchain::StringRef kTypeDescriptorSeparator = ".dt.";
static constexpr toolchain::StringRef kKindParameterSeparator = ".kp.";
static constexpr toolchain::StringRef kLenKindSeparator = ".lpk.";
static constexpr toolchain::StringRef kLenParameterSeparator = ".lv.";
static constexpr toolchain::StringRef kNameStringSeparator = ".n.";
static constexpr toolchain::StringRef kProcPtrSeparator = ".p.";
static constexpr toolchain::StringRef kSpecialBindingSeparator = ".s.";
static constexpr toolchain::StringRef kBindingTableSeparator = ".v.";
static constexpr toolchain::StringRef boxprocSuffix = "UnboxProc";
static constexpr toolchain::StringRef kDerivedTypeInitSuffix = "DerivedInit";

/// Internal name mangling of identifiers
///
/// In order to generate symbolically referencable artifacts in a ModuleOp,
/// it is required that those symbols be uniqued.  This is a simple interface
/// for converting Fortran symbols into unique names.
///
/// This is intentionally bijective. Given a symbol's parse name, type, and
/// scope-like information, we can generate a uniqued (mangled) name.  Given a
/// uniqued name, we can return the symbol parse name, type of the symbol, and
/// any scope-like information for that symbol.
struct NameUniquer {
  enum class IntrinsicType { CHARACTER, COMPLEX, INTEGER, LOGICAL, REAL };

  /// The sort of the unique name
  enum class NameKind {
    NOT_UNIQUED,
    BLOCK_DATA_NAME,
    COMMON,
    CONSTANT,
    DERIVED_TYPE,
    DISPATCH_TABLE,
    GENERATED,
    INTRINSIC_TYPE_DESC,
    NAMELIST_GROUP,
    PROCEDURE,
    TYPE_DESC,
    VARIABLE
  };

  /// Components of an unparsed unique name
  struct DeconstructedName {
    DeconstructedName(toolchain::StringRef name) : name{name} {}
    DeconstructedName(toolchain::ArrayRef<std::string> modules,
                      toolchain::ArrayRef<std::string> procs, std::int64_t blockId,
                      toolchain::StringRef name, toolchain::ArrayRef<std::int64_t> kinds)
        : modules{modules}, procs{procs}, blockId{blockId}, name{name},
          kinds{kinds} {}

    toolchain::SmallVector<std::string> modules;
    toolchain::SmallVector<std::string> procs;
    std::int64_t blockId;
    std::string name;
    toolchain::SmallVector<std::int64_t> kinds;
  };

  /// Unique a common block name
  static std::string doCommonBlock(toolchain::StringRef name);

  /// Unique a (global) constant name
  static std::string doConstant(toolchain::ArrayRef<toolchain::StringRef> modules,
                                toolchain::ArrayRef<toolchain::StringRef> procs,
                                std::int64_t block, toolchain::StringRef name);

  /// Unique a dispatch table name
  static std::string doDispatchTable(toolchain::ArrayRef<toolchain::StringRef> modules,
                                     toolchain::ArrayRef<toolchain::StringRef> procs,
                                     std::int64_t block, toolchain::StringRef name,
                                     toolchain::ArrayRef<std::int64_t> kinds);

  /// Unique a compiler generated name without scope context.
  static std::string doGenerated(toolchain::StringRef name);
  /// Unique a compiler generated name with scope context.
  static std::string doGenerated(toolchain::ArrayRef<toolchain::StringRef> modules,
                                 toolchain::ArrayRef<toolchain::StringRef> procs,
                                 std::int64_t blockId, toolchain::StringRef name);

  /// Unique an intrinsic type descriptor
  static std::string
  doIntrinsicTypeDescriptor(toolchain::ArrayRef<toolchain::StringRef> modules,
                            toolchain::ArrayRef<toolchain::StringRef> procs,
                            std::int64_t block, IntrinsicType type,
                            std::int64_t kind);

  /// Unique a procedure name
  static std::string doProcedure(toolchain::ArrayRef<toolchain::StringRef> modules,
                                 toolchain::ArrayRef<toolchain::StringRef> procs,
                                 toolchain::StringRef name);

  /// Unique a derived type name
  static std::string doType(toolchain::ArrayRef<toolchain::StringRef> modules,
                            toolchain::ArrayRef<toolchain::StringRef> procs,
                            std::int64_t block, toolchain::StringRef name,
                            toolchain::ArrayRef<std::int64_t> kinds);

  /// Unique a (derived) type descriptor name
  static std::string doTypeDescriptor(toolchain::ArrayRef<toolchain::StringRef> modules,
                                      toolchain::ArrayRef<toolchain::StringRef> procs,
                                      std::int64_t block, toolchain::StringRef name,
                                      toolchain::ArrayRef<std::int64_t> kinds);
  static std::string doTypeDescriptor(toolchain::ArrayRef<std::string> modules,
                                      toolchain::ArrayRef<std::string> procs,
                                      std::int64_t block, toolchain::StringRef name,
                                      toolchain::ArrayRef<std::int64_t> kinds);

  /// Unique a (global) variable name. A variable with save attribute
  /// defined inside a subprogram also needs to be handled here
  static std::string doVariable(toolchain::ArrayRef<toolchain::StringRef> modules,
                                toolchain::ArrayRef<toolchain::StringRef> procs,
                                std::int64_t block, toolchain::StringRef name);

  /// Unique a namelist group name
  static std::string doNamelistGroup(toolchain::ArrayRef<toolchain::StringRef> modules,
                                     toolchain::ArrayRef<toolchain::StringRef> procs,
                                     toolchain::StringRef name);

  /// Entry point for the PROGRAM (called by the runtime)
  /// Can be overridden with the `--main-entry-name=<name>` option.
  static toolchain::StringRef doProgramEntry();

  /// Decompose `uniquedName` into the parse name, symbol type, and scope info
  static std::pair<NameKind, DeconstructedName>
  deconstruct(toolchain::StringRef uniquedName);

  /// Check if the name is an external facing name.
  static bool isExternalFacingUniquedName(
      const std::pair<NameKind, DeconstructedName> &deconstructResult);

  /// Check whether the name should be re-mangle with external ABI convention.
  static bool needExternalNameMangling(toolchain::StringRef uniquedName);

  /// Does \p uniquedName belong to module \p moduleName?
  static bool belongsToModule(toolchain::StringRef uniquedName,
                              toolchain::StringRef moduleName);

  /// Given a mangled derived type name, get the name of the related derived
  /// type descriptor object. Returns an empty string if \p mangledTypeName is
  /// not a valid mangled derived type name.
  static std::string getTypeDescriptorName(toolchain::StringRef mangledTypeName);

  static std::string
  getTypeDescriptorAssemblyName(toolchain::StringRef mangledTypeName);

  /// Given a mangled derived type name, get the name of the related binding
  /// table object. Returns an empty string if \p mangledTypeName is not a valid
  /// mangled derived type name.
  static std::string
  getTypeDescriptorBindingTableName(toolchain::StringRef mangledTypeName);

  /// Given a mangled derived type name and a component name, get the name of
  /// the global object containing the component default initialization.
  static std::string getComponentInitName(toolchain::StringRef mangledTypeName,
                                          toolchain::StringRef componentName);

  /// Remove markers that have been added when doing partial type
  /// conversions. mlir::Type cannot be mutated in a pass, so new
  /// fir::RecordType must be created when lowering member types.
  /// Suffixes added to these new types are meaningless and are
  /// dropped in the names passed to LLVM.
  static toolchain::StringRef
  dropTypeConversionMarkers(toolchain::StringRef mangledTypeName);

  static std::string replaceSpecialSymbols(const std::string &name);

  /// Returns true if the passed name denotes a special symbol (e.g. global
  /// symbol generated for derived type description).
  static bool isSpecialSymbol(toolchain::StringRef name);

private:
  static std::string intAsString(std::int64_t i);
  static std::string doKind(std::int64_t kind);
  static std::string doKinds(toolchain::ArrayRef<std::int64_t> kinds);
  static std::string toLower(toolchain::StringRef name);

  NameUniquer() = delete;
  NameUniquer(const NameUniquer &) = delete;
  NameUniquer(NameUniquer &&) = delete;
  NameUniquer &operator=(const NameUniquer &) = delete;
};

} // namespace fir

#endif // FORTRAN_OPTIMIZER_SUPPORT_INTERNALNAMES_H
