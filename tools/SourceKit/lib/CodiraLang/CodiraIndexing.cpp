//===--- CodiraIndexing.cpp ------------------------------------------------===//
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

#include "CodiraASTManager.h"
#include "CodiraLangSupport.h"
#include "SourceKit/Support/Logging.h"
#include "SourceKit/Support/Tracing.h"
#include "SourceKit/Support/UIdent.h"

#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/Index/Index.h"
#include "language/Index/IndexRecord.h"
#include "language/Serialization/SerializedModuleLoader.h"
// This is included only for createLazyResolver(). Move to different header ?
#include "language/Sema/IDETypeChecking.h"
#include "toolchain/Support/Path.h"

using namespace SourceKit;
using namespace language;
using namespace language::index;

static UIdent KindImportModuleClang("source.lang.code.import.module.clang");
static UIdent KindImportModuleCodira("source.lang.code.import.module.code");

static UIdent getUIDForDependencyKind(bool isClangModule) {
  return isClangModule ? KindImportModuleClang : KindImportModuleCodira;
}

class SKIndexDataConsumer : public IndexDataConsumer {
public:
  SKIndexDataConsumer(IndexingConsumer &C) : impl(C) {}

private:
  void failed(StringRef error) override { impl.failed(error); }

  void warning(StringRef warning) override {
    LOG_WARN_FUNC(warning);
  }

  bool enableWarnings() override {
    return Logger::isLoggingEnabledForLevel(Logger::Level::Warning);
  }

  bool startDependency(StringRef name, StringRef path, bool isClangModule, bool isSystem) override {
    auto kindUID = getUIDForDependencyKind(isClangModule);
    return impl.startDependency(kindUID, name, path, isSystem);
  }

  bool finishDependency(bool isClangModule) override {
    return impl.finishDependency(getUIDForDependencyKind(isClangModule));
  }

  Action startSourceEntity(const IndexSymbol &symbol) override {
    if (symbol.symInfo.Kind == SymbolKind::Parameter)
      return Skip;

    // report any parent relations to this reference
    if (symbol.roles & (SymbolRoleSet)SymbolRole::RelationBaseOf) {
      withEntityInfo(symbol, [this](const EntityInfo &info) {
        return impl.recordRelatedEntity(info);
      });
    }

    // filter out references with invalid locations
    if (symbol.roles & (SymbolRoleSet)SymbolRole::Reference &&
        (symbol.line == 0 || symbol.column == 0))
      return Skip;


    // start the entity (ref or def)
    if (!withEntityInfo(symbol, [this](const EntityInfo &info) {
        return impl.startSourceEntity(info);
    })) return Abort;


    // report relations this occurrence has
    for (auto Relation: symbol.Relations) {
      if (Relation.roles & (SymbolRoleSet)SymbolRole::RelationOverrideOf) {
        if (!withEntityInfo(Relation, [this](const EntityInfo &info) {
          return impl.recordRelatedEntity(info);
        })) return Abort;
      }
    }
    return Continue;
  }

  bool finishSourceEntity(SymbolInfo symInfo, SymbolRoleSet roles) override {
    bool isRef = roles & (unsigned)SymbolRole::Reference;
    auto UID = CodiraLangSupport::getUIDForSymbol(symInfo, isRef);
    return impl.finishSourceEntity(UID);
  }

  std::vector<UIdent> getDeclAttributeUIDs(const Decl *decl) {
    std::vector<UIdent> uidAttrs =
      CodiraLangSupport::UIDsFromDeclAttributes(decl->getAttrs());

    // check if we should report an implicit @objc attribute
    if (!decl->getAttrs().getAttribute(DeclAttrKind::ObjC)) {
      if (auto *VD = dyn_cast<ValueDecl>(decl)) {
        if (VD->isObjC()) {
            uidAttrs.push_back(CodiraLangSupport::getUIDForObjCAttr());
        }
      }
    }
    return uidAttrs;
  }

  template <typename F>
  bool withEntityInfo(const IndexSymbol &symbol, F fn) {
    EntityInfo info;
    bool isRef = symbol.roles & (unsigned)SymbolRole::Reference;
    bool isImplicit = symbol.roles & (unsigned)SymbolRole::Implicit;

    info.Kind = CodiraLangSupport::getUIDForSymbol(symbol.symInfo, isRef);
    info.Name = isImplicit? "" : symbol.name;
    info.USR = symbol.USR;
    info.Group = symbol.group;
    info.Line = symbol.line;
    info.Column = symbol.column;
    info.ReceiverUSR = symbol.getReceiverUSR();
    info.IsDynamic = symbol.roles & (unsigned)SymbolRole::Dynamic;
    info.IsImplicit = symbol.roles & (unsigned)SymbolRole::Implicit;
    info.IsTestCandidate = symbol.symInfo.Properties & SymbolProperty::UnitTest;
    std::vector<UIdent> uidAttrs;
    if (!isRef && symbol.decl) {
      uidAttrs = getDeclAttributeUIDs(symbol.decl);
      info.Attrs = uidAttrs;
      if (auto *VD = dyn_cast<ValueDecl>(symbol.decl)) {
        if (shouldOutputEffectiveAccessOfValueSymbol(symbol.symInfo)) {
          AccessScope accessScope = VD->getFormalAccessScope();
          UIdent AttrUID = CodiraLangSupport::getUIDForFormalAccessScope(accessScope);
          info.EffectiveAccess = AttrUID;
        }
      }
    }
    return fn(info);
  }

  bool shouldOutputEffectiveAccessOfValueSymbol(SymbolInfo Info) {
    SymbolKind Kind = Info.Kind;
    SymbolSubKind SubKind = Info.SubKind;
    switch (SubKind) {
      case SymbolSubKind::AccessorGetter:
      case SymbolSubKind::AccessorSetter:
      case SymbolSubKind::CodiraAccessorWillSet:
      case SymbolSubKind::CodiraAccessorDidSet:
      case SymbolSubKind::CodiraAccessorAddressor:
      case SymbolSubKind::CodiraAccessorMutableAddressor:
      case SymbolSubKind::CodiraGenericTypeParam:
        return false;
      default:
        break;
    }
    switch (Kind) {
      case SymbolKind::Enum:
      case SymbolKind::Struct:
      case SymbolKind::Class:
      case SymbolKind::Protocol:
      case SymbolKind::Constructor:
      case SymbolKind::EnumConstant:
      case SymbolKind::Function:
      case SymbolKind::StaticMethod:
      case SymbolKind::Variable:
      case SymbolKind::InstanceMethod:
      case SymbolKind::ClassMethod:
      case SymbolKind::InstanceProperty:
      case SymbolKind::ClassProperty:
      case SymbolKind::StaticProperty:
      case SymbolKind::TypeAlias:
        return true;
     default:
        return false;
    }
  }

  template <typename F>
  bool withEntityInfo(const IndexRelation &relation, F fn) {
    EntityInfo info;
    bool isRef = (relation.roles & (unsigned)SymbolRole::Reference) ||
      (relation.roles & (unsigned)SymbolRole::RelationOverrideOf);
    info.Kind = CodiraLangSupport::getUIDForSymbol(relation.symInfo, isRef);
    info.Name = relation.name;
    info.USR = relation.USR;
    info.Group = relation.group;
    info.IsDynamic = relation.roles & (unsigned)SymbolRole::Dynamic;
    info.IsTestCandidate = relation.symInfo.Properties & SymbolProperty::UnitTest;
    std::vector<UIdent> uidAttrs;
    if (!isRef && relation.decl) {
      uidAttrs = getDeclAttributeUIDs(relation.decl);
      info.Attrs = uidAttrs;
    }
    return fn(info);
  }

private:
  IndexingConsumer &impl;
};

static void indexModule(toolchain::MemoryBuffer *Input,
                        StringRef ModuleName,
                        IndexingConsumer &IdxConsumer,
                        CompilerInstance &CI,
                        ArrayRef<const char *> Args) {
  ASTContext &Ctx = CI.getASTContext();
  std::unique_ptr<ImplicitSerializedModuleLoader> Loader;
  ModuleDecl *Mod = nullptr;
  if (ModuleName == Ctx.StdlibModuleName.str()) {
    Mod = Ctx.getModuleByIdentifier(Ctx.StdlibModuleName);
  } else {
    Loader = ImplicitSerializedModuleLoader::create(Ctx);
    auto Buf = std::unique_ptr<toolchain::MemoryBuffer>(
        toolchain::MemoryBuffer::getMemBuffer(Input->getBuffer(),
                                         Input->getBufferIdentifier()));

    // FIXME: These APIs allocate memory on the ASTContext, meaning it may not
    // be freed for a long time.
    Mod = ModuleDecl::create(Ctx.getIdentifier(ModuleName), Ctx,
                             [&](ModuleDecl *Mod, auto addFile) {
      // Indexing is not using documentation now, so don't open the module
      // documentation file.
      // FIXME: refactor the frontend to provide an easy way to figure out
      // the correct filename here.
      auto FUnit =
          Loader->loadAST(*Mod, std::nullopt, /*moduleInterfacePath=*/"",
                          /*moduleInterfaceSourcePath=*/"", std::move(Buf),
                          nullptr, nullptr,
                          /*isFramework=*/false);

      if (!FUnit) {
        Mod->setFailedToLoad();
        return;
      }

      addFile(FUnit);
      Mod->setHasResolvedImports();
    });

    // FIXME: Not knowing what went wrong is pretty bad. loadModule()
    // should be more modular, rather than emitting diagnostics itself.
    if (Mod->failedToLoad()) {
      IdxConsumer.failed("failed to load module");
      return;
    }
  }

  SKIndexDataConsumer IdxDataConsumer(IdxConsumer);
  index::indexModule(Mod, IdxDataConsumer);
}


//===----------------------------------------------------------------------===//
// IndexSource
//===----------------------------------------------------------------------===//

template <typename Str>
static void initTraceInfoImpl(trace::CodiraInvocation &CodiraArgs,
                              StringRef InputFile,
                              ArrayRef<Str> Args) {
  toolchain::raw_string_ostream OS(CodiraArgs.Args.Arguments);
  interleave(Args, [&OS](StringRef arg) { OS << arg; }, [&OS] { OS << ' '; });
  CodiraArgs.Args.PrimaryFile = InputFile.str();
}

void trace::initTraceInfo(trace::CodiraInvocation &CodiraArgs,
                          StringRef InputFile,
                          ArrayRef<const char *> Args) {
  initTraceInfoImpl(CodiraArgs, InputFile, Args);
}

void trace::initTraceInfo(trace::CodiraInvocation &CodiraArgs,
                          StringRef InputFile,
                          ArrayRef<std::string> Args) {
  initTraceInfoImpl(CodiraArgs, InputFile, Args);
}

void CodiraLangSupport::indexSource(StringRef InputFile,
                                   IndexingConsumer &IdxConsumer,
                                   ArrayRef<const char *> OrigArgs) {
  std::string Error;
  auto InputBuf =
      ASTMgr->getMemoryBuffer(InputFile, toolchain::vfs::getRealFileSystem(), Error);
  if (!InputBuf) {
    IdxConsumer.failed(Error);
    return;
  }

  StringRef Filename = toolchain::sys::path::filename(InputFile);
  StringRef FileExt = toolchain::sys::path::extension(Filename);

  bool IsModuleIndexing = (FileExt == ".codemodule" || FileExt == ".pcm");
  CompilerInstance CI;
  // Display diagnostics to stderr.
  PrintingDiagnosticConsumer PrintDiags;
  CI.addDiagnosticConsumer(&PrintDiags);

  // Add -disable-typo-correction, since the errors won't be captured in the
  // response, and it can be expensive to do typo-correction when there are many
  // errors, which is common in indexing.
  SmallVector<const char *, 16> Args(OrigArgs.begin(), OrigArgs.end());
  Args.push_back("-Xfrontend");
  Args.push_back("-disable-typo-correction");

  CompilerInvocation Invocation;
  bool Failed = true;
  if (IsModuleIndexing) {
    Failed = getASTManager()->initCompilerInvocationNoInputs(
        Invocation, Args, FrontendOptions::ActionType::Typecheck, CI.getDiags(),
        Error);
  } else {
    Failed = getASTManager()->initCompilerInvocation(
        Invocation, Args, FrontendOptions::ActionType::Typecheck, CI.getDiags(),
        InputFile, Error);
  }
  if (Failed) {
    IdxConsumer.failed(Error);
    return;
  }

  if (IsModuleIndexing) {
    std::string InstanceSetupError;
    if (CI.setup(Invocation, InstanceSetupError)) {
      IdxConsumer.failed(InstanceSetupError);
      return;
    }
    // Indexing needs IDE requests
    registerIDERequestFunctions(CI.getASTContext().evaluator);
    bool IsClangModule = (FileExt == ".pcm");
    if (IsClangModule) {
      IdxConsumer.failed("Clang module files are not supported");
      return;
    }

    indexModule(InputBuf.get(), toolchain::sys::path::stem(Filename),
                IdxConsumer, CI, Args);
    return;
  }

  if (!Invocation.getFrontendOptions().InputsAndOutputs.hasInputs()) {
    IdxConsumer.failed("no input filenames specified");
    return;
  }

  std::string InstanceSetupError;
  if (CI.setup(Invocation, InstanceSetupError)) {
    IdxConsumer.failed(InstanceSetupError);
    return;
  }
  // Indexing needs IDE requests
  registerIDERequestFunctions(CI.getASTContext().evaluator);
  trace::TracedOperation TracedOp(trace::OperationKind::IndexSource);
  if (TracedOp.enabled()) {
    trace::CodiraInvocation CodiraArgs;
    trace::initTraceInfo(CodiraArgs, InputFile, Args);
    TracedOp.start(CodiraArgs);
  }

  CI.performSema();

  // NOTE: performSema() may end up with some gruesome error preventing it from
  // setting primary file correctly
  if (!CI.getPrimarySourceFile()) {
    IdxConsumer.failed("no primary source file found");
    return;
  }
  
  SKIndexDataConsumer IdxDataConsumer(IdxConsumer);
  index::indexSourceFile(CI.getPrimarySourceFile(), IdxDataConsumer);
}

static void emitIndexDataForSourceFile(SourceFile &PrimarySourceFile,
                                       IndexStoreOptions IndexOpts,
                                       const CompilerInstance &Instance) {
  const auto &Invocation = Instance.getInvocation();
//  FIXME: Compiler arguments should be the default for setting options, but this is currently broken (see PR #69076)
//  const auto &Opts = Invocation.getFrontendOptions();

  bool isDebugCompilation;
  switch (Invocation.getSILOptions().OptMode) {
    case OptimizationMode::NotSet:
    case OptimizationMode::NoOptimization:
      isDebugCompilation = true;
      break;
    case OptimizationMode::ForSpeed:
    case OptimizationMode::ForSize:
      isDebugCompilation = false;
      break;
  }

  (void) index::indexAndRecord(&PrimarySourceFile,
                               IndexOpts.IndexUnitOutputPath,
                               IndexOpts.IndexStorePath,
                               !IndexOpts.IgnoreClangModules,
                               IndexOpts.IncludeSystemModules,
                               IndexOpts.IgnoreStdlib,
                               IndexOpts.IncludeLocals,
                               isDebugCompilation,
                               IndexOpts.DisableImplicitModules,
                               Invocation.getTargetTriple(),
                               *Instance.getDependencyTracker(),
                               Invocation.getIRGenOptions().FilePrefixMap);
}

void CodiraLangSupport::indexToStore(
    StringRef PrimaryFilePath, ArrayRef<const char *> Args,
    IndexStoreOptions Opts,
    SourceKitCancellationToken CancellationToken,
    IndexToStoreReceiver Receiver) {
  std::string Error;
  CodiraInvocationRef Invok =
      ASTMgr->getTypecheckInvocation(Args, PrimaryFilePath, Error);
  if (!Invok) {
    LOG_WARN_FUNC("failed to create an ASTInvocation: " << Error);
    Receiver(RequestResult<IndexStoreInfo>::fromError(Error));
    return;
  }

  struct IndexStoreASTConsumer : public CodiraASTConsumer {
    IndexToStoreReceiver Receiver;
    IndexStoreOptions Opts;

    IndexStoreASTConsumer(IndexToStoreReceiver Receiver, IndexStoreOptions Opts)
        : Receiver(std::move(Receiver)), Opts(std::move(Opts)) {}

    void handlePrimaryAST(ASTUnitRef AstUnit) override {
      auto &SF = AstUnit->getPrimarySourceFile();
      auto &CI = AstUnit->getCompilerInstance();
      emitIndexDataForSourceFile(SF, Opts, CI);
      Receiver(RequestResult<IndexStoreInfo>::fromResult(IndexStoreInfo{}));
    }

    void cancelled() override {
      Receiver(RequestResult<IndexStoreInfo>::cancelled());
    }

    void failed(StringRef Error) override {
      Receiver(RequestResult<IndexStoreInfo>::fromError(Error));
    }
  };

  auto ASTConsumer = std::make_shared<IndexStoreASTConsumer>(std::move(Receiver), std::move(Opts));
  getASTManager()->processASTAsync(Invok, ASTConsumer,
                                   /*OncePerASTToken=*/nullptr,
                                   CancellationToken,
                                   toolchain::vfs::getRealFileSystem());
}
