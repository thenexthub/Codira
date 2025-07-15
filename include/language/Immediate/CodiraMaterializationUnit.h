//===--- CodiraMaterializationUnit.h - JIT Codira ASTs ------------*- C++ -*-===//
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
// Defines the `CodiraMaterializationUnit` class, which allows you to JIT
// individual Codira AST declarations.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IMMEDIATE_LANGUAGEMATERIALIZATIONUNIT_H
#define LANGUAGE_IMMEDIATE_LANGUAGEMATERIALIZATIONUNIT_H

#include <memory>

#include "toolchain/ADT/StringRef.h"
#include "toolchain/ExecutionEngine/JITLink/JITLink.h"
#include "toolchain/ExecutionEngine/Orc/Core.h"
#include "toolchain/ExecutionEngine/Orc/EPCIndirectionUtils.h"
#include "toolchain/ExecutionEngine/Orc/IndirectionUtils.h"
#include "toolchain/ExecutionEngine/Orc/LLJIT.h"
#include "toolchain/ExecutionEngine/Orc/LazyReexports.h"
#include "toolchain/ExecutionEngine/Orc/SymbolStringPool.h"

#include "language/AST/TBDGenRequests.h"

namespace language {

class CompilerInstance;

/// A JIT stack able to lazily JIT Codira programs
class CodiraJIT {
public:
  CodiraJIT(const CodiraJIT &) = delete;
  CodiraJIT(CodiraJIT &&) = delete;
  CodiraJIT &operator=(const CodiraJIT &) = delete;
  CodiraJIT &operator=(CodiraJIT &&) = delete;

  /// Attempt to create and initialize a new `CodiraJIT` with lazy compilation
  /// enabled and an attached generator to search for symbols defined in the
  /// current process.
  static toolchain::Expected<std::unique_ptr<CodiraJIT>> Create(CompilerInstance &CI);

  /// Adds a plugin that will rename function symbols for lazy reexports.
  /// Should be called only once.
  void addRenamer();

  ~CodiraJIT();

  /// Get the dylib associated with the main program
  toolchain::orc::JITDylib &getMainJITDylib();

  /// Register a the materialization unit `MU` with the `JITDylib``JD` and
  /// create lazy reexports for all functions defined in the interface of `MU`
  toolchain::Error addCodira(toolchain::orc::JITDylib &JD,
                       std::unique_ptr<toolchain::orc::MaterializationUnit> MU);

  /// Return a linker-mangled version of `Name`
  std::string mangle(toolchain::StringRef Name);

  /// Add a symbol name to the underlying `SymbolStringPool` and return
  /// a pointer to it
  toolchain::orc::SymbolStringPtr intern(toolchain::StringRef Name);

  /// Return a linker-mangled version of `Name` and intern the result
  toolchain::orc::SymbolStringPtr mangleAndIntern(toolchain::StringRef Name);

  /// Get the `IRCompileLayer` associated with this `CodiraJIT`
  toolchain::orc::IRCompileLayer &getIRCompileLayer();

  /// Get the `ObjectTransformLayer` associated with this `CodiraJIT`
  toolchain::orc::ObjectTransformLayer &getObjTransformLayer();

  /// Initialize the main `JITDylib`, lookup the main symbol, execute it,
  /// deinitialize the main `JITDylib`, and return the exit code of the
  /// JIT'd program
  toolchain::Expected<int> runMain(toolchain::ArrayRef<std::string> Args);

private:
  static toolchain::Expected<std::unique_ptr<toolchain::orc::LLJIT>>
  CreateLLJIT(CompilerInstance &CI);

  /// An ORC layer to rename the names of function bodies to support lazy
  /// reexports
  class Plugin : public toolchain::orc::ObjectLinkingLayer::Plugin {
    void
    modifyPassConfig(toolchain::orc::MaterializationResponsibility &MR,
                     toolchain::jitlink::LinkGraph &G,
                     toolchain::jitlink::PassConfiguration &PassConfig) override;

    toolchain::Error
    notifyFailed(toolchain::orc::MaterializationResponsibility &MR) override;

    toolchain::Error notifyRemovingResources(toolchain::orc::JITDylib &JD,
                                        toolchain::orc::ResourceKey K) override;

    void notifyTransferringResources(toolchain::orc::JITDylib &JD,
                                     toolchain::orc::ResourceKey DstKey,
                                     toolchain::orc::ResourceKey SrcKey) override;
  };

  static void handleLazyCompilationFailure();

  CodiraJIT(std::unique_ptr<toolchain::orc::LLJIT> J,
           std::unique_ptr<toolchain::orc::EPCIndirectionUtils> EPCIU);

  std::unique_ptr<toolchain::orc::LLJIT> J;
  std::unique_ptr<toolchain::orc::EPCIndirectionUtils> EPCIU;
  toolchain::orc::LazyCallThroughManager &LCTM;
  std::unique_ptr<toolchain::orc::IndirectStubsManager> ISM;
};

/// Lazily JITs a Codira AST using function at a time compilation
class LazyCodiraMaterializationUnit : public toolchain::orc::MaterializationUnit {
public:
  /// Create a new `LazyCodiraMaterializationUnit` with the associated
  /// JIT stack `JIT` and compiler instance `CI`
  static std::unique_ptr<LazyCodiraMaterializationUnit>
  Create(CodiraJIT &JIT, CompilerInstance &CI);

  toolchain::StringRef getName() const override;

private:
  LazyCodiraMaterializationUnit(CodiraJIT &JIT, CompilerInstance &CI,
                               const SymbolSourceMap *Sources,
                               toolchain::orc::SymbolFlagsMap Symbols);
  void materialize(
      std::unique_ptr<toolchain::orc::MaterializationResponsibility> MR) override;

  void discard(const toolchain::orc::JITDylib &JD,
               const toolchain::orc::SymbolStringPtr &Sym) override;

  const SymbolSourceMap *Sources;
  CodiraJIT &JIT;
  CompilerInstance &CI;
};

/// Eagerly materializes a whole `SILModule`
class EagerCodiraMaterializationUnit : public toolchain::orc::MaterializationUnit {
public:
  /// Create a new `EagerCodiraMaterializationUnit` with the JIT stack `JIT`
  /// and provided compiler options
  EagerCodiraMaterializationUnit(CodiraJIT &JIT, const CompilerInstance &CI,
                                const IRGenOptions &IRGenOpts,
                                std::unique_ptr<SILModule> SM);

  StringRef getName() const override;

private:
  void materialize(
      std::unique_ptr<toolchain::orc::MaterializationResponsibility> MR) override;

  /// Get the linker-level interface defined by the `SILModule` being
  /// materialized
  static MaterializationUnit::Interface
  getInterface(CodiraJIT &JIT, const CompilerInstance &CI);

  void dumpJIT(const toolchain::Module &Module);

  void discard(const toolchain::orc::JITDylib &JD,
               const toolchain::orc::SymbolStringPtr &Sym) override;

  CodiraJIT &JIT;
  const CompilerInstance &CI;
  const IRGenOptions &IRGenOpts;
  std::unique_ptr<SILModule> SM;
};

} // end namespace language

#endif // LANGUAGE_IMMEDIATE_LANGUAGEMATERIALIZATIONUNIT_H
