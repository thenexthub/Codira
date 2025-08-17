//===- ASTDeserializationListener.h - Decl/Type PCH Read Events -*- C++ -*-===//
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
//  This file defines the ASTDeserializationListener class, which is notified
//  by the ASTReader whenever a type or declaration is deserialized.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SERIALIZATION_ASTDESERIALIZATIONLISTENER_H
#define LANGUAGE_CORE_SERIALIZATION_ASTDESERIALIZATIONLISTENER_H

#include "language/Core/Basic/IdentifierTable.h"
#include "language/Core/Serialization/ASTBitCodes.h"

namespace language::Core {

class Decl;
class ASTReader;
class QualType;
class MacroDefinitionRecord;
class MacroInfo;
class Module;
class SourceLocation;

// IMPORTANT: when you add a new interface to this class, please update the
// DelegatingDeserializationListener below.
class ASTDeserializationListener {
public:
  virtual ~ASTDeserializationListener();

  /// The ASTReader was initialized.
  virtual void ReaderInitialized(ASTReader *Reader) { }

  /// An identifier was deserialized from the AST file.
  virtual void IdentifierRead(serialization::IdentifierID ID,
                              IdentifierInfo *II) { }
  /// A macro was read from the AST file.
  virtual void MacroRead(serialization::MacroID ID, MacroInfo *MI) { }
  /// A type was deserialized from the AST file. The ID here has the
  ///        qualifier bits already removed, and T is guaranteed to be locally
  ///        unqualified.
  virtual void TypeRead(serialization::TypeIdx Idx, QualType T) { }
  /// A decl was deserialized from the AST file.
  //
  // Note: Implementors should be cautious when introducing additional
  // serialization (e.g., printing the qualified name of the declaration) within
  // the callback. Doing so may lead to unintended and complex side effects, or
  // even cause a crash.
  virtual void DeclRead(GlobalDeclID ID, const Decl *D) {}
  /// A predefined decl was built during the serialization.
  virtual void PredefinedDeclBuilt(PredefinedDeclIDs ID, const Decl *D) {}
  /// A selector was read from the AST file.
  virtual void SelectorRead(serialization::SelectorID iD, Selector Sel) {}
  /// A macro definition was read from the AST file.
  virtual void MacroDefinitionRead(serialization::PreprocessedEntityID,
                                   MacroDefinitionRecord *MD) {}
  /// A module definition was read from the AST file.
  virtual void ModuleRead(serialization::SubmoduleID ID, Module *Mod) {}
  /// A module import was read from the AST file.
  virtual void ModuleImportRead(serialization::SubmoduleID ID,
                                SourceLocation ImportLoc) {}
};

class DelegatingDeserializationListener : public ASTDeserializationListener {
  ASTDeserializationListener *Previous;
  bool DeletePrevious;

public:
  explicit DelegatingDeserializationListener(
      ASTDeserializationListener *Previous, bool DeletePrevious)
      : Previous(Previous), DeletePrevious(DeletePrevious) {}
  ~DelegatingDeserializationListener() override {
    if (DeletePrevious)
      delete Previous;
  }

  DelegatingDeserializationListener(const DelegatingDeserializationListener &) =
      delete;
  DelegatingDeserializationListener &
  operator=(const DelegatingDeserializationListener &) = delete;

  void ReaderInitialized(ASTReader *Reader) override {
    if (Previous)
      Previous->ReaderInitialized(Reader);
  }
  void IdentifierRead(serialization::IdentifierID ID,
                      IdentifierInfo *II) override {
    if (Previous)
      Previous->IdentifierRead(ID, II);
  }
  void MacroRead(serialization::MacroID ID, MacroInfo *MI) override {
    if (Previous)
      Previous->MacroRead(ID, MI);
  }
  void TypeRead(serialization::TypeIdx Idx, QualType T) override {
    if (Previous)
      Previous->TypeRead(Idx, T);
  }
  void DeclRead(GlobalDeclID ID, const Decl *D) override {
    if (Previous)
      Previous->DeclRead(ID, D);
  }
  void PredefinedDeclBuilt(PredefinedDeclIDs ID, const Decl *D) override {
    if (Previous)
      Previous->PredefinedDeclBuilt(ID, D);
  }
  void SelectorRead(serialization::SelectorID ID, Selector Sel) override {
    if (Previous)
      Previous->SelectorRead(ID, Sel);
  }
  void MacroDefinitionRead(serialization::PreprocessedEntityID PPID,
                           MacroDefinitionRecord *MD) override {
    if (Previous)
      Previous->MacroDefinitionRead(PPID, MD);
  }
  void ModuleRead(serialization::SubmoduleID ID, Module *Mod) override {
    if (Previous)
      Previous->ModuleRead(ID, Mod);
  }
  void ModuleImportRead(serialization::SubmoduleID ID,
                        SourceLocation ImportLoc) override {
    if (Previous)
      Previous->ModuleImportRead(ID, ImportLoc);
  }
};

} // namespace language::Core

#endif
