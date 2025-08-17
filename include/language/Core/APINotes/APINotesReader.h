//===--- APINotesReader.h - API Notes Reader --------------------*- C++ -*-===//
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
// This file defines the \c APINotesReader class that reads source API notes
// data providing additional information about source code as a separate input,
// such as the non-nil/nilable annotations for method parameters.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_APINOTES_READER_H
#define LANGUAGE_CORE_APINOTES_READER_H

#include "language/Core/APINotes/Types.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/VersionTuple.h"
#include <memory>

namespace language::Core {
namespace api_notes {

/// A class that reads API notes data from a binary file that was written by
/// the \c APINotesWriter.
class APINotesReader {
  class Implementation;
  std::unique_ptr<Implementation> Implementation;

  APINotesReader(toolchain::MemoryBuffer *InputBuffer,
                 toolchain::VersionTuple SwiftVersion, bool &Failed);

public:
  /// Create a new API notes reader from the given member buffer, which
  /// contains the contents of a binary API notes file.
  ///
  /// \returns the new API notes reader, or null if an error occurred.
  static std::unique_ptr<APINotesReader>
  Create(std::unique_ptr<toolchain::MemoryBuffer> InputBuffer,
         toolchain::VersionTuple SwiftVersion);

  ~APINotesReader();

  APINotesReader(const APINotesReader &) = delete;
  APINotesReader &operator=(const APINotesReader &) = delete;

  /// Captures the completed versioned information for a particular part of
  /// API notes, including both unversioned API notes and each versioned API
  /// note for that particular entity.
  template <typename T> class VersionedInfo {
    /// The complete set of results.
    toolchain::SmallVector<std::pair<toolchain::VersionTuple, T>, 1> Results;

    /// The index of the result that is the "selected" set based on the desired
    /// Swift version, or null if nothing matched.
    std::optional<unsigned> Selected;

  public:
    /// Form an empty set of versioned information.
    VersionedInfo(std::nullopt_t) : Selected(std::nullopt) {}

    /// Form a versioned info set given the desired version and a set of
    /// results.
    VersionedInfo(
        toolchain::VersionTuple Version,
        toolchain::SmallVector<std::pair<toolchain::VersionTuple, T>, 1> Results);

    /// Retrieve the selected index in the result set.
    std::optional<unsigned> getSelected() const { return Selected; }

    /// Return the number of versioned results we know about.
    unsigned size() const { return Results.size(); }

    /// Access all versioned results.
    const std::pair<toolchain::VersionTuple, T> *begin() const {
      assert(!Results.empty());
      return Results.begin();
    }
    const std::pair<toolchain::VersionTuple, T> *end() const {
      return Results.end();
    }

    /// Access a specific versioned result.
    const std::pair<toolchain::VersionTuple, T> &operator[](unsigned index) const {
      assert(index < Results.size());
      return Results[index];
    }
  };

  /// Look for the context ID of the given Objective-C class.
  ///
  /// \param Name The name of the class we're looking for.
  ///
  /// \returns The ID, if known.
  std::optional<ContextID> lookupObjCClassID(toolchain::StringRef Name);

  /// Look for information regarding the given Objective-C class.
  ///
  /// \param Name The name of the class we're looking for.
  ///
  /// \returns The information about the class, if known.
  VersionedInfo<ContextInfo> lookupObjCClassInfo(toolchain::StringRef Name);

  /// Look for the context ID of the given Objective-C protocol.
  ///
  /// \param Name The name of the protocol we're looking for.
  ///
  /// \returns The ID of the protocol, if known.
  std::optional<ContextID> lookupObjCProtocolID(toolchain::StringRef Name);

  /// Look for information regarding the given Objective-C protocol.
  ///
  /// \param Name The name of the protocol we're looking for.
  ///
  /// \returns The information about the protocol, if known.
  VersionedInfo<ContextInfo> lookupObjCProtocolInfo(toolchain::StringRef Name);

  /// Look for information regarding the given Objective-C property in
  /// the given context.
  ///
  /// \param CtxID The ID that references the context we are looking for.
  /// \param Name The name of the property we're looking for.
  /// \param IsInstance Whether we are looking for an instance property (vs.
  /// a class property).
  ///
  /// \returns Information about the property, if known.
  VersionedInfo<ObjCPropertyInfo>
  lookupObjCProperty(ContextID CtxID, toolchain::StringRef Name, bool IsInstance);

  /// Look for information regarding the given Objective-C method in
  /// the given context.
  ///
  /// \param CtxID The ID that references the context we are looking for.
  /// \param Selector The selector naming the method we're looking for.
  /// \param IsInstanceMethod Whether we are looking for an instance method.
  ///
  /// \returns Information about the method, if known.
  VersionedInfo<ObjCMethodInfo> lookupObjCMethod(ContextID CtxID,
                                                 ObjCSelectorRef Selector,
                                                 bool IsInstanceMethod);

  /// Look for information regarding the given field of a C struct.
  ///
  /// \param Name The name of the field.
  ///
  /// \returns information about the field, if known.
  VersionedInfo<FieldInfo> lookupField(ContextID CtxID, toolchain::StringRef Name);

  /// Look for information regarding the given C++ method in the given C++ tag
  /// context.
  ///
  /// \param CtxID The ID that references the parent context, i.e. a C++ tag.
  /// \param Name The name of the C++ method we're looking for.
  ///
  /// \returns Information about the method, if known.
  VersionedInfo<CXXMethodInfo> lookupCXXMethod(ContextID CtxID,
                                               toolchain::StringRef Name);

  /// Look for information regarding the given global variable.
  ///
  /// \param Name The name of the global variable.
  ///
  /// \returns information about the global variable, if known.
  VersionedInfo<GlobalVariableInfo>
  lookupGlobalVariable(toolchain::StringRef Name,
                       std::optional<Context> Ctx = std::nullopt);

  /// Look for information regarding the given global function.
  ///
  /// \param Name The name of the global function.
  ///
  /// \returns information about the global function, if known.
  VersionedInfo<GlobalFunctionInfo>
  lookupGlobalFunction(toolchain::StringRef Name,
                       std::optional<Context> Ctx = std::nullopt);

  /// Look for information regarding the given enumerator.
  ///
  /// \param Name The name of the enumerator.
  ///
  /// \returns information about the enumerator, if known.
  VersionedInfo<EnumConstantInfo> lookupEnumConstant(toolchain::StringRef Name);

  /// Look for the context ID of the given C++ tag.
  ///
  /// \param Name The name of the tag we're looking for.
  /// \param ParentCtx The context in which this tag is declared, e.g. a C++
  /// namespace.
  ///
  /// \returns The ID, if known.
  std::optional<ContextID>
  lookupTagID(toolchain::StringRef Name,
              std::optional<Context> ParentCtx = std::nullopt);

  /// Look for information regarding the given tag
  /// (struct/union/enum/C++ class).
  ///
  /// \param Name The name of the tag.
  ///
  /// \returns information about the tag, if known.
  VersionedInfo<TagInfo> lookupTag(toolchain::StringRef Name,
                                   std::optional<Context> Ctx = std::nullopt);

  /// Look for information regarding the given typedef.
  ///
  /// \param Name The name of the typedef.
  ///
  /// \returns information about the typedef, if known.
  VersionedInfo<TypedefInfo>
  lookupTypedef(toolchain::StringRef Name,
                std::optional<Context> Ctx = std::nullopt);

  /// Look for the context ID of the given C++ namespace.
  ///
  /// \param Name The name of the class we're looking for.
  ///
  /// \returns The ID, if known.
  std::optional<ContextID>
  lookupNamespaceID(toolchain::StringRef Name,
                    std::optional<ContextID> ParentNamespaceID = std::nullopt);
};

} // end namespace api_notes
} // end namespace language::Core

#endif // LANGUAGE_CORE_APINOTES_READER_H
