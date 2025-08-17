//===-- APINotesWriter.h - API Notes Writer ---------------------*- C++ -*-===//
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
// This file defines the \c APINotesWriter class that writes out source
// API notes data providing additional information about source code as
// a separate input, such as the non-nil/nilable annotations for
// method parameters.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_APINOTES_WRITER_H
#define LANGUAGE_CORE_APINOTES_WRITER_H

#include "language/Core/APINotes/Types.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/VersionTuple.h"
#include "toolchain/Support/raw_ostream.h"

#include <memory>

namespace language::Core {
class FileEntry;

namespace api_notes {

/// A class that writes API notes data to a binary representation that can be
/// read by the \c APINotesReader.
class APINotesWriter {
  class Implementation;
  std::unique_ptr<Implementation> Implementation;

public:
  /// Create a new API notes writer with the given module name and
  /// (optional) source file.
  APINotesWriter(toolchain::StringRef ModuleName, const FileEntry *SF);
  ~APINotesWriter();

  APINotesWriter(const APINotesWriter &) = delete;
  APINotesWriter &operator=(const APINotesWriter &) = delete;

  void writeToStream(toolchain::raw_ostream &OS);

  /// Add information about a specific Objective-C class or protocol or a C++
  /// namespace.
  ///
  /// \param Name The name of this class/protocol/namespace.
  /// \param Kind Whether this is a class, a protocol, or a namespace.
  /// \param Info Information about this class/protocol/namespace.
  ///
  /// \returns the ID of the class, protocol, or namespace, which can be used to
  /// add properties and methods to the class/protocol/namespace.
  ContextID addContext(std::optional<ContextID> ParentCtxID,
                       toolchain::StringRef Name, ContextKind Kind,
                       const ContextInfo &Info,
                       toolchain::VersionTuple SwiftVersion);

  /// Add information about a specific Objective-C property.
  ///
  /// \param CtxID The context in which this property resides.
  /// \param Name The name of this property.
  /// \param Info Information about this property.
  void addObjCProperty(ContextID CtxID, toolchain::StringRef Name,
                       bool IsInstanceProperty, const ObjCPropertyInfo &Info,
                       toolchain::VersionTuple SwiftVersion);

  /// Add information about a specific Objective-C method.
  ///
  /// \param CtxID The context in which this method resides.
  /// \param Selector The selector that names this method.
  /// \param IsInstanceMethod Whether this method is an instance method
  /// (vs. a class method).
  /// \param Info Information about this method.
  void addObjCMethod(ContextID CtxID, ObjCSelectorRef Selector,
                     bool IsInstanceMethod, const ObjCMethodInfo &Info,
                     toolchain::VersionTuple SwiftVersion);

  /// Add information about a specific C++ method.
  ///
  /// \param CtxID The context in which this method resides, i.e. a C++ tag.
  /// \param Name The name of the method.
  /// \param Info Information about this method.
  void addCXXMethod(ContextID CtxID, toolchain::StringRef Name,
                    const CXXMethodInfo &Info, toolchain::VersionTuple SwiftVersion);

  /// Add information about a specific C record field.
  ///
  /// \param CtxID The context in which this field resides, i.e. a C/C++ tag.
  /// \param Name The name of the field.
  /// \param Info Information about this field.
  void addField(ContextID CtxID, toolchain::StringRef Name, const FieldInfo &Info,
                toolchain::VersionTuple SwiftVersion);

  /// Add information about a global variable.
  ///
  /// \param Name The name of this global variable.
  /// \param Info Information about this global variable.
  void addGlobalVariable(std::optional<Context> Ctx, toolchain::StringRef Name,
                         const GlobalVariableInfo &Info,
                         toolchain::VersionTuple SwiftVersion);

  /// Add information about a global function.
  ///
  /// \param Name The name of this global function.
  /// \param Info Information about this global function.
  void addGlobalFunction(std::optional<Context> Ctx, toolchain::StringRef Name,
                         const GlobalFunctionInfo &Info,
                         toolchain::VersionTuple SwiftVersion);

  /// Add information about an enumerator.
  ///
  /// \param Name The name of this enumerator.
  /// \param Info Information about this enumerator.
  void addEnumConstant(toolchain::StringRef Name, const EnumConstantInfo &Info,
                       toolchain::VersionTuple SwiftVersion);

  /// Add information about a tag (struct/union/enum/C++ class).
  ///
  /// \param Name The name of this tag.
  /// \param Info Information about this tag.
  void addTag(std::optional<Context> Ctx, toolchain::StringRef Name,
              const TagInfo &Info, toolchain::VersionTuple SwiftVersion);

  /// Add information about a typedef.
  ///
  /// \param Name The name of this typedef.
  /// \param Info Information about this typedef.
  void addTypedef(std::optional<Context> Ctx, toolchain::StringRef Name,
                  const TypedefInfo &Info, toolchain::VersionTuple SwiftVersion);
};
} // namespace api_notes
} // namespace language::Core

#endif // LANGUAGE_CORE_APINOTES_WRITER_H
