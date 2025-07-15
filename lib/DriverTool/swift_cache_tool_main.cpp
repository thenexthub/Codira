//===--- language_cache_tool_main.cpp - Codira caching tool for inspection ----===//
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
// Utility tool for inspecting and accessing language cache.
//
//===----------------------------------------------------------------------===//
//
#include "language/AST/DiagnosticsFrontend.h"
#include "language/Basic/FileTypes.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/Basic/Version.h"
#include "language/Frontend/CachingUtils.h"
#include "language/Frontend/CompileJobCacheKey.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/Parse/ParseVersion.h"
#include "clang/CAS/CASOptions.h"
#include "clang/CAS/IncludeTree.h"
#include "toolchain/CAS/ActionCache.h"
#include "toolchain/CAS/BuiltinUnifiedCASDatabases.h"
#include "toolchain/CAS/ObjectStore.h"
#include "toolchain/Option/OptTable.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/JSON.h"
#include "toolchain/Support/MemoryBuffer.h"
#include <memory>

using namespace language;
using namespace toolchain::opt;
using namespace toolchain::cas;

namespace {

enum class CodiraCacheToolAction {
  Invalid,
  PrintBaseKey,
  PrintOutputKeys,
  ValidateOutputs,
  RenderDiags,
  PrintIncludeTreeList,
  PrintCompileCacheKey,
};

struct OutputEntry {
  std::string InputPath;
  std::string CacheKey;
  std::vector<std::pair<std::string, std::string>> Outputs;
};

enum ID {
  OPT_INVALID = 0, // This is not an option ID.
#define OPTION(...) TOOLCHAIN_MAKE_OPT_ID(__VA_ARGS__),
#include "CodiraCacheToolOptions.inc"
  LastOption
#undef OPTION
};

#define PREFIX(NAME, VALUE)                                                    \
  constexpr toolchain::StringLiteral NAME##_init[] = VALUE;                         \
  constexpr toolchain::ArrayRef<toolchain::StringLiteral> NAME(                          \
      NAME##_init, std::size(NAME##_init) - 1);
#include "CodiraCacheToolOptions.inc"
#undef PREFIX

static const OptTable::Info InfoTable[] = {
#define OPTION(...) TOOLCHAIN_CONSTRUCT_OPT_INFO(__VA_ARGS__),
#include "CodiraCacheToolOptions.inc"
#undef OPTION
};

class CacheToolOptTable : public toolchain::opt::GenericOptTable {
public:
  CacheToolOptTable() : GenericOptTable(InfoTable) {}
};

class CodiraCacheToolInvocation {
private:
  CompilerInstance Instance;
  CompilerInvocation Invocation;
  PrintingDiagnosticConsumer PDC;
  std::string MainExecutablePath;
  clang::CASOptions CASOpts;
  std::vector<std::string> Inputs;
  std::vector<std::string> FrontendArgs;
  CodiraCacheToolAction ActionKind = CodiraCacheToolAction::Invalid;

public:
  CodiraCacheToolInvocation(const std::string &ExecPath)
      : MainExecutablePath(ExecPath) {
    Instance.addDiagnosticConsumer(&PDC);
  }

  int parseArgs(ArrayRef<const char *> Args) {
    auto &Diags = Instance.getDiags();

    CacheToolOptTable Table;
    unsigned MissingIndex;
    unsigned MissingCount;
    toolchain::opt::InputArgList ParsedArgs =
        Table.ParseArgs(Args, MissingIndex, MissingCount);
    if (MissingCount) {
      Diags.diagnose(SourceLoc(), diag::error_missing_arg_value,
                     ParsedArgs.getArgString(MissingIndex), MissingCount);
      return 1;
    }

    if (ParsedArgs.getLastArg(OPT_help)) {
      std::string ExecutableName =
          toolchain::sys::path::stem(MainExecutablePath).str();
      Table.printHelp(toolchain::outs(), ExecutableName.c_str(), "Codira Cache Tool",
                      0, 0, /*ShowAllAliases*/ false);
      return 0;
    }

    if (const Arg* PluginPath = ParsedArgs.getLastArg(OPT_cas_plugin_path))
      CASOpts.PluginPath = PluginPath->getValue();
    if (const Arg* OnDiskPath = ParsedArgs.getLastArg(OPT_cas_path))
      CASOpts.CASPath = OnDiskPath->getValue();
    for (StringRef Opt : ParsedArgs.getAllArgValues(OPT_cas_plugin_option)) {
      StringRef Name, Value;
      std::tie(Name, Value) = Opt.split('=');
      CASOpts.PluginOptions.emplace_back(std::string(Name), std::string(Value));
    }

    // Fallback to default path if not set.
    if (CASOpts.CASPath.empty() && CASOpts.PluginPath.empty())
      CASOpts.CASPath = getDefaultOnDiskCASPath();

    Inputs = ParsedArgs.getAllArgValues(OPT_INPUT);
    FrontendArgs = ParsedArgs.getAllArgValues(OPT__DASH_DASH);
    if (auto *A = ParsedArgs.getLastArg(OPT_cache_tool_action))
      ActionKind =
          toolchain::StringSwitch<CodiraCacheToolAction>(A->getValue())
              .Case("print-base-key", CodiraCacheToolAction::PrintBaseKey)
              .Case("print-output-keys", CodiraCacheToolAction::PrintOutputKeys)
              .Case("validate-outputs", CodiraCacheToolAction::ValidateOutputs)
              .Case("render-diags", CodiraCacheToolAction::RenderDiags)
              .Case("print-include-tree-list",
                    CodiraCacheToolAction::PrintIncludeTreeList)
              .Case("print-compile-cache-key",
                    CodiraCacheToolAction::PrintCompileCacheKey)
              .Default(CodiraCacheToolAction::Invalid);

    if (ActionKind == CodiraCacheToolAction::Invalid) {
      toolchain::errs()
          << "Invalid option specified for -cache-tool-action: "
          << "print-base-key|print-output-keys|validate-outputs|render-diags|"
          << "print-include-tree-list|print-compile-cache-key\n";
      return 1;
    }

    return 0;
  }

  int run() {
    switch (ActionKind) {
    case CodiraCacheToolAction::PrintBaseKey:
      return printBaseKey();
    case CodiraCacheToolAction::PrintOutputKeys:
      return printOutputKeys();
    case CodiraCacheToolAction::ValidateOutputs:
      return validateOutputs();
    case CodiraCacheToolAction::RenderDiags:
      return renderDiags();
    case CodiraCacheToolAction::PrintIncludeTreeList:
      return printIncludeTreeList();
    case CodiraCacheToolAction::PrintCompileCacheKey:
      return printCompileCacheKey();
    case CodiraCacheToolAction::Invalid:
      return 0; // No action. Probably just print help. Return.
    }
  }

private:
  bool setupCompiler() {
    // Setup invocation.
    SmallString<128> workingDirectory;
    toolchain::sys::fs::current_path(workingDirectory);

    // Parse arguments.
    if (FrontendArgs.empty()) {
      toolchain::errs() << "missing language-frontend command-line after --\n";
      return true;
    }
    // drop language-frontend executable path and leading `-frontend` from
    // command-line.
    if (StringRef(FrontendArgs[0]).ends_with("language-frontend"))
      FrontendArgs.erase(FrontendArgs.begin());
    if (StringRef(FrontendArgs[0]) == "-frontend")
      FrontendArgs.erase(FrontendArgs.begin());

    SmallVector<std::unique_ptr<toolchain::MemoryBuffer>, 4>
        configurationFileBuffers;
    std::vector<const char*> Args;
    for (auto &A: FrontendArgs)
      Args.emplace_back(A.c_str());

    // Make sure CASPath is the same between invocation and cache-tool by
    // appending the cas-path since the option doesn't affect cache key.
    if (!CASOpts.CASPath.empty()) {
      Args.emplace_back("-cas-path");
      Args.emplace_back(CASOpts.CASPath.c_str());
    }
    if (!CASOpts.PluginPath.empty()) {
      Args.emplace_back("-cas-plugin-path");
      Args.emplace_back(CASOpts.PluginPath.c_str());
    }
    std::vector<std::string> PluginJoinedOpts;
    for (const auto& Opt: CASOpts.PluginOptions) {
      PluginJoinedOpts.emplace_back(Opt.first + "=" + Opt.second);
      Args.emplace_back("-cas-plugin-option");
      Args.emplace_back(PluginJoinedOpts.back().c_str());
    }

    if (Invocation.parseArgs(Args, Instance.getDiags(),
                             &configurationFileBuffers, workingDirectory,
                             MainExecutablePath))
      return true;

    if (!Invocation.getCASOptions().EnableCaching) {
      toolchain::errs() << "Requested command-line arguments do not enable CAS\n";
      return true;
    }

    // Setup instance.
    std::string InstanceSetupError;
    if (Instance.setup(Invocation, InstanceSetupError, Args)) {
      toolchain::errs() << "language-frontend invocation setup error: "
                   << InstanceSetupError << "\n";
      return true;
    }

    // Disable diagnostic caching from this fake instance.
    if (auto *CDP = Instance.getCachingDiagnosticsProcessor())
      CDP->endDiagnosticCapture();

    return false;
  }

  std::optional<ObjectRef> getBaseKey() {
    auto BaseKey = Instance.getCompilerBaseKey();
    if (!BaseKey) {
      Instance.getDiags().diagnose(SourceLoc(), diag::error_cas,
                                   "query base cache key",
                                   "base cache key doesn't exist");
      return std::nullopt;
    }

    return *BaseKey;
  }

  int printBaseKey() {
    if (setupCompiler())
      return 1;

    auto &CAS = Instance.getObjectStore();
    auto BaseKey = getBaseKey();
    if (!BaseKey)
      return 1;

    if (ActionKind == CodiraCacheToolAction::PrintBaseKey)
      toolchain::outs() << CAS.getID(*BaseKey).toString() << "\n";

    return 0;
  }

  int printOutputKeys();
  int validateOutputs();
  int renderDiags();
  int printIncludeTreeList();
  int printCompileCacheKey();
};

} // end anonymous namespace

int CodiraCacheToolInvocation::printOutputKeys() {
  if (setupCompiler())
    return 1;

  auto &CAS = Instance.getObjectStore();
  auto BaseKey = getBaseKey();
  if (!BaseKey)
    return 1;

  std::vector<OutputEntry> OutputKeys;
  bool hasError = false;
  auto addFromInputFile = [&](const InputFile &Input, unsigned InputIndex) {
    auto InputPath = Input.getFileName();
    auto OutputKey =
        createCompileJobCacheKeyForOutput(CAS, *BaseKey, InputIndex);
    if (!OutputKey) {
      toolchain::errs() << "cannot create cache key for " << InputPath << ": "
                   << toString(OutputKey.takeError()) << "\n";
      hasError = true;
    }
    OutputKeys.emplace_back(
        OutputEntry{InputPath, CAS.getID(*OutputKey).toString(), {}});
    auto &Outputs = OutputKeys.back().Outputs;
    if (!Input.outputFilename().empty())
      Outputs.emplace_back(file_types::getTypeName(
                               Invocation.getFrontendOptions()
                                   .InputsAndOutputs.getPrincipalOutputType()),
                           Input.outputFilename());
    Input.getPrimarySpecificPaths()
        .SupplementaryOutputs.forEachSetOutputAndType(
            [&](const std::string &File, file_types::ID ID) {
              // Dont print serialized diagnostics.
              if (file_types::isProducedFromDiagnostics(ID))
                return;
              Outputs.emplace_back(file_types::getTypeName(ID), File);
            });
  };
  auto AllInputs =
      Invocation.getFrontendOptions().InputsAndOutputs.getAllInputs();
  for (unsigned Index = 0; Index < AllInputs.size(); ++Index)
    addFromInputFile(AllInputs[Index], Index);

  // Add diagnostics file.
  if (!OutputKeys.empty())
    OutputKeys.front().Outputs.emplace_back(
        file_types::getTypeName(file_types::ID::TY_CachedDiagnostics),
        "<cached-diagnostics>");

  if (hasError)
    return 1;

  toolchain::json::OStream Out(toolchain::outs(), /*IndentSize=*/4);
  Out.array([&] {
    for (const auto &E : OutputKeys) {
      Out.object([&] {
        Out.attribute("Input", E.InputPath);
        Out.attribute("CacheKey", E.CacheKey);
        Out.attributeArray("Outputs", [&] {
          for (const auto &OutEntry : E.Outputs) {
            Out.object([&] {
              Out.attribute("Kind", OutEntry.first);
              Out.attribute("Path", OutEntry.second);
            });
          }
        });
      });
    }
  });

  return 0;
}

static toolchain::Expected<toolchain::json::Array>
readOutputEntriesFromFile(StringRef Path) {
  auto JSONContent = toolchain::MemoryBuffer::getFile(Path);
  if (!JSONContent)
    return toolchain::createStringError(JSONContent.getError(),
                                   "failed to read input file");

  auto JSONValue = toolchain::json::parse((*JSONContent)->getBuffer());
  if (!JSONValue)
    return JSONValue.takeError();

  auto Keys = JSONValue->getAsArray();
  if (!Keys)
    return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                   "invalid JSON format for input file");

  return *Keys;
}

int CodiraCacheToolInvocation::validateOutputs() {
  auto DB = CASOpts.getOrCreateDatabases();
  if (!DB)
    report_fatal_error(DB.takeError());

  auto &CAS = *DB->first;
  auto &Cache = *DB->second;

  PrintingDiagnosticConsumer PDC;
  Instance.getDiags().addConsumer(PDC);

  auto lookupFailed = [&](StringRef Key) {
    toolchain::errs() << "failed to find output for cache key " << Key << "\n";
    return true;
  };
  auto lookupError = [&](toolchain::Error Err, StringRef Key) {
    toolchain::errs() << "failed to find output for cache key " << Key << ": "
                 << toString(std::move(Err)) << "\n";
    return true;
  };

  auto validateCacheKeysFromFile = [&](const std::string &Path) {
    auto Keys = readOutputEntriesFromFile(Path);
    if (!Keys) {
      toolchain::errs() << "cannot read file " << Path << ": "
                   << toString(Keys.takeError()) << "\n";
      return true;
    }

    for (const auto& Entry : *Keys) {
      if (auto *Obj = Entry.getAsObject()) {
        if (auto Key = Obj->getString("CacheKey")) {
          auto ID = CAS.parseID(*Key);
          if (!ID) {
            toolchain::errs() << "failed to parse ID " << Key << ": "
                         << toString(ID.takeError()) << "\n";
            return true;
          }
          auto Ref = CAS.getReference(*ID);
          if (!Ref)
            return lookupFailed(*Key);
          auto KeyID = CAS.getID(*Ref);
          auto Lookup = Cache.get(KeyID);
          if (!Lookup)
            return lookupError(Lookup.takeError(), *Key);

          if (!*Lookup)
            return lookupFailed(*Key);

          auto OutputRef = CAS.getReference(**Lookup);
          if (!OutputRef)
            return lookupFailed(*Key);

          cas::CachedResultLoader Loader(CAS, *OutputRef);
          if (auto Err = Loader.replay(
                  [&](file_types::ID Kind, ObjectRef Ref) -> toolchain::Error {
                    auto Proxy = CAS.getProxy(Ref);
                    if (!Proxy)
                      return Proxy.takeError();
                    return toolchain::Error::success();
                  })) {
            toolchain::errs() << "failed to find output for cache key " << *Key
                         << ": " << toString(std::move(Err)) << "\n";
            return true;
          }
          continue;
        }
      }
      toolchain::errs() << "can't read cache key from " << Path << "\n";
      return true;
    }

    return false;
  };

  return toolchain::any_of(Inputs, validateCacheKeysFromFile);
}

int CodiraCacheToolInvocation::renderDiags() {
  if (setupCompiler())
    return 1;

  auto *CDP = Instance.getCachingDiagnosticsProcessor();
  if (!CDP) {
    toolchain::errs() << "provided commandline doesn't support cached diagnostics\n";
    return 1;
  }

  auto renderDiagsFromFile = [&](const std::string &Path) {
    auto Keys = readOutputEntriesFromFile(Path);
    if (!Keys) {
      toolchain::errs() << "cannot read file " << Path << ": "
                   << toString(Keys.takeError()) << "\n";
      return true;
    }

    for (const auto& Entry : *Keys) {
      if (auto *Obj = Entry.getAsObject()) {
        if (auto Kind = Obj->getString("OutputKind")) {
          if (*Kind != "cached-diagnostics")
            continue;
        }
        if (auto Key = Obj->getString("CacheKey")) {
          if (auto Buffer = loadCachedCompileResultFromCacheKey(
                  Instance.getObjectStore(), Instance.getActionCache(),
                  Instance.getDiags(), *Key,
                  file_types::ID::TY_CachedDiagnostics)) {
            if (auto E = CDP->replayCachedDiagnostics(Buffer->getBuffer())) {
              toolchain::errs() << "failed to replay cache: "
                           << toString(std::move(E)) << "\n";
              return true;
            }
            return false;
          }
        }
      }
    }
    toolchain::errs() << "cannot locate cached diagnostics in file\n";
    return true;
  };

  return toolchain::any_of(Inputs, renderDiagsFromFile);
}

int CodiraCacheToolInvocation::printIncludeTreeList() {
  auto error = [](toolchain::Error err) {
    toolchain::errs() << toolchain::toString(std::move(err)) << "\n";
    return 1;
  };
  auto DB = CASOpts.getOrCreateDatabases();
  if (!DB) {
    return error(DB.takeError());
  }
  auto CAS = DB->first;
  for (auto &input: Inputs) {
    auto ID = CAS->parseID(input);
    if (!ID)
      return error(ID.takeError());

    auto Ref = CAS->getReference(*ID);
    if (!Ref) {
      toolchain::errs() << "CAS object not found: " << input << "\n";
      return 1;
    }

    auto fileList = clang::cas::IncludeTree::FileList::get(*CAS, *Ref);
    if (!fileList)
      return error(fileList.takeError());

    if (auto err = fileList->print(toolchain::outs()))
      return error(std::move(err));
  }

  return 0;
}

int CodiraCacheToolInvocation::printCompileCacheKey() {
  auto error = [](toolchain::Error err) {
    toolchain::errs() << "cannot print cache key: " << toolchain::toString(std::move(err))
                 << "\n";
    return 1;
  };
  if (Inputs.size() != 1) {
    toolchain::errs() << "expect 1 CASID as input\n";
    return 1;
  }
  auto DB = CASOpts.getOrCreateDatabases();
  if (!DB) {
    return error(DB.takeError());
  }
  auto CAS = DB->first;
  auto &input = Inputs.front();
  auto ID = CAS->parseID(input);
  if (!ID)
    return error(ID.takeError());

  auto Ref = CAS->getReference(*ID);
  if (!Ref) {
    toolchain::errs() << "CAS object not found: " << input << "\n";
    return 1;
  }

  if (auto err = language::printCompileJobCacheKey(*CAS, *Ref, toolchain::outs()))
    return error(std::move(err));

  return 0;
}

int language_cache_tool_main(ArrayRef<const char *> Args, const char *Argv0,
                          void *MainAddr) {
  INITIALIZE_LLVM();

  CodiraCacheToolInvocation Invocation(
      toolchain::sys::fs::getMainExecutable(Argv0, MainAddr));

  if (Invocation.parseArgs(Args) != 0)
    return EXIT_FAILURE;

  if (Invocation.run() != 0)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
