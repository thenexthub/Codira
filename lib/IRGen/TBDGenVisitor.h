//===--- TBDGenVisitor.h - AST Visitor for TBD generation -----------------===//
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
//  This file defines the visitor that finds all symbols in a language AST.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_TBDGEN_TBDGENVISITOR_H
#define LANGUAGE_TBDGEN_TBDGENVISITOR_H

#include "language/AST/ASTMangler.h"
#include "language/AST/FileUnit.h"
#include "language/AST/Module.h"
#include "language/Basic/Toolchain.h"
#include "language/IRGen/IRSymbolVisitor.h"
#include "language/IRGen/Linking.h"
#include "language/SIL/SILDeclRef.h"
#include "toolchain/ADT/StringSet.h"
#include "toolchain/TargetParser/Triple.h"
#include "toolchain/TextAPI/InterfaceFile.h"

using namespace language::irgen;
using StringSet = toolchain::StringSet<>;

namespace toolchain {
class DataLayout;
}

namespace language {

class TBDGenDescriptor;
struct TBDGenOptions;
class SymbolSource;

namespace tbdgen {

enum class LinkerPlatformId: uint8_t {
#define LD_PLATFORM(Name, Id) Name = Id,
#include "ldPlatformKinds.def"
};

struct InstallNameStore {
  // The default install name to use when no specific install name is specified.
  std::string InstallName;
  // The install name specific to the platform id. This takes precedence over
  // the default install name.
  std::map<LinkerPlatformId, std::string> PlatformInstallName;
  StringRef getInstallName(LinkerPlatformId Id) const;
};

/// A set of callbacks for recording APIs.
class APIRecorder {
public:
  virtual ~APIRecorder() {}

  virtual void addSymbol(StringRef name, toolchain::MachO::EncodeKind kind,
                         SymbolSource source, Decl *decl,
                         toolchain::MachO::SymbolFlags flags) {}
  virtual void addObjCInterface(const ClassDecl *decl) {}
  virtual void addObjCCategory(const ExtensionDecl *decl) {}
  virtual void addObjCMethod(const GenericContext *ctx, SILDeclRef method) {}
};

class SimpleAPIRecorder final : public APIRecorder {
public:
  using SymbolCallbackFn =
      toolchain::function_ref<void(StringRef, toolchain::MachO::EncodeKind, SymbolSource,
                              Decl *, toolchain::MachO::SymbolFlags)>;

  SimpleAPIRecorder(SymbolCallbackFn fn) : fn(fn) {}

  void addSymbol(StringRef symbol, toolchain::MachO::EncodeKind kind,
                 SymbolSource source, Decl *decl,
                 toolchain::MachO::SymbolFlags flags) override {
    fn(symbol, kind, source, decl, flags);
  }

private:
  SymbolCallbackFn fn;
};

class TBDGenVisitor : public IRSymbolVisitor {
#ifndef NDEBUG
  /// Tracks the symbols emitted to ensure we don't emit any duplicates.
  toolchain::StringSet<> DuplicateSymbolChecker;
#endif

  std::optional<toolchain::DataLayout> DataLayout = std::nullopt;
  const StringRef DataLayoutDescription;

  UniversalLinkageInfo UniversalLinkInfo;
  ModuleDecl *CodiraModule;
  const TBDGenOptions &Opts;
  APIRecorder &recorder;

  using EncodeKind = toolchain::MachO::EncodeKind;
  using SymbolFlags = toolchain::MachO::SymbolFlags;

  std::vector<Decl*> DeclStack;
  std::unique_ptr<std::map<std::string, InstallNameStore>>
    previousInstallNameMap;
  std::unique_ptr<std::map<std::string, InstallNameStore>>
    parsePreviousModuleInstallNameMap();
  void addSymbolInternal(StringRef name, EncodeKind kind, SymbolSource source,
                         SymbolFlags);
  void addLinkerDirectiveSymbolsLdHide(StringRef name, EncodeKind kind);
  void addLinkerDirectiveSymbolsLdPrevious(StringRef name, EncodeKind kind);
  void addSymbol(StringRef name, SymbolSource source, SymbolFlags flags,
                 EncodeKind kind = EncodeKind::GlobalSymbol);

  bool addClassMetadata(ClassDecl *CD);

public:
  TBDGenVisitor(const toolchain::Triple &target, const StringRef dataLayoutString,
                ModuleDecl *languageModule, const TBDGenOptions &opts,
                APIRecorder &recorder)
      : DataLayoutDescription(dataLayoutString),
        UniversalLinkInfo(target, opts.HasMultipleIGMs, /*forcePublic*/ false,
                          /*static=*/false, /*mergeableSymbols*/false),
        CodiraModule(languageModule), Opts(opts), recorder(recorder),
        previousInstallNameMap(parsePreviousModuleInstallNameMap()) {}

  /// Create a new visitor using the target and layout information from a
  /// TBDGenDescriptor.
  TBDGenVisitor(const TBDGenDescriptor &desc, APIRecorder &recorder);

  ~TBDGenVisitor() { assert(DeclStack.empty()); }

  /// Adds the global symbols associated with the first file.
  void addFirstFileSymbols();

  /// Visit the files specified by a given TBDGenDescriptor.
  void visit(const TBDGenDescriptor &desc);

  // --- IRSymbolVisitor ---

  bool willVisitDecl(Decl *D) override;
  void didVisitDecl(Decl *D) override;

  void addFunction(SILDeclRef declRef) override;
  void addFunction(StringRef name, SILDeclRef declRef) override;
  void addGlobalVar(VarDecl *VD) override;
  void addLinkEntity(LinkEntity entity) override;
  void addObjCInterface(ClassDecl *CD) override;
  void addObjCMethod(AbstractFunctionDecl *AFD) override;
  void addProtocolWitnessThunk(RootProtocolConformance *C,
                               ValueDecl *requirementDecl) override;
};
} // end namespace tbdgen
} // end namespace language

#endif
