//===--- LayoutOverrideSource.h --Override Record Layouts -------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_FRONTEND_LAYOUTOVERRIDESOURCE_H
#define LANGUAGE_CORE_FRONTEND_LAYOUTOVERRIDESOURCE_H

#include "language/Core/AST/ExternalASTSource.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {
  /// An external AST source that overrides the layout of
  /// a specified set of record types.
  ///
  /// This class is used only for testing the ability of external AST sources
  /// to override the layout of record types. Its input is the output format
  /// of the command-line argument -fdump-record-layouts.
  class LayoutOverrideSource : public ExternalASTSource {
    /// The layout of a given record.
    struct Layout {
      /// The size of the record.
      uint64_t Size;

      /// The alignment of the record.
      uint64_t Align;

      /// The offsets of non-virtual base classes in the record.
      SmallVector<CharUnits, 8> BaseOffsets;

      /// The offsets of virtual base classes in the record.
      SmallVector<CharUnits, 8> VBaseOffsets;

      /// The offsets of the fields, in source order.
      SmallVector<uint64_t, 8> FieldOffsets;
    };

    /// The set of layouts that will be overridden.
    toolchain::StringMap<Layout> Layouts;

  public:
    /// Create a new AST source that overrides the layout of some
    /// set of record types.
    ///
    /// The file is the result of passing -fdump-record-layouts to a file.
    explicit LayoutOverrideSource(StringRef Filename);

    /// If this particular record type has an overridden layout,
    /// return that layout.
    bool
    layoutRecordType(const RecordDecl *Record,
       uint64_t &Size, uint64_t &Alignment,
       toolchain::DenseMap<const FieldDecl *, uint64_t> &FieldOffsets,
       toolchain::DenseMap<const CXXRecordDecl *, CharUnits> &BaseOffsets,
       toolchain::DenseMap<const CXXRecordDecl *,
                      CharUnits> &VirtualBaseOffsets) override;

    /// Dump the overridden layouts.
    void dump();
  };
}

#endif
