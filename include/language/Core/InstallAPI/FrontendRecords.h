//===- InstallAPI/FrontendRecords.h ------------------------------*- C++-*-===//
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

#ifndef LANGUAGE_CORE_INSTALLAPI_FRONTENDRECORDS_H
#define LANGUAGE_CORE_INSTALLAPI_FRONTENDRECORDS_H

#include "language/Core/AST/Availability.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/InstallAPI/HeaderFile.h"
#include "language/Core/InstallAPI/MachO.h"

namespace language::Core {
namespace installapi {

/// Frontend information captured about records.
struct FrontendAttrs {
  const AvailabilityInfo Avail;
  const Decl *D;
  const SourceLocation Loc;
  const HeaderType Access;
};

// Represents a collection of frontend records for a library that are tied to a
// darwin target triple.
class FrontendRecordsSlice : public toolchain::MachO::RecordsSlice {
public:
  FrontendRecordsSlice(const toolchain::Triple &T)
      : toolchain::MachO::RecordsSlice({T}) {}

  /// Add non-ObjC global record with attributes from AST.
  ///
  /// \param Name The name of symbol.
  /// \param Linkage The linkage of symbol.
  /// \param GV The kind of global.
  /// \param Avail The availability information tied to the active target
  /// triple.
  /// \param D The pointer to the declaration from traversing AST.
  /// \param Access The intended access level of symbol.
  /// \param Flags The flags that describe attributes of the symbol.
  /// \param Inlined Whether declaration is inlined, only applicable to
  /// functions.
  /// \return The non-owning pointer to added record in slice with it's frontend
  /// attributes.
  std::pair<GlobalRecord *, FrontendAttrs *>
  addGlobal(StringRef Name, RecordLinkage Linkage, GlobalRecord::Kind GV,
            const language::Core::AvailabilityInfo Avail, const Decl *D,
            const HeaderType Access, SymbolFlags Flags = SymbolFlags::None,
            bool Inlined = false);

  /// Add ObjC Class record with attributes from AST.
  ///
  /// \param Name The name of class, not symbol.
  /// \param Linkage The linkage of symbol.
  /// \param Avail The availability information tied to the active target
  /// triple.
  /// \param D The pointer to the declaration from traversing AST.
  /// \param Access The intended access level of symbol.
  /// \param IsEHType Whether declaration has an exception attribute.
  /// \return The non-owning pointer to added record in slice with it's frontend
  /// attributes.
  std::pair<ObjCInterfaceRecord *, FrontendAttrs *>
  addObjCInterface(StringRef Name, RecordLinkage Linkage,
                   const language::Core::AvailabilityInfo Avail, const Decl *D,
                   HeaderType Access, bool IsEHType);

  /// Add ObjC Category record with attributes from AST.
  ///
  /// \param ClassToExtend The name of class that is extended by category, not
  /// symbol.
  /// \param CategoryName The name of category, not symbol.
  /// \param Avail The availability information tied
  /// to the active target triple.
  /// \param D The pointer to the declaration from traversing AST.
  /// \param Access The intended access level of symbol.
  /// \return The non-owning pointer to added record in slice with it's frontend
  /// attributes.
  std::pair<ObjCCategoryRecord *, FrontendAttrs *>
  addObjCCategory(StringRef ClassToExtend, StringRef CategoryName,
                  const language::Core::AvailabilityInfo Avail, const Decl *D,
                  HeaderType Access);

  /// Add ObjC IVar record with attributes from AST.
  ///
  /// \param Container The owning pointer for instance variable.
  /// \param Name The name of ivar, not symbol.
  /// \param Linkage The linkage of symbol.
  /// \param Avail The availability information tied to the active target
  /// triple.
  /// \param D The pointer to the declaration from traversing AST.
  /// \param Access The intended access level of symbol.
  /// \param AC The access control tied to the ivar declaration.
  /// \return The non-owning pointer to added record in slice with it's frontend
  /// attributes.
  std::pair<ObjCIVarRecord *, FrontendAttrs *>
  addObjCIVar(ObjCContainerRecord *Container, StringRef IvarName,
              RecordLinkage Linkage, const language::Core::AvailabilityInfo Avail,
              const Decl *D, HeaderType Access,
              const language::Core::ObjCIvarDecl::AccessControl AC);

private:
  /// Mapping of records stored in slice to their frontend attributes.
  toolchain::DenseMap<Record *, FrontendAttrs> FrontendRecords;
};

} // namespace installapi
} // namespace language::Core

#endif // LANGUAGE_CORE_INSTALLAPI_FRONTENDRECORDS_H
