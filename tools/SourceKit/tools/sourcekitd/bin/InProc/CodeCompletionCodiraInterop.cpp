//===----------------------------------------------------------------------===//
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

#include "CodeCompletionCodiraInterop.h"
#include "SourceKit/Core/Context.h"
#include "sourcekitd/sourcekitdInProc-Internal.h"
#include "language/AST/ASTPrinter.h"
#include "language/AST/USRGeneration.h"
#include "language/Basic/StringExtras.h"
#include "language/Driver/FrontendUtil.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/IDE/CodeCompletion.h"
#include "language/IDE/CodeCompletionCache.h"
#include "language/IDE/CodeCompletionResultPrinter.h"
#include "language/IDE/FuzzyStringMatcher.h"
#include "language/IDE/Utils.h"
#include "language/IDETool/CompilerInvocation.h"
#include "language/IDETool/IDEInspectionInstance.h"
#include "toolchain/Support/Signals.h"
#include <mutex>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using namespace language;
using namespace language::ide;
using namespace sourcekitdInProc;

static std::string getRuntimeResourcesPath();

using FileSystemRef = toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem>;

namespace {
struct CompletionResult;

class SharedStringInMemoryFS : public toolchain::vfs::InMemoryFileSystem {
  SmallVector<std::shared_ptr<std::string>, 8> strings;

public:
  SharedStringInMemoryFS(
      const toolchain::StringMap<std::shared_ptr<std::string>> &files) {
    strings.reserve(files.size());
    for (auto &pair : files) {
      strings.push_back(pair.getValue());
      auto buffer =
          toolchain::MemoryBuffer::getMemBuffer(*pair.getValue(), pair.getKey());
      if (!addFile(pair.getKey(), /*ModTime=*/0, std::move(buffer))) {
        // FIXME: report error!
      }
    }
  }
};

class Connection {
  IDEInspectionInstance *ideInspectionInstance;
  /// If the connection was not passed an `IDEInspectionInstance` it creates
  /// its own. This unique_ptr scopes the lifetime of 'ideInspectionInstance'
  /// to the lifetime of 'Connection'.
  std::unique_ptr<IDEInspectionInstance> ownedIDEInspectionInstance;
  toolchain::StringMap<std::shared_ptr<std::string>> modifiedFiles;
  std::shared_ptr<CodeCompletionCache> completionCache;
  std::string languageExecutablePath;
  std::string runtimeResourcePath;
  std::shared_ptr<SourceKit::RequestTracker> requestTracker;

public:
  std::unique_ptr<CompletionResult> currentResponse;
  const time_t sessionTimestamp;

  Connection(IDEInspectionInstance *ideInspectionInstance)
      : ideInspectionInstance(ideInspectionInstance),
        completionCache(std::make_shared<CodeCompletionCache>()),
        languageExecutablePath(getCodiraExecutablePath()),
        runtimeResourcePath(getRuntimeResourcesPath()),
        requestTracker(new SourceKit::RequestTracker()),
        sessionTimestamp(toolchain::sys::toTimeT(std::chrono::system_clock::now())) {
    if (ideInspectionInstance == nullptr) {
      this->ownedIDEInspectionInstance.reset(new IDEInspectionInstance());
      this->ideInspectionInstance = this->ownedIDEInspectionInstance.get();
    }
  }

  void setFileContents(StringRef path, const char *contents) {
    if (contents) {
      modifiedFiles[path] = std::make_shared<std::string>(contents);
    } else {
      modifiedFiles.erase(path);
    }
  }

  std::shared_ptr<CodeCompletionCache> getCompletionCache() const {
    return completionCache;
  }

  FileSystemRef createFileSystem() {
    auto *overlayFS =
        new toolchain::vfs::OverlayFileSystem(toolchain::vfs::getRealFileSystem());
    overlayFS->pushOverlay(new SharedStringInMemoryFS(modifiedFiles));
    return overlayFS;
  }

  void cancelRequest(SourceKit::SourceKitCancellationToken cancellationToken) {
    requestTracker->cancel(cancellationToken);
  }

  void codeComplete(
      StringRef path, unsigned offset, ArrayRef<const char *> args,
      FileSystemRef fileSystem, ide::CodeCompletionContext &completionContext,
      SourceKit::SourceKitCancellationToken cancellationToken,
      toolchain::function_ref<void(CancellableResult<CodeCompleteResult>)> callback);

  void markCachedCompilerInstanceShouldBeInvalidated() {
    ideInspectionInstance->markCachedCompilerInstanceShouldBeInvalidated();
  }
};

} // anonymous namespace

languageide_connection_t languageide_connection_create(void) {
  return languageide_connection_create_with_inspection_instance(nullptr);
}

languageide_connection_t languageide_connection_create_with_inspection_instance(
    void *opaqueIDECodiraInspectionInstance) {
  static std::once_flag once;
  std::call_once(
      once, [] { toolchain::sys::PrintStackTraceOnErrorSignal("IDECodiraInterop"); });
  IDEInspectionInstance *inspectInstance =
      static_cast<IDEInspectionInstance *>(opaqueIDECodiraInspectionInstance);

  return static_cast<languageide_connection_t>(new Connection(inspectInstance));
}

void languageide_connection_dispose(languageide_connection_t conn) {
  assert(conn);
  delete static_cast<Connection *>(conn);
}

void languageide_connection_mark_cached_compiler_instance_should_be_invalidated(
    languageide_connection_t _conn, languageide_cache_invalidation_options_t _opts) {
  auto *conn = static_cast<Connection *>(_conn);
  // '_opts' is not used at this point.
  assert(conn);
  conn->markCachedCompilerInstanceShouldBeInvalidated();
}

void languageide_set_file_contents(languageide_connection_t _conn, const char *path,
                                const char *contents) {
  auto *conn = static_cast<Connection *>(_conn);
  assert(conn);
  conn->setFileContents(path, contents);
}

namespace {
struct CompletionResult {
  Connection &conn;
  CodeCompletionContext context;
  ImportDepth importDepth;
  std::string error;
  bool isCancelled = false;
  SmallString<0> scratch;
  CodeCompletionResultSink resultSink;
  /// The compiler instance that produced the results. Used to lazily compute
  /// diagnostics for results.
  std::shared_ptr<CompilerInstance> compilerInstance;

  CompletionResult(Connection &conn)
      : conn(conn), context(*conn.getCompletionCache()) {
    // Pre-allocate a whole page. Empirically, this is enough to cover the vast
    // majority of cases.
    scratch.reserve(4096);
  }

  bool hasError() const { return !error.empty(); }

  ArrayRef<CodeCompletionResult *> getCompletions() {
    return resultSink.Results;
  }
};

struct CodiraInteropCodeCompletionConsumer : public ide::CodeCompletionConsumer {
  CompletionResult &result;

  CodiraInteropCodeCompletionConsumer(CompletionResult &result)
      : result(result) {}

  void handleResults(CodeCompletionContext &context) override {
    assert(&context == &(result.context));
  }
};

struct CompletionRequest {
  toolchain::BumpPtrAllocator allocator;
  StringRef path;
  unsigned offset;
  std::vector<const char *> compilerArguments;
  bool annotateResult = false;
  bool includeObjectLiterals = true;
  bool addInitsToTopLevel = false;
  bool addCallWithNoDefaultArgs = true;

  CompletionRequest(const char *path, unsigned offset,
                    ArrayRef<const char *> args) {
    this->path = StringRef(path).copy(allocator);
    this->offset = offset;
    compilerArguments.reserve(args.size());
    for (const char *arg : args) {
      compilerArguments.push_back(copyCString(arg, allocator));
    }
  }
};

} // namespace

void languageide_cancel_request(languageide_connection_t _conn,
                             languageide_request_handle_t handle) {
  assert(_conn);
  auto *conn = static_cast<Connection *>(_conn);
  conn->cancelRequest(handle);
}

languageide_completion_request_t
languageide_completion_request_create(const char *path, uint32_t offset,
                                   char *const *const compiler_args,
                                   uint32_t num_compiler_args) {

  return new CompletionRequest(
      path, offset, toolchain::ArrayRef(compiler_args, num_compiler_args));
}

void languageide_completion_request_dispose(languageide_completion_request_t _req) {
  delete static_cast<CompletionRequest *>(_req);
}

void languageide_completion_request_set_annotate_result(
    languageide_completion_request_t _req, bool annotate) {
  auto &req = *static_cast<CompletionRequest *>(_req);
  req.annotateResult = annotate;
}

void languageide_completion_request_set_include_objectliterals(
    languageide_completion_request_t _req, bool flag) {
  auto &req = *static_cast<CompletionRequest *>(_req);
  req.includeObjectLiterals = flag;
}

void languageide_completion_request_set_add_inits_to_top_level(
    languageide_completion_request_t _req, bool flag) {
  auto &req = *static_cast<CompletionRequest *>(_req);
  req.addInitsToTopLevel = flag;
}

void languageide_completion_request_set_add_call_with_no_default_args(
    languageide_completion_request_t _req, bool flag) {
  auto &req = *static_cast<CompletionRequest *>(_req);
  req.addCallWithNoDefaultArgs = flag;
}

languageide_completion_response_t
languageide_complete_cancellable(languageide_connection_t _conn,
                              languageide_completion_request_t _req,
                              languageide_request_handle_t handle) {
  assert(_conn && _req);
  auto *conn = static_cast<Connection *>(_conn);
  auto &req = *static_cast<CompletionRequest *>(_req);

  if (conn->currentResponse) {
    toolchain::report_fatal_error(
        "must dispose of previous response before completing again");
  }

  conn->currentResponse = std::make_unique<CompletionResult>(*conn);
  auto result = conn->currentResponse.get();

  CodiraInteropCodeCompletionConsumer consumer(*result);

  auto fileSystem = conn->createFileSystem();

  result->context.setAnnotateResult(req.annotateResult);
  result->context.setIncludeObjectLiterals(req.includeObjectLiterals);
  result->context.setAddInitsToTopLevel(req.addInitsToTopLevel);
  result->context.setAddCallWithNoDefaultArgs(req.addCallWithNoDefaultArgs);

  conn->codeComplete(
      req.path, req.offset, req.compilerArguments, fileSystem, result->context,
      handle, [&result](CancellableResult<CodeCompleteResult> completeResult) {
        switch (completeResult.getKind()) {
        case CancellableResultKind::Success:
          result->importDepth = completeResult->ImportDep;
          result->resultSink = completeResult->ResultSink;
          result->compilerInstance = completeResult->Info.compilerInstance;
          break;
        case CancellableResultKind::Failure:
          result->error = completeResult.getError();
          break;
        case CancellableResultKind::Cancelled:
          result->isCancelled = true;
          break;
        }
      });

  return static_cast<languageide_completion_response_t>(result);
}

languageide_completion_response_t
languageide_complete(languageide_connection_t _conn,
                  languageide_completion_request_t _req) {
  // handle = nullptr indicates that the request can't be cancelled.
  return languageide_complete_cancellable(_conn, _req, /*handle=*/nullptr);
}

void Connection::codeComplete(
    StringRef path, unsigned offset, ArrayRef<const char *> args,
    FileSystemRef fileSystem, ide::CodeCompletionContext &completionContext,
    SourceKit::SourceKitCancellationToken cancellationToken,
    toolchain::function_ref<void(CancellableResult<CodeCompleteResult>)> callback) {
  using ResultType = CancellableResult<CodeCompleteResult>;
  // Resolve symlinks for the input file; we resolve them for the input files
  // in the arguments as well.
  // FIXME: We need the Codira equivalent of Clang's FileEntry.
  toolchain::SmallString<128> bufferIdentifier;
  if (auto err = fileSystem->getRealPath(path, bufferIdentifier))
    bufferIdentifier = path;

  auto inputFile = fileSystem->openFileForRead(path);
  if (auto err = inputFile.getError()) {
    std::string error;
    toolchain::raw_string_ostream OS(error);
    OS << "failed to open '" << path << "': " << err.message();
    callback(ResultType::failure(error));
    return;
  }

  auto inputBuffer = inputFile->get()->getBuffer(bufferIdentifier);
  if (auto err = inputBuffer.getError()) {
    std::string error;
    toolchain::raw_string_ostream OS(error);
    OS << "failed to read '" << path << "': " << err.message();
    callback(ResultType::failure(error));
    return;
  }

  // Create a buffer for code completion. This contains '\0' at 'Offset'
  // position of 'UnresolvedInputFile' buffer.
  auto newBuffer = ide::makeCodeCompletionMemoryBuffer(
      inputBuffer.get().get(), offset, bufferIdentifier);

  CompilerInvocation invocation;
  SourceManager SM;
  DiagnosticEngine diags(SM);
  ForwardingDiagnosticConsumer ciDiags(diags);
  PrintingDiagnosticConsumer printDiags;
  diags.addConsumer(printDiags);

  std::string compilerInvocationError;
  bool creatingInvocationFailed = initCompilerInvocation(
      invocation, args, FrontendOptions::ActionType::Typecheck, diags, path,
      fileSystem, languageExecutablePath, runtimeResourcePath, sessionTimestamp,
      compilerInvocationError);
  if (creatingInvocationFailed) {
    callback(ResultType::failure(compilerInvocationError));
    return;
  } else if (!invocation.getFrontendOptions().InputsAndOutputs.hasInputs()) {
    callback(ResultType::failure("no input filenames specified"));
    return;
  }
  auto cancellationFlag = std::make_shared<std::atomic<bool>>(false);
  requestTracker->setCancellationHandler(
      cancellationToken, [cancellationFlag]() {
        cancellationFlag->store(true, std::memory_order_relaxed);
      });

  ideInspectionInstance->codeComplete(
      invocation, args, fileSystem, newBuffer.get(), offset, &ciDiags,
      completionContext, cancellationFlag, callback);
}

void languageide_completion_result_dispose(languageide_completion_response_t result) {
  auto *response = static_cast<CompletionResult *>(result);
  auto &conn = response->conn;
  assert(conn.currentResponse.get() == response);
  conn.currentResponse = nullptr;
}

bool languageide_completion_result_is_error(
    languageide_completion_response_t _result) {
  auto &result = *static_cast<CompletionResult *>(_result);
  return result.hasError();
}

const char *languageide_completion_result_get_error_description(
    languageide_completion_response_t _result) {
  auto &result = *static_cast<CompletionResult *>(_result);
  return result.error.c_str();
}

bool languageide_completion_result_is_cancelled(
    languageide_completion_response_t _result) {
  auto result = static_cast<CompletionResult *>(_result);
  return result->isCancelled;
}

/// Copies a string representation of the completion result. This string should
/// be disposed of with \c free when done.
const char *languageide_completion_result_description_copy(
    languageide_completion_response_t _result) {
  auto &result = *static_cast<CompletionResult *>(_result);
  std::string desc;
  do {
    toolchain::raw_string_ostream OS(desc);
    if (result.hasError()) {
      OS << "error: " << result.error;
      break;
    }

    /// FXIME: this code copied from PrintingCodeCompletionConsumer
    OS << "Begin completions, " << result.getCompletions().size() << " items\n";
    for (auto *item : result.getCompletions()) {
      item->printPrefix(OS);
      if (result.context.getAnnotateResult()) {
        printCodeCompletionResultDescriptionAnnotated(
            *item, OS, /*leadingPunctuation=*/false);
        OS << "; typename=";
        printCodeCompletionResultTypeNameAnnotated(*item, OS);
      } else {
        item->getCompletionString()->print(OS);
      }

      OS << "; name=" << item->getFilterName();
      OS << "\n";
    }
    OS << "End completions\n";
  } while (0);
  return strdup(desc.c_str());
}

void languageide_completion_result_get_completions(
    languageide_completion_response_t _result,
    void (^completions_handler)(const languageide_completion_item_t *completions,
                                const char **filter_names,
                                uint64_t num_completions)) {
  auto &result = *static_cast<CompletionResult *>(_result);
  if (result.hasError() || result.getCompletions().empty()) {
    completions_handler(nullptr, nullptr, 0);
    return;
  }

  std::vector<const char *> filterNames;
  filterNames.reserve(result.getCompletions().size());
  for (auto *item : result.getCompletions()) {
    filterNames.push_back(item->getFilterName().data());
  }

  assert(filterNames.size() == result.getCompletions().size());

  completions_handler(
      (const languageide_completion_item_t *)result.getCompletions().data(),
      filterNames.data(), result.getCompletions().size());
}

languageide_completion_item_t languageide_completion_result_get_completion_at_index(
    languageide_completion_response_t _response, uint64_t index) {
  auto &response = *static_cast<CompletionResult *>(_response);
  if (response.hasError() || response.getCompletions().size() < index) {
    return nullptr;
  }
  return response.getCompletions()[index];
}

languageide_completion_kind_t
languageide_completion_result_get_kind(languageide_completion_response_t _response) {
  auto &response = *static_cast<CompletionResult *>(_response);
  switch (response.context.CodeCompletionKind) {
  case CompletionKind::None:
    return LANGUAGEIDE_COMPLETION_KIND_NONE;
  case CompletionKind::Import:
    return LANGUAGEIDE_COMPLETION_KIND_IMPORT;
  case CompletionKind::Using:
    return LANGUAGEIDE_COMPLETION_KIND_USING;
  case CompletionKind::UnresolvedMember:
    return LANGUAGEIDE_COMPLETION_KIND_UNRESOLVEDMEMBER;
  case CompletionKind::DotExpr:
    return LANGUAGEIDE_COMPLETION_KIND_DOTEXPR;
  case CompletionKind::StmtOrExpr:
    return LANGUAGEIDE_COMPLETION_KIND_STMTOREXPR;
  case CompletionKind::PostfixExprBeginning:
    return LANGUAGEIDE_COMPLETION_KIND_POSTFIXEXPRBEGINNING;
  case CompletionKind::PostfixExpr:
    return LANGUAGEIDE_COMPLETION_KIND_POSTFIXEXPR;
  case CompletionKind::KeyPathExprObjC:
    return LANGUAGEIDE_COMPLETION_KIND_KEYPATHEXPROBJC;
  case CompletionKind::KeyPathExprCodira:
    return LANGUAGEIDE_COMPLETION_KIND_KEYPATHEXPRLANGUAGE;
  case CompletionKind::TypePossibleFunctionParamBeginning:
    return LANGUAGEIDE_COMPLETION_KIND_TYPEPOSSIBLEFUNCTIONPARAMBEGINNING;
  case CompletionKind::TypeDeclResultBeginning:
    return LANGUAGEIDE_COMPLETION_KIND_TYPEDECLRESULTBEGINNING;
  case CompletionKind::TypeBeginning:
    return LANGUAGEIDE_COMPLETION_KIND_TYPEBEGINNING;
  case CompletionKind::TypeSimpleOrComposition:
    return LANGUAGEIDE_COMPLETION_KIND_TYPESIMPLEORCOMPOSITION;
  case CompletionKind::TypeSimpleBeginning:
    return LANGUAGEIDE_COMPLETION_KIND_TYPESIMPLEBEGINNING;
  case CompletionKind::TypeSimpleWithDot:
    // TODO: check if this is still correct
    return LANGUAGEIDE_COMPLETION_KIND_TYPEIDENTIFIERWITHDOT;
  case CompletionKind::TypeSimpleWithoutDot:
    // TODO: check if this is still correct
    return LANGUAGEIDE_COMPLETION_KIND_TYPEIDENTIFIERWITHOUTDOT;
  case CompletionKind::CaseStmtKeyword:
    return LANGUAGEIDE_COMPLETION_KIND_CASESTMTKEYWORD;
  case CompletionKind::CaseStmtBeginning:
    return LANGUAGEIDE_COMPLETION_KIND_CASESTMTBEGINNING;
  case CompletionKind::NominalMemberBeginning:
    return LANGUAGEIDE_COMPLETION_KIND_NOMINALMEMBERBEGINNING;
  case CompletionKind::AccessorBeginning:
    return LANGUAGEIDE_COMPLETION_KIND_ACCESSORBEGINNING;
  case CompletionKind::AttributeBegin:
    return LANGUAGEIDE_COMPLETION_KIND_ATTRIBUTEBEGIN;
  case CompletionKind::AttributeDeclParen:
    return LANGUAGEIDE_COMPLETION_KIND_ATTRIBUTEDECLPAREN;
  case CompletionKind::EffectsSpecifier:
    return LANGUAGEIDE_COMPLETION_KIND_EFFECTSSPECIFIER;
  case CompletionKind::PoundAvailablePlatform:
    return LANGUAGEIDE_COMPLETION_KIND_POUNDAVAILABLEPLATFORM;
  case CompletionKind::CallArg:
    return LANGUAGEIDE_COMPLETION_KIND_CALLARG;
  case CompletionKind::ReturnStmtExpr:
    return LANGUAGEIDE_COMPLETION_KIND_RETURNSTMTEXPR;
  case CompletionKind::YieldStmtExpr:
    return LANGUAGEIDE_COMPLETION_KIND_YIELDSTMTEXPR;
  case CompletionKind::ForEachSequence:
    return LANGUAGEIDE_COMPLETION_KIND_FOREACHSEQUENCE;
  case CompletionKind::ForEachInKw:
    return LANGUAGEIDE_COMPLETION_KIND_FOREACHKWIN;
  case CompletionKind::AfterPoundExpr:
    return LANGUAGEIDE_COMPLETION_KIND_AFTERPOUNDEXPR;
  case CompletionKind::AfterPoundDirective:
    return LANGUAGEIDE_COMPLETION_KIND_AFTERPOUNDDIRECTIVE;
  case CompletionKind::PlatformConditon:
    return LANGUAGEIDE_COMPLETION_KIND_PLATFORMCONDITON;
  case CompletionKind::AfterIfStmtElse:
    return LANGUAGEIDE_COMPLETION_KIND_AFTERIFSTMTELSE;
  case CompletionKind::GenericRequirement:
    return LANGUAGEIDE_COMPLETION_KIND_GENERICREQUIREMENT;
  case CompletionKind::PrecedenceGroup:
    return LANGUAGEIDE_COMPLETION_KIND_PRECEDENCEGROUP;
  case CompletionKind::StmtLabel:
    return LANGUAGEIDE_COMPLETION_KIND_STMTLABEL;
  case CompletionKind::ForEachPatternBeginning:
    return LANGUAGEIDE_COMPLETION_KIND_FOREACHPATTERNBEGINNING;
  case CompletionKind::TypeAttrBeginning:
    return LANGUAGEIDE_COMPLETION_KIND_TYPEATTRBEGINNING;
  case CompletionKind::TypeAttrInheritanceBeginning:
    return LANGUAGEIDE_COMPLETION_KIND_TYPEATTRINHERITANCEBEGINNING;
  case CompletionKind::OptionalBinding:
    return LANGUAGEIDE_COMPLETION_KIND_OPTIONALBINDING;
  case CompletionKind::TypeSimpleInverted:
    return LANGUAGEIDE_COMPLETION_KIND_TYPESIMPLEINVERTED;
  case CompletionKind::ThenStmtExpr:
    return LANGUAGEIDE_COMPLETION_KIND_THENSTMTEXPR;
  }
}

void languageide_completion_result_foreach_baseexpr_typename(
    languageide_completion_response_t _response, bool (^handler)(const char *)) {
  auto &response = *static_cast<CompletionResult *>(_response);
  for (const auto typeName : response.context.LookedupNominalTypeNames) {
    if (/*shouldStop=*/handler(typeName.data())) {
      return;
    }
  }
}

bool languageide_completion_result_is_reusing_astcontext(
    languageide_completion_response_t _response) {
  auto &response = *static_cast<CompletionResult *>(_response);
  return response.context.ReusingASTContext;
}

const char *
languageide_completion_item_description_copy(languageide_completion_item_t _item) {
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  toolchain::SmallString<256> buffer;
  {
    toolchain::raw_svector_ostream OS(buffer);
    item.printPrefix(OS);
    item.getCompletionString()->print(OS);
  }
  return strdup(buffer.c_str());
}

void languageide_completion_item_get_label(
    languageide_completion_response_t _response, languageide_completion_item_t _item,
    bool annotate, void (^handler)(const char *)) {
  auto &response = *static_cast<CompletionResult *>(_response);
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  response.scratch.clear();
  {
    toolchain::raw_svector_ostream OS(response.scratch);
    if (annotate) {
      printCodeCompletionResultDescriptionAnnotated(item, OS, false);
    } else {
      printCodeCompletionResultDescription(item, OS, false);
    }
  }
  handler(response.scratch.c_str());
}

void languageide_completion_item_get_source_text(
    languageide_completion_response_t _response, languageide_completion_item_t _item,
    void (^handler)(const char *)) {
  auto &response = *static_cast<CompletionResult *>(_response);
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  response.scratch.clear();
  {
    toolchain::raw_svector_ostream OS(response.scratch);
    printCodeCompletionResultSourceText(item, OS);
  }
  handler(response.scratch.c_str());
}

void languageide_completion_item_get_type_name(
    languageide_completion_response_t _response, languageide_completion_item_t _item,
    bool annotate, void (^handler)(const char *)) {
  auto &response = *static_cast<CompletionResult *>(_response);
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  response.scratch.clear();
  {
    toolchain::raw_svector_ostream OS(response.scratch);
    if (annotate) {
      printCodeCompletionResultTypeNameAnnotated(item, OS);
    } else {
      printCodeCompletionResultTypeName(item, OS);
    }
  }
  handler(response.scratch.empty() ? nullptr : response.scratch.c_str());
}

void languageide_completion_item_get_doc_brief(
    languageide_completion_response_t _response, languageide_completion_item_t _item,
    void (^handler)(const char *)) {
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  if (item.getBriefDocComment().empty()) {
    return handler(nullptr);
  }
  handler(item.getBriefDocComment().data());
}

void languageide_completion_item_get_associated_usrs(
    languageide_completion_response_t _response, languageide_completion_item_t _item,
    void (^handler)(const char **, uint64_t)) {
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  toolchain::SmallVector<const char *, 4> usrs;
  for (auto usr : item.getAssociatedUSRs()) {
    usrs.push_back(usr.data());
  }
  handler(usrs.data(), usrs.size());
}

uint32_t languageide_completion_item_get_kind(languageide_completion_item_t _item) {
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  // FIXME: keep this in sync with ide::CodeCompletionResult
  return static_cast<languageide_completion_item_kind_t>(item.getKind());
}

uint32_t
languageide_completion_item_get_associated_kind(languageide_completion_item_t _item) {
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  // FIXME: keep this in sync with ide::CodeCompletionResult
  switch (item.getKind()) {
  case CodeCompletionResultKind::Declaration:
    switch (item.getAssociatedDeclKind()) {
#define CASE(KIND, VAL)                                                        \
  case language::ide::CodeCompletionDeclKind::KIND:                               \
    return LANGUAGEIDE_COMPLETION_ITEM_DECL_KIND_##VAL;

      CASE(Module, MODULE)
      CASE(Class, CLASS)
      CASE(Actor, ACTOR)
      CASE(Struct, STRUCT)
      CASE(Enum, ENUM)
      CASE(EnumElement, ENUMELEMENT)
      CASE(Protocol, PROTOCOL)
      CASE(AssociatedType, ASSOCIATEDTYPE)
      CASE(TypeAlias, TYPEALIAS)
      CASE(GenericTypeParam, GENERICTYPEPARAM)
      CASE(Constructor, CONSTRUCTOR)
      CASE(Destructor, DESTRUCTOR)
      CASE(Subscript, SUBSCRIPT)
      CASE(StaticMethod, STATICMETHOD)
      CASE(InstanceMethod, INSTANCEMETHOD)
      CASE(PrefixOperatorFunction, PREFIXOPERATORFUNCTION)
      CASE(PostfixOperatorFunction, POSTFIXOPERATORFUNCTION)
      CASE(InfixOperatorFunction, INFIXOPERATORFUNCTION)
      CASE(FreeFunction, FREEFUNCTION)
      CASE(StaticVar, STATICVAR)
      CASE(InstanceVar, INSTANCEVAR)
      CASE(LocalVar, LOCALVAR)
      CASE(GlobalVar, GLOBALVAR)
      CASE(PrecedenceGroup, PRECEDENCEGROUP)
      CASE(Macro, MACRO)
#undef CASE
    }
    toolchain_unreachable("unhandled enum value");
  case CodeCompletionResultKind::Literal:
    return static_cast<uint32_t>(item.getLiteralKind());
  case CodeCompletionResultKind::Keyword:
    return static_cast<uint32_t>(item.getKeywordKind());
  default:
    return 0;
  }
}

uint32_t languageide_completion_item_get_semantic_context(
    languageide_completion_item_t _item) {
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  switch (item.getSemanticContext()) {
  case language::ide::SemanticContextKind::None:
    return LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_NONE;
  case language::ide::SemanticContextKind::Local:
    return LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_LOCAL;
  case language::ide::SemanticContextKind::CurrentNominal:
    return LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_CURRENTNOMINAL;
  case language::ide::SemanticContextKind::Super:
    return LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_SUPER;
  case language::ide::SemanticContextKind::OutsideNominal:
    return LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_OUTSIDENOMINAL;
  case language::ide::SemanticContextKind::CurrentModule:
    return LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_CURRENTMODULE;
  case language::ide::SemanticContextKind::OtherModule:
    return LANGUAGEIDE_COMPLETION_SEMANTIC_CONTEXT_OTHERMODULE;
  }
}

uint32_t languageide_completion_item_get_flair(languageide_completion_item_t _item) {
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  uint32_t result = 0;
  auto flair = item.getFlair();
  if (flair.contains(CodeCompletionFlairBit::ExpressionSpecific))
    result |= LANGUAGEIDE_COMPLETION_FLAIR_EXPRESSIONSPECIFIC;
  if (flair.contains(CodeCompletionFlairBit::SuperChain))
    result |= LANGUAGEIDE_COMPLETION_FLAIR_SUPERCHAIN;
  if (flair.contains(CodeCompletionFlairBit::ArgumentLabels))
    result |= LANGUAGEIDE_COMPLETION_FLAIR_ARGUMENTLABELS;
  if (flair.contains(CodeCompletionFlairBit::CommonKeywordAtCurrentPosition))
    result |= LANGUAGEIDE_COMPLETION_FLAIR_COMMONKEYWORDATCURRENTPOSITION;
  if (flair.contains(CodeCompletionFlairBit::RareKeywordAtCurrentPosition))
    result |= LANGUAGEIDE_COMPLETION_FLAIR_RAREKEYWORDATCURRENTPOSITION;
  if (flair.contains(CodeCompletionFlairBit::RareTypeAtCurrentPosition))
    result |= LANGUAGEIDE_COMPLETION_FLAIR_RARETYPEATCURRENTPOSITION;
  if (flair.contains(
          CodeCompletionFlairBit::ExpressionAtNonScriptOrMainFileScope))
    result |= LANGUAGEIDE_COMPLETION_FLAIR_EXPRESSIONATNONSCRIPTORMAINFILESCOPE;
  return result;
}

bool languageide_completion_item_is_not_recommended(
    languageide_completion_item_t _item) {
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  return item.isNotRecommended();
}

uint32_t languageide_completion_item_not_recommended_reason(
    languageide_completion_item_t _item) {
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  switch (item.getNotRecommendedReason()) {
  case NotRecommendedReason::None:
    return LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_NONE;
  case NotRecommendedReason::RedundantImport:
    return LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_REDUNDANT_IMPORT;
  case NotRecommendedReason::RedundantImportIndirect:
    return LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_REDUNDANT_IMPORT_INDIRECT;
  case NotRecommendedReason::Deprecated:
    return LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_DEPRECATED;
  case NotRecommendedReason::SoftDeprecated:
    return LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_SOFTDEPRECATED;
  case NotRecommendedReason::VariableUsedInOwnDefinition:
    return LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_VARIABLE_USED_IN_OWN_DEFINITION;
  case NotRecommendedReason::NonAsyncAlternativeUsedInAsyncContext:
    return LANGUAGEIDE_COMPLETION_NOT_RECOMMENDED_NON_ASYNC_ALTERNATIVE_USED_IN_ASYNC_CONTEXT;
  }
}

bool languageide_completion_item_has_diagnostic(languageide_completion_item_t _item) {
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  return (item.getNotRecommendedReason() != NotRecommendedReason::None);
}

void languageide_completion_item_get_diagnostic(
    languageide_completion_response_t _response, languageide_completion_item_t _item,
    void (^handler)(languageide_completion_diagnostic_severity_t, const char *)) {
  auto &response = *static_cast<CompletionResult *>(_response);
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  languageide_completion_diagnostic_severity_t severity;
  toolchain::SmallString<64> scratch;
  auto severityAndMessage = item.getDiagnosticSeverityAndMessage(
      scratch, response.compilerInstance->getASTContext());
  switch (severityAndMessage.first) {
  case CodeCompletionDiagnosticSeverity::None:
    handler(LANGUAGEIDE_COMPLETION_DIAGNOSTIC_SEVERITY_NONE, nullptr);
    return;
  case CodeCompletionDiagnosticSeverity::Error:
    severity = LANGUAGEIDE_COMPLETION_DIAGNOSTIC_SEVERITY_ERROR;
    break;
  case CodeCompletionDiagnosticSeverity::Warning:
    severity = LANGUAGEIDE_COMPLETION_DIAGNOSTIC_SEVERITY_WARNING;
    break;
  case CodeCompletionDiagnosticSeverity::Remark:
    severity = LANGUAGEIDE_COMPLETION_DIAGNOSTIC_SEVERITY_REMARK;
    break;
  case CodeCompletionDiagnosticSeverity::Note:
    severity = LANGUAGEIDE_COMPLETION_DIAGNOSTIC_SEVERITY_NOTE;
    break;
  }

  handler(severity, severityAndMessage.second.data());
}

bool languageide_completion_item_is_system(languageide_completion_item_t _item) {
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  return item.isSystem();
}

void languageide_completion_item_get_module_name(
    languageide_completion_response_t _response, languageide_completion_item_t _item,
    void (^handler)(const char *)) {
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  handler(item.getContextFreeResult().getModuleName().data());
}

uint32_t languageide_completion_item_get_num_bytes_to_erase(
    languageide_completion_item_t _item) {
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  return item.getNumBytesToErase();
}

uint32_t
languageide_completion_item_get_type_relation(languageide_completion_item_t _item) {
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  // FIXME: keep this in sync with ide::CodeCompletionResult
  return static_cast<languageide_completion_type_relation_t>(
      item.getExpectedTypeRelation());
}

uint32_t
languageide_completion_item_import_depth(languageide_completion_response_t _response,
                                      languageide_completion_item_t _item) {
  auto &response = *static_cast<CompletionResult *>(_response);
  auto &item = *static_cast<CodeCompletionResult *>(_item);
  if (item.getSemanticContext() == SemanticContextKind::OtherModule) {
    if (auto depth = response.importDepth.lookup(item.getModuleName())) {
      return *depth;
    } else {
      return ~0u;
    }
  } else {
    return 0;
  }
}

languageide_fuzzy_match_pattern_t
languageide_fuzzy_match_pattern_create(const char *pattern) {
  return static_cast<languageide_fuzzy_match_pattern_t>(
      new FuzzyStringMatcher(pattern));
}

bool languageide_fuzzy_match_pattern_matches_candidate(
    languageide_fuzzy_match_pattern_t _pattern, const char *_candidate,
    double *outScore) {
  auto &matcher = *static_cast<FuzzyStringMatcher *>(_pattern);
  StringRef candidate = _candidate;
  if (matcher.matchesCandidate(candidate)) {
    if (outScore)
      *outScore = matcher.scoreCandidate(candidate);
    return true;
  }
  return false;
}

void languageide_fuzzy_match_pattern_dispose(
    languageide_fuzzy_match_pattern_t _pattern) {
  delete static_cast<FuzzyStringMatcher *>(_pattern);
}

static std::string getRuntimeResourcesPath() {
  auto libPath = getRuntimeLibPath();
  toolchain::SmallString<128> libPathTmp(libPath);
  toolchain::sys::path::append(libPathTmp, "language");
  return libPathTmp.str().str();
}
