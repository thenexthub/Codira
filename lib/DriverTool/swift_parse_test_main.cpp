//===--- language_parse_test_main.cpp ----------------------------------------===//
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
// A utility tool to measure the parser performance.
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTContext.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/LangOptions.h"
#include "language/Bridging/ASTGen.h"
#include "language/Subsystems.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Error.h"

#include <chrono>
#include <ctime>

using namespace language;

namespace {

enum class Executor {
  CodiraParser,
  LibParse,
  ASTGen,
};

enum class ExecuteOptionFlag {
  /// Enable body skipping
  SkipBodies = 1 << 0,
  /// Dump result
  Dump = 1 << 1,
};
using ExecuteOptions = OptionSet<ExecuteOptionFlag>;

struct CodiraParseTestOptions {
  toolchain::cl::list<Executor> Executors = toolchain::cl::list<Executor>(
      toolchain::cl::desc("Available parsers"),
      toolchain::cl::values(
          clEnumValN(Executor::CodiraParser, "language-parser", "CodiraParser"),
          clEnumValN(Executor::ASTGen, "ast-gen", "ASTGen with CodiraParser"),
          clEnumValN(Executor::LibParse, "lib-parse", "libParse")));

  toolchain::cl::opt<unsigned int> Iterations = toolchain::cl::opt<unsigned int>(
      "n", toolchain::cl::desc("iteration"), toolchain::cl::init(1));

  toolchain::cl::opt<bool> SkipBodies = toolchain::cl::opt<bool>(
      "skip-bodies",
      toolchain::cl::desc("skip function bodies and type members if possible"));

  toolchain::cl::opt<bool> Dump = toolchain::cl::opt<bool>(
      "dump", toolchain::cl::desc("dump result for each iteration"));

  toolchain::cl::list<std::string> InputPaths = toolchain::cl::list<std::string>(
      toolchain::cl::Positional, toolchain::cl::desc("input paths"));
};

struct LibParseExecutor {
  constexpr static StringRef name = "libParse";

  static toolchain::Error performParse(toolchain::MemoryBufferRef buffer,
                                  ExecuteOptions opts) {
    SourceManager SM;
    unsigned bufferID =
        SM.addNewSourceBuffer(toolchain::MemoryBuffer::getMemBuffer(buffer));
    DiagnosticEngine diagEngine(SM);
    LangOptions langOpts;
    TypeCheckerOptions typeckOpts;
    SILOptions silOpts;
    SearchPathOptions searchPathOpts;
    ClangImporterOptions clangOpts;
    symbolgraphgen::SymbolGraphOptions symbolOpts;
    CASOptions casOpts;
    SerializationOptions serializationOpts;
    std::unique_ptr<ASTContext> ctx(ASTContext::get(
        langOpts, typeckOpts, silOpts, searchPathOpts, clangOpts, symbolOpts,
        casOpts, serializationOpts, SM, diagEngine));
    auto &eval = ctx->evaluator;
    registerParseRequestFunctions(eval);
    registerTypeCheckerRequestFunctions(eval);

    SourceFile::ParsingOptions parseOpts;
    parseOpts |= SourceFile::ParsingFlags::DisablePoundIfEvaluation;
    if (!opts.contains(ExecuteOptionFlag::SkipBodies))
      parseOpts |= SourceFile::ParsingFlags::DisableDelayedBodies;

    ModuleDecl *M = ModuleDecl::createEmpty(Identifier(), *ctx);
    SourceFile *SF =
        new (*ctx) SourceFile(*M, SourceFileKind::Library, bufferID, parseOpts);

    auto items = evaluateOrDefault(eval, ParseSourceFileRequest{SF}, {}).TopLevelItems;

    if (opts.contains(ExecuteOptionFlag::Dump)) {
      for (auto &item : items) {
        item.dump(toolchain::outs());
      }
    }

    return toolchain::Error::success();
  }
};

struct CodiraParserExecutor {
  constexpr static StringRef name = "CodiraParser";

  static toolchain::Error performParse(toolchain::MemoryBufferRef buffer,
                                  ExecuteOptions opts) {
#if LANGUAGE_BUILD_LANGUAGE_SYNTAX
    // TODO: Implement 'ExecuteOptionFlag::SkipBodies'
    auto sourceFile = language_ASTGen_parseSourceFile(
        buffer.getBuffer(),
        /*moduleName=*/StringRef(), buffer.getBufferIdentifier(),
        /*declContextPtr=*/nullptr, BridgedGeneratedSourceFileKindNone);
    language_ASTGen_destroySourceFile(sourceFile);

    if (opts.contains(ExecuteOptionFlag::Dump)) {
      // TODO: Implement.
    }

    return toolchain::Error::success();
#else
    return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                   "CodiraParser is not supported");
#endif
  }
};

struct ASTGenExecutor {
  constexpr static StringRef name = "ASTGen with CodiraParser";

  static toolchain::Error performParse(toolchain::MemoryBufferRef buffer,
                                  ExecuteOptions opts) {
#if LANGUAGE_BUILD_LANGUAGE_SYNTAX

    SourceManager SM;
    unsigned bufferID =
        SM.addNewSourceBuffer(toolchain::MemoryBuffer::getMemBuffer(buffer));
    DiagnosticEngine diagEngine(SM);
    LangOptions langOpts;
    TypeCheckerOptions typeckOpts;
    SILOptions silOpts;
    SearchPathOptions searchPathOpts;
    ClangImporterOptions clangOpts;
    CASOptions casOpts;
    symbolgraphgen::SymbolGraphOptions symbolOpts;
    SerializationOptions serializationOpts;

    // Enable ASTGen.
    langOpts.enableFeature(Feature::ParserASTGen);

    std::unique_ptr<ASTContext> ctx(ASTContext::get(
        langOpts, typeckOpts, silOpts, searchPathOpts, clangOpts, symbolOpts,
        casOpts, serializationOpts, SM, diagEngine));
    auto &eval = ctx->evaluator;
    registerParseRequestFunctions(eval);
    registerTypeCheckerRequestFunctions(eval);

    SourceFile::ParsingOptions parseOpts;
    parseOpts |= SourceFile::ParsingFlags::DisablePoundIfEvaluation;
    if (!opts.contains(ExecuteOptionFlag::SkipBodies))
      parseOpts |= SourceFile::ParsingFlags::DisableDelayedBodies;

    ModuleDecl *M = ModuleDecl::createEmpty(Identifier(), *ctx);
    SourceFile *SF =
        new (*ctx) SourceFile(*M, SourceFileKind::Library, bufferID, parseOpts);

    auto items = evaluateOrDefault(eval, ParseSourceFileRequest{SF}, {}).TopLevelItems;

    if (opts.contains(ExecuteOptionFlag::Dump)) {
      for (auto &item : items) {
        item.dump(toolchain::outs());
      }
    }

    return toolchain::Error::success();
#else
    return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                   "ASTGen/CodiraParser is not supported");
#endif
  }
};

static void _loadCodiraFilesRecursively(
    StringRef path,
    SmallVectorImpl<std::unique_ptr<toolchain::MemoryBuffer>> &buffers) {
  if (toolchain::sys::fs::is_directory(path)) {
    std::error_code err;
    for (auto I = toolchain::sys::fs::directory_iterator(path, err),
              E = toolchain::sys::fs::directory_iterator();
         I != E; I.increment(err)) {
      _loadCodiraFilesRecursively(I->path(), buffers);
    }
  } else if (path.ends_with(".code")) {
    if (auto buffer = toolchain::MemoryBuffer::getFile(path)) {
      buffers.push_back(std::move(*buffer));
    }
  }
}

/// Load all '.code' files in the specified \p filePaths into \p buffers.
/// If the path is a directory, this recursively collects the files in it.
static void
loadSources(ArrayRef<std::string> filePaths,
            SmallVectorImpl<std::unique_ptr<toolchain::MemoryBuffer>> &buffers) {
  for (auto path : filePaths) {
    _loadCodiraFilesRecursively(path, buffers);
  }
}

/// Measure the duration of \p body execution. Returns a pair of "wall clock
/// time" and "CPU time".
template <typename Duration>
static std::pair<Duration, Duration> measure(toolchain::function_ref<void()> body) {
  auto cStart = std::clock();
  auto tStart = std::chrono::steady_clock::now();
  body();
  auto cEnd = std::clock();
  auto tEnd = std::chrono::steady_clock::now();

  auto clockMultiply =
      CLOCKS_PER_SEC > 0
          ? (Duration::period::den / CLOCKS_PER_SEC / Duration::period::num)
          : 0;

  Duration cDuration((cEnd - cStart) * clockMultiply);
  return {std::chrono::duration_cast<Duration>(tEnd - tStart),
          std::chrono::duration_cast<Duration>(cDuration)};
}

/// Perform the performance measurement using \c Executor .
/// Parse all \p buffers using \c Executor , \p iteration times, and print out
/// the measurement to the \c stdout.
template <typename Executor>
static toolchain::Error
perform(const SmallVectorImpl<std::unique_ptr<toolchain::MemoryBuffer>> &buffers,
        ExecuteOptions opts, unsigned iteration, uintmax_t totalBytes,
        uintmax_t totalLines) {

  toolchain::outs() << "----\n";
  toolchain::outs() << "parser: " << Executor::name << "\n";

  using duration_t = std::chrono::nanoseconds;
  // Wall clock time.
  auto tDuration = duration_t::zero();
  // CPU time.
  auto cDuration = duration_t::zero();

  toolchain::Error err = toolchain::Error::success();
  (void)bool(err);

  for (unsigned i = 0; i < iteration; i++) {
    for (auto &buffer : buffers) {
      std::pair<duration_t, duration_t> elapsed = measure<duration_t>([&] {
        err = Executor::performParse(buffer->getMemBufferRef(), opts);
      });
      if (err)
        return err;
      tDuration += elapsed.first;
      cDuration += elapsed.second;
    }
  }

  auto tDisplay =
      std::chrono::duration_cast<std::chrono::milliseconds>(tDuration).count();
  auto cDisplay =
      std::chrono::duration_cast<std::chrono::milliseconds>(cDuration).count();
  toolchain::outs() << toolchain::format("wall clock time (ms): %8d\n", tDisplay)
               << toolchain::format("cpu time (ms):        %8d\n", cDisplay);

  if (cDuration.count() > 0) {
    // Throughputs are based on CPU time.
    auto byteTPS = totalBytes * duration_t::period::den / cDuration.count();
    auto lineTPS = totalLines * duration_t::period::den / cDuration.count();

    toolchain::outs() << toolchain::format("throughput (byte/s):  %8d\n", byteTPS)
                 << toolchain::format("throughput (line/s):  %8d\n", lineTPS);
  }

  return toolchain::Error::success();
}

} // namespace

int language_parse_test_main(ArrayRef<const char *> args, const char *argv0,
                          void *mainAddr) {
  CodiraParseTestOptions options;
  toolchain::cl::ParseCommandLineOptions(args.size(), args.data(),
                                    "Codira parse test\n");

  unsigned iterations = options.Iterations;
  ExecuteOptions execOptions;
  if (options.SkipBodies)
    execOptions |= ExecuteOptionFlag::SkipBodies;
  if (options.Dump)
    execOptions |= ExecuteOptionFlag::Dump;

  SmallVector<std::unique_ptr<toolchain::MemoryBuffer>> buffers;
  loadSources(options.InputPaths, buffers);
  unsigned int byteCount = 0;
  unsigned int lineCount = 0;
  for (auto &buffer : buffers) {
    byteCount += buffer->getBufferSize();
    lineCount += buffer->getBuffer().count('\n');
  }

  toolchain::outs() << toolchain::format("file count:  %8d\n", buffers.size())
               << toolchain::format("total bytes: %8d\n", byteCount)
               << toolchain::format("total lines: %8d\n", lineCount)
               << toolchain::format("iterations:  %8d\n", iterations);

  toolchain::Error err = toolchain::Error::success();
  (void)bool(err);

  for (auto mode : options.Executors) {
    switch (mode) {
#define CASE(NAME, EXECUTOR)                                                   \
  case Executor::NAME:                                                         \
    err = perform<EXECUTOR>(buffers, execOptions, iterations, byteCount,       \
                            lineCount);                                        \
    break;
      CASE(LibParse, LibParseExecutor)
      CASE(CodiraParser, CodiraParserExecutor)
      CASE(ASTGen, ASTGenExecutor)
    }
    if (err)
      break;
  }

  if (err) {
    toolchain::handleAllErrors(std::move(err), [](toolchain::ErrorInfoBase &info) {
      toolchain::errs() << "error: " << info.message() << "\n";
    });
    return 1;
  }

  return 0;
}
