//===--- language-scan-test.cpp - Test libCodiraScan Dylib --------------------===//
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
// A simple program to test libCodiraScan interfaces.
//
//===----------------------------------------------------------------------===//

#include "language-c/DependencyScan/DependencyScan.h"
#include "language/Basic/Defer.h"
#include "language/Basic/FileTypes.h"
#include "language/DependencyScan/DependencyScanJSON.h"
#include "language/DependencyScan/StringUtils.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/Support/Allocator.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/StringSaver.h"
#include "toolchain/Support/ThreadPool.h"

using namespace toolchain;

namespace {
enum Actions {
  compute_cache_key,
  compute_cache_key_from_index,
  cache_query,
  replay_result,
  scan_dependency,
  get_chained_bridging_header,
};

toolchain::cl::OptionCategory Category("language-scan-test Options");
toolchain::cl::opt<std::string> CASPath("cas-path", toolchain::cl::desc("<path>"),
                                   toolchain::cl::cat(Category));
toolchain::cl::opt<std::string> CASID("id", toolchain::cl::desc("<casid>"),
                                 toolchain::cl::cat(Category));
toolchain::cl::opt<std::string> Input("input", toolchain::cl::desc("<file|index>"),
                                 toolchain::cl::cat(Category));
toolchain::cl::opt<unsigned> Threads("threads",
                                toolchain::cl::desc("<number of threads>"),
                                toolchain::cl::cat(Category), cl::init(1));
toolchain::cl::opt<std::string> WorkingDirectory("cwd", toolchain::cl::desc("<path>"),
                                            toolchain::cl::cat(Category));
toolchain::cl::opt<Actions>
    Action("action", toolchain::cl::desc("<action>"),
           toolchain::cl::values(clEnumVal(compute_cache_key, "compute cache key"),
                            clEnumVal(compute_cache_key_from_index,
                                      "compute cache key from index"),
                            clEnumVal(cache_query, "cache query"),
                            clEnumVal(replay_result, "replay result"),
                            clEnumVal(scan_dependency, "scan dependency"),
                            clEnumVal(get_chained_bridging_header,
                                      "get cached bridging header")),
           toolchain::cl::cat(Category));
toolchain::cl::list<std::string>
    CodiraCommands(toolchain::cl::Positional, toolchain::cl::desc("<language-frontend args>"),
                  toolchain::cl::cat(Category));

} // namespace

static StringRef toString(languagescan_string_ref_t str) {
  return StringRef((const char *)str.data, str.length);
}

static int printError(languagescan_string_ref_t err) {
  toolchain::errs() << toString(err) << "\n";
  languagescan_string_dispose(err);
  return EXIT_FAILURE;
}

static int action_compute_cache_key(languagescan_cas_t cas, StringRef input,
                                    std::vector<const char *> &Args) {
  if (input.empty()) {
    toolchain::errs() << "-input is not specified for compute_cache_key\n";
    return EXIT_FAILURE;
  }

  languagescan_string_ref_t err_msg;
  auto key = languagescan_cache_compute_key(cas, Args.size(), Args.data(),
                                         input.str().c_str(), &err_msg);
  if (key.length == 0)
    return printError(err_msg);

  toolchain::outs() << toString(key) << "\n";
  languagescan_string_dispose(key);

  return EXIT_SUCCESS;
}

static int
action_compute_cache_key_from_index(languagescan_cas_t cas, StringRef index,
                                    std::vector<const char *> &Args) {
  unsigned inputIndex = 0;
  if (!to_integer(index, inputIndex)) {
    toolchain::errs() << "-input is not a number for compute_cache_key_from_index\n";
    return EXIT_FAILURE;
  }

  languagescan_string_ref_t err_msg;
  auto key = languagescan_cache_compute_key_from_input_index(
      cas, Args.size(), Args.data(), inputIndex, &err_msg);
  if (key.length == 0)
    return printError(err_msg);

  toolchain::outs() << toString(key) << "\n";
  languagescan_string_dispose(key);

  return EXIT_SUCCESS;
}

static int print_cached_compilation(languagescan_cached_compilation_t comp,
                                    const char *key) {
  auto numOutput = languagescan_cached_compilation_get_num_outputs(comp);
  toolchain::outs() << "Cached Compilation for key \"" << key << "\" has "
               << numOutput << " outputs: \n";

  for (unsigned i = 0; i < numOutput; ++i) {
    languagescan_cached_output_t out =
        languagescan_cached_compilation_get_output(comp, i);
    languagescan_string_ref_t id = languagescan_cached_output_get_casid(out);
    languagescan_string_ref_t kind = languagescan_cached_output_get_name(out);
    LANGUAGE_DEFER { languagescan_string_dispose(kind); };
    toolchain::outs() << toString(kind) << ": " << toString(id) << "\n";
    languagescan_string_dispose(id);
  }
  toolchain::outs() << "\n";
  return EXIT_SUCCESS;
}

static int action_cache_query(languagescan_cas_t cas, const char *key) {
  languagescan_string_ref_t err_msg;
  auto comp = languagescan_cache_query(cas, key, /*globally=*/false, &err_msg);
  if (err_msg.length != 0)
    return printError(err_msg);

  if (!comp) {
    toolchain::errs() << "cached output not found for \"" << key << "\"\n";
    return EXIT_FAILURE;
  }

  LANGUAGE_DEFER { languagescan_cached_compilation_dispose(comp); };
  return print_cached_compilation(comp, key);
}

static int action_replay_result(languagescan_cas_t cas, const char *key,
                                std::vector<const char *> &Args) {
  languagescan_string_ref_t err_msg;
  auto comp = languagescan_cache_query(cas, key, /*globally=*/false, &err_msg);
  if (!comp) {
    if (err_msg.length != 0)
      return printError(err_msg);

    toolchain::errs() << "key " << key << " not found for replay\n";
    return EXIT_FAILURE;
  }

  LANGUAGE_DEFER { languagescan_cached_compilation_dispose(comp); };
  auto numOutput = languagescan_cached_compilation_get_num_outputs(comp);
  for (unsigned i = 0; i < numOutput; ++i) {
    auto output = languagescan_cached_compilation_get_output(comp, i);
    LANGUAGE_DEFER { languagescan_cached_output_dispose(output); };

    if (languagescan_cached_output_is_materialized(output))
      continue;

    auto load = languagescan_cached_output_load(output, &err_msg);
    if (err_msg.length != 0)
      return printError(err_msg);

    if (!load) {
      toolchain::errs() << "output at index " << i
                   << " cannot be loaded for key: " << key << "\n";
      return EXIT_FAILURE;
    }
  }

  auto instance = languagescan_cache_replay_instance_create(Args.size(),
                                                         Args.data(), &err_msg);
  if (!instance)
    return printError(err_msg);
  LANGUAGE_DEFER { languagescan_cache_replay_instance_dispose(instance); };

  auto result = languagescan_cache_replay_compilation(instance, comp, &err_msg);
  if (err_msg.length != 0)
    return printError(err_msg);

  LANGUAGE_DEFER { languagescan_cache_replay_result_dispose(result); };

  toolchain::outs() << toString(languagescan_cache_replay_result_get_stdout(result));
  toolchain::errs() << toString(languagescan_cache_replay_result_get_stderr(result));
  return EXIT_SUCCESS;
}

static int action_scan_dependency(std::vector<const char *> &Args,
                                  StringRef WorkingDirectory,
                                  bool PrintChainedBridgingHeader) {
  auto scanner = languagescan_scanner_create();
  auto invocation = languagescan_scan_invocation_create();
  auto error = [&](StringRef msg) {
    toolchain::errs() << msg << "\n";
    languagescan_scan_invocation_dispose(invocation);
    languagescan_scanner_dispose(scanner);
    return EXIT_FAILURE;
  };

  languagescan_scan_invocation_set_working_directory(
      invocation, WorkingDirectory.str().c_str());
  languagescan_scan_invocation_set_argv(invocation, Args.size(), Args.data());

  auto graph = languagescan_dependency_graph_create(scanner, invocation);
  if (!graph)
    return error("dependency scanning failed");

  auto diags = languagescan_dependency_graph_get_diagnostics(graph);
  for (unsigned i = 0; i < diags->count; ++i) {
    auto msg = languagescan_diagnostic_get_message(diags->diagnostics[i]);
    switch (languagescan_diagnostic_get_severity(diags->diagnostics[i])) {
    case LANGUAGESCAN_DIAGNOSTIC_SEVERITY_ERROR:
      toolchain::errs() << "error: ";
      break;
    case LANGUAGESCAN_DIAGNOSTIC_SEVERITY_WARNING:
      toolchain::errs() << "warning: ";
      break;
    case LANGUAGESCAN_DIAGNOSTIC_SEVERITY_NOTE:
      toolchain::errs() << "note: ";
      break;
    case LANGUAGESCAN_DIAGNOSTIC_SEVERITY_REMARK:
      toolchain::errs() << "remark: ";
      break;
    }
    toolchain::errs() << language::c_string_utils::get_C_string(msg) << "\n";
  }

  if (PrintChainedBridgingHeader) {
    auto deps = languagescan_dependency_graph_get_dependencies(graph);
    if (!deps || deps->count < 1)
      error("failed to get dependencies");
    // Assume the main module is the first only.
    auto details = languagescan_module_info_get_details(deps->modules[0]);
    auto kind = languagescan_module_detail_get_kind(details);
    if (kind != LANGUAGESCAN_DEPENDENCY_INFO_LANGUAGE_TEXTUAL)
      error("not the correct dependency kind");

    auto content =
        languagescan_language_textual_detail_get_chained_bridging_header_content(
            details);
    toolchain::outs() << toString(content);
  } else
    language::dependencies::writeJSON(toolchain::outs(), graph);

  languagescan_scan_invocation_dispose(invocation);
  languagescan_scanner_dispose(scanner);
  return EXIT_SUCCESS;
}

static std::vector<const char *>
createArgs(ArrayRef<std::string> Cmd, StringSaver &Saver, Actions Action) {
  if (!Cmd.empty() && StringRef(Cmd.front()).ends_with("language-frontend"))
    Cmd = Cmd.drop_front();

  // Quote all the arguments before passing to scanner. The scanner is currently
  // tokenize the command-line again before parsing.
  bool Quoted = (Action == Actions::scan_dependency ||
                 Action == Actions::get_chained_bridging_header);

  std::vector<const char *> Args;
  for (auto A : Cmd) {
    if (Quoted)
      A = std::string("\"") + A + "\"";
    StringRef Arg = Saver.save(A);
    Args.push_back(Arg.data());
  }

  return Args;
}

int main(int argc, char *argv[]) {
  toolchain::cl::HideUnrelatedOptions(Category);
  toolchain::cl::ParseCommandLineOptions(argc, argv,
                                    "Test libCodiraScan interfaces\n");

  // Create CAS.
  auto option = languagescan_cas_options_create();
  LANGUAGE_DEFER { languagescan_cas_options_dispose(option); };
  languagescan_cas_options_set_ondisk_path(option, CASPath.c_str());

  languagescan_string_ref_t err_msg;
  auto cas = languagescan_cas_create_from_options(option, &err_msg);
  if (!cas)
    return printError(err_msg);
  LANGUAGE_DEFER { languagescan_cas_dispose(cas); };

  // Convert commands.
  toolchain::BumpPtrAllocator Alloc;
  toolchain::StringSaver Saver(Alloc);
  auto Args = createArgs(CodiraCommands, Saver, Action);

  std::atomic<int> Ret = 0;
  toolchain::StdThreadPool Pool(toolchain::hardware_concurrency(Threads));
  for (unsigned i = 0; i < Threads; ++i) {
    Pool.async([&]() {
      switch (Action) {
      case compute_cache_key:
        Ret += action_compute_cache_key(cas, Input, Args);
        break;
      case compute_cache_key_from_index:
        Ret += action_compute_cache_key_from_index(cas, Input, Args);
        break;
      case cache_query:
        Ret += action_cache_query(cas, CASID.c_str());
        break;
      case replay_result:
        Ret += action_replay_result(cas, CASID.c_str(), Args);
        break;
      case scan_dependency:
      case get_chained_bridging_header:
        Ret += action_scan_dependency(Args, WorkingDirectory,
                                      Action == get_chained_bridging_header);
      }
    });
  }
  Pool.wait();

  return Ret;
}
