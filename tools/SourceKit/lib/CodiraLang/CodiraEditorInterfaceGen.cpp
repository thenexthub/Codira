//===--- CodiraEditorInterfaceGen.cpp --------------------------------------===//
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

#include "CodiraLangSupport.h"
#include "CodiraInterfaceGenContext.h"
#include "CodiraASTManager.h"

#include "language/AST/ASTPrinter.h"
#include "language/AST/ASTWalker.h"
#include "language/Basic/Version.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/IDE/ModuleInterfacePrinting.h"
#include "language/IDE/SyntaxModel.h"
#include "language/IDETool/CompilerInvocation.h"
#include "language/Parse/ParseVersion.h"
#include "language/Strings.h"

#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/ConvertUTF.h"

using namespace SourceKit;
using namespace language;
using namespace ide;

class CodiraInterfaceGenContext::Implementation {
public:
  struct TextRange {
    unsigned Offset;
    unsigned Length;
  };

  struct TextReference {
    /// The declaration from the module.
    const ValueDecl *Dcl = nullptr;
    const ModuleEntity Mod;
    TextRange Range;

    TextReference(const ValueDecl *D, unsigned Offset, unsigned Length)
      : Dcl(D), Mod(), Range{Offset, Length} {}
    TextReference(const ModuleEntity Mod, unsigned Offset, unsigned Length)
    : Mod(Mod), Range{Offset, Length} {}
  };

  struct TextDecl {
    /// The declaration from the module.
    const Decl *Dcl = nullptr;
    /// The range in the interface source.
    TextRange Range;
    /// The USR, if the declaration has one
    StringRef USR;

    TextDecl(const Decl *D, TextRange Range)
      : Dcl(D), Range(Range) {}
    TextDecl() = default;
  };

  struct SourceTextInfo {
    std::string Text;
    std::vector<TextReference> References;
    std::vector<TextDecl> Decls;
    /// Do not clear the map or erase elements,
    /// without keeping the Decls vector in sync
    toolchain::StringMap<TextDecl> USRMap;
  };

  // Hold an AstUnit so that the Decl* we have are always valid.
  ASTUnitRef AstUnit;
  std::string DocumentName;
  bool IsModule = false;
  std::string ModuleOrHeaderName;
  CompilerInvocation Invocation;
  PrintingDiagnosticConsumer DiagConsumer;
  CompilerInstance Instance;
  ModuleDecl *Mod = nullptr;
  SourceTextInfo Info;
  // This is the non-typechecked AST for the generated interface source.
  CompilerInstance TextCI;
  // Synchronize access to the embedded compiler instance (if we don't have an
  // ASTUnit).
  WorkQueue Queue{WorkQueue::Dequeuing::Serial,
                  "sourcekit.code.InterfaceGenContext"};
};

typedef CodiraInterfaceGenContext::Implementation::TextRange TextRange;
typedef CodiraInterfaceGenContext::Implementation::TextReference TextReference;
typedef CodiraInterfaceGenContext::Implementation::TextDecl TextDecl;
typedef CodiraInterfaceGenContext::Implementation::SourceTextInfo SourceTextInfo;

namespace {
class AnnotatingPrinter : public StreamPrinter {
  SourceTextInfo &Info;

  struct DeclUSR {
    const Decl *Dcl = nullptr;
    std::string USR;
    DeclUSR(const Decl *D, StringRef USR) : Dcl(D), USR(USR) {}
  };
  std::vector<DeclUSR> DeclUSRs;

  // For members of a synthesized extension, we should append the USR of the
  // synthesize target to the original USR.
  std::string TargetUSR;

public:
  AnnotatingPrinter(SourceTextInfo &Info, toolchain::raw_ostream &OS)
    : StreamPrinter(OS), Info(Info) { }

  ~AnnotatingPrinter() override {
    assert(DeclUSRs.empty() && "unmatched printDeclLoc call ?");
  }

  void
  printSynthesizedExtensionPre(const ExtensionDecl *ED,
                               TypeOrExtensionDecl Target,
                               std::optional<BracketOptions> Bracket) override {
    // When we start print a synthesized extension, record the target's USR.
    toolchain::SmallString<64> Buf;
    toolchain::raw_svector_ostream OS(Buf);
    auto TargetNTD = Target.getBaseNominal();
    if (!CodiraLangSupport::printUSR(TargetNTD, OS)) {
      TargetUSR = std::string(OS.str());
    }
  }

  void printSynthesizedExtensionPost(
      const ExtensionDecl *ED, TypeOrExtensionDecl Target,
      std::optional<BracketOptions> Bracket) override {
    // When we leave a synthesized extension, clear target's USR.
    TargetUSR = "";
  }

  void printDeclLoc(const Decl *D) override {
    unsigned LocOffset = OS.tell();
    TextDecl Entry(D, TextRange{LocOffset, 0});
    Info.Decls.emplace_back(Entry);

    if (auto VD = dyn_cast<ValueDecl>(D)) {
      // Only record non-local USRs.
      if (D->getDeclContext()->getLocalContext())
        return;

      toolchain::SmallString<64> Buf;
      toolchain::raw_svector_ostream OS(Buf);
      if (!CodiraLangSupport::printUSR(VD, OS)) {

        // Append target's USR if this is a member of a synthesized extension.
        if (!TargetUSR.empty()) {
          OS << LangSupport::SynthesizedUSRSeparator;
          OS << TargetUSR;
        }
        StringRef USR = OS.str();
        DeclUSRs.emplace_back(VD, USR);
        auto iterator = Info.USRMap.insert_or_assign(USR, Entry).first;
        // Set the USR in the declarations to the key in the USRMap, because the
        // lifetime of that matches/exceeds the lifetime of Decls. String keys
        // in the StringMap are heap allocated and only get destroyed on
        // explicit erase() or clear() calls, or on destructor calls (the
        // Programmer's Manual description itself also states that StringMap
        // "only ever copies a string if a value is inserted").
        // Thus this never results in a dangling reference, as the USRMap is
        // never cleared and no elements are erased in its lifetime.
        Info.Decls.back().USR = iterator->getKey();
      }
    }
  }

  void printDeclNameOrSignatureEndLoc(const Decl *D) override {
    unsigned Offset = OS.tell();

    if (!Info.Decls.empty() && Info.Decls.back().Dcl == D) {
      TextDecl &Entry = Info.Decls.back();
      Entry.Range.Length = Offset - Entry.Range.Offset;
    }

    if (!DeclUSRs.empty() && DeclUSRs.back().Dcl == D) {
      TextDecl &Entry = Info.USRMap[DeclUSRs.back().USR];
      assert(Entry.Dcl == D);
      Entry.Range.Length = Offset - Entry.Range.Offset;
      DeclUSRs.pop_back();
    }
  }

  void printTypeRef(
      Type T, const TypeDecl *TD, Identifier Name,
      PrintNameContext NameContext = PrintNameContext::Normal) override {
    unsigned StartOffset = OS.tell();
    Info.References.emplace_back(TD, StartOffset, Name.str().size());
    StreamPrinter::printTypeRef(T, TD, Name, NameContext);
  }

  void printModuleRef(ModuleEntity Mod, Identifier Name) override {
    unsigned StartOffset = OS.tell();
    Info.References.emplace_back(Mod, StartOffset, Name.str().size());
    StreamPrinter::printModuleRef(Mod, Name);
  }
};

class DocSyntaxWalker : public SyntaxModelWalker {
  SourceManager &SM;
  unsigned BufferID;
  EditorConsumer &Consumer;

public:
  DocSyntaxWalker(SourceManager &SM, unsigned BufferID,
                  EditorConsumer &Consumer)
    : SM(SM), BufferID(BufferID), Consumer(Consumer) {}

  bool walkToNodePre(SyntaxNode Node) override {
    unsigned Offset = SM.getLocOffsetInBuffer(Node.Range.getStart(), BufferID);
    unsigned Length = Node.Range.getByteLength();

    UIdent UID = CodiraLangSupport::getUIDForSyntaxNodeKind(Node.Kind);
    if (UID.isValid())
      Consumer.handleSyntaxMap(Offset, Length, UID);
    return true;
  }
};

} // end anonymous namespace

static bool makeParserAST(CompilerInstance &CI, StringRef Text,
                          CompilerInvocation Invocation) {
  Invocation.getFrontendOptions().InputsAndOutputs.clearInputs();
  Invocation.setModuleName("main");
  Invocation.getLangOptions().DisablePoundIfEvaluation = true;

  std::unique_ptr<toolchain::MemoryBuffer> Buf;
  Buf = toolchain::MemoryBuffer::getMemBuffer(Text, "<module-interface>");
  Invocation.getFrontendOptions().InputsAndOutputs.addInput(
      InputFile(Buf.get()->getBufferIdentifier(), /*isPrimary*/false, Buf.get(),
                file_types::TY_Codira));
  std::string InstanceSetupError;
  return CI.setup(Invocation, InstanceSetupError);
}

static void reportSyntacticAnnotations(CompilerInstance &CI,
                                       EditorConsumer &Consumer) {
  auto SF = dyn_cast<SourceFile>(CI.getMainModule()->getFiles()[0]);
  SyntaxModelContext SyntaxContext(*SF);
  DocSyntaxWalker SyntaxWalker(CI.getSourceMgr(), SF->getBufferID(),
                               Consumer);
  SyntaxContext.walk(SyntaxWalker);
}

static void reportDocumentStructure(CompilerInstance &CI,
                                    EditorConsumer &Consumer) {
  auto SF = dyn_cast<SourceFile>(CI.getMainModule()->getFiles()[0]);
  CodiraEditorDocument::reportDocumentStructure(*SF, Consumer);
}

static void reportSemanticAnnotations(const SourceTextInfo &IFaceInfo,
                                      EditorConsumer &Consumer) {
  for (auto &Ref : IFaceInfo.References) {
    UIdent Kind;
    bool IsSystem;
    if (Ref.Mod) {
      Kind = CodiraLangSupport::getUIDForModuleRef();
      IsSystem = Ref.Mod.isNonUserModule();
    } else if (Ref.Dcl) {
      Kind = CodiraLangSupport::getUIDForDecl(Ref.Dcl, /*IsRef=*/true);
      IsSystem = Ref.Dcl->getModuleContext()->isNonUserModule();
    }
    if (Kind.isInvalid())
      continue;
    unsigned Offset = Ref.Range.Offset;
    unsigned Length = Ref.Range.Length;
    Consumer.handleSemanticAnnotation(Offset, Length, Kind, IsSystem);
  }
}

/// Create the declarations array (sourcekitd::DeclarationsArrayBuilder) from
/// the SourceTextInfo about declarations
static void reportDeclarations(const SourceTextInfo &IFaceInfo,
                               EditorConsumer &Consumer) {
  for (auto &Dcl : IFaceInfo.Decls) {
    if (!Dcl.Dcl)
      continue;
    UIdent Kind = CodiraLangSupport::getUIDForDecl(Dcl.Dcl);
    if (Kind.isInvalid())
      continue;
    Consumer.handleDeclaration(Dcl.Range.Offset, Dcl.Range.Length, Kind,
                               Dcl.USR);
  }
}

namespace {
/// A diagnostic consumer that picks up module loading errors.
class ModuleLoadingErrorConsumer final : public DiagnosticConsumer {
  toolchain::SmallVector<std::string, 2> DiagMessages;

  void handleDiagnostic(SourceManager &SM,
                        const DiagnosticInfo &Info) override {
    // Only record errors for now. In the future it might be useful to pick up
    // some notes, but some notes are just noise.
    if (Info.Kind != DiagnosticKind::Error)
      return;

    std::string Message;
    {
      toolchain::raw_string_ostream Out(Message);
      DiagnosticEngine::formatDiagnosticText(Out, Info.FormatString,
                                             Info.FormatArgs);
    }
    // We're only interested in the first and last errors. For a clang module
    // failure, the first error will be the reason why the module failed to
    // load, and the last error will be a generic "could not build Obj-C module"
    // error. For a Codira module, we'll typically only emit one error.
    //
    // NOTE: Currently when loading transitive dependencies for a Codira module,
    // we'll only diagnose the root failure, and not record the error for the
    // top-level module failure, as we stop emitting errors after a fatal error
    // has been recorded. This is currently fine for our use case though, as
    // we already include the top-level module name in the error we hand back.
    if (DiagMessages.size() < 2) {
      DiagMessages.emplace_back(std::move(Message));
    } else {
      DiagMessages.back() = std::move(Message);
    }
  }

public:
  ArrayRef<std::string> getDiagMessages() { return DiagMessages; }
};
} // end anonymous namespace

static bool getModuleInterfaceInfo(
    ASTContext &Ctx, StringRef ModuleName, std::optional<StringRef> Group,
    CodiraInterfaceGenContext::Implementation &Impl, std::string &ErrMsg,
    bool SynthesizedExtensions, std::optional<StringRef> InterestedUSR) {
  ModuleDecl *&Mod = Impl.Mod;
  SourceTextInfo &Info = Impl.Info;

  if (ModuleName.empty()) {
    ErrMsg = "Module name is empty";
    return true;
  }

  // Get the (sub)module to generate, recording the errors emitted.
  ModuleLoadingErrorConsumer DiagConsumer;
  {
    DiagnosticConsumerRAII R(Ctx.Diags, DiagConsumer);
    Mod = Ctx.getModuleByName(ModuleName);
  }

  // Check to see if we either couldn't find the module, or we failed to load
  // it, and report an error message back that includes the diagnostics we
  // collected, which should help pinpoint what the issue was. Note we do this
  // even if `Mod` is null, as the clang importer currently returns nullptr
  // when a module fails to load, and there may be interesting errors to
  // collect there.
  // Note that us failing here also means the caller may retry with e.g C++
  // interoperability enabled.
  if (!Mod || Mod->failedToLoad()) {
    toolchain::raw_string_ostream OS(ErrMsg);

    OS << "Could not load module: ";
    OS << ModuleName;
    auto ModuleErrs = DiagConsumer.getDiagMessages();
    if (!ModuleErrs.empty()) {
      // We print the errors in reverse, as they are typically emitted in
      // a bottom-up manner by module loading, and a top-down presentation
      // makes more sense.
      OS << " (";
      toolchain::interleaveComma(toolchain::reverse(ModuleErrs), OS);
      OS << ")";
    }
    return true;
  }

  PrintOptions Options = PrintOptions::printModuleInterface(
      Ctx.TypeCheckerOpts.PrintFullConvention);
  if (Mod->findUnderlyingClangModule()) {
    if (Ctx.LangOpts.EnableCXXInterop) {
      // Show unavailable C++ APIs.
      Options.SkipUnavailable = false;
      // Skip over inline namespaces.
      Options.SkipInlineCXXNamespace = true;
    }
  }
  // Skip submodules but include any exported modules that have the same public
  // module name as this module.
  ModuleTraversalOptions TraversalOptions =
      ModuleTraversal::VisitMatchingExported;

  SmallString<128> Text;
  toolchain::raw_svector_ostream OS(Text);
  AnnotatingPrinter Printer(Info, OS);
  if (!Group && InterestedUSR) {
    Group = findGroupNameForUSR(Mod, InterestedUSR.value());
  }
  printModuleInterface(Mod,
                       Group.has_value() ? toolchain::ArrayRef(Group.value())
                                         : ArrayRef<StringRef>(),
                       TraversalOptions, Printer, Options,
                       Group.has_value() && SynthesizedExtensions);

  Info.Text = std::string(OS.str());
  return false;
}

static bool getHeaderInterfaceInfo(ASTContext &Ctx,
                                   StringRef HeaderName,
                                   SourceTextInfo &Info,
                                   std::string &ErrMsg) {
  if (HeaderName.empty()) {
    ErrMsg = "Header name is empty";
    return true;
  }

  PrintOptions Options = PrintOptions::printModuleInterface(
      Ctx.TypeCheckerOpts.PrintFullConvention);

  SmallString<128> Text;
  toolchain::raw_svector_ostream OS(Text);
  AnnotatingPrinter Printer(Info, OS);
  printHeaderInterface(HeaderName, Ctx, Printer, Options);

  Info.Text = std::string(OS.str());
  return false;
}

CodiraInterfaceGenContextRef
CodiraInterfaceGenContext::createForCodiraSource(StringRef DocumentName,
                                               StringRef SourceFileName,
                                               ASTUnitRef AstUnit,
                                               CompilerInvocation Invocation,
                                               std::string &ErrMsg) {
  CodiraInterfaceGenContextRef IFaceGenCtx{ new CodiraInterfaceGenContext() };
  IFaceGenCtx->Impl.DocumentName = DocumentName.str();
  IFaceGenCtx->Impl.IsModule = true;
  IFaceGenCtx->Impl.ModuleOrHeaderName = SourceFileName.str();
  IFaceGenCtx->Impl.AstUnit = AstUnit;

  PrintOptions Options = PrintOptions::printCodiraFileInterface(
      Invocation.getFrontendOptions().PrintFullConvention);
  SmallString<128> Text;
  toolchain::raw_svector_ostream OS(Text);
  AnnotatingPrinter Printer(IFaceGenCtx->Impl.Info, OS);
  printCodiraSourceInterface(AstUnit->getPrimarySourceFile(), Printer, Options);
  IFaceGenCtx->Impl.Info.Text = std::string(OS.str());
  if (makeParserAST(IFaceGenCtx->Impl.TextCI, IFaceGenCtx->Impl.Info.Text,
                    Invocation)) {
    ErrMsg = "Error during syntactic parsing";
    return nullptr;
  }
  return IFaceGenCtx;
}

CodiraInterfaceGenContextRef CodiraInterfaceGenContext::create(
    StringRef DocumentName, bool IsModule, StringRef ModuleOrHeaderName,
    std::optional<StringRef> Group, CompilerInvocation Invocation,
    std::string &ErrMsg, bool SynthesizedExtensions,
    std::optional<StringRef> InterestedUSR) {
  CodiraInterfaceGenContextRef IFaceGenCtx{ new CodiraInterfaceGenContext() };
  IFaceGenCtx->Impl.DocumentName = DocumentName.str();
  IFaceGenCtx->Impl.IsModule = IsModule;
  IFaceGenCtx->Impl.ModuleOrHeaderName = ModuleOrHeaderName.str();
  IFaceGenCtx->Impl.Invocation = Invocation;
  CompilerInstance &CI = IFaceGenCtx->Impl.Instance;

  // Display diagnostics to stderr.
  CI.addDiagnosticConsumer(&IFaceGenCtx->Impl.DiagConsumer);

  Invocation.getFrontendOptions().InputsAndOutputs.clearInputs();
  if (CI.setup(Invocation, ErrMsg)) {
    return nullptr;
  }

  ASTContext &Ctx = CI.getASTContext();
  CloseClangModuleFiles scopedCloseFiles(*Ctx.getClangModuleLoader());

  // Load implicit imports so that Clang importer can use them.
  for (auto unloadedImport :
       CI.getMainModule()->getImplicitImportInfo().AdditionalUnloadedImports) {
    (void)Ctx.getModule(unloadedImport.module.getModulePath());
  }

  if (IsModule) {
    if (getModuleInterfaceInfo(Ctx, ModuleOrHeaderName, Group, IFaceGenCtx->Impl,
                               ErrMsg, SynthesizedExtensions, InterestedUSR))
      return nullptr;
  } else {
    auto &FEOpts = Invocation.getFrontendOptions();
    if (FEOpts.ImplicitObjCHeaderPath.empty()) {
      ErrMsg = "Implicit ObjC header path is empty";
      return nullptr;
    }

    auto &Importer = static_cast<ClangImporter &>(*Ctx.getClangModuleLoader());
    Importer.importBridgingHeader(FEOpts.ImplicitObjCHeaderPath,
                                  CI.getMainModule(),
                                  /*diagLoc=*/{},
                                  /*trackParsedSymbols=*/true);
    if (getHeaderInterfaceInfo(Ctx, ModuleOrHeaderName,
                               IFaceGenCtx->Impl.Info, ErrMsg))
      return nullptr;
  }

  if (makeParserAST(IFaceGenCtx->Impl.TextCI, IFaceGenCtx->Impl.Info.Text,
                    Invocation)) {
    ErrMsg = "Error during syntactic parsing";
    return nullptr;
  }

  return IFaceGenCtx;
}

CodiraInterfaceGenContextRef
CodiraInterfaceGenContext::createForTypeInterface(CompilerInvocation Invocation,
                                                 StringRef TypeUSR,
                                                 std::string &ErrorMsg) {
  CodiraInterfaceGenContextRef IFaceGenCtx{ new CodiraInterfaceGenContext() };
  IFaceGenCtx->Impl.IsModule = false;
  IFaceGenCtx->Impl.ModuleOrHeaderName = TypeUSR.str();
  IFaceGenCtx->Impl.Invocation = Invocation;
  CompilerInstance &CI = IFaceGenCtx->Impl.Instance;
  SourceTextInfo &Info = IFaceGenCtx->Impl.Info;

  // Display diagnostics to stderr.
  CI.addDiagnosticConsumer(&IFaceGenCtx->Impl.DiagConsumer);

  if (CI.setup(Invocation, ErrorMsg)) {
    return nullptr;
  }
  registerIDETypeCheckRequestFunctions(CI.getASTContext().evaluator);
  CI.performSema();
  ASTContext &Ctx = CI.getASTContext();
  CloseClangModuleFiles scopedCloseFiles(*Ctx.getClangModuleLoader());

  // Load standard library so that Clang importer can use it.
  auto *Stdlib = Ctx.getModuleByIdentifier(Ctx.StdlibModuleName);
  if (!Stdlib) {
    ErrorMsg = "Could not load the stdlib module";
    return nullptr;
  }
  auto *Module = CI.getMainModule();
  if (!Module) {
    ErrorMsg = "Could not load the main module";
    return nullptr;
  }
  SmallString<128> Text;
  toolchain::raw_svector_ostream OS(Text);
  AnnotatingPrinter Printer(Info, OS);
  if (ide::printTypeInterface(Module, TypeUSR, Printer,
                              IFaceGenCtx->Impl.DocumentName, ErrorMsg))
    return nullptr;
  IFaceGenCtx->Impl.Info.Text = std::string(OS.str());
  if (makeParserAST(IFaceGenCtx->Impl.TextCI, IFaceGenCtx->Impl.Info.Text,
                    Invocation)) {
    ErrorMsg = "Error during syntactic parsing";
    return nullptr;
  }
  return IFaceGenCtx;
}

CodiraInterfaceGenContext::CodiraInterfaceGenContext()
  : Impl(*new Implementation) {
}
CodiraInterfaceGenContext::~CodiraInterfaceGenContext() {
  delete &Impl;
}

StringRef CodiraInterfaceGenContext::getDocumentName() const {
  return Impl.DocumentName;
}

StringRef CodiraInterfaceGenContext::getModuleOrHeaderName() const {
  return Impl.ModuleOrHeaderName;
}

bool CodiraInterfaceGenContext::isModule() const {
  return Impl.IsModule;
}

ModuleDecl *CodiraInterfaceGenContext::getModuleDecl() const {
  return Impl.Mod;
}

bool CodiraInterfaceGenContext::matches(StringRef ModuleName,
                                       const language::CompilerInvocation &Invok) {
  if (!Impl.IsModule)
    return false;
  if (ModuleName != Impl.ModuleOrHeaderName)
    return false;

  if (Invok.getTargetTriple() != Impl.Invocation.getTargetTriple())
    return false;

  if (ModuleName == STDLIB_NAME)
    return true;

  if (Invok.getSDKPath() != Impl.Invocation.getSDKPath())
    return false;

  if (Impl.Mod->isNonUserModule())
    return true;

  const SearchPathOptions &SPOpts = Invok.getSearchPathOptions();
  const SearchPathOptions &ImplSPOpts = Impl.Invocation.getSearchPathOptions();
  if (SPOpts.getImportSearchPaths() != ImplSPOpts.getImportSearchPaths())
    return false;
  if (SPOpts.getFrameworkSearchPaths() != ImplSPOpts.getFrameworkSearchPaths())
    return false;

  if (Invok.getClangImporterOptions().ExtraArgs !=
      Impl.Invocation.getClangImporterOptions().ExtraArgs)
    return false;

  return true;
}

void CodiraInterfaceGenContext::reportEditorInfo(EditorConsumer &Consumer) const {
  Consumer.handleSourceText(Impl.Info.Text);
  reportSyntacticAnnotations(Impl.TextCI, Consumer);
  reportDocumentStructure(Impl.TextCI, Consumer);
  reportSemanticAnnotations(Impl.Info, Consumer);

  reportDeclarations(Impl.Info, Consumer);
  Consumer.finished();
}

void CodiraInterfaceGenContext::accessASTAsync(std::function<void()> Fn) {
  if (Impl.AstUnit) {
    Impl.AstUnit->performAsync(std::move(Fn));
  } else {
    Impl.Queue.dispatch(std::move(Fn));
  }
}

CodiraInterfaceGenContext::ResolvedEntity
CodiraInterfaceGenContext::resolveEntityForOffset(unsigned Offset) const {
  // Search among the references.
  {
    auto Pos = std::upper_bound(Impl.Info.References.begin(),
                                Impl.Info.References.end(),
                                Offset,
      [&](unsigned Offset, const TextReference &RHS) -> bool {
        return Offset < RHS.Range.Offset+RHS.Range.Length;
      });
    if (Pos != Impl.Info.References.end() && Pos->Range.Offset <= Offset) {
      if (Pos->Mod)
        return ResolvedEntity(Pos->Mod, true);
      else
        return ResolvedEntity(Pos->Dcl, true);
    }
  }

  SourceManager &SM = Impl.TextCI.getSourceMgr();
  auto SF = dyn_cast<SourceFile>(Impl.TextCI.getMainModule()->getFiles()[0]);
  unsigned BufferID = SF->getBufferID();
  SourceLoc Loc = Lexer::getLocForStartOfToken(SM, BufferID, Offset);
  Offset = SM.getLocOffsetInBuffer(Loc, BufferID);

  // Search among the declarations.
  {
    auto Pos = std::lower_bound(Impl.Info.Decls.begin(),
                                Impl.Info.Decls.end(),
                                Offset,
      [&](const TextDecl &LHS, unsigned Offset) -> bool {
        return LHS.Range.Offset < Offset;
      });
    if (Pos != Impl.Info.Decls.end() && Pos->Range.Offset == Offset)
      return ResolvedEntity(dyn_cast<ValueDecl>(Pos->Dcl), false);
  }

  return ResolvedEntity();
}

std::optional<std::pair<unsigned, unsigned>>
CodiraInterfaceGenContext::findUSRRange(StringRef USR) const {
  auto Pos = Impl.Info.USRMap.find(USR);
  if (Pos == Impl.Info.USRMap.end())
    return std::nullopt;

  return std::make_pair(Pos->getValue().Range.Offset,
                        Pos->getValue().Range.Length);
}

void CodiraInterfaceGenContext::applyTo(
    language::CompilerInvocation &CompInvok) const {
  CompInvok = Impl.Invocation;
}

CodiraInterfaceGenContextRef CodiraInterfaceGenMap::get(StringRef Name) const {
  toolchain::sys::ScopedLock L(Mtx);
  auto It = IFaceGens.find(Name);
  if (It == IFaceGens.end())
    return nullptr;
  return It->second;
}

void CodiraInterfaceGenMap::set(StringRef Name,
                               CodiraInterfaceGenContextRef IFaceGen) {
  toolchain::sys::ScopedLock L(Mtx);
  IFaceGens[Name] = IFaceGen;
}

bool CodiraInterfaceGenMap::remove(StringRef Name) {
  toolchain::sys::ScopedLock L(Mtx);
  return IFaceGens.erase(Name);
}

CodiraInterfaceGenContextRef
CodiraInterfaceGenMap::find(StringRef ModuleName,
                           const CompilerInvocation &Invok) {
  toolchain::sys::ScopedLock L(Mtx);
  for (auto &Entry : IFaceGens) {
    if (Entry.getValue()->matches(ModuleName, Invok))
      return Entry.getValue();
  }
  return nullptr;
}
//===----------------------------------------------------------------------===//
// EditorOpenTypeInterface
//===----------------------------------------------------------------------===//
void CodiraLangSupport::editorOpenTypeInterface(EditorConsumer &Consumer,
                                               ArrayRef<const char *> Args,
                                               StringRef TypeUSR) {
  CompilerInstance CI;
  // Display diagnostics to stderr.
  PrintingDiagnosticConsumer PrintDiags;
  CI.addDiagnosticConsumer(&PrintDiags);

  CompilerInvocation Invocation;
  std::string Error;
  if (getASTManager()->initCompilerInvocation(
          Invocation, Args, FrontendOptions::ActionType::Typecheck,
          CI.getDiags(), StringRef(), Error)) {
    Consumer.handleRequestError(Error.c_str());
    return;
  }
  Invocation.getClangImporterOptions().ImportForwardDeclarations = true;

  std::string ErrMsg;
  auto IFaceGenRef = CodiraInterfaceGenContext::createForTypeInterface(
                                                      Invocation,
                                                      TypeUSR,
                                                      ErrMsg);
  if (!IFaceGenRef) {
    Consumer.handleRequestError(ErrMsg.c_str());
    return;
  }

  IFaceGenRef->reportEditorInfo(Consumer);
  // reportEditorInfo requires exclusive access to the AST, so don't add this
  // to the service cache until it has returned.
  IFaceGenContexts.set(TypeUSR, IFaceGenRef);
}

//===----------------------------------------------------------------------===//
// EditorOpenInterface
//===----------------------------------------------------------------------===//
void CodiraLangSupport::editorOpenInterface(
    EditorConsumer &Consumer, StringRef Name, StringRef ModuleName,
    std::optional<StringRef> Group, ArrayRef<const char *> Args,
    bool SynthesizedExtensions, std::optional<StringRef> InterestedUSR) {
  CompilerInstance CI;
  // Display diagnostics to stderr.
  PrintingDiagnosticConsumer PrintDiags;
  CI.addDiagnosticConsumer(&PrintDiags);

  CompilerInvocation Invocation;
  std::string Error;
  if (getASTManager()->initCompilerInvocationNoInputs(
          Invocation, Args, FrontendOptions::ActionType::Typecheck,
          CI.getDiags(), Error)) {
    Consumer.handleRequestError(Error.c_str());
    return;
  }

  Invocation.getClangImporterOptions().ImportForwardDeclarations = true;

  std::string ErrMsg;
  auto IFaceGenRef = CodiraInterfaceGenContext::create(Name,
                                                      /*IsModule=*/true,
                                                      ModuleName,
                                                      Group,
                                                      Invocation,
                                                      ErrMsg,
                                                      SynthesizedExtensions,
                                                      InterestedUSR);
  if (!IFaceGenRef) {
      // Retry to generate a module interface with C++ interop enabled,
      // if the first attempt failed.
      bool retryWithCxxEnabled = true;
      for (const auto &arg: Args) {
          if (StringRef(arg).starts_with("-cxx-interoperability-mode=") ||
              StringRef(arg).starts_with("-enable-experimental-cxx-interop")) {
              retryWithCxxEnabled = false;
              break;
          }
      }
      if (retryWithCxxEnabled) {
          std::vector<const char *> AdjustedArgs(Args.begin(), Args.end());
          AdjustedArgs.push_back("-cxx-interoperability-mode=default");
          return editorOpenInterface(Consumer, Name, ModuleName, Group, AdjustedArgs,
                                     SynthesizedExtensions, InterestedUSR);
      }
      else {
          Consumer.handleRequestError(ErrMsg.c_str());
          return;
      }
  }

  IFaceGenRef->reportEditorInfo(Consumer);
  // reportEditorInfo requires exclusive access to the AST, so don't add this
  // to the service cache until it has returned.
  IFaceGenContexts.set(Name, IFaceGenRef);
}

class PrimaryFileInterfaceConsumer : public CodiraASTConsumer {

  std::string Name;
  std::string SourceFileName;
  CodiraInterfaceGenMap &Contexts;
  std::shared_ptr<EditorConsumer> Consumer;
  CodiraInvocationRef ASTInvok;

public:
  PrimaryFileInterfaceConsumer(StringRef Name, StringRef SourceFileName,
                               CodiraInterfaceGenMap &Contexts,
                               std::shared_ptr<EditorConsumer> Consumer,
                               CodiraInvocationRef ASTInvok) :
    Name(Name), SourceFileName(SourceFileName), Contexts(Contexts),
      Consumer(Consumer), ASTInvok(ASTInvok) {}

  void failed(StringRef Error) override {
    Consumer->handleRequestError(Error.data());
  }

  void handlePrimaryAST(ASTUnitRef AstUnit) override {
    CompilerInvocation CompInvok;
    ASTInvok->applyTo(CompInvok);
    std::string Error;
    auto IFaceGenRef = CodiraInterfaceGenContext::createForCodiraSource(Name,
      SourceFileName, AstUnit, CompInvok, Error);
    if (!Error.empty())
      Consumer->handleRequestError(Error.data());
    Contexts.set(Name, IFaceGenRef);
    IFaceGenRef->reportEditorInfo(*Consumer);
  }
};

void CodiraLangSupport::editorOpenCodiraSourceInterface(
    StringRef Name, StringRef SourceName, ArrayRef<const char *> Args,
    bool CancelOnSubsequentRequest,
    SourceKitCancellationToken CancellationToken,
    std::shared_ptr<EditorConsumer> Consumer) {
  std::string Error;
  auto Invocation = ASTMgr->getTypecheckInvocation(Args, SourceName, Error);
  if (!Invocation) {
    Consumer->handleRequestError(Error.c_str());
    return;
  }
  auto AstConsumer = std::make_shared<PrimaryFileInterfaceConsumer>(Name,
    SourceName, IFaceGenContexts, Consumer, Invocation);
  static const char OncePerASTToken = 0;
  const void *Once = CancelOnSubsequentRequest ? &OncePerASTToken : nullptr;
  getASTManager()->processASTAsync(Invocation, AstConsumer, Once,
                                   CancellationToken,
                                   toolchain::vfs::getRealFileSystem());
}

void CodiraLangSupport::editorOpenHeaderInterface(EditorConsumer &Consumer,
                                                 StringRef Name,
                                                 StringRef HeaderName,
                                                 ArrayRef<const char *> Args,
                                                 bool UsingCodiraArgs,
                                                 bool SynthesizedExtensions,
                                                 StringRef languageVersion) {
  CompilerInstance CI;
  // Display diagnostics to stderr.
  PrintingDiagnosticConsumer PrintDiags;
  CI.addDiagnosticConsumer(&PrintDiags);

  CompilerInvocation Invocation;
  std::string Error;

  ArrayRef<const char *> CodiraArgs = UsingCodiraArgs ? Args : std::nullopt;
  if (getASTManager()->initCompilerInvocationNoInputs(
          Invocation, CodiraArgs, FrontendOptions::ActionType::Typecheck,
          CI.getDiags(), Error)) {
    Consumer.handleRequestError(Error.c_str());
    return;
  }

  if (!UsingCodiraArgs && initInvocationByClangArguments(Args, Invocation, Error)) {
    Consumer.handleRequestError(Error.c_str());
    return;
  }

  Invocation.getClangImporterOptions().ImportForwardDeclarations = true;
  if (!languageVersion.empty()) {
    auto languageVer =
        VersionParser::parseVersionString(languageVersion, SourceLoc(), nullptr);
    if (languageVer.has_value())
      Invocation.getLangOptions().EffectiveLanguageVersion =
          languageVer.value();
  }
  auto IFaceGenRef = CodiraInterfaceGenContext::create(
      Name,
      /*IsModule=*/false, HeaderName, std::nullopt, Invocation, Error,
      SynthesizedExtensions, std::nullopt);
  if (!IFaceGenRef) {
    Consumer.handleRequestError(Error.c_str());
    return;
  }

  IFaceGenRef->reportEditorInfo(Consumer);
  // reportEditorInfo requires exclusive access to the AST, so don't add this
  // to the service cache until it has returned.
  IFaceGenContexts.set(Name, IFaceGenRef);
}

void CodiraLangSupport::findInterfaceDocument(StringRef ModuleName,
                                             ArrayRef<const char *> Args,
                       std::function<void(const RequestResult<InterfaceDocInfo> &)> Receiver) {
  InterfaceDocInfo Info;

  CompilerInstance CI;
  // Display diagnostics to stderr.
  PrintingDiagnosticConsumer PrintDiags;
  CI.addDiagnosticConsumer(&PrintDiags);

  CompilerInvocation Invocation;
  std::string Error;
  if (getASTManager()->initCompilerInvocation(
          Invocation, Args, FrontendOptions::ActionType::Typecheck,
          CI.getDiags(), StringRef(), Error)) {
    return Receiver(RequestResult<InterfaceDocInfo>::fromError(Error));
  }

  if (auto IFaceGenRef = IFaceGenContexts.find(ModuleName, Invocation))
    Info.ModuleInterfaceName = IFaceGenRef->getDocumentName();

  SmallString<128> Buf;
  SmallVector<std::pair<unsigned, unsigned>, 16> ArgOffs;
  auto addArgPair = [&](StringRef Arg, StringRef Val) {
    assert(!Arg.empty());
    if (Val.empty())
      return;
    unsigned ArgBegin = Buf.size();
    Buf += Arg;
    unsigned ArgEnd = Buf.size();
    unsigned ValBegin = Buf.size();
    Buf += Val;
    unsigned ValEnd = Buf.size();
    ArgOffs.push_back(std::make_pair(ArgBegin, ArgEnd));
    ArgOffs.push_back(std::make_pair(ValBegin, ValEnd));
  };
  auto addSingleArg = [&](StringRef Arg) {
    assert(!Arg.empty());
    unsigned ArgBegin = Buf.size();
    Buf += Arg;
    unsigned ArgEnd = Buf.size();
    ArgOffs.push_back(std::make_pair(ArgBegin, ArgEnd));
  };

  addArgPair("-target", Invocation.getTargetTriple());

  const auto &SPOpts = Invocation.getSearchPathOptions();
  addArgPair("-sdk", SPOpts.getSDKPath());
  for (const auto &FramePath : SPOpts.getFrameworkSearchPaths()) {
    if (FramePath.IsSystem)
      addArgPair("-Fsystem", FramePath.Path);
    else
      addArgPair("-F", FramePath.Path);
  }
  for (const auto &Path : SPOpts.getImportSearchPaths()) {
    if (Path.IsSystem)
      addArgPair("-Isystem", Path.Path);
    else
      addArgPair("-I", Path.Path);
  }

  const auto &ClangOpts = Invocation.getClangImporterOptions();
  addArgPair("-module-cache-path", ClangOpts.ModuleCachePath);
  for (auto &ExtraArg : ClangOpts.ExtraArgs)
    addArgPair("-Xcc", ExtraArg);

  if (Invocation.getFrontendOptions().ImportUnderlyingModule)
    addSingleArg("-import-underlying-module");
  addArgPair("-import-objc-header",
             Invocation.getFrontendOptions().ImplicitObjCHeaderPath);

  SmallVector<StringRef, 16> NewArgs;
  for (auto Pair : ArgOffs) {
    NewArgs.push_back(StringRef(Buf.begin()+Pair.first, Pair.second-Pair.first));
  }
  Info.CompilerArgs = NewArgs;

  return Receiver(RequestResult<InterfaceDocInfo>::fromResult(Info));
}
