//===--- CodiraLangSupport.cpp ---------------------------------------------===//
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
#include "CodiraASTManager.h"
#include "CodiraEditorDiagConsumer.h"
#include "SourceKit/Core/Context.h"
#include "SourceKit/CodiraLang/Factory.h"
#include "SourceKit/Support/FileSystemProvider.h"
#include "SourceKit/Support/UIdent.h"

#include "language/AST/ASTVisitor.h"
#include "language/AST/ClangModuleLoader.h"
#include "language/AST/ParameterList.h"
#include "language/AST/SILOptions.h"
#include "language/AST/USRGeneration.h"
#include "language/Config.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/IDE/CodeCompletionCache.h"
#include "language/IDE/SyntaxModel.h"
#include "language/IDE/Utils.h"
#include "language/IDETool/IDEInspectionInstance.h"
#include "language/IDETool/SyntacticMacroExpansion.h"

#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "toolchain/ADT/APInt.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/Support/ConvertUTF.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/raw_ostream.h"

#if !defined(_WIN32)
#include <sys/param.h>
#else
#define WIN32_MEAN_AND_LEAN
#define NOMINMAX
#include <windows.h>
#endif

using namespace SourceKit;
using namespace language;
using namespace language::ide;
using language::index::SymbolKind;
using language::index::SymbolSubKind;
using language::index::SymbolProperty;
using language::index::SymbolPropertySet;
using language::index::SymbolInfo;
using language::index::SymbolRole;
using language::index::SymbolRoleSet;

#define KIND(NAME, CONTENT) static UIdent Kind##NAME(CONTENT);
#include "SourceKit/Core/ProtocolUIDs.def"

#define REFACTORING(KIND, NAME, ID) static UIdent Kind##Refactoring##KIND("source.refactoring.kind."#ID);
#include "language/Refactoring/RefactoringKinds.def"

static UIdent Attr_IBAction("source.decl.attribute.ibaction");
static UIdent Attr_IBOutlet("source.decl.attribute.iboutlet");
static UIdent Attr_IBDesignable("source.decl.attribute.ibdesignable");
static UIdent Attr_IBInspectable("source.decl.attribute.ibinspectable");
static UIdent Attr_GKInspectable("source.decl.attribute.gkinspectable");
static UIdent Attr_Objc("source.decl.attribute.objc");
static UIdent Attr_ObjcNamed("source.decl.attribute.objc.name");
static UIdent Attr_Private("source.decl.attribute.private");
static UIdent Attr_FilePrivate("source.decl.attribute.fileprivate");
static UIdent Attr_Internal("source.decl.attribute.internal");
static UIdent Attr_Package("source.decl.attribute.package");
static UIdent Attr_Public("source.decl.attribute.public");
static UIdent Attr_Open("source.decl.attribute.open");
static UIdent Attr_Setter_Private("source.decl.attribute.setter_access.private");
static UIdent Attr_Setter_FilePrivate("source.decl.attribute.setter_access.fileprivate");
static UIdent Attr_Setter_Internal("source.decl.attribute.setter_access.internal");
static UIdent Attr_Setter_Package("source.decl.attribute.setter_access.package");
static UIdent Attr_Setter_Public("source.decl.attribute.setter_access.public");
static UIdent Attr_Setter_Open("source.decl.attribute.setter_access.open");
static UIdent EffectiveAccess_Public("source.decl.effective_access.public");
static UIdent EffectiveAccess_Internal("source.decl.effective_access.internal");
static UIdent EffectiveAccess_FilePrivate("source.decl.effective_access.fileprivate");
static UIdent EffectiveAccess_LessThanFilePrivate("source.decl.effective_access.less_than_fileprivate");

std::unique_ptr<LangSupport>
SourceKit::createCodiraLangSupport(SourceKit::Context &SKCtx) {
  return std::unique_ptr<LangSupport>(new CodiraLangSupport(SKCtx));
}

const std::string LangSupport::SynthesizedUSRSeparator = "::SYNTHESIZED::";

namespace {

class UIdentVisitor : public ASTVisitor<UIdentVisitor,
                                        UIdent, UIdent, UIdent, UIdent > {
  const bool IsRef;

public:
  explicit UIdentVisitor(bool IsRef) : IsRef(IsRef) { }

  /// TODO: reconsider whether having a default case is a good idea.
  UIdent visitDecl(const Decl *D) { return UIdent(); }

  UIdent visitFuncDecl(const FuncDecl *D);
  UIdent visitVarDecl(const VarDecl *D);
  UIdent visitParamDecl(const ParamDecl *D);
  UIdent visitExtensionDecl(const ExtensionDecl *D);

#define UID_FOR(CLASS) \
  UIdent visit##CLASS##Decl(const CLASS##Decl *) { \
    return IsRef ? KindRef##CLASS : KindDecl##CLASS; \
  }
  UID_FOR(Class)
  UID_FOR(Struct)
  UID_FOR(Enum)
  UID_FOR(EnumElement)
  UID_FOR(Protocol)
  UID_FOR(TypeAlias)
  UID_FOR(AssociatedType)
  UID_FOR(GenericTypeParam)
  UID_FOR(Constructor)
  UID_FOR(Destructor)
  UID_FOR(Subscript)
  UID_FOR(OpaqueType)
  UID_FOR(Macro)
#undef UID_FOR
};

} // anonymous namespace

UIdent UIdentVisitor::visitFuncDecl(const FuncDecl *D) {
  if (auto AD = dyn_cast<AccessorDecl>(D)) {
    return CodiraLangSupport::getUIDForAccessor(AD->getStorage(),
                                               AD->getAccessorKind(),
                                               IsRef);
  }

  if (auto *Op = D->getOperatorDecl()) {
    switch (Op->getKind()) {
    case DeclKind::PrefixOperator:
      return IsRef ? KindRefFunctionPrefixOperator : KindDeclFunctionPrefixOperator;
    case DeclKind::PostfixOperator:
      return IsRef ? KindRefFunctionPostfixOperator : KindDeclFunctionPostfixOperator;
    case DeclKind::InfixOperator:
      return IsRef ? KindRefFunctionInfixOperator : KindDeclFunctionInfixOperator;
    default:
      toolchain_unreachable("unexpected operator kind");
    }
  }

  const DeclContext *DC = D->getDeclContext();
  if (DC->isTypeContext()) {
    if (D->isStatic()) {
      if (D->getCorrectStaticSpelling() == StaticSpellingKind::KeywordClass)
        return IsRef ? KindRefMethodClass : KindDeclMethodClass;
      else
        return IsRef ? KindRefMethodStatic : KindDeclMethodStatic;
    }
    return IsRef ? KindRefMethodInstance : KindDeclMethodInstance;
  }
  return IsRef ? KindRefFunctionFree : KindDeclFunctionFree;
}

UIdent UIdentVisitor::visitVarDecl(const VarDecl *D) {
  const DeclContext *DC = D->getDeclContext();
  if (DC->isTypeContext()) {
    if (D->isStatic()) {
      if (D->getCorrectStaticSpelling() == StaticSpellingKind::KeywordClass)
        return IsRef ? KindRefVarClass : KindDeclVarClass;
      else
        return IsRef ? KindRefVarStatic : KindDeclVarStatic;
    }
    return IsRef ? KindRefVarInstance : KindDeclVarInstance;
  }
  if (DC->isLocalContext())
    return IsRef ? KindRefVarLocal : KindDeclVarLocal;
  return IsRef ? KindRefVarGlobal : KindDeclVarGlobal;
}

UIdent UIdentVisitor::visitParamDecl(const ParamDecl *D) {
  // There is no KindRefVarParam. It's not usually an interesting difference.
  return IsRef ? KindRefVarLocal : KindDeclVarParam;
}

UIdent UIdentVisitor::visitExtensionDecl(const ExtensionDecl *D) {
  assert(!IsRef && "reference to an extension ?");
  if (NominalTypeDecl *NTD = D->getExtendedNominal()) {
    if (isa<StructDecl>(NTD))
      return KindDeclExtensionStruct;
    if (isa<ClassDecl>(NTD))
      return KindDeclExtensionClass;
    if (isa<EnumDecl>(NTD))
      return KindDeclExtensionEnum;
    if (isa<ProtocolDecl>(NTD))
      return KindDeclExtensionProtocol;
  }
  return UIdent();
}

namespace {
/// A simple FileSystemProvider that creates an InMemoryFileSystem for a given
/// dictionary of file contents and overlays that on top of the real filesystem.
class InMemoryFileSystemProvider: public SourceKit::FileSystemProvider {
  /// Provides the real filesystem, overlaid with an InMemoryFileSystem that
  /// contains specified files at specified locations.
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem>
  getFileSystem(OptionsDictionary &options, std::string &error) override {
    auto InMemoryFS = toolchain::IntrusiveRefCntPtr<toolchain::vfs::InMemoryFileSystem>(
        new toolchain::vfs::InMemoryFileSystem());

    static UIdent KeyFiles("key.files");
    static UIdent KeyName("key.name");
    static UIdent KeySourceFile("key.sourcefile");
    static UIdent KeySourceText("key.sourcetext");
    bool failed = options.forEach(KeyFiles, [&](OptionsDictionary &file) {
      StringRef name;
      if (!file.valueForOption(KeyName, name)) {
        error = "missing 'key.name'";
        return true;
      }

      StringRef content;
      if (file.valueForOption(KeySourceText, content)) {
        auto buffer = toolchain::MemoryBuffer::getMemBufferCopy(content, name);
        InMemoryFS->addFile(name, 0, std::move(buffer));
        return false;
      }

      StringRef mappedPath;
      if (!file.valueForOption(KeySourceFile, mappedPath)) {
        error = "missing 'key.sourcefile' or 'key.sourcetext'";
        return true;
      }

      auto bufferOrErr = toolchain::MemoryBuffer::getFile(mappedPath);
      if (auto err = bufferOrErr.getError()) {
        toolchain::raw_string_ostream errStream(error);
        errStream << "error reading target file '" << mappedPath
                  << "': " << err.message() << "\n";
        return true;
      }

      auto renamedBuffer = toolchain::MemoryBuffer::getMemBufferCopy(
          bufferOrErr.get()->getBuffer(), name);
      InMemoryFS->addFile(name, 0, std::move(renamedBuffer));
      return false;
    });

    if (failed)
      return nullptr;

    auto OverlayFS = toolchain::IntrusiveRefCntPtr<toolchain::vfs::OverlayFileSystem>(
        new toolchain::vfs::OverlayFileSystem(toolchain::vfs::getRealFileSystem()));
    OverlayFS->pushOverlay(std::move(InMemoryFS));
    return OverlayFS;
  }
};
}

static void configureIDEInspectionInstance(
    std::shared_ptr<IDEInspectionInstance> IDEInspectionInst,
    std::shared_ptr<GlobalConfig> GlobalConfig) {
  auto Opts = GlobalConfig->getIDEInspectionOpts();
  IDEInspectionInst->setOptions({
    Opts.MaxASTContextReuseCount,
    Opts.CheckDependencyInterval
  });
}

CodiraLangSupport::CodiraLangSupport(SourceKit::Context &SKCtx)
    : NotificationCtr(SKCtx.getNotificationCenter()),
      CodiraExecutablePath(SKCtx.getCodiraExecutablePath()),
      ReqTracker(SKCtx.getRequestTracker()), CCCache(new CodiraCompletionCache) {
  toolchain::SmallString<128> LibPath(SKCtx.getRuntimeLibPath());
  toolchain::sys::path::append(LibPath, "language");
  RuntimeResourcePath = std::string(LibPath.str());

  Stats = std::make_shared<CodiraStatistics>();
  EditorDocuments = std::make_shared<CodiraEditorDocumentFileMap>();

  std::shared_ptr<PluginRegistry> Plugins = std::make_shared<PluginRegistry>();

  ASTMgr = std::make_shared<CodiraASTManager>(
      EditorDocuments, SKCtx.getGlobalConfiguration(), Stats, ReqTracker,
      Plugins, CodiraExecutablePath, RuntimeResourcePath);

  IDEInspectionInst = std::make_shared<IDEInspectionInstance>(Plugins);
  configureIDEInspectionInstance(IDEInspectionInst,
                                 SKCtx.getGlobalConfiguration());

  CompileManager = std::make_shared<compile::SessionManager>(
      CodiraExecutablePath, RuntimeResourcePath, Plugins);

  // By default, just use the in-memory cache.
  CCCache->inMemory = std::make_unique<ide::CodeCompletionCache>();

  SyntacticMacroExpansions =
      std::make_shared<SyntacticMacroExpansion>(CodiraExecutablePath, Plugins);

  // Provide a default file system provider.
  setFileSystemProvider("in-memory-vfs", std::make_unique<InMemoryFileSystemProvider>());
}

CodiraLangSupport::~CodiraLangSupport() {
}

void CodiraLangSupport::globalConfigurationUpdated(
    std::shared_ptr<GlobalConfig> Config) {
  configureIDEInspectionInstance(IDEInspectionInst, Config);
}

void CodiraLangSupport::dependencyUpdated() {
  IDEInspectionInst->markCachedCompilerInstanceShouldBeInvalidated();
}

UIdent CodiraLangSupport::getUIDForDeclLanguage(const language::Decl *D) {
  if (D->hasClangNode())
    return KindObjC;
  return KindCodira;
}

UIdent CodiraLangSupport::getUIDForDecl(const Decl *D, bool IsRef) {
  return UIdentVisitor(IsRef).visit(const_cast<Decl*>(D));
}

UIdent CodiraLangSupport::getUIDForExtensionOfDecl(const Decl *D) {
  switch (D->getKind()) {
    case language::DeclKind::Struct:
      return KindDeclExtensionStruct;
    case language::DeclKind::Enum:
      return KindDeclExtensionEnum;
    case language::DeclKind::Class:
      return KindDeclExtensionClass;
    case language::DeclKind::Protocol:
      return KindDeclExtensionProtocol;
    default:
      toolchain_unreachable("cannot have extension.");
  }
}

UIdent CodiraLangSupport::getUIDForLocalVar(bool IsRef) {
  return IsRef ? KindRefVarLocal : KindDeclVarLocal;
}

UIdent CodiraLangSupport::getUIDForAccessor(const ValueDecl *D,
                                           AccessorKind AccKind,
                                           bool IsRef) {
  switch (AccKind) {
  case AccessorKind::Get:
  case AccessorKind::DistributedGet:
    return IsRef ? KindRefAccessorGetter : KindDeclAccessorGetter;
  case AccessorKind::Set:
    return IsRef ? KindRefAccessorSetter : KindDeclAccessorSetter;
  case AccessorKind::WillSet:
    return IsRef ? KindRefAccessorWillSet : KindDeclAccessorWillSet;
  case AccessorKind::DidSet:
    return IsRef ? KindRefAccessorDidSet : KindDeclAccessorDidSet;
  case AccessorKind::Address:
    return IsRef ? KindRefAccessorAddress : KindDeclAccessorAddress;
  case AccessorKind::MutableAddress:
    return IsRef ? KindRefAccessorMutableAddress
                 : KindDeclAccessorMutableAddress;
  case AccessorKind::Read:
  case AccessorKind::Read2:
    return IsRef ? KindRefAccessorRead : KindDeclAccessorRead;
  case AccessorKind::Modify:
  case AccessorKind::Modify2:
    return IsRef ? KindRefAccessorModify : KindDeclAccessorModify;
  case AccessorKind::Init:
    return IsRef ? KindRefAccessorInit : KindDeclAccessorInit;
  }

  toolchain_unreachable("Unhandled AccessorKind in switch.");
}

SourceKit::UIdent CodiraLangSupport::getUIDForModuleRef() {
  return KindRefModule;
}

SourceKit::UIdent CodiraLangSupport::getUIDForObjCAttr() {
  return Attr_Objc;
}

UIdent CodiraLangSupport::getUIDForRefactoringKind(ide::RefactoringKind Kind){
  switch(Kind) {
  case ide::RefactoringKind::None: toolchain_unreachable("cannot end up here.");
#define REFACTORING(KIND, NAME, ID)                                            \
  case ide::RefactoringKind::KIND: return KindRefactoring##KIND;
#include "language/Refactoring/RefactoringKinds.def"
  }
}

UIdent CodiraSemanticToken::getUIdentForKind() {
  return CodiraLangSupport::getUIDForCodeCompletionDeclKind(Kind, IsRef);
}

UIdent CodiraLangSupport::getUIDForCodeCompletionDeclKind(
    ide::CodeCompletionDeclKind Kind, bool IsRef) {
  if (IsRef) {
    // FIXME: The code-completion kind is also used for semantic annotations.
    // We should either create a new kind or rename the code-completion kind to
    // something more general.
    switch (Kind) {
    case CodeCompletionDeclKind::Module: return KindRefModule;
    case CodeCompletionDeclKind::Class: return KindRefClass;
    case CodeCompletionDeclKind::Actor: return KindRefActor;
    case CodeCompletionDeclKind::Struct: return KindRefStruct;
    case CodeCompletionDeclKind::Enum: return KindRefEnum;
    case CodeCompletionDeclKind::EnumElement: return KindRefEnumElement;
    case CodeCompletionDeclKind::Protocol: return KindRefProtocol;
    case CodeCompletionDeclKind::AssociatedType: return KindRefAssociatedType;
    case CodeCompletionDeclKind::TypeAlias: return KindRefTypeAlias;
    case CodeCompletionDeclKind::GenericTypeParam: return KindRefGenericTypeParam;
    case CodeCompletionDeclKind::Constructor: return KindRefConstructor;
    case CodeCompletionDeclKind::Destructor: return KindRefDestructor;
    case CodeCompletionDeclKind::Subscript: return KindRefSubscript;
    case CodeCompletionDeclKind::StaticMethod: return KindRefMethodClass;
    case CodeCompletionDeclKind::InstanceMethod: return KindRefMethodInstance;
    case CodeCompletionDeclKind::PrefixOperatorFunction: return KindRefFunctionPrefixOperator;
    case CodeCompletionDeclKind::PostfixOperatorFunction: return KindRefFunctionPostfixOperator;
    case CodeCompletionDeclKind::InfixOperatorFunction: return KindRefFunctionInfixOperator;
    case CodeCompletionDeclKind::PrecedenceGroup: return KindRefPrecedenceGroup;
    case CodeCompletionDeclKind::FreeFunction: return KindRefFunctionFree;
    case CodeCompletionDeclKind::StaticVar: return KindRefVarClass;
    case CodeCompletionDeclKind::InstanceVar: return KindRefVarInstance;
    case CodeCompletionDeclKind::LocalVar: return KindRefVarLocal;
    case CodeCompletionDeclKind::GlobalVar: return KindRefVarGlobal;
    case CodeCompletionDeclKind::Macro: return KindRefMacro;
    }
  }

  switch (Kind) {
  case CodeCompletionDeclKind::Module: return KindDeclModule;
  case CodeCompletionDeclKind::Class: return KindDeclClass;
  case CodeCompletionDeclKind::Actor: return KindDeclActor;
  case CodeCompletionDeclKind::Struct: return KindDeclStruct;
  case CodeCompletionDeclKind::Enum: return KindDeclEnum;
  case CodeCompletionDeclKind::EnumElement: return KindDeclEnumElement;
  case CodeCompletionDeclKind::Protocol: return KindDeclProtocol;
  case CodeCompletionDeclKind::AssociatedType: return KindDeclAssociatedType;
  case CodeCompletionDeclKind::TypeAlias: return KindDeclTypeAlias;
  case CodeCompletionDeclKind::GenericTypeParam: return KindDeclGenericTypeParam;
  case CodeCompletionDeclKind::Constructor: return KindDeclConstructor;
  case CodeCompletionDeclKind::Destructor: return KindDeclDestructor;
  case CodeCompletionDeclKind::Subscript: return KindDeclSubscript;
  case CodeCompletionDeclKind::StaticMethod: return KindDeclMethodClass;
  case CodeCompletionDeclKind::InstanceMethod: return KindDeclMethodInstance;
  case CodeCompletionDeclKind::PrefixOperatorFunction: return KindDeclFunctionPrefixOperator;
  case CodeCompletionDeclKind::PostfixOperatorFunction: return KindDeclFunctionPostfixOperator;
  case CodeCompletionDeclKind::InfixOperatorFunction: return KindDeclFunctionInfixOperator;
  case CodeCompletionDeclKind::PrecedenceGroup: return KindDeclPrecedenceGroup;
  case CodeCompletionDeclKind::FreeFunction: return KindDeclFunctionFree;
  case CodeCompletionDeclKind::StaticVar: return KindDeclVarClass;
  case CodeCompletionDeclKind::InstanceVar: return KindDeclVarInstance;
  case CodeCompletionDeclKind::LocalVar: return KindDeclVarLocal;
  case CodeCompletionDeclKind::GlobalVar: return KindDeclVarGlobal;
  case CodeCompletionDeclKind::Macro: return KindDeclMacro;
  }

  toolchain_unreachable("Unhandled CodeCompletionDeclKind in switch.");
}

UIdent CodiraLangSupport::getUIDForSyntaxNodeKind(SyntaxNodeKind SC) {
  switch (SC) {
  case SyntaxNodeKind::Keyword:
    return KindKeyword;
  case SyntaxNodeKind::Identifier:
  case SyntaxNodeKind::DollarIdent:
    return KindIdentifier;
  case SyntaxNodeKind::Operator:
    return KindOperator;
  case SyntaxNodeKind::Integer:
  case SyntaxNodeKind::Floating:
    return KindNumber;
  case SyntaxNodeKind::String:
    return KindString;
  case SyntaxNodeKind::StringInterpolationAnchor:
    return KindStringInterpolation;
  case SyntaxNodeKind::CommentLine:
  case SyntaxNodeKind::CommentBlock:
    return KindComment;
  case SyntaxNodeKind::CommentMarker:
    return KindCommentMarker;
  case SyntaxNodeKind::CommentURL:
    return KindCommentURL;
  case SyntaxNodeKind::DocCommentLine:
  case SyntaxNodeKind::DocCommentBlock:
    return KindDocComment;
  case SyntaxNodeKind::DocCommentField:
    return KindDocCommentField;
  case SyntaxNodeKind::TypeId:
    return KindTypeIdentifier;
  case SyntaxNodeKind::BuildConfigKeyword:
    return KindBuildConfigKeyword;
  case SyntaxNodeKind::BuildConfigId:
    return KindBuildConfigId;
  case SyntaxNodeKind::PoundDirectiveKeyword:
    return KindPoundDirectiveKeyword;
  case SyntaxNodeKind::AttributeId:
    return KindAttributeId;
  case SyntaxNodeKind::AttributeBuiltin:
    return KindAttributeBuiltin;
  case SyntaxNodeKind::EditorPlaceholder:
    return KindPlaceholder;
  case SyntaxNodeKind::ObjectLiteral:
    return KindObjectLiteral;
  }

  // Default to a known kind to prevent crashing in non-asserts builds
  assert(0 && "Unhandled SyntaxNodeKind in switch.");
  return KindIdentifier;
}

UIdent CodiraLangSupport::getUIDForSyntaxStructureKind(
    SyntaxStructureKind Kind) {
  switch (Kind) {
    case SyntaxStructureKind::Class:
      return KindDeclClass;
    case SyntaxStructureKind::Struct:
      return KindDeclStruct;
    case SyntaxStructureKind::Protocol:
      return KindDeclProtocol;
    case SyntaxStructureKind::Enum:
      return KindDeclEnum;
    case SyntaxStructureKind::Extension:
      return KindDeclExtension;
    case SyntaxStructureKind::FreeFunction:
      return KindDeclFunctionFree;
    case SyntaxStructureKind::InstanceFunction:
      return KindDeclMethodInstance;
    case SyntaxStructureKind::StaticFunction:
      return KindDeclMethodStatic;
    case SyntaxStructureKind::ClassFunction:
      return KindDeclMethodClass;
    case SyntaxStructureKind::GlobalVariable:
      return KindDeclVarGlobal;
    case SyntaxStructureKind::InstanceVariable:
      return KindDeclVarInstance;
    case SyntaxStructureKind::StaticVariable:
      return KindDeclVarStatic;
    case SyntaxStructureKind::ClassVariable:
      return KindDeclVarClass;
    case SyntaxStructureKind::LocalVariable:
      return KindDeclVarLocal;
    case SyntaxStructureKind::EnumCase:
      return KindDeclEnumCase;
    case SyntaxStructureKind::EnumElement:
      return KindDeclEnumElement;
    case SyntaxStructureKind::TypeAlias:
      return KindDeclTypeAlias;
    case SyntaxStructureKind::Subscript:
      return KindDeclSubscript;
    case SyntaxStructureKind::AssociatedType:
      return KindDeclAssociatedType;
    case SyntaxStructureKind::GenericTypeParam:
      return KindDeclGenericTypeParam;
    case SyntaxStructureKind::Parameter:
      return KindDeclVarParam;
    case SyntaxStructureKind::ForEachStatement:
      return KindStmtForEach;
    case SyntaxStructureKind::WhileStatement:
      return KindStmtWhile;
    case SyntaxStructureKind::RepeatWhileStatement:
      return KindStmtRepeatWhile;
    case SyntaxStructureKind::IfStatement:
      return KindStmtIf;
    case SyntaxStructureKind::GuardStatement:
      return KindStmtGuard;
    case SyntaxStructureKind::SwitchStatement:
      return KindStmtSwitch;
    case SyntaxStructureKind::CaseStatement:
      return KindStmtCase;
    case SyntaxStructureKind::BraceStatement:
      return KindStmtBrace;
    case SyntaxStructureKind::CallExpression:
      return KindExprCall;
    case SyntaxStructureKind::ArrayExpression:
      return KindExprArray;
    case SyntaxStructureKind::DictionaryExpression:
      return KindExprDictionary;
    case SyntaxStructureKind::ObjectLiteralExpression:
      return KindExprObjectLiteral;
    case SyntaxStructureKind::TupleExpression:
      return KindExprTuple;
    case SyntaxStructureKind::ClosureExpression:
      return KindExprClosure;
    case SyntaxStructureKind::Argument:
      return KindExprArg;
  }

  toolchain_unreachable("Unhandled SyntaxStructureKind in switch.");
}

UIdent CodiraLangSupport::getUIDForSyntaxStructureElementKind(
    SyntaxStructureElementKind Kind) {
  switch (Kind) {
    case SyntaxStructureElementKind::Id: return KindStructureElemId;
    case SyntaxStructureElementKind::Expr: return KindStructureElemExpr;
    case SyntaxStructureElementKind::InitExpr: return KindStructureElemInitExpr;
    case SyntaxStructureElementKind::ConditionExpr: return KindStructureElemCondExpr;
    case SyntaxStructureElementKind::Pattern: return KindStructureElemPattern;
    case SyntaxStructureElementKind::TypeRef: return KindStructureElemTypeRef;
  }

  toolchain_unreachable("Unhandled SyntaxStructureElementKind in switch.");
}

SourceKit::UIdent CodiraLangSupport::
getUIDForRangeKind(language::ide::RangeKind Kind) {
  switch (Kind) {
    case language::ide::RangeKind::SingleExpression: return KindRangeSingleExpression;
    case language::ide::RangeKind::SingleStatement: return KindRangeSingleStatement;
    case language::ide::RangeKind::SingleDecl: return KindRangeSingleDeclaration;
    case language::ide::RangeKind::MultiStatement: return KindRangeMultiStatement;
    case language::ide::RangeKind::MultiTypeMemberDecl:
      return KindRangeMultiTypeMemberDeclaration;
    case language::ide::RangeKind::PartOfExpression: return KindRangeInvalid;
    case language::ide::RangeKind::Invalid: return KindRangeInvalid;
  }

  toolchain_unreachable("Unhandled RangeKind in switch.");
}

SourceKit::UIdent CodiraLangSupport::
getUIDForRegionType(language::ide::RegionType Type) {
  switch (Type) {
    case language::ide::RegionType::ActiveCode: return KindEditActive;
    case language::ide::RegionType::InactiveCode: return KindEditInactive;
    case language::ide::RegionType::Selector: return KindEditSelector;
    case language::ide::RegionType::String: return KindEditString;
    case language::ide::RegionType::Comment: return KindEditComment;
    case language::ide::RegionType::Mismatch: return KindEditMismatch;
    case language::ide::RegionType::Unmatched: return KindEditUnknown;
  }
}

SourceKit::UIdent CodiraLangSupport::
getUIDForRefactoringRangeKind(ide::RefactoringRangeKind Kind) {
  switch (Kind) {
  case ide::RefactoringRangeKind::BaseName:
    return KindRenameRangeBase;
  case ide::RefactoringRangeKind::KeywordBaseName:
    return KindRenameRangeKeywordBase;
  case ide::RefactoringRangeKind::ParameterName:
    return KindRenameRangeParam;
  case ide::RefactoringRangeKind::NoncollapsibleParameterName:
    return KindRenameRangeNoncollapsibleParam;
  case ide::RefactoringRangeKind::DeclArgumentLabel:
    return KindRenameRangeDeclArgLabel;
  case ide::RefactoringRangeKind::CallArgumentLabel:
    return KindRenameRangeCallArgLabel;
  case ide::RefactoringRangeKind::CallArgumentColon:
    return KindRenameRangeCallArgColon;
  case ide::RefactoringRangeKind::CallArgumentCombined:
    return KindRenameRangeCallArgCombined;
  case ide::RefactoringRangeKind::SelectorArgumentLabel:
    return KindRenameRangeSelectorArgLabel;
  }
}

UIdent CodiraLangSupport::getUIDForSymbol(SymbolInfo sym, bool isRef) {

#define UID_FOR(CLASS) isRef ? KindRef##CLASS : KindDecl##CLASS;

  switch (sym.SubKind) {
  default: break;
  case SymbolSubKind::AccessorGetter: return UID_FOR(AccessorGetter);
  case SymbolSubKind::AccessorSetter: return UID_FOR(AccessorSetter);
  case SymbolSubKind::CodiraAccessorWillSet: return UID_FOR(AccessorWillSet);
  case SymbolSubKind::CodiraAccessorDidSet: return UID_FOR(AccessorDidSet);
  case SymbolSubKind::CodiraAccessorAddressor: return UID_FOR(AccessorAddress);
  case SymbolSubKind::CodiraAccessorMutableAddressor: return UID_FOR(AccessorMutableAddress);
  }

#define SIMPLE_CASE(KIND) \
  case SymbolKind::KIND: \
    return UID_FOR(KIND);

  switch (sym.Kind) {
  SIMPLE_CASE(Enum)
  SIMPLE_CASE(Struct)
  SIMPLE_CASE(Class)
  SIMPLE_CASE(Protocol)
  SIMPLE_CASE(Constructor)
  SIMPLE_CASE(Destructor)
  SIMPLE_CASE(Macro)

  case SymbolKind::EnumConstant:
    return UID_FOR(EnumElement);

  case SymbolKind::TypeAlias:
    if (sym.SubKind == SymbolSubKind::CodiraAssociatedType)
      return UID_FOR(AssociatedType);
    if (sym.SubKind == SymbolSubKind::CodiraGenericTypeParam)
      return UID_FOR(GenericTypeParam);
    return UID_FOR(TypeAlias);

  case SymbolKind::Function:
  case SymbolKind::StaticMethod:
    if (sym.SubKind == SymbolSubKind::CodiraPrefixOperator)
      return UID_FOR(FunctionPrefixOperator);
    if (sym.SubKind == SymbolSubKind::CodiraPostfixOperator)
      return UID_FOR(FunctionPostfixOperator);
    if (sym.SubKind == SymbolSubKind::CodiraInfixOperator)
      return UID_FOR(FunctionInfixOperator);
    if (sym.Kind == SymbolKind::StaticMethod) {
      return UID_FOR(MethodStatic);
    } else {
      return UID_FOR(FunctionFree);
    }
  case SymbolKind::Variable:
    return UID_FOR(VarGlobal);
  case SymbolKind::InstanceMethod:
    return UID_FOR(MethodInstance);
  case SymbolKind::ClassMethod:
    return UID_FOR(MethodClass);
  case SymbolKind::InstanceProperty:
    if (sym.SubKind == SymbolSubKind::CodiraSubscript)
      return UID_FOR(Subscript);
    return UID_FOR(VarInstance);
  case SymbolKind::ClassProperty:
    return UID_FOR(VarClass);
  case SymbolKind::StaticProperty:
    return UID_FOR(VarStatic);

  case SymbolKind::Extension:
    assert(!isRef && "reference to extension decl?");
    if (sym.SubKind == SymbolSubKind::CodiraExtensionOfStruct) {
      return KindDeclExtensionStruct;
    } else if (sym.SubKind == SymbolSubKind::CodiraExtensionOfClass) {
      return KindDeclExtensionClass;
    } else if (sym.SubKind == SymbolSubKind::CodiraExtensionOfEnum) {
      return KindDeclExtensionEnum;
    } else if (sym.SubKind == SymbolSubKind::CodiraExtensionOfProtocol) {
      return KindDeclExtensionProtocol;
    } else {
      toolchain_unreachable("missing extension sub kind");
    }

  case SymbolKind::Module:
    return KindRefModule;

  case SymbolKind::CommentTag:
    return KindCommentTag;

  default:
    // TODO: reconsider whether having a default case is a good idea.
    toolchain_unreachable("unhandled symbol kind Codira doesn't use");
  }

#undef SIMPLE_CASE
#undef UID_FOR
}

SourceKit::UIdent CodiraLangSupport::getUIDForNameKind(language::ide::NameKind Kind) {
  switch(Kind) {
  case language::ide::NameKind::ObjC: return KindNameObjc;
  case language::ide::NameKind::Codira: return KindNameCodira;
  }

  toolchain_unreachable("Unhandled NameKind in switch.");
}

language::ide::NameKind CodiraLangSupport::getNameKindForUID(SourceKit::UIdent Id) {
  if (Id == KindNameObjc)
    return language::ide::NameKind::ObjC;
  assert(Id == KindNameCodira);
  return language::ide::NameKind::Codira;
}

std::optional<UIdent>
CodiraLangSupport::getUIDForDeclAttribute(const language::DeclAttribute *Attr) {
  // Check special-case names first.
  switch (Attr->getKind()) {
  case DeclAttrKind::IBAction: {
    return Attr_IBAction;
  }
  case DeclAttrKind::IBSegueAction: {
    static UIdent Attr_IBSegueAction("source.decl.attribute.ibsegueaction");
    return Attr_IBSegueAction;
  }
  case DeclAttrKind::IBOutlet: {
    return Attr_IBOutlet;
  }
  case DeclAttrKind::IBDesignable: {
    return Attr_IBDesignable;
  }
  case DeclAttrKind::IBInspectable: {
    return Attr_IBInspectable;
  }
  case DeclAttrKind::GKInspectable: {
    return Attr_GKInspectable;
  }
  case DeclAttrKind::ObjC: {
    if (cast<ObjCAttr>(Attr)->hasName()) {
      return Attr_ObjcNamed;
    } else {
      return Attr_Objc;
    }
  }
  case DeclAttrKind::AccessControl: {
    switch (cast<AbstractAccessControlAttr>(Attr)->getAccess()) {
    case AccessLevel::Private:
      return Attr_Private;
    case AccessLevel::FilePrivate:
      return Attr_FilePrivate;
    case AccessLevel::Internal:
      return Attr_Internal;
    case AccessLevel::Package:
      return Attr_Package;
    case AccessLevel::Public:
      return Attr_Public;
    case AccessLevel::Open:
      return Attr_Open;
    }
  }
  case DeclAttrKind::SetterAccess: {
    switch (cast<AbstractAccessControlAttr>(Attr)->getAccess()) {
    case AccessLevel::Private:
      return Attr_Setter_Private;
    case AccessLevel::FilePrivate:
      return Attr_Setter_FilePrivate;
    case AccessLevel::Internal:
      return Attr_Setter_Internal;
    case AccessLevel::Package:
      return Attr_Setter_Package;
    case AccessLevel::Public:
      return Attr_Setter_Public;
    case AccessLevel::Open:
      return Attr_Setter_Open;
    }
  }

    // Ignore these.
  case DeclAttrKind::ShowInInterface:
  case DeclAttrKind::RawDocComment:
  case DeclAttrKind::HasInitialValue:
  case DeclAttrKind::HasStorage:
    return std::nullopt;
  default:
    break;
  }

  switch (Attr->getKind()) {
#define DECL_ATTR(X, CLASS, ...)                                               \
  case DeclAttrKind::CLASS: {                                                  \
    static UIdent Attr_##X("source.decl.attribute." #X);                       \
    return Attr_##X;                                                           \
  }
#include "language/AST/DeclAttr.def"
  }

  return std::nullopt;
}

UIdent CodiraLangSupport::getUIDForFormalAccessScope(const language::AccessScope Scope) {
  if (Scope.isPublic()) {
    return EffectiveAccess_Public;
  } else if (Scope.isInternal()) {
    return EffectiveAccess_Internal;
  } else if (Scope.isFileScope()) {
    return EffectiveAccess_FilePrivate;
  } else if (Scope.isPrivate()) {
    return EffectiveAccess_LessThanFilePrivate;
  } else {
    toolchain_unreachable("Unsupported access scope");
  }
}

std::vector<UIdent> CodiraLangSupport::UIDsFromDeclAttributes(const DeclAttributes &Attrs) {
  std::vector<UIdent> AttrUIDs;

  for (auto Attr : Attrs) {
    if (auto AttrUID = getUIDForDeclAttribute(Attr)) {
      AttrUIDs.push_back(AttrUID.value());
    }
  }

  return AttrUIDs;
}

bool CodiraLangSupport::printDisplayName(const language::ValueDecl *D,
                                        toolchain::raw_ostream &OS) {
  if (!D->hasName())
    return true;

  D->getName().print(OS, /*skipEmptyArgumentNames*/ false,
                     /*escapeIfNeeded*/ true);
  return false;
}

bool CodiraLangSupport::printUSR(const ValueDecl *D, toolchain::raw_ostream &OS,
                                bool distinguishSynthesizedDecls) {
  return ide::printValueDeclUSR(D, OS, distinguishSynthesizedDecls);
}

bool CodiraLangSupport::printDeclTypeUSR(const ValueDecl *D, toolchain::raw_ostream &OS) {
  return ide::printDeclTypeUSR(D, OS);
}

bool CodiraLangSupport::printTypeUSR(Type Ty, toolchain::raw_ostream &OS) {
  return ide::printTypeUSR(Ty, OS);
}

bool CodiraLangSupport::printAccessorUSR(const AbstractStorageDecl *D,
                                        AccessorKind AccKind,
                                        toolchain::raw_ostream &OS) {
  return ide::printAccessorUSR(D, AccKind, OS);
}

void CodiraLangSupport::printMemberDeclDescription(const language::ValueDecl *VD,
                                                  language::Type baseTy,
                                                  bool usePlaceholder,
                                                  toolchain::raw_ostream &OS) {
  // Base name.
  OS << VD->getBaseName().userFacingName();

  // Parameters.
  auto substMap = baseTy->getMemberSubstitutionMap(VD);
  auto printSingleParam = [&](ParamDecl *param) {
    auto paramTy = param->getInterfaceType();

    // Label.
    if (!param->getArgumentName().empty())
      OS << param->getArgumentName() << ": ";

    // InOut.
    if (param->isInOut()) {
      OS << "&";
      paramTy = paramTy->getInOutObjectType();
    }

    // Type.
    if (usePlaceholder)
      OS << "<#T##";

    paramTy = paramTy.subst(substMap);
    if (paramTy->hasError() && param->getTypeRepr()) {
      // Fallback to 'TypeRepr' printing.
      param->getTypeRepr()->print(OS);
    } else {
      paramTy.print(OS);
    }

    if (usePlaceholder)
      OS << "#>";
  };
  auto printParams = [&](const ParameterList *params) {
    OS << '(';
    bool isFirst = true;
    for (auto param : params->getArray()) {
      if (isFirst)
        isFirst = false;
      else
        OS << ", ";
      printSingleParam(param);
    }
    OS << ')';
  };
  if (isa<EnumElementDecl>(VD) || isa<FuncDecl>(VD)) {
    if (const auto ParamList = VD->getParameterList()) {
      printParams(ParamList);
    }
  } else if (isa<VarDecl>(VD)) {
    // Var decl doesn't have parameters.
  } else {
    toolchain_unreachable("Unsupported Decl kind for printMemberDeclDescription()");
  }
}

std::string CodiraLangSupport::resolvePathSymlinks(StringRef FilePath) {
  std::string InputPath = FilePath.str();
  toolchain::SmallString<256> output;
  if (toolchain::sys::fs::real_path(InputPath, output))
    return InputPath;
  return std::string(output.str());
}

void CodiraLangSupport::getStatistics(StatisticsReceiver receiver) {
  std::vector<Statistic *> stats = {
#define LANGUAGE_STATISTIC(VAR, UID, DESC) &Stats->VAR,
#include "CodiraStatistics.def"
  };
  receiver(stats);
}

FileSystemProvider *CodiraLangSupport::getFileSystemProvider(StringRef Name) {
  auto It = FileSystemProviders.find(Name);
  if (It == FileSystemProviders.end())
    return nullptr;
  return It->second.get();
}

void CodiraLangSupport::setFileSystemProvider(
     StringRef Name, std::unique_ptr<FileSystemProvider> FileSystemProvider) {
  assert(FileSystemProvider);
  auto Result = FileSystemProviders.try_emplace(Name, std::move(FileSystemProvider));
  assert(Result.second && "tried to set existing FileSystemProvider");
}

toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem>
CodiraLangSupport::getFileSystem(const std::optional<VFSOptions> &vfsOptions,
                                std::optional<StringRef> primaryFile,
                                std::string &error) {
  // First, try the specified vfsOptions.
  if (vfsOptions) {
    auto provider = getFileSystemProvider(vfsOptions->name);
    if (!provider) {
      error = "unknown virtual filesystem '" + vfsOptions->name + "'";
      return nullptr;
    }

    return provider->getFileSystem(*vfsOptions->options, error);
  }

  // Otherwise, try to find an open document with a filesystem.
  if (primaryFile) {
    if (auto doc = EditorDocuments->getByUnresolvedName(*primaryFile)) {
      return doc->getFileSystem();
    }
  }

  // Fallback to the real filesystem.
  return toolchain::vfs::getRealFileSystem();
}

void CodiraLangSupport::performWithParamsToCompletionLikeOperation(
    toolchain::MemoryBuffer *UnresolvedInputFile, unsigned Offset,
    bool InsertCodeCompletionToken, ArrayRef<const char *> Args,
    toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FileSystem,
    SourceKitCancellationToken CancellationToken,
    toolchain::function_ref<void(CancellableResult<CompletionLikeOperationParams>)>
        PerformOperation) {
  assert(FileSystem);

  // Resolve symlinks for the input file; we resolve them for the input files
  // in the arguments as well.
  // FIXME: We need the Codira equivalent of Clang's FileEntry.
  toolchain::SmallString<128> bufferIdentifier;
  if (auto err = FileSystem->getRealPath(
          UnresolvedInputFile->getBufferIdentifier(), bufferIdentifier))
    bufferIdentifier = UnresolvedInputFile->getBufferIdentifier();

  // Create a buffer for code completion. This contains '\0' at 'Offset'
  // position of 'UnresolvedInputFile' buffer.
  auto origOffset = Offset;

  // If we inserted a code completion token into the buffer, this keeps the
  // buffer alive. Should never be accessed, `buffer` should be used instead.
  std::unique_ptr<toolchain::MemoryBuffer> codeCompletionBuffer;
  toolchain::MemoryBuffer *buffer;
  if (InsertCodeCompletionToken) {
    codeCompletionBuffer = ide::makeCodeCompletionMemoryBuffer(
        UnresolvedInputFile, Offset, bufferIdentifier);
    buffer = codeCompletionBuffer.get();
  } else {
    buffer = UnresolvedInputFile;
  }

  SourceManager SM;
  DiagnosticEngine Diags(SM);
  PrintingDiagnosticConsumer PrintDiags;
  EditorDiagConsumer TraceDiags;
  trace::TracedOperation TracedOp{trace::OperationKind::CodeCompletion};

  Diags.addConsumer(PrintDiags);
  if (TracedOp.enabled()) {
    Diags.addConsumer(TraceDiags);
    trace::CodiraInvocation CodiraArgs;
    trace::initTraceInfo(CodiraArgs, bufferIdentifier, Args);
    TracedOp.setDiagnosticProvider(
        [&](SmallVectorImpl<DiagnosticEntryInfo> &diags) {
          TraceDiags.getAllDiagnostics(diags);
        });
    TracedOp.start(
        CodiraArgs,
        {std::make_pair("OriginalOffset", std::to_string(origOffset)),
         std::make_pair("Offset", std::to_string(Offset))});
  }
  ForwardingDiagnosticConsumer CIDiags(Diags);

  CompilerInvocation Invocation;
  std::string CompilerInvocationError;
  bool CreatingInvocationFailed = getASTManager()->initCompilerInvocation(
      Invocation, Args, FrontendOptions::ActionType::Typecheck, Diags,
      buffer->getBufferIdentifier(), FileSystem, CompilerInvocationError);
  if (CreatingInvocationFailed) {
    PerformOperation(CancellableResult<CompletionLikeOperationParams>::failure(
        CompilerInvocationError));
    return;
  }
  if (!Invocation.getFrontendOptions().InputsAndOutputs.hasInputs()) {
    PerformOperation(CancellableResult<CompletionLikeOperationParams>::failure(
        "no input filenames specified"));
    return;
  }

  // Pin completion instance.
  auto CompletionInst = getIDEInspectionInstance();

  auto CancellationFlag = std::make_shared<std::atomic<bool>>(false);
  ReqTracker->setCancellationHandler(CancellationToken, [CancellationFlag] {
    CancellationFlag->store(true, std::memory_order_relaxed);
  });

  CompletionLikeOperationParams Params = {Invocation, buffer, &CIDiags,
                                          CancellationFlag};
  PerformOperation(
      CancellableResult<CompletionLikeOperationParams>::success(Params));
}

CloseClangModuleFiles::~CloseClangModuleFiles() {
  clang::Preprocessor &PP = loader.getClangPreprocessor();
  clang::ModuleMap &ModMap = PP.getHeaderSearchInfo().getModuleMap();
  for (auto I = ModMap.module_begin(), E = ModMap.module_end(); I != E; ++I) {
    clang::Module *M = I->second;
    if (!M->isSubModule() && M->getASTFile())
      M->getASTFile()->closeFile();
  }
}

void SourceKit::disableExpensiveSILOptions(SILOptions &Opts) {
  // Disable the sanitizers.
  Opts.Sanitizers = {};

  // Disable PGO and code coverage.
  Opts.GenerateProfile = false;
  Opts.EmitProfileCoverageMapping = false;
  Opts.UseProfile = "";
}
