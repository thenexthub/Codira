//===--- CodiraLangSupport.h - -----------------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKIT_LIB_LANGUAGELANG_LANGUAGELANGSUPPORT_H
#define TOOLCHAIN_SOURCEKIT_LIB_LANGUAGELANG_LANGUAGELANGSUPPORT_H

#include "CodeCompletion.h"
#include "SourceKit/Core/Context.h"
#include "SourceKit/Core/LangSupport.h"
#include "SourceKit/Support/Concurrency.h"
#include "SourceKit/Support/Statistic.h"
#include "SourceKit/Support/ThreadSafeRefCntPtr.h"
#include "SourceKit/Support/Tracing.h"
#include "CodiraInterfaceGenContext.h"
#include "language/AST/DiagnosticConsumer.h"
#include "language/AST/PluginRegistry.h"
#include "language/Basic/ThreadSafeRefCounted.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/IDE/CancellableResult.h"
#include "language/IDE/Indenting.h"
#include "language/IDETool/CompileInstance.h"
#include "language/IDETool/IDEInspectionInstance.h"
#include "language/Index/IndexSymbol.h"
#include "language/Refactoring/Refactoring.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/Support/Mutex.h"
#include <map>
#include <string>

namespace language {
  class ASTContext;
  class ClangModuleLoader;
  class CompilerInstance;
  class CompilerInvocation;
  class Decl;
  class Type;
  class AbstractStorageDecl;
  class SourceFile;
  class SILOptions;
  class ValueDecl;
  class GenericSignature;
  enum class AccessorKind;

namespace ide {
  class CodeCompletionCache;
  class IDEInspectionInstance;
  class OnDiskCodeCompletionCache;
  class SourceEditConsumer;
  class SyntacticMacroExpansion;
  enum class CodeCompletionDeclKind : uint8_t;
  enum class SyntaxNodeKind : uint8_t;
  enum class SyntaxStructureKind : uint8_t;
  enum class SyntaxStructureElementKind : uint8_t;
  enum class RangeKind : int8_t;
  class CodeCompletionConsumer;

  enum class NameKind {
    ObjC,
    Codira,
  };
}
}

namespace SourceKit {
  class FileSystemProvider;
  class ImmutableTextSnapshot;
  typedef RefPtr<ImmutableTextSnapshot> ImmutableTextSnapshotRef;
  class CodiraASTManager;
  class CodiraLangSupport;
  class Context;
  class NotificationCenter;

  using TypeContextKind = language::ide::CodeCompletionContext::TypeContextKind;

class CodiraEditorDocument :
    public ThreadSafeRefCountedBase<CodiraEditorDocument> {

  struct Implementation;
  Implementation &Impl;

public:

  CodiraEditorDocument(StringRef FilePath, CodiraLangSupport &LangSupport,
       toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> fileSystem,
       language::ide::CodeFormatOptions Options = language::ide::CodeFormatOptions());
  ~CodiraEditorDocument();

  ImmutableTextSnapshotRef
  initializeText(toolchain::MemoryBuffer *Buf, ArrayRef<const char *> Args,
                 bool ProvideSemanticInfo,
                 toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> fileSystem);
  ImmutableTextSnapshotRef replaceText(unsigned Offset, unsigned Length,
                                       toolchain::MemoryBuffer *Buf,
                                       bool ProvideSemanticInfo,
                                       std::string &error);

  void updateSemaInfo(SourceKitCancellationToken CancellationToken);

  void removeCachedAST();
  void cancelBuildsForCachedAST();

  ImmutableTextSnapshotRef getLatestSnapshot() const;

  void resetSyntaxInfo(ImmutableTextSnapshotRef Snapshot,
                       CodiraLangSupport &Lang);
  void readSyntaxInfo(EditorConsumer &consumer, bool ReportDiags);
  void readSemanticInfo(ImmutableTextSnapshotRef Snapshot,
                        EditorConsumer& Consumer);

  void applyFormatOptions(OptionsDictionary &FmtOptions);
  void formatText(unsigned Line, unsigned Length, EditorConsumer &Consumer);
  void expandPlaceholder(unsigned Offset, unsigned Length,
                         EditorConsumer &Consumer);
  const language::ide::CodeFormatOptions &getFormatOptions();

  static void reportDocumentStructure(language::SourceFile &SrcFile,
                                      EditorConsumer &Consumer);

  std::string getFilePath() const;
      
  /// Returns the virtual filesystem associated with this document.
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> getFileSystem() const;
};

typedef IntrusiveRefCntPtr<CodiraEditorDocument> CodiraEditorDocumentRef;

class CodiraEditorDocumentFileMap {
  WorkQueue Queue{ WorkQueue::Dequeuing::Concurrent,
                   "sourcekit.code.EditorDocFileMap" };
  struct DocInfo {
    CodiraEditorDocumentRef DocRef;
    std::string ResolvedPath;
  };
  toolchain::StringMap<DocInfo> Docs;

public:
  bool getOrUpdate(StringRef FilePath,
                   CodiraLangSupport &LangSupport,
                   CodiraEditorDocumentRef &EditorDoc);
  /// Looks up the document only by the path name that was given initially.
  CodiraEditorDocumentRef getByUnresolvedName(StringRef FilePath);
  /// Looks up the document by resolving symlinks in the paths.
  /// If \p IsRealpath is \c true, then \p FilePath must already be
  /// canonicalized to a realpath.
  CodiraEditorDocumentRef findByPath(StringRef FilePath,
                                    bool IsRealpath = false);
  CodiraEditorDocumentRef remove(StringRef FilePath);
};

namespace CodeCompletion {

/// Provides a thread-safe cache for code completion results that remain valid
/// for the duration of a 'session' - for example, from the point that a user
/// invokes code completion until they accept a completion, or otherwise close
/// the list of completions.
///
/// The contents of the cache can be modified asynchronously during the session,
/// but the contained objects are immutable.
class SessionCache : public ThreadSafeRefCountedBase<SessionCache> {
  std::unique_ptr<toolchain::MemoryBuffer> buffer;
  std::vector<std::string> args;
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> fileSystem;
  CompletionSink sink;
  std::vector<Completion *> sortedCompletions;
  CompletionKind completionKind;
  TypeContextKind typeContextKind;
  bool completionMayUseImplicitMemberExpr;
  FilterRules filterRules;
  toolchain::sys::Mutex mtx;

public:
  SessionCache(CompletionSink &&sink,
               std::unique_ptr<toolchain::MemoryBuffer> &&buffer,
               std::vector<std::string> &&args,
               toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> fileSystem,
               CompletionKind completionKind,
               TypeContextKind typeContextKind, bool mayUseImplicitMemberExpr,
               FilterRules filterRules)
      : buffer(std::move(buffer)), args(std::move(args)),
        fileSystem(std::move(fileSystem)), sink(std::move(sink)),
        completionKind(completionKind), typeContextKind(typeContextKind),
        completionMayUseImplicitMemberExpr(mayUseImplicitMemberExpr),
        filterRules(std::move(filterRules)) {}
  void setSortedCompletions(std::vector<Completion *> &&completions);
  ArrayRef<Completion *> getSortedCompletions();
  toolchain::MemoryBuffer *getBuffer();
  ArrayRef<std::string> getCompilerArgs();
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> getFileSystem();
  const FilterRules &getFilterRules();
  CompletionKind getCompletionKind();
  TypeContextKind getCompletionTypeContextKind();
  bool getCompletionMayUseImplicitMemberExpr();
};
typedef RefPtr<SessionCache> SessionCacheRef;

/// A thread-safe map from (buffer, code complete offset) to \c SessionCache.
class SessionCacheMap {
  mutable unsigned nextBufferID = 0;
  mutable toolchain::StringMap<unsigned> nameToBufferMap;
  typedef std::pair<unsigned, unsigned> Key;
  toolchain::DenseMap<Key, SessionCacheRef> sessions;
  mutable toolchain::sys::Mutex mtx;

  // Should only be called with Mtx locked.
  unsigned getBufferID(StringRef name) const;

public:
  SessionCacheRef get(StringRef name, unsigned offset) const;
  bool set(StringRef name, unsigned offset, SessionCacheRef session);
  bool remove(StringRef name, unsigned offset);
};
} // end namespace CodeCompletion

namespace TypeContextInfo {
struct Options {
  // TypeContextInfo doesn't receive any options at this point.
};
} // namespace TypeContextInfo

namespace ConformingMethodList {
struct Options {
  // ConformingMethodList doesn't receive any options at this point.
};
} // namespace ConformingMethodList

class CodiraInterfaceGenMap {
  toolchain::StringMap<CodiraInterfaceGenContextRef> IFaceGens;
  mutable toolchain::sys::Mutex Mtx;

public:
  CodiraInterfaceGenContextRef get(StringRef Name) const;
  void set(StringRef Name, CodiraInterfaceGenContextRef IFaceGen);
  bool remove(StringRef Name);
  CodiraInterfaceGenContextRef find(StringRef ModuleName,
                                   const language::CompilerInvocation &Invok);
};

struct CodiraCompletionCache
    : public ThreadSafeRefCountedBase<CodiraCompletionCache> {
  std::unique_ptr<language::ide::CodeCompletionCache> inMemory;
  std::unique_ptr<language::ide::OnDiskCodeCompletionCache> onDisk;
  language::ide::CodeCompletionCache &getCache();
  CodiraCompletionCache() = default;
  ~CodiraCompletionCache();
};

struct CodiraPopularAPI : public ThreadSafeRefCountedBase<CodiraPopularAPI> {
  toolchain::StringMap<CodeCompletion::PopularityFactor> nameToFactor;
};

struct CodiraCustomCompletions
    : public ThreadSafeRefCountedBase<CodiraCustomCompletions> {
  std::vector<CustomCompletionInfo> customCompletions;
};

class RequestRefactoringEditConsumer: public language::ide::SourceEditConsumer,
                                      public language::DiagnosticConsumer {
  class Implementation;
  Implementation &Impl;
public:
  RequestRefactoringEditConsumer(CategorizedEditsReceiver Receiver);
  ~RequestRefactoringEditConsumer();
  void accept(language::SourceManager &SM, language::ide::RegionType RegionType,
              ArrayRef<language::ide::Replacement> Replacements) override;
  void handleDiagnostic(language::SourceManager &SM,
                        const language::DiagnosticInfo &Info) override;
};

namespace compile {
class Session {
  language::ide::CompileInstance Compiler;

public:
  Session(const std::string &CodiraExecutablePath,
          const std::string &RuntimeResourcePath,
          std::shared_ptr<language::PluginRegistry> Plugins)
      : Compiler(CodiraExecutablePath, RuntimeResourcePath, Plugins) {}

  bool
  performCompile(toolchain::ArrayRef<const char *> Args,
                 toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FileSystem,
                 language::DiagnosticConsumer *DiagC,
                 std::shared_ptr<std::atomic<bool>> CancellationFlag) {
    return Compiler.performCompile(Args, FileSystem, DiagC, CancellationFlag);
  }
};

class SessionManager {
  const std::string &CodiraExecutablePath;
  const std::string &RuntimeResourcePath;
  const std::shared_ptr<language::PluginRegistry> Plugins;

  toolchain::StringMap<std::shared_ptr<Session>> sessions;
  WorkQueue compileQueue{WorkQueue::Dequeuing::Concurrent,
                         "sourcekit.code.Compile"};
  mutable toolchain::sys::Mutex mtx;

public:
  SessionManager(const std::string &CodiraExecutablePath,
                 const std::string &RuntimeResourcePath,
                 const std::shared_ptr<language::PluginRegistry> Plugins)
      : CodiraExecutablePath(CodiraExecutablePath),
        RuntimeResourcePath(RuntimeResourcePath), Plugins(Plugins) {}

  std::shared_ptr<Session> getSession(StringRef name);

  void clearSession(StringRef name);

  void performCompileAsync(
      StringRef Name, ArrayRef<const char *> Args,
      toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> fileSystem,
      std::shared_ptr<std::atomic<bool>> CancellationFlag,
      std::function<void(const RequestResult<CompilationResult> &)> Receiver);
};
} // namespace compile

struct CodiraStatistics {
#define LANGUAGE_STATISTIC(VAR, UID, DESC)                                        \
  Statistic VAR{UIdent{"source.statistic." #UID}, DESC};
#include "CodiraStatistics.def"
};

class CodiraLangSupport : public LangSupport {
  std::shared_ptr<NotificationCenter> NotificationCtr;
  /// The path of the language-frontend executable.
  /// Used to find clang relative to it.
  std::string CodiraExecutablePath;
  std::string RuntimeResourcePath;
  std::shared_ptr<CodiraASTManager> ASTMgr;
  std::shared_ptr<CodiraEditorDocumentFileMap> EditorDocuments;
  std::shared_ptr<RequestTracker> ReqTracker;
  CodiraInterfaceGenMap IFaceGenContexts;
  ThreadSafeRefCntPtr<CodiraCompletionCache> CCCache;
  ThreadSafeRefCntPtr<CodiraPopularAPI> PopularAPI;
  CodeCompletion::SessionCacheMap CCSessions;
  ThreadSafeRefCntPtr<CodiraCustomCompletions> CustomCompletions;
  std::shared_ptr<CodiraStatistics> Stats;
  toolchain::StringMap<std::unique_ptr<FileSystemProvider>> FileSystemProviders;
  std::shared_ptr<language::ide::IDEInspectionInstance> IDEInspectionInst;
  std::shared_ptr<compile::SessionManager> CompileManager;
  std::shared_ptr<language::ide::SyntacticMacroExpansion> SyntacticMacroExpansions;

public:
  explicit CodiraLangSupport(SourceKit::Context &SKCtx);
  ~CodiraLangSupport();

  std::shared_ptr<NotificationCenter> getNotificationCenter() const {
    return NotificationCtr;
  }

  StringRef getRuntimeResourcePath() const { return RuntimeResourcePath; }

  std::shared_ptr<CodiraASTManager> getASTManager() { return ASTMgr; }

  std::shared_ptr<CodiraEditorDocumentFileMap> getEditorDocuments() { return EditorDocuments; }
  CodiraInterfaceGenMap &getIFaceGenContexts() { return IFaceGenContexts; }
  IntrusiveRefCntPtr<CodiraCompletionCache> getCodeCompletionCache() {
    return CCCache;
  }

  std::shared_ptr<language::ide::IDEInspectionInstance>
  getIDEInspectionInstance() {
    return IDEInspectionInst;
  }

  /// Returns the FileSystemProvider registered under Name, or nullptr if not
  /// found.
  FileSystemProvider *getFileSystemProvider(StringRef Name);

  /// Registers the given FileSystemProvider under Name. The caller is
  /// responsible for keeping FileSystemProvider alive at least as long as
  /// this Context.
  /// This should only be called during setup because it is not synchronized.
  /// \param FileSystemProvider must be non-null
  void setFileSystemProvider(StringRef Name, std::unique_ptr<FileSystemProvider> FileSystemProvider);

  /// Returns the filesystem appropriate to use in a request with the given
  /// \p vfsOptions and \p primaryFile.
  ///
  /// If \p vfsOptions is provided, returns a virtual filesystem created from
  /// a registered FileSystemProvider, or nullptr if there is an error.
  ///
  /// If \p vfsOptions is None and \p primaryFile is provided, returns the
  /// virtual filesystem associated with that editor document, if any.
  ///
  /// Otherwise, returns the real filesystem.
  ///
  /// \param vfsOptions Options to select and initialize the VFS, or None to
  ///                   get the real file system.
  /// \param error Set to a description of the error, if appropriate.
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem>
  getFileSystem(const std::optional<VFSOptions> &vfsOptions,
                std::optional<StringRef> primaryFile, std::string &error);

  static SourceKit::UIdent getUIDForDeclLanguage(const language::Decl *D);
  static SourceKit::UIdent getUIDForDecl(const language::Decl *D,
                                         bool IsRef = false);
  static SourceKit::UIdent getUIDForExtensionOfDecl(const language::Decl *D);
  static SourceKit::UIdent getUIDForLocalVar(bool IsRef = false);
  static SourceKit::UIdent getUIDForRefactoringKind(
      language::ide::RefactoringKind Kind);
  static SourceKit::UIdent getUIDForCodeCompletionDeclKind(
      language::ide::CodeCompletionDeclKind Kind, bool IsRef = false);
  static SourceKit::UIdent getUIDForAccessor(const language::ValueDecl *D,
                                             language::AccessorKind AccKind,
                                             bool IsRef = false);
  static SourceKit::UIdent getUIDForModuleRef();
  static SourceKit::UIdent getUIDForObjCAttr();
  static SourceKit::UIdent getUIDForSyntaxNodeKind(
      language::ide::SyntaxNodeKind Kind);
  static SourceKit::UIdent getUIDForSyntaxStructureKind(
      language::ide::SyntaxStructureKind Kind);
  static SourceKit::UIdent getUIDForSyntaxStructureElementKind(
      language::ide::SyntaxStructureElementKind Kind);

  static SourceKit::UIdent getUIDForSymbol(language::index::SymbolInfo sym,
                                           bool isRef);

  static SourceKit::UIdent getUIDForRangeKind(language::ide::RangeKind Kind);

  static SourceKit::UIdent getUIDForRegionType(language::ide::RegionType Type);

  static SourceKit::UIdent getUIDForRefactoringRangeKind(language::ide::RefactoringRangeKind Kind);

  static std::optional<UIdent>
  getUIDForDeclAttribute(const language::DeclAttribute *Attr);

  static SourceKit::UIdent getUIDForFormalAccessScope(const language::AccessScope Scope);

  static std::vector<UIdent> UIDsFromDeclAttributes(const language::DeclAttributes &Attrs);

  static SourceKit::UIdent getUIDForNameKind(language::ide::NameKind Kind);

  static language::ide::NameKind getNameKindForUID(SourceKit::UIdent Id);

  static bool printDisplayName(const language::ValueDecl *D, toolchain::raw_ostream &OS);

  /// Generate a USR for a Decl, including the prefix.
  /// @param distinguishSynthesizedDecls Whether to use the USR of the
  /// synthesized declaration instead of the USR of the underlying Clang USR.
  /// \returns true if the results should be ignored, false otherwise.
  static bool printUSR(const language::ValueDecl *D, toolchain::raw_ostream &OS,
                       bool distinguishSynthesizedDecls = false);

  /// Generate a USR for the Type of a given decl.
  /// \returns true if the results should be ignored, false otherwise.
  static bool printDeclTypeUSR(const language::ValueDecl *D, toolchain::raw_ostream &OS);

  /// Generate a USR for of a given type.
  /// \returns true if the results should be ignored, false otherwise.
  static bool printTypeUSR(language::Type Ty, toolchain::raw_ostream &OS);

  /// Generate a USR for an accessor, including the prefix.
  /// \returns true if the results should be ignored, false otherwise.
  static bool printAccessorUSR(const language::AbstractStorageDecl *D,
                               language::AccessorKind AccKind,
                               toolchain::raw_ostream &OS);

  /// Annotates a declaration with XML tags that describe the key substructure
  /// of the declaration for CursorInfo/DocInfo.
  ///
  /// Prints declarations with decl- and type-specific tags derived from the
  /// UIDs used for decl/refs.
  ///
  /// FIXME: This move to libIDE, but currently depends on the UIdentVisitor.
  static void printFullyAnnotatedDeclaration(const language::ValueDecl *VD,
                                             language::Type BaseTy,
                                             toolchain::raw_ostream &OS);

  static void printFullyAnnotatedDeclaration(const language::ExtensionDecl *VD,
                                             toolchain::raw_ostream &OS);

  static void
  printFullyAnnotatedSynthesizedDeclaration(const language::ValueDecl *VD,
                                            language::TypeOrExtensionDecl Target,
                                            toolchain::raw_ostream &OS);

  static void
  printFullyAnnotatedSynthesizedDeclaration(const language::ExtensionDecl *ED,
                                            language::TypeOrExtensionDecl Target,
                                            toolchain::raw_ostream &OS);

  /// Print 'description' or 'sourcetext' the given \p VD to \p OS. If
  /// \p usePlaceholder is \c true, call argument positions are substituted with
  /// a typed editor placeholders which is suitable for 'sourcetext'.
  static void
  printMemberDeclDescription(const language::ValueDecl *VD, language::Type baseTy,
                             bool usePlaceholder, toolchain::raw_ostream &OS);

  /// Tries to resolve the path to the real file-system path. If it fails it
  /// returns the original path;
  static std::string resolvePathSymlinks(StringRef FilePath);

  /// The result returned from \c performWithParamsToCompletionLikeOperation.
  struct CompletionLikeOperationParams {
    language::CompilerInvocation &Invocation;
    toolchain::MemoryBuffer *completionBuffer;
    language::DiagnosticConsumer *DiagC;
    std::shared_ptr<std::atomic<bool>> CancellationFlag;
  };

  /// Execute \p PerformOperation synchronously with the parameters necessary to
  /// invoke a completion-like operation on \c IDEInspectionInstance.
  /// If \p InsertCodeCompletionToken is \c true, a code completion token will
  /// be inserted into the source buffer, if \p InsertCodeCompletionToken is \c
  /// false, the buffer is left as-is.
  void performWithParamsToCompletionLikeOperation(
      toolchain::MemoryBuffer *UnresolvedInputFile, unsigned Offset,
      bool InsertCodeCompletionToken, ArrayRef<const char *> Args,
      toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FileSystem,
      SourceKitCancellationToken CancellationToken,
      toolchain::function_ref<
          void(language::ide::CancellableResult<CompletionLikeOperationParams>)>
          PerformOperation);

  //==========================================================================//
  // LangSupport Interface
  //==========================================================================//

  void *getOpaqueCodiraIDEInspectionInstance() override {
    return IDEInspectionInst.get();
  }

  void globalConfigurationUpdated(std::shared_ptr<GlobalConfig> Config) override;

  void dependencyUpdated() override;

  void demangleNames(
      ArrayRef<const char *> MangledNames, bool Simplified,
      std::function<void(const RequestResult<ArrayRef<std::string>> &)>
          Receiver) override;

  void mangleSimpleClassNames(
      ArrayRef<std::pair<StringRef, StringRef>> ModuleClassPairs,
      std::function<void(const RequestResult<ArrayRef<std::string>> &)>
          Receiver) override;

  void indexSource(StringRef Filename, IndexingConsumer &Consumer,
                   ArrayRef<const char *> Args) override;

  void indexToStore(StringRef InputFile,
                    ArrayRef<const char *> Args,
                    IndexStoreOptions Opts,
                    SourceKitCancellationToken CancellationToken,
                    IndexToStoreReceiver Receiver) override;

  void codeComplete(toolchain::MemoryBuffer *InputBuf, unsigned Offset,
                    OptionsDictionary *options,
                    SourceKit::CodeCompletionConsumer &Consumer,
                    ArrayRef<const char *> Args,
                    std::optional<VFSOptions> vfsOptions,
                    SourceKitCancellationToken CancellationToken) override;

  void codeCompleteOpen(StringRef name, toolchain::MemoryBuffer *inputBuf,
                        unsigned offset, OptionsDictionary *options,
                        ArrayRef<FilterRule> rawFilterRules,
                        GroupedCodeCompletionConsumer &consumer,
                        ArrayRef<const char *> args,
                        std::optional<VFSOptions> vfsOptions,
                        SourceKitCancellationToken CancellationToken) override;

  void codeCompleteClose(StringRef name, unsigned offset,
                         GroupedCodeCompletionConsumer &consumer) override;

  void codeCompleteUpdate(StringRef name, unsigned offset,
                          OptionsDictionary *options,
                          SourceKitCancellationToken CancellationToken,
                          GroupedCodeCompletionConsumer &consumer) override;

  void codeCompleteCacheOnDisk(StringRef path) override;

  void codeCompleteSetPopularAPI(ArrayRef<const char *> popularAPI,
                                 ArrayRef<const char *> unpopularAPI) override;

  void
  codeCompleteSetCustom(ArrayRef<CustomCompletionInfo> completions) override;

  void editorOpen(StringRef Name, toolchain::MemoryBuffer *Buf,
                  EditorConsumer &Consumer, ArrayRef<const char *> Args,
                  std::optional<VFSOptions> vfsOptions) override;

  void editorOpenInterface(EditorConsumer &Consumer, StringRef Name,
                           StringRef ModuleName, std::optional<StringRef> Group,
                           ArrayRef<const char *> Args,
                           bool SynthesizedExtensions,
                           std::optional<StringRef> InterestedUSR) override;

  void editorOpenTypeInterface(EditorConsumer &Consumer,
                               ArrayRef<const char *> Args,
                               StringRef TypeUSR) override;

  void editorOpenHeaderInterface(EditorConsumer &Consumer,
                                 StringRef Name,
                                 StringRef HeaderName,
                                 ArrayRef<const char *> Args,
                                 bool UsingCodiraArgs,
                                 bool SynthesizedExtensions,
                                 StringRef languageVersion) override;

  void editorOpenCodiraSourceInterface(
      StringRef Name, StringRef SourceName, ArrayRef<const char *> Args,
      bool CancelOnSubsequentRequest,
      SourceKitCancellationToken CancellationToken,
      std::shared_ptr<EditorConsumer> Consumer) override;

  void editorClose(StringRef Name, bool CancelBuilds,
                   bool RemoveCache) override;

  void editorReplaceText(StringRef Name, toolchain::MemoryBuffer *Buf,
                         unsigned Offset, unsigned Length,
                         EditorConsumer &Consumer) override;

  void editorApplyFormatOptions(StringRef Name,
                                OptionsDictionary &FmtOptions) override;

  void editorFormatText(StringRef Name, unsigned Line, unsigned Length,
                        EditorConsumer &Consumer) override;

  void editorExtractTextFromComment(StringRef Source,
                                    EditorConsumer &Consumer) override;

  void editorConvertMarkupToXML(StringRef Source,
                                EditorConsumer &Consumer) override;

  void editorExpandPlaceholder(StringRef Name, unsigned Offset, unsigned Length,
                               EditorConsumer &Consumer) override;

  void getCursorInfo(StringRef PrimaryFilePath, StringRef InputBufferName,
                     unsigned Offset, unsigned Length, bool Actionables,
                     bool SymbolGraph, bool CancelOnSubsequentRequest,
                     ArrayRef<const char *> Args,
                     std::optional<VFSOptions> vfsOptions,
                     SourceKitCancellationToken CancellationToken,
                     std::function<void(const RequestResult<CursorInfoData> &)>
                         Receiver) override;

  void
  getDiagnostics(StringRef PrimaryFilePath, ArrayRef<const char *> Args,
                 std::optional<VFSOptions> VfsOptions,
                 SourceKitCancellationToken CancellationToken,
                 std::function<void(const RequestResult<DiagnosticsResult> &)>
                     Receiver) override;

  void getSemanticTokens(
      StringRef PrimaryFilePath, StringRef InputBufferName,
      ArrayRef<const char *> Args, std::optional<VFSOptions> VfsOptions,
      SourceKitCancellationToken CancellationToken,
      std::function<void(const RequestResult<SemanticTokensResult> &)> Receiver)
      override;

  void getNameInfo(
      StringRef PrimaryFilePath, StringRef InputBufferName, unsigned Offset,
      NameTranslatingInfo &Input, ArrayRef<const char *> Args,
      SourceKitCancellationToken CancellationToken,
      std::function<void(const RequestResult<NameTranslatingInfo> &)> Receiver)
      override;

  void getRangeInfo(
      StringRef PrimaryFilePath, StringRef InputBufferName, unsigned Offset,
      unsigned Length, bool CancelOnSubsequentRequest,
      ArrayRef<const char *> Args, SourceKitCancellationToken CancellationToken,
      std::function<void(const RequestResult<RangeInfo> &)> Receiver) override;

  void getCursorInfoFromUSR(
      StringRef PrimaryFilePath, StringRef InputBufferName, StringRef USR,
      bool CancelOnSubsequentRequest, ArrayRef<const char *> Args,
      std::optional<VFSOptions> vfsOptions,
      SourceKitCancellationToken CancellationToken,
      std::function<void(const RequestResult<CursorInfoData> &)> Receiver)
      override;

  void findRelatedIdentifiersInFile(
      StringRef PrimaryFilePath, StringRef InputBufferName, unsigned Offset,
      bool IncludeNonEditableBaseNames, bool CancelOnSubsequentRequest,
      ArrayRef<const char *> Args, SourceKitCancellationToken CancellationToken,
      std::function<void(const RequestResult<RelatedIdentsResult> &)> Receiver)
      override;

  void findActiveRegionsInFile(
      StringRef PrimaryFilePath, StringRef InputBufferName,
      ArrayRef<const char *> Args, SourceKitCancellationToken CancellationToken,
      std::function<void(const RequestResult<ActiveRegionsInfo> &)> Receiver)
      override;

  CancellableResult<std::vector<CategorizedRenameRanges>>
  findRenameRanges(toolchain::MemoryBuffer *InputBuf,
                   ArrayRef<RenameLoc> RenameLocations,
                   ArrayRef<const char *> Args) override;

  void findLocalRenameRanges(StringRef Filename, unsigned Line, unsigned Column,
                             unsigned Length, ArrayRef<const char *> Args,
                             bool CancelOnSubsequentRequest,
                             SourceKitCancellationToken CancellationToken,
                             CategorizedRenameRangesReceiver Receiver) override;

  void collectExpressionTypes(
      StringRef PrimaryFilePath, StringRef InputBufferName,
      ArrayRef<const char *> Args, ArrayRef<const char *> ExpectedProtocols,
      bool FullyQualified, bool CanonicalType,
      SourceKitCancellationToken CancellationToken,
      std::function<void(const RequestResult<ExpressionTypesInFile> &)>
          Receiver) override;

  void collectVariableTypes(
      StringRef PrimaryFilePath, StringRef InputBufferName,
      ArrayRef<const char *> Args, std::optional<unsigned> Offset,
      std::optional<unsigned> Length, bool FullyQualified,
      bool CancelOnSubsequentRequest,
      SourceKitCancellationToken CancellationToken,
      std::function<void(const RequestResult<VariableTypesInFile> &)> Receiver)
      override;

  void semanticRefactoring(StringRef PrimaryFilePath,
                           SemanticRefactoringInfo Info,
                           ArrayRef<const char *> Args,
                           bool CancelOnSubsequentRequest,
                           SourceKitCancellationToken CancellationToken,
                           CategorizedEditsReceiver Receiver) override;

  void getDocInfo(toolchain::MemoryBuffer *InputBuf,
                  StringRef ModuleName,
                  ArrayRef<const char *> Args,
                  DocInfoConsumer &Consumer) override;

  std::optional<std::pair<unsigned, unsigned>>
  findUSRRange(StringRef DocumentName, StringRef USR) override;

  void findInterfaceDocument(StringRef ModuleName, ArrayRef<const char *> Args,
               std::function<void(const RequestResult<InterfaceDocInfo> &)> Receiver) override;

  void findModuleGroups(StringRef ModuleName, ArrayRef<const char *> Args,
               std::function<void(const RequestResult<ArrayRef<StringRef>> &)> Receiver) override;

  void getExpressionContextInfo(toolchain::MemoryBuffer *inputBuf, unsigned Offset,
                                OptionsDictionary *options,
                                ArrayRef<const char *> Args,
                                SourceKitCancellationToken CancellationToken,
                                TypeContextInfoConsumer &Consumer,
                                std::optional<VFSOptions> vfsOptions) override;

  void getConformingMethodList(toolchain::MemoryBuffer *inputBuf, unsigned Offset,
                               OptionsDictionary *options,
                               ArrayRef<const char *> Args,
                               ArrayRef<const char *> ExpectedTypes,
                               SourceKitCancellationToken CancellationToken,
                               ConformingMethodListConsumer &Consumer,
                               std::optional<VFSOptions> vfsOptions) override;

  void expandMacroSyntactically(toolchain::MemoryBuffer *inputBuf,
                                ArrayRef<const char *> args,
                                ArrayRef<MacroExpansionInfo> expansions,
                                CategorizedEditsReceiver receiver) override;

  void
  performCompile(StringRef Name, ArrayRef<const char *> Args,
                 std::optional<VFSOptions> vfsOptions,
                 SourceKitCancellationToken CancellationToken,
                 std::function<void(const RequestResult<CompilationResult> &)>
                     Receiver) override;

  void closeCompile(StringRef Name) override;

  void getStatistics(StatisticsReceiver) override;

private:
  language::SourceFile *getSyntacticSourceFile(toolchain::MemoryBuffer *InputBuf,
                                            ArrayRef<const char *> Args,
                                            language::CompilerInstance &ParseCI,
                                            std::string &Error);
};

namespace trace {
  void initTraceInfo(trace::CodiraInvocation &CodiraArgs,
                     StringRef InputFile,
                     ArrayRef<const char *> Args);
  void initTraceInfo(trace::CodiraInvocation &CodiraArgs,
                     StringRef InputFile,
                     ArrayRef<std::string> Args);
}

/// When we cannot build any more clang modules, close the .pcm / files to
/// prevent fd leaks in clients that cache the AST.
// FIXME: Remove this once rdar://problem/19720334 is complete.
class CloseClangModuleFiles {
  language::ClangModuleLoader &loader;

public:
  CloseClangModuleFiles(language::ClangModuleLoader &loader) : loader(loader) {}
  ~CloseClangModuleFiles();
};


/// Disable expensive SIL options which do not affect indexing or diagnostics.
void disableExpensiveSILOptions(language::SILOptions &Opts);

language::SourceFile *retrieveInputFile(StringRef inputBufferName,
                                     const language::CompilerInstance &CI,
                                     bool haveRealPath = false);

} // namespace SourceKit

#endif
