//===--- DebuggerClient.h - Interfaces LLDB uses for parsing ----*- C++ -*-===//
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
//
// This file defines the abstract DebuggerClient class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_DEBUGGERCLIENT_H
#define LANGUAGE_DEBUGGERCLIENT_H

#include "language/AST/NameLookup.h"

namespace language {

class SILDebuggerClient;

class DebuggerClient {
protected:
  ASTContext &Ctx;
public:
  typedef SmallVectorImpl<LookupResultEntry> ResultVector;

  DebuggerClient(ASTContext &C) : Ctx(C) { }
  virtual ~DebuggerClient() = default;
  
  // DebuggerClient is consulted at the beginning of the parsing
  // of various DeclKinds to see whether the decl should be parsed
  // in the global context rather than the current context.
  // This question will only be asked if the decl's current context
  // is a function marked with the LLDBDebuggerFunction attribute.
  virtual bool shouldGlobalize(Identifier Name, DeclKind kind) = 0;
  
  virtual void didGlobalize (Decl *Decl) = 0;

  /// DebuggerClient is consulted at two times during name
  /// lookup.  This is the first time: after all names in a
  /// source file have been checked but before external
  /// Modules are checked.  The results in the ResultVector will
  /// be consulted first.  Return true if results have been added
  /// to RV.
  /// FIXME: I don't think this ever does anything useful.
  virtual bool lookupOverrides(DeclBaseName Name, DeclContext *DC,
                               SourceLoc Loc, bool IsTypeLookup,
                               ResultVector &RV) = 0;

  /// This is the second time DebuggerClient is consulted:
  /// after all names in external Modules are checked, the client
  /// gets a chance to add names to the list of candidates that
  /// have been found in the external module lookup.

  virtual bool lookupAdditions(DeclBaseName Name, DeclContext *DC,
                               SourceLoc Loc, bool IsTypeLookup,
                               ResultVector &RV) = 0;

  /// The following functions allow the debugger to modify the results of a
  /// qualified lookup as needed. These methods may add, remove or modify the
  /// entries in `decls`. See the corresponding DeclContext::lookupInXYZ
  /// functions defined in NameLookup.cpp for more context.
  ///

  virtual void finishLookupInNominals(const DeclContext *dc,
                                      ArrayRef<NominalTypeDecl *> types,
                                      DeclName member, NLOptions options,
                                      SmallVectorImpl<ValueDecl *> &decls) {}

  virtual void finishLookupInModule(const DeclContext *dc, ModuleDecl *module,
                                    DeclName member, NLOptions options,
                                    SmallVectorImpl<ValueDecl *> &decls) {}

  virtual void finishLookupInAnyObject(const DeclContext *dc, DeclName member,
                                       NLOptions options,
                                       SmallVectorImpl<ValueDecl *> &decls) {}

  /// When evaluating an expression in the context of an existing source file,
  /// we may want to prefer declarations from that source file.
  /// The DebuggerClient can return a private-discriminator to tell lookup to
  /// prefer these certain decls.
  virtual Identifier getPreferredPrivateDiscriminator() = 0;

  virtual SILDebuggerClient *getAsSILDebuggerClient() = 0;
private:
  virtual void anchor();
};

} // namespace language

#endif
