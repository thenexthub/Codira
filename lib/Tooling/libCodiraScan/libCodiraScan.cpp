//===------------ DependencyScanImpl.cpp - Codira Compiler -----------------===//
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
// Implementation of the dependency scanning C API
//
//===----------------------------------------------------------------------===//

#include "language-c/DependencyScan/DependencyScan.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/Basic/InitializeCodiraModules.h"
#include "language/DependencyScan/DependencyScanImpl.h"
#include "language/DependencyScan/DependencyScanningTool.h"
#include "language/DependencyScan/StringUtils.h"
#include "language/DriverTool/DriverTool.h"
#include "language/Option/Options.h"
#include "language/SIL/SILBridging.h"

using namespace language::dependencies;

DEFINE_SIMPLE_CONVERSION_FUNCTIONS(DependencyScanningTool, languagescan_scanner_t)

//=== Private Cleanup Functions -------------------------------------------===//
void languagescan_macro_dependency_dispose(
    languagescan_macro_dependency_set_t *macro) {
  if (!macro)
    return;

  for (unsigned i = 0; i < macro->count; ++i) {
    languagescan_string_dispose(macro->macro_dependencies[i]->module_name);
    languagescan_string_dispose(macro->macro_dependencies[i]->library_path);
    languagescan_string_dispose(macro->macro_dependencies[i]->executable_path);
    delete macro->macro_dependencies[i];
  }
  delete[] macro->macro_dependencies;
  delete macro;
}

void languagescan_dependency_info_details_dispose(
    languagescan_module_details_t details) {
  languagescan_module_details_s *details_impl = details;
  switch (details_impl->kind) {
  case LANGUAGESCAN_DEPENDENCY_INFO_LANGUAGE_TEXTUAL:
    languagescan_string_dispose(
        details_impl->language_textual_details.module_interface_path);
    languagescan_string_set_dispose(
        details_impl->language_textual_details.compiled_module_candidates);
    languagescan_string_dispose(
        details_impl->language_textual_details.bridging_header_path);
    languagescan_string_set_dispose(
        details_impl->language_textual_details.bridging_source_files);
    languagescan_string_set_dispose(
        details_impl->language_textual_details.bridging_module_dependencies);
    languagescan_string_set_dispose(
        details_impl->language_textual_details.code_overlay_module_dependencies);
    languagescan_string_set_dispose(
        details_impl->language_textual_details.source_import_module_dependencies);
    languagescan_string_set_dispose(
        details_impl->language_textual_details.command_line);
    languagescan_string_dispose(details_impl->language_textual_details.context_hash);
    languagescan_string_dispose(
        details_impl->language_textual_details.cas_fs_root_id);
    languagescan_string_dispose(
        details_impl->language_textual_details.bridging_header_include_tree);
    languagescan_string_dispose(
        details_impl->language_textual_details.module_cache_key);
    languagescan_macro_dependency_dispose(
        details_impl->language_textual_details.macro_dependencies);
    languagescan_string_dispose(
        details_impl->language_textual_details.user_module_version);
    break;
  case LANGUAGESCAN_DEPENDENCY_INFO_LANGUAGE_BINARY:
    languagescan_string_dispose(
        details_impl->language_binary_details.compiled_module_path);
    languagescan_string_dispose(
        details_impl->language_binary_details.module_doc_path);
    languagescan_string_dispose(
        details_impl->language_binary_details.module_source_info_path);
    languagescan_string_set_dispose(
        details_impl->language_binary_details.code_overlay_module_dependencies);
      languagescan_string_dispose(
        details_impl->language_binary_details.header_dependency);
    languagescan_string_dispose(
        details_impl->language_binary_details.module_cache_key);
    languagescan_string_dispose(
        details_impl->language_binary_details.user_module_version);
    break;
  case LANGUAGESCAN_DEPENDENCY_INFO_CLANG:
    languagescan_string_dispose(details_impl->clang_details.module_map_path);
    languagescan_string_dispose(details_impl->clang_details.context_hash);
    languagescan_string_set_dispose(details_impl->clang_details.command_line);
    languagescan_string_dispose(details_impl->clang_details.cas_fs_root_id);
    languagescan_string_dispose(details_impl->clang_details.module_cache_key);
    break;
  }
  delete details_impl;
}

void languagescan_link_library_set_dispose(languagescan_link_library_set_t *set) {
  for (size_t i = 0; i < set->count; ++i) {
    auto info = set->link_libraries[i];
    languagescan_string_dispose(info->name);
    delete info;
  }
  delete[] set->link_libraries;
  delete set;
}

void languagescan_dependency_info_dispose(languagescan_dependency_info_t info) {
  languagescan_string_dispose(info->module_name);
  languagescan_string_dispose(info->module_path);
  languagescan_string_set_dispose(info->source_files);
  languagescan_string_set_dispose(info->direct_dependencies);
  languagescan_link_library_set_dispose(info->link_libraries);
  languagescan_dependency_info_details_dispose(info->details);
  delete info;
}

void languagescan_dependency_set_dispose(languagescan_dependency_set_t *set) {
  if (set) {
    for (size_t i = 0; i < set->count; ++i) {
      languagescan_dependency_info_dispose(set->modules[i]);
    }
    delete[] set->modules;
    delete set;
  }
}

//=== Scanner Functions ---------------------------------------------------===//

languagescan_scanner_t languagescan_scanner_create(void) {
  static std::mutex initializationMutex;
  std::lock_guard<std::mutex> lock(initializationMutex);
  INITIALIZE_LLVM();
  if (!languageModulesInitialized())
    initializeCodiraModules();
  return wrap(new DependencyScanningTool());
}

void languagescan_scanner_dispose(languagescan_scanner_t c_scanner) {
  delete unwrap(c_scanner);
}

languagescan_dependency_graph_t
languagescan_dependency_graph_create(languagescan_scanner_t scanner,
                                  languagescan_scan_invocation_t invocation) {
  DependencyScanningTool *ScanningTool = unwrap(scanner);
  int argc = invocation->argv->count;
  std::vector<const char *> Compilation;
  for (int i = 0; i < argc; ++i)
    Compilation.push_back(language::c_string_utils::get_C_string(invocation->argv->strings[i]));

  // Execute the scan and bridge the result
  auto ScanResult = ScanningTool->getDependencies(
      Compilation,
      language::c_string_utils::get_C_string(invocation->working_directory));
  if (ScanResult.getError())
    return nullptr;
  auto DependencyGraph = std::move(*ScanResult);
  return DependencyGraph;
}

languagescan_import_set_t
languagescan_import_set_create(languagescan_scanner_t scanner,
                            languagescan_scan_invocation_t invocation) {
  DependencyScanningTool *ScanningTool = unwrap(scanner);
  int argc = invocation->argv->count;
  std::vector<const char *> Compilation;
  for (int i = 0; i < argc; ++i)
    Compilation.push_back(language::c_string_utils::get_C_string(invocation->argv->strings[i]));

  // Execute the scan and bridge the result
  auto PreScanResult = ScanningTool->getImports(
      Compilation,
      language::c_string_utils::get_C_string(invocation->working_directory));
  if (PreScanResult.getError())
    return nullptr;
  auto ImportSet = std::move(*PreScanResult);
  return ImportSet;
}

//=== Dependency Result Functions -----------------------------------------===//

languagescan_string_ref_t languagescan_dependency_graph_get_main_module_name(
    languagescan_dependency_graph_t result) {
  return result->main_module_name;
}

languagescan_dependency_set_t *languagescan_dependency_graph_get_dependencies(
    languagescan_dependency_graph_t result) {
  return result->dependencies;
}

languagescan_diagnostic_set_t *languagescan_dependency_graph_get_diagnostics(
    languagescan_dependency_graph_t result) {
  return result->diagnostics;
}

//=== Module Dependency Info query APIs -----------------------------------===//

languagescan_string_ref_t
languagescan_module_info_get_module_name(languagescan_dependency_info_t info) {
  return info->module_name;
}

languagescan_string_ref_t
languagescan_module_info_get_module_path(languagescan_dependency_info_t info) {
  return info->module_path;
}

languagescan_string_set_t *
languagescan_module_info_get_source_files(languagescan_dependency_info_t info) {
  return info->source_files;
}

languagescan_string_set_t *languagescan_module_info_get_direct_dependencies(
    languagescan_dependency_info_t info) {
  return info->direct_dependencies;
}


languagescan_link_library_set_t *languagescan_module_info_get_link_libraries(
    languagescan_dependency_info_t info) {
  return info->link_libraries;
}

languagescan_import_info_set_t *languagescan_module_info_get_imports(
    languagescan_dependency_info_t info) {
  return info->imports;
}

languagescan_module_details_t
languagescan_module_info_get_details(languagescan_dependency_info_t info) {
  return info->details;
}

//=== Link Library Info query APIs ---------------------------------------===//

languagescan_string_ref_t
languagescan_link_library_info_get_link_name(languagescan_link_library_info_t info) {
  return info->name;
}

bool languagescan_link_library_info_get_is_static(
    languagescan_link_library_info_t info) {
  return info->isStatic;
}

bool
languagescan_link_library_info_get_is_framework(languagescan_link_library_info_t info) {
  return info->isFramework;
}

bool
languagescan_link_library_info_get_should_force_load(languagescan_link_library_info_t info) {
  return info->forceLoad;
}

//=== Import Details Query APIs ------------------------------------------===//
languagescan_source_location_set_t *
languagescan_import_info_get_source_locations(languagescan_import_info_t info) {
  return info->source_locations;
}

languagescan_string_ref_t
languagescan_import_info_get_identifier(languagescan_import_info_t info) {
  return info->import_identifier;
}

languagescan_access_level_t
languagescan_import_info_get_access_level(languagescan_import_info_t info) {
  return info->access_level;
}

//=== Codira Textual Module Details query APIs ----------------------------===//

languagescan_dependency_info_kind_t
languagescan_module_detail_get_kind(languagescan_module_details_t details) {
  return details->kind;
}

languagescan_string_ref_t languagescan_language_textual_detail_get_module_interface_path(
    languagescan_module_details_t details) {
  return details->language_textual_details.module_interface_path;
}

languagescan_string_set_t *
languagescan_language_textual_detail_get_compiled_module_candidates(
    languagescan_module_details_t details) {
  return details->language_textual_details.compiled_module_candidates;
}

languagescan_string_ref_t languagescan_language_textual_detail_get_bridging_header_path(
    languagescan_module_details_t details) {
  return details->language_textual_details.bridging_header_path;
}

languagescan_string_set_t *
languagescan_language_textual_detail_get_bridging_source_files(
    languagescan_module_details_t details) {
  return details->language_textual_details.bridging_source_files;
}

languagescan_string_set_t *
languagescan_language_textual_detail_get_bridging_module_dependencies(
    languagescan_module_details_t details) {
  return details->language_textual_details.bridging_module_dependencies;
}

languagescan_string_set_t *languagescan_language_textual_detail_get_command_line(
    languagescan_module_details_t details) {
  return details->language_textual_details.command_line;
}

languagescan_string_set_t *
languagescan_language_textual_detail_get_bridging_pch_command_line(
    languagescan_module_details_t details) {
  return details->language_textual_details.bridging_pch_command_line;
}

languagescan_string_ref_t languagescan_language_textual_detail_get_context_hash(
    languagescan_module_details_t details) {
  return details->language_textual_details.context_hash;
}

bool languagescan_language_textual_detail_get_is_framework(
    languagescan_module_details_t details) {
  return details->language_textual_details.is_framework;
}

languagescan_string_set_t *languagescan_language_textual_detail_get_language_overlay_dependencies(
    languagescan_module_details_t details) {
  return details->language_textual_details.code_overlay_module_dependencies;
}

languagescan_string_set_t *languagescan_language_textual_detail_get_language_source_import_module_dependencies(
    languagescan_module_details_t details) {
  return details->language_textual_details.source_import_module_dependencies;
}

languagescan_string_ref_t languagescan_language_textual_detail_get_cas_fs_root_id(
    languagescan_module_details_t details) {
  return details->language_textual_details.cas_fs_root_id;
}

languagescan_string_ref_t languagescan_language_textual_detail_get_module_cache_key(
    languagescan_module_details_t details) {
  return details->language_textual_details.module_cache_key;
}

languagescan_string_ref_t languagescan_language_textual_detail_get_user_module_version(
    languagescan_module_details_t details) {
  return details->language_textual_details.user_module_version;
}

languagescan_string_ref_t
languagescan_language_textual_detail_get_chained_bridging_header_path(
    languagescan_module_details_t details) {
  return details->language_textual_details.chained_bridging_header_path;
}

languagescan_string_ref_t
languagescan_language_textual_detail_get_chained_bridging_header_content(
    languagescan_module_details_t details) {
  return details->language_textual_details.chained_bridging_header_content;
}

//=== Codira Binary Module Details query APIs ------------------------------===//

languagescan_string_ref_t languagescan_language_binary_detail_get_compiled_module_path(
    languagescan_module_details_t details) {
  return details->language_binary_details.compiled_module_path;
}

languagescan_string_ref_t languagescan_language_binary_detail_get_module_doc_path(
    languagescan_module_details_t details) {
  return details->language_binary_details.module_doc_path;
}

languagescan_string_ref_t
languagescan_language_binary_detail_get_module_source_info_path(
    languagescan_module_details_t details) {
  return details->language_binary_details.module_source_info_path;
}

languagescan_string_set_t *
languagescan_language_binary_detail_get_language_overlay_dependencies(
    languagescan_module_details_t details) {
  return details->language_binary_details.code_overlay_module_dependencies;
}

languagescan_string_ref_t
languagescan_language_binary_detail_get_header_dependency(
    languagescan_module_details_t details) {
  return details->language_binary_details.header_dependency;
}

languagescan_string_set_t *
languagescan_language_binary_detail_get_header_dependency_module_dependencies(
    languagescan_module_details_t details) {
  return details->language_binary_details.header_dependencies_module_dependnecies;
}

bool languagescan_language_binary_detail_get_is_framework(
    languagescan_module_details_t details) {
  return details->language_binary_details.is_framework;
}

languagescan_string_ref_t languagescan_language_binary_detail_get_module_cache_key(
    languagescan_module_details_t details) {
  return details->language_binary_details.module_cache_key;
}

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_binary_detail_get_user_module_version(
    languagescan_module_details_t details) {
  return details->language_binary_details.user_module_version;
}

//=== Codira Placeholder Module Details query APIs - DEPRECATED -----------===//

languagescan_string_ref_t
languagescan_language_placeholder_detail_get_compiled_module_path(
    languagescan_module_details_t details) {
  return language::c_string_utils::create_null();
}

languagescan_string_ref_t languagescan_language_placeholder_detail_get_module_doc_path(
    languagescan_module_details_t details) {
  return language::c_string_utils::create_null();
}

languagescan_string_ref_t
languagescan_language_placeholder_detail_get_module_source_info_path(
    languagescan_module_details_t details) {
  return language::c_string_utils::create_null();
}

//=== Clang Module Details query APIs -------------------------------------===//

languagescan_string_ref_t
languagescan_clang_detail_get_module_map_path(languagescan_module_details_t details) {
  return details->clang_details.module_map_path;
}

languagescan_string_ref_t
languagescan_clang_detail_get_context_hash(languagescan_module_details_t details) {
  return details->clang_details.context_hash;
}

languagescan_string_set_t *
languagescan_clang_detail_get_command_line(languagescan_module_details_t details) {
  return details->clang_details.command_line;
}

languagescan_string_ref_t
languagescan_clang_detail_get_cas_fs_root_id(languagescan_module_details_t details) {
  return details->clang_details.cas_fs_root_id;
}

languagescan_string_ref_t languagescan_clang_detail_get_module_cache_key(
    languagescan_module_details_t details) {
  return details->clang_details.module_cache_key;
}

//=== Prescan Result Functions --------------------------------------------===//

languagescan_string_set_t *
languagescan_import_set_get_imports(languagescan_import_set_t result) {
  return result->imports;
}

languagescan_diagnostic_set_t *
languagescan_import_set_get_diagnostics(languagescan_import_set_t result) {
  return result->diagnostics;
}

//=== Scanner Invocation Functions ----------------------------------------===//

languagescan_scan_invocation_t languagescan_scan_invocation_create() {
  return new languagescan_scan_invocation_s;
}

void languagescan_scan_invocation_set_working_directory(
    languagescan_scan_invocation_t invocation, const char *working_directory) {
  invocation->working_directory = language::c_string_utils::create_clone(working_directory);
}

void
languagescan_scan_invocation_set_argv(languagescan_scan_invocation_t invocation,
                                   int argc, const char **argv) {
  invocation->argv = language::c_string_utils::create_set(argc, argv);
}

languagescan_string_ref_t languagescan_scan_invocation_get_working_directory(
    languagescan_scan_invocation_t invocation) {
  return invocation->working_directory;
}

int languagescan_scan_invocation_get_argc(languagescan_scan_invocation_t invocation) {
  return invocation->argv->count;
}

languagescan_string_set_t *
languagescan_scan_invocation_get_argv(languagescan_scan_invocation_t invocation) {
  return invocation->argv;
}

//=== Public Cleanup Functions --------------------------------------------===//

void languagescan_string_dispose(languagescan_string_ref_t string) {
  if (string.data)
    free(const_cast<void *>(string.data));
}

void languagescan_string_set_dispose(languagescan_string_set_t *set) {
  for (unsigned SI = 0, SE = set->count; SI < SE; ++SI)
    languagescan_string_dispose(set->strings[SI]);
  if (set->count > 0)
    delete[] set->strings;
  delete set;
}

void languagescan_dependency_graph_dispose(languagescan_dependency_graph_t result) {
  languagescan_string_dispose(result->main_module_name);
  languagescan_dependency_set_dispose(result->dependencies);
  languagescan_diagnostics_set_dispose(result->diagnostics);
  delete result;
}

void languagescan_import_set_dispose(languagescan_import_set_t result) {
  languagescan_string_set_dispose(result->imports);
  languagescan_diagnostics_set_dispose(result->diagnostics);
  delete result;
}

void languagescan_scan_invocation_dispose(languagescan_scan_invocation_t invocation) {
  languagescan_string_dispose(invocation->working_directory);
  languagescan_string_set_dispose(invocation->argv);
  delete invocation;
}

//=== Feature-Query Functions -----------------------------------------===//
static void addFrontendFlagOption(toolchain::opt::OptTable &table,
                                  language::options::ID id,
                                  std::vector<std::string> &frontendOptions) {
  if (table.getOption(id).hasFlag(language::options::FrontendOption)) {
    auto name = table.getOptionName(id);
    if (!name.empty()) {
      frontendOptions.push_back(std::string(name));
    }
  }
}

languagescan_string_ref_t
languagescan_compiler_target_info_query(languagescan_scan_invocation_t invocation) {
  return languagescan_compiler_target_info_query_v2(invocation, nullptr);
}

languagescan_string_ref_t
languagescan_compiler_target_info_query_v2(languagescan_scan_invocation_t invocation,
                                        const char *main_executable_path) {
  int argc = invocation->argv->count;
  std::vector<const char *> Compilation;
  for (int i = 0; i < argc; ++i)
    Compilation.push_back(language::c_string_utils::get_C_string(invocation->argv->strings[i]));

  auto TargetInfo = language::dependencies::getTargetInfo(Compilation, main_executable_path);
  if (TargetInfo.getError())
    return language::c_string_utils::create_null();
  return TargetInfo.get();
}

languagescan_string_set_t *
languagescan_compiler_supported_arguments_query() {
  std::unique_ptr<toolchain::opt::OptTable> table = language::createCodiraOptTable();
  std::vector<std::string> frontendFlags;
#define OPTION(...)                                                            \
  addFrontendFlagOption(*table, language::options::TOOLCHAIN_MAKE_OPT_ID(__VA_ARGS__), \
                        frontendFlags);
#include "language/Option/Options.inc"
#undef OPTION
  return language::c_string_utils::create_set(frontendFlags);
}

languagescan_string_set_t *
languagescan_compiler_supported_features_query() {
  std::vector<std::string> allFeatures;
  allFeatures.emplace_back("library-level");
  allFeatures.emplace_back("emit-abi-descriptor");
  return language::c_string_utils::create_set(allFeatures);
}

//=== Scanner Diagnostics -------------------------------------------------===//
languagescan_diagnostic_set_t*
languagescan_scanner_diagnostics_query(languagescan_scanner_t scanner) {
  // This method is deprecated
  languagescan_diagnostic_set_t *set = new languagescan_diagnostic_set_t;
  set->count = 0;
  return set;
}

void
languagescan_scanner_diagnostics_reset(languagescan_scanner_t scanner) {
  // This method is deprecated
}

languagescan_string_ref_t
languagescan_diagnostic_get_message(languagescan_diagnostic_info_t diagnostic) {
  return diagnostic->message;
}

languagescan_diagnostic_severity_t
languagescan_diagnostic_get_severity(languagescan_diagnostic_info_t diagnostic) {
  return diagnostic->severity;
}

languagescan_source_location_t
languagescan_diagnostic_get_source_location(languagescan_diagnostic_info_t diagnostic) {
  return diagnostic->source_location;
}

void languagescan_diagnostic_dispose(languagescan_diagnostic_info_t diagnostic) {
  languagescan_string_dispose(diagnostic->message);
  if (diagnostic->source_location) {
    languagescan_string_dispose(diagnostic->source_location->buffer_identifier);
    delete diagnostic->source_location;
  }
  delete diagnostic;
}

void
languagescan_diagnostics_set_dispose(languagescan_diagnostic_set_t* diagnostics){
  if (diagnostics) {
    for (size_t i = 0; i < diagnostics->count; ++i) {
      languagescan_diagnostic_dispose(diagnostics->diagnostics[i]);
    }
    delete[] diagnostics->diagnostics;
    delete diagnostics;
  }
}

//=== Source Location -----------------------------------------------------===//

languagescan_string_ref_t
languagescan_source_location_get_buffer_identifier(languagescan_source_location_t source_location) {
  return source_location->buffer_identifier;
}

int64_t
languagescan_source_location_get_line_number(languagescan_source_location_t source_location) {
  return source_location->line_number;
}

int64_t
languagescan_source_location_get_column_number(languagescan_source_location_t source_location) {
  return source_location->column_number;
}

//=== Experimental Compiler Invocation Functions ------------------------===//

int invoke_language_compiler(int argc, const char **argv) {
  return language::mainEntry(argc, argv);
}

//=== Deprecated Function Stubs -----------------------------------------===//
languagescan_batch_scan_result_t *
languagescan_batch_scan_result_create(languagescan_scanner_t scanner,
                                   languagescan_batch_scan_input_t *batch_input,
                                   languagescan_scan_invocation_t invocation) {
  return nullptr;
}
languagescan_string_set_t *languagescan_language_textual_detail_get_extra_pcm_args(
   languagescan_module_details_t details) {
  return language::c_string_utils::create_empty_set();
}
languagescan_string_set_t *
languagescan_clang_detail_get_captured_pcm_args(languagescan_module_details_t details) {
  return language::c_string_utils::create_empty_set();
}
languagescan_batch_scan_input_t *languagescan_batch_scan_input_create() {
  return nullptr;
}
void languagescan_batch_scan_input_set_modules(
   languagescan_batch_scan_input_t *input, int count,
   languagescan_batch_scan_entry_t *modules) {}

languagescan_batch_scan_entry_t languagescan_batch_scan_entry_create() {
  return nullptr;
}
void languagescan_batch_scan_entry_set_module_name(
   languagescan_batch_scan_entry_t entry, const char *name) {}
void languagescan_batch_scan_entry_set_arguments(
   languagescan_batch_scan_entry_t entry, const char *arguments) {}
void languagescan_batch_scan_entry_set_is_language(languagescan_batch_scan_entry_t entry,
                                            bool is_language) {}
languagescan_string_ref_t
languagescan_batch_scan_entry_get_module_name(languagescan_batch_scan_entry_t entry) {
  return language::c_string_utils::create_null();
}
languagescan_string_ref_t
languagescan_batch_scan_entry_get_arguments(languagescan_batch_scan_entry_t entry) {
  return language::c_string_utils::create_null();
}
bool languagescan_batch_scan_entry_get_is_language(
   languagescan_batch_scan_entry_t entry) {
  return false;
}
void languagescan_batch_scan_entry_dispose(languagescan_batch_scan_entry_t entry) {}
void languagescan_batch_scan_input_dispose(languagescan_batch_scan_input_t *input) {}
void languagescan_batch_scan_result_dispose(languagescan_batch_scan_result_t *result) {}
