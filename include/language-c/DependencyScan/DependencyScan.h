//===--- DependencyScan.h - C API for Codira Dependency Scanning ---*- C -*-===//
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
// This C API is primarily intended to serve as the Codira Driver's
// dependency scanning facility (https://github.com/apple/language-driver).
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_C_DEPENDENCY_SCAN_H
#define LANGUAGE_C_DEPENDENCY_SCAN_H

#include "DependencyScanMacros.h"
#include "language-c/CommonString/CommonString.h"

/// The version constants for the CodiraDependencyScan C API.
/// LANGUAGESCAN_VERSION_MINOR should increase when there are API additions.
/// LANGUAGESCAN_VERSION_MAJOR is intended for "major" source/ABI breaking changes.
#define LANGUAGESCAN_VERSION_MAJOR 2
#define LANGUAGESCAN_VERSION_MINOR 2

LANGUAGESCAN_BEGIN_DECLS

//=== Public Scanner Data Types -------------------------------------------===//

typedef enum {
  // This dependency info encodes two ModuleDependencyKind types:
  // CodiraInterface and CodiraSource.
  LANGUAGESCAN_DEPENDENCY_INFO_LANGUAGE_TEXTUAL = 0,
  LANGUAGESCAN_DEPENDENCY_INFO_LANGUAGE_BINARY = 1,
  LANGUAGESCAN_DEPENDENCY_INFO_CLANG = 3
} languagescan_dependency_info_kind_t;

/// Opaque container of the details specific to a given module dependency.
typedef struct languagescan_module_details_s *languagescan_module_details_t;

/// Opaque container to a dependency info of a given module.
typedef struct languagescan_dependency_info_s *languagescan_dependency_info_t;

/// Opaque container to a link library info.
typedef struct languagescan_link_library_info_s *languagescan_link_library_info_t;

/// Opaque container to an import info.
typedef struct languagescan_import_info_s *languagescan_import_info_t;

/// Opaque container to a macro dependency.
typedef struct languagescan_macro_dependency_s *languagescan_macro_dependency_t;

/// Opaque container to an overall result of a dependency scan.
typedef struct languagescan_dependency_graph_s *languagescan_dependency_graph_t;

/// Opaque container to contain the result of a dependency prescan.
typedef struct languagescan_import_set_s *languagescan_import_set_t;

/// Opaque container to contain the info of a diagnostics emitted by the scanner.
typedef struct languagescan_diagnostic_info_s *languagescan_diagnostic_info_t;

/// Opaque container to contain the info of a source location.
typedef struct languagescan_source_location_s *languagescan_source_location_t;

/// Full Dependency Graph (Result)
typedef struct {
  languagescan_dependency_info_t *modules;
  size_t count;
} languagescan_dependency_set_t;

/// Set of linked libraries
typedef struct {
  languagescan_link_library_info_t *link_libraries;
  size_t count;
} languagescan_link_library_set_t;

/// Set of details about source imports
typedef struct {
  languagescan_import_info_t *imports;
  size_t count;
} languagescan_import_info_set_t;

/// Set of source location infos
typedef struct {
  languagescan_source_location_t *source_locations;
  size_t count;
} languagescan_source_location_set_t;

/// Set of macro dependency
typedef struct {
  languagescan_macro_dependency_t *macro_dependencies;
  size_t count;
} languagescan_macro_dependency_set_t;

typedef enum {
  LANGUAGESCAN_DIAGNOSTIC_SEVERITY_ERROR = 0,
  LANGUAGESCAN_DIAGNOSTIC_SEVERITY_WARNING = 1,
  LANGUAGESCAN_DIAGNOSTIC_SEVERITY_NOTE = 2,
  LANGUAGESCAN_DIAGNOSTIC_SEVERITY_REMARK = 3
} languagescan_diagnostic_severity_t;

// Must maintain consistency with language::AccessLevel
typedef enum {
  LANGUAGESCAN_ACCESS_LEVEL_PRIVATE = 0,
  LANGUAGESCAN_ACCESS_LEVEL_FILEPRIVATE = 1,
  LANGUAGESCAN_ACCESS_LEVEL_INTERNAL = 2,
  LANGUAGESCAN_ACCESS_LEVEL_PACKAGE = 3,
  LANGUAGESCAN_ACCESS_LEVEL_PUBLIC = 4
} languagescan_access_level_t;

typedef struct {
  languagescan_diagnostic_info_t *diagnostics;
  size_t count;
} languagescan_diagnostic_set_t;

//=== Batch Scan Input Specification -------DEPRECATED--------------------===//

 /// Opaque container to a container of batch scan entry information.
 typedef struct languagescan_batch_scan_entry_s *languagescan_batch_scan_entry_t;

 typedef struct {
   languagescan_batch_scan_entry_t *modules;
   size_t count;
 } languagescan_batch_scan_input_t;

 typedef struct {
   languagescan_dependency_graph_t *results;
   size_t count;
 } languagescan_batch_scan_result_t;

//=== Scanner Invocation Specification ------------------------------------===//

/// Opaque container of all relevant context required to launch a dependency
/// scan (command line arguments, working directory, etc.)
typedef struct languagescan_scan_invocation_s *languagescan_scan_invocation_t;

//=== Dependency Result Functions -----------------------------------------===//

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_dependency_graph_get_main_module_name(
    languagescan_dependency_graph_t result);

LANGUAGESCAN_PUBLIC languagescan_dependency_set_t *
languagescan_dependency_graph_get_dependencies(
    languagescan_dependency_graph_t result);

// Return value disposed of together with the dependency_graph
// using `languagescan_dependency_graph_dispose`
LANGUAGESCAN_PUBLIC languagescan_diagnostic_set_t *
languagescan_dependency_graph_get_diagnostics(
    languagescan_dependency_graph_t result);

//=== Dependency Module Info Functions ------------------------------------===//

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_module_info_get_module_name(languagescan_dependency_info_t info);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_module_info_get_module_path(languagescan_dependency_info_t info);

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_module_info_get_source_files(languagescan_dependency_info_t info);

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_module_info_get_direct_dependencies(languagescan_dependency_info_t info);

LANGUAGESCAN_PUBLIC languagescan_link_library_set_t *
languagescan_module_info_get_link_libraries(languagescan_dependency_info_t info);

LANGUAGESCAN_PUBLIC languagescan_import_info_set_t *
languagescan_module_info_get_imports(languagescan_dependency_info_t info);

LANGUAGESCAN_PUBLIC languagescan_module_details_t
languagescan_module_info_get_details(languagescan_dependency_info_t info);

//=== Import Details Functions -------------------------------------------===//
LANGUAGESCAN_PUBLIC languagescan_source_location_set_t *
languagescan_import_info_get_source_locations(languagescan_import_info_t info);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_import_info_get_identifier(languagescan_import_info_t info);

LANGUAGESCAN_PUBLIC languagescan_access_level_t
languagescan_import_info_get_access_level(languagescan_import_info_t info);

//=== Link Library Info Functions ----------------------------------------===//
LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_link_library_info_get_link_name(
    languagescan_link_library_info_t info);
LANGUAGESCAN_PUBLIC bool
languagescan_link_library_info_get_is_static(languagescan_link_library_info_t info);
LANGUAGESCAN_PUBLIC bool languagescan_link_library_info_get_is_framework(
    languagescan_link_library_info_t info);
LANGUAGESCAN_PUBLIC bool languagescan_link_library_info_get_should_force_load(
    languagescan_link_library_info_t info);

//=== Dependency Module Info Details Functions ----------------------------===//

LANGUAGESCAN_PUBLIC languagescan_dependency_info_kind_t
languagescan_module_detail_get_kind(languagescan_module_details_t details);

//=== Codira Textual Module Details query APIs -----------------------------===//
LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_textual_detail_get_module_interface_path(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_language_textual_detail_get_compiled_module_candidates(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_textual_detail_get_bridging_header_path(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_language_textual_detail_get_bridging_source_files(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_language_textual_detail_get_bridging_module_dependencies(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_language_textual_detail_get_command_line(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_language_textual_detail_get_bridging_pch_command_line(
    languagescan_module_details_t details);

// DEPRECATED
LANGUAGESCAN_PUBLIC languagescan_string_set_t *
 languagescan_language_textual_detail_get_extra_pcm_args(
     languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_textual_detail_get_context_hash(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC bool languagescan_language_textual_detail_get_is_framework(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_language_textual_detail_get_language_overlay_dependencies(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_language_textual_detail_get_language_source_import_module_dependencies(
languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_textual_detail_get_cas_fs_root_id(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_textual_detail_get_module_cache_key(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_textual_detail_get_user_module_version(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_textual_detail_get_chained_bridging_header_path(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_textual_detail_get_chained_bridging_header_content(
    languagescan_module_details_t details);

//=== Codira Binary Module Details query APIs ------------------------------===//

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_binary_detail_get_compiled_module_path(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_binary_detail_get_module_doc_path(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_binary_detail_get_module_source_info_path(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_language_binary_detail_get_language_overlay_dependencies(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_binary_detail_get_header_dependency(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_language_binary_detail_get_header_dependency_module_dependencies(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC bool
languagescan_language_binary_detail_get_is_framework(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_binary_detail_get_module_cache_key(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_binary_detail_get_user_module_version(
    languagescan_module_details_t details);

//=== Codira Placeholder Module Details query APIs - DEPRECATED -----------===//

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_placeholder_detail_get_compiled_module_path(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_placeholder_detail_get_module_doc_path(
    languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_language_placeholder_detail_get_module_source_info_path(
    languagescan_module_details_t details);

//=== Clang Module Details query APIs -------------------------------------===//

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_clang_detail_get_module_map_path(languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_clang_detail_get_context_hash(languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_clang_detail_get_command_line(languagescan_module_details_t details);

// DEPRECATED
LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_clang_detail_get_captured_pcm_args(languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_clang_detail_get_cas_fs_root_id(languagescan_module_details_t details);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_clang_detail_get_module_cache_key(languagescan_module_details_t details);

//=== Batch Scan Input Functions ------DEPRECATED---------------------------===//
 /// Deprecated
 LANGUAGESCAN_PUBLIC languagescan_batch_scan_input_t *
 languagescan_batch_scan_input_create();
 /// Deprecated
 LANGUAGESCAN_PUBLIC void
 languagescan_batch_scan_input_set_modules(languagescan_batch_scan_input_t *input,
                                        int count,
                                        languagescan_batch_scan_entry_t *modules);
 /// Deprecated
 LANGUAGESCAN_PUBLIC languagescan_batch_scan_entry_t
 languagescan_batch_scan_entry_create();
 /// Deprecated
 LANGUAGESCAN_PUBLIC void
 languagescan_batch_scan_entry_set_module_name(languagescan_batch_scan_entry_t entry,
                                            const char *name);
 /// Deprecated
 LANGUAGESCAN_PUBLIC void
 languagescan_batch_scan_entry_set_arguments(languagescan_batch_scan_entry_t entry,
                                          const char *arguments);
 /// Deprecated
 LANGUAGESCAN_PUBLIC void
 languagescan_batch_scan_entry_set_is_language(languagescan_batch_scan_entry_t entry,
                                         bool is_language);
 /// Deprecated
 LANGUAGESCAN_PUBLIC languagescan_string_ref_t
 languagescan_batch_scan_entry_get_module_name(languagescan_batch_scan_entry_t entry);
 /// Deprecated
 LANGUAGESCAN_PUBLIC languagescan_string_ref_t
 languagescan_batch_scan_entry_get_arguments(languagescan_batch_scan_entry_t entry);
 /// Deprecated
 LANGUAGESCAN_PUBLIC bool
 languagescan_batch_scan_entry_get_is_language(languagescan_batch_scan_entry_t entry);

//=== Prescan Result Functions --------------------------------------------===//

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_import_set_get_imports(languagescan_import_set_t result);

// Return value disposed of together with the dependency_graph
// using `languagescan_import_set_dispose`
LANGUAGESCAN_PUBLIC languagescan_diagnostic_set_t *
languagescan_import_set_get_diagnostics(languagescan_import_set_t result);

//=== Scanner Invocation Functions ----------------------------------------===//

/// Create an \c languagescan_scan_invocation_t instance.
/// The returned \c languagescan_scan_invocation_t is owned by the caller and must be disposed
/// of using \c languagescan_scan_invocation_dispose .
LANGUAGESCAN_PUBLIC languagescan_scan_invocation_t languagescan_scan_invocation_create();

LANGUAGESCAN_PUBLIC void languagescan_scan_invocation_set_working_directory(
    languagescan_scan_invocation_t invocation, const char *working_directory);

LANGUAGESCAN_PUBLIC void
languagescan_scan_invocation_set_argv(languagescan_scan_invocation_t invocation,
                                   int argc, const char **argv);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_scan_invocation_get_working_directory(
    languagescan_scan_invocation_t invocation);

LANGUAGESCAN_PUBLIC int
languagescan_scan_invocation_get_argc(languagescan_scan_invocation_t invocation);

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_scan_invocation_get_argv(languagescan_scan_invocation_t invocation);

//=== Cleanup Functions ---------------------------------------------------===//

LANGUAGESCAN_PUBLIC void
languagescan_string_set_dispose(languagescan_string_set_t *set);

LANGUAGESCAN_PUBLIC void
languagescan_string_dispose(languagescan_string_ref_t string);

LANGUAGESCAN_PUBLIC void
languagescan_dependency_graph_dispose(languagescan_dependency_graph_t result);

LANGUAGESCAN_PUBLIC void
languagescan_import_set_dispose(languagescan_import_set_t result);

/// Deprecated
LANGUAGESCAN_PUBLIC void
languagescan_batch_scan_entry_dispose(languagescan_batch_scan_entry_t entry);
/// Deprecated
LANGUAGESCAN_PUBLIC void
languagescan_batch_scan_input_dispose(languagescan_batch_scan_input_t *input);
/// Deprecated
LANGUAGESCAN_PUBLIC void
languagescan_batch_scan_result_dispose(languagescan_batch_scan_result_t *result);
/// Deprecated
LANGUAGESCAN_PUBLIC void
languagescan_scan_invocation_dispose(languagescan_scan_invocation_t invocation);

//=== Feature-Query Functions ---------------------------------------------===//
LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_compiler_supported_arguments_query();

LANGUAGESCAN_PUBLIC languagescan_string_set_t *
languagescan_compiler_supported_features_query();

//=== Target-Info Functions -----------------------------------------------===//
LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_compiler_target_info_query(languagescan_scan_invocation_t invocation);
LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_compiler_target_info_query_v2(languagescan_scan_invocation_t invocation,
                                        const char *main_executable_path);

//=== Scanner Functions ---------------------------------------------------===//

/// Container of the configuration state and shared cache for dependency
/// scanning.
typedef void *languagescan_scanner_t;

/// Create an \c languagescan_scanner_t instance.
/// The returned \c languagescan_scanner_t is owned by the caller and must be disposed
/// of using \c languagescan_scanner_dispose .
LANGUAGESCAN_PUBLIC languagescan_scanner_t languagescan_scanner_create(void);
LANGUAGESCAN_PUBLIC void languagescan_scanner_dispose(languagescan_scanner_t);

/// Invoke a dependency scan using arguments specified in the \c
/// languagescan_scan_invocation_t argument. The returned \c
/// languagescan_dependency_graph_t is owned by the caller and must be disposed of
/// using \c languagescan_dependency_graph_dispose .
LANGUAGESCAN_PUBLIC languagescan_dependency_graph_t languagescan_dependency_graph_create(
    languagescan_scanner_t scanner, languagescan_scan_invocation_t invocation);

/// Invoke the import prescan using arguments specified in the \c
/// languagescan_scan_invocation_t argument. The returned \c languagescan_import_set_t
/// is owned by the caller and must be disposed of using \c
/// languagescan_import_set_dispose .
LANGUAGESCAN_PUBLIC languagescan_import_set_t languagescan_import_set_create(
    languagescan_scanner_t scanner, languagescan_scan_invocation_t invocation);

/// Deprecated
LANGUAGESCAN_PUBLIC languagescan_batch_scan_result_t *
languagescan_batch_scan_result_create(languagescan_scanner_t scanner,
                                   languagescan_batch_scan_input_t *batch_input,
                                   languagescan_scan_invocation_t invocation);

//=== Scanner Diagnostics -------------------------------------------------===//
/// For the specified \c scanner instance, query all insofar emitted diagnostics
LANGUAGESCAN_PUBLIC languagescan_diagnostic_set_t*
languagescan_scanner_diagnostics_query(languagescan_scanner_t scanner);

/// For the specified \c scanner instance, reset its diagnostic state
LANGUAGESCAN_PUBLIC void
languagescan_scanner_diagnostics_reset(languagescan_scanner_t scanner);

LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_diagnostic_get_message(languagescan_diagnostic_info_t diagnostic);

LANGUAGESCAN_PUBLIC languagescan_diagnostic_severity_t
languagescan_diagnostic_get_severity(languagescan_diagnostic_info_t diagnostic);

LANGUAGESCAN_PUBLIC languagescan_source_location_t
languagescan_diagnostic_get_source_location(languagescan_diagnostic_info_t diagnostic);

LANGUAGESCAN_PUBLIC void
languagescan_diagnostics_set_dispose(languagescan_diagnostic_set_t* diagnostics);

//=== Source Location -----------------------------------------------------===//
LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_source_location_get_buffer_identifier(languagescan_source_location_t source_location);

LANGUAGESCAN_PUBLIC int64_t
languagescan_source_location_get_line_number(languagescan_source_location_t source_location);

LANGUAGESCAN_PUBLIC int64_t
languagescan_source_location_get_column_number(languagescan_source_location_t source_location);

//=== Scanner Cache Operations --------------------------------------------===//
// The following operations expose an implementation detail of the dependency
// scanner: its module dependencies cache. This is done in order
// to allow clients to perform incremental dependency scans by having the
// scanner's state be serializable and re-usable.

/// For the specified \c scanner instance, reset its internal state, ensuring subsequent
/// scanning queries are done "from-scratch".
LANGUAGESCAN_PUBLIC void
languagescan_scanner_cache_reset(languagescan_scanner_t scanner);

/// An entry point to invoke the compiler via a library call.
LANGUAGESCAN_PUBLIC int invoke_language_compiler(int argc, const char **argv);

//=== Scanner CAS Operations ----------------------------------------------===//

/// Opaque container for a CASOptions that describe how CAS should be created.
typedef struct languagescan_cas_options_s *languagescan_cas_options_t;

/// Opaque container for a CAS instance that includes both ObjectStore and
/// ActionCache.
typedef struct languagescan_cas_s *languagescan_cas_t;

/// Opaque container for a cached compilation.
typedef struct languagescan_cached_compilation_s *languagescan_cached_compilation_t;

/// Opaque container for a cached compilation output.
typedef struct languagescan_cached_output_s *languagescan_cached_output_t;

/// Opaque type for a cache replay instance.
typedef struct languagescan_cache_replay_instance_s
    *languagescan_cache_replay_instance_t;

/// Opaque container for a cached compilation replay result.
typedef struct languagescan_cache_replay_result_s *languagescan_cache_replay_result_t;

/// Opaque type for a cancellation token for async cache operations.
typedef struct languagescan_cache_cancellation_token_s
    *languagescan_cache_cancellation_token_t;

/// Create a \c CASOptions for creating CAS inside scanner specified.
LANGUAGESCAN_PUBLIC languagescan_cas_options_t languagescan_cas_options_create(void);

/// Dispose \c CASOptions.
LANGUAGESCAN_PUBLIC void
languagescan_cas_options_dispose(languagescan_cas_options_t options);

/// Set on-disk path for the \c cas.
LANGUAGESCAN_PUBLIC void
languagescan_cas_options_set_ondisk_path(languagescan_cas_options_t options,
                                      const char *path);

/// Set plugin path for the \c cas.
LANGUAGESCAN_PUBLIC void
languagescan_cas_options_set_plugin_path(languagescan_cas_options_t options,
                                      const char *path);

/// Set option using a name/value pair. Return true if error.
/// If error happens, the error message is returned via `error` parameter, and
/// caller needs to free the error message via `languagescan_string_dispose`.
LANGUAGESCAN_PUBLIC bool
languagescan_cas_options_set_plugin_option(languagescan_cas_options_t options,
                                        const char *name, const char *value,
                                        languagescan_string_ref_t *error);

/// Create a \c cas instance from plugin. Return NULL if error.
/// If error happens, the error message is returned via `error` parameter, and
/// caller needs to free the error message via `languagescan_string_dispose`.
LANGUAGESCAN_PUBLIC languagescan_cas_t languagescan_cas_create_from_options(
    languagescan_cas_options_t options, languagescan_string_ref_t *error);

/// Store content into CAS. Return \c CASID as string. Return NULL on error.
/// If error happens, the error message is returned via `error` parameter, and
/// caller needs to free the error message via `languagescan_string_dispose`.
LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_cas_store(languagescan_cas_t cas, uint8_t *data, unsigned size,
                    languagescan_string_ref_t *error);

/// Get the local storage size for the CAS in bytes. Return the local storage
/// size of the CAS/cache data, or -1 if the implementation does not support
/// reporting such size, or -2 if an error occurred.
/// If error happens, the error message is returned via `error` parameter, and
/// caller needs to free the error message via `languagescan_string_dispose`.
LANGUAGESCAN_PUBLIC int64_t
languagescan_cas_get_ondisk_size(languagescan_cas_t, languagescan_string_ref_t *error);

/// Set the size for the limiting disk storage size for CAS. \c size_limit is
/// the maximum size limit in bytes (0 means no limit, negative is invalid).
/// Return true if error. If error happens, the error message is returned via
/// `error` parameter, and caller needs to free the error message via
/// `languagescan_string_dispose`.
LANGUAGESCAN_PUBLIC bool
languagescan_cas_set_ondisk_size_limit(languagescan_cas_t, int64_t size_limit,
                                    languagescan_string_ref_t *error);

/// Prune local CAS storage according to the size limit. Return true if error.
/// If error happens, the error message is returned via `error` parameter, and
/// caller needs to free the error message via `languagescan_string_dispose`.
LANGUAGESCAN_PUBLIC bool
languagescan_cas_prune_ondisk_data(languagescan_cas_t, languagescan_string_ref_t *error);

/// Dispose the \c cas instance.
LANGUAGESCAN_PUBLIC void languagescan_cas_dispose(languagescan_cas_t cas);

/// Compute \c CacheKey for the outputs of a primary input file from a compiler
/// invocation with command-line \c argc and \c argv. When primary input file
/// is not available for compilation, e.g., using WMO, primary file is the first
/// language input on the command-line by convention. Return \c CacheKey as string.
/// If error happens, the error message is returned via `error` parameter, and
/// caller needs to free the error message via `languagescan_string_dispose`.
/// This API is DEPRECATED and in favor of using
/// `languagescan_cache_compute_key_from_input_index`.
LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_cache_compute_key(languagescan_cas_t cas, int argc, const char **argv,
                            const char *input, languagescan_string_ref_t *error);

/// Compute \c CacheKey for the outputs of a primary input file from a compiler
/// invocation with command-line \c argc and \c argv and the index for the
/// input. The index of the input is computed from the position of the input
/// file from all input files. When primary input file is not available for
/// compilation, e.g., using WMO, primary file is the first language input on the
/// command-line by convention. Return \c CacheKey as string. If error happens,
/// the error message is returned via `error` parameter, and caller needs to
/// free the error message via `languagescan_string_dispose`.
LANGUAGESCAN_PUBLIC languagescan_string_ref_t
languagescan_cache_compute_key_from_input_index(languagescan_cas_t cas, int argc,
                                             const char **argv,
                                             unsigned input_index,
                                             languagescan_string_ref_t *error);

/// Query the result of the compilation using the output cache key. \c globally
/// suggests if the lookup should check remote cache if such operation exists.
/// Returns the cached compilation of the result if found, or nullptr if output
/// is not found or an error occurs. When an error occurs, the error message is
/// returned via \c error parameter and its caller needs to free the message
/// using `languagescan_string_dispose`. The returned cached compilation needs to
/// be freed via `languagescan_cached_compilation_dispose`.
LANGUAGESCAN_PUBLIC languagescan_cached_compilation_t
languagescan_cache_query(languagescan_cas_t cas, const char *key, bool globally,
                      languagescan_string_ref_t *error);

/// Async version of `languagescan_cache_query` where result is returned via
/// callback. Both cache_result enum and cached compilation will be provided to
/// callback. \c ctx is an opaque value that passed to the callback and \c
/// languagescan_cache_cancellation_token_t will return an token that can be used
/// to cancel the async operation. The token needs to be freed by caller using
/// `languagescan_cache_cancellation_token_dispose`. If no token is needed, nullptr
/// can be passed and no token will be returned.
LANGUAGESCAN_PUBLIC void languagescan_cache_query_async(
    languagescan_cas_t cas, const char *key, bool globally, void *ctx,
    void (*callback)(void *ctx, languagescan_cached_compilation_t,
                     languagescan_string_ref_t error),
    languagescan_cache_cancellation_token_t *);

/// Query the number of outputs from a cached compilation.
LANGUAGESCAN_PUBLIC unsigned languagescan_cached_compilation_get_num_outputs(
    languagescan_cached_compilation_t);

/// Get the cached output for the given index in the cached compilation.
LANGUAGESCAN_PUBLIC languagescan_cached_output_t
languagescan_cached_compilation_get_output(languagescan_cached_compilation_t,
                                        unsigned idx);

/// Check if the requested cached compilation is uncacheable. That means the
/// compiler decides to skip caching its output even the compilation is
/// successful.
LANGUAGESCAN_PUBLIC bool
    languagescan_cached_compilation_is_uncacheable(languagescan_cached_compilation_t);

/// Make the cache compilation available globally. \c callback will be called
/// on completion.
/// \c languagescan_cache_cancellation_token_t will return an token that can be
/// used to cancel the async operation. The token needs to be freed by caller
/// using `languagescan_cache_cancellation_token_dispose`. If no token is needed,
/// nullptr can be passed and no token will be returned.
LANGUAGESCAN_PUBLIC void languagescan_cached_compilation_make_global_async(
    languagescan_cached_compilation_t, void *ctx,
    void (*callback)(void *ctx, languagescan_string_ref_t error),
    languagescan_cache_cancellation_token_t *);

/// Dispose a cached compilation.
LANGUAGESCAN_PUBLIC
void languagescan_cached_compilation_dispose(languagescan_cached_compilation_t);

/// Download and materialize the cached output if needed from a remote CAS.
/// Return true if load is successful, else false if not found or error. If
/// error, the error message is returned via \c error parameter and its caller
/// needs to free the message using `languagescan_string_dispose`.
LANGUAGESCAN_PUBLIC bool
languagescan_cached_output_load(languagescan_cached_output_t,
                             languagescan_string_ref_t *error);

/// Async version of `languagescan_cached_output_load` where result is
/// returned via callback. \c ctx is an opaque value that passed to the callback
/// and \c languagescan_cache_cancellation_token_t will return an token that can be
/// used to cancel the async operation. The token needs to be freed by caller
/// using `languagescan_cache_cancellation_token_dispose`. If no token is needed,
/// nullptr can be passed and no token will be returned.
LANGUAGESCAN_PUBLIC void languagescan_cached_output_load_async(
    languagescan_cached_output_t, void *ctx,
    void (*callback)(void *ctx, bool success, languagescan_string_ref_t error),
    languagescan_cache_cancellation_token_t *);

/// Check if cached output is materialized locally and can be accessed
/// without downloading.
LANGUAGESCAN_PUBLIC bool
    languagescan_cached_output_is_materialized(languagescan_cached_output_t);

/// Return the casid for the cached output as \c languagescan_string_ref_t and the
/// returned string needs to be freed using `languagescan_string_dispose`. CASID
/// can be requested before loading/materializing.
LANGUAGESCAN_PUBLIC languagescan_string_ref_t
    languagescan_cached_output_get_casid(languagescan_cached_output_t);

/// Get the output name for cached compilation. The
/// returned name needs to be freed by `languagescan_string_dispose`.
LANGUAGESCAN_PUBLIC languagescan_string_ref_t
    languagescan_cached_output_get_name(languagescan_cached_output_t);

/// Dispose a cached output.
LANGUAGESCAN_PUBLIC
void languagescan_cached_output_dispose(languagescan_cached_output_t);

/// Cancel the async cache action that is associated with token.
LANGUAGESCAN_PUBLIC void
    languagescan_cache_action_cancel(languagescan_cache_cancellation_token_t);

/// Dispose the cancellation token.
LANGUAGESCAN_PUBLIC void languagescan_cache_cancellation_token_dispose(
    languagescan_cache_cancellation_token_t);

/// Async load CASObject using CASID. This is only to perform the download to
/// local CAS where result is returned via callback. \c ctx is an opaque value
/// that passed to the callback and \c languagescan_cache_cancellation_token_t will
/// return an token that can be used to cancel the async operation. The token
/// needs to be freed by caller using
/// `languagescan_cache_cancellation_token_dispose`. If no token is needed, nullptr
/// can be passed and no token will be returned.
LANGUAGESCAN_PUBLIC void languagescan_cache_download_cas_object_async(
    languagescan_cas_t, const char *id, void *ctx,
    void (*callback)(void *ctx, bool success, languagescan_string_ref_t error),
    languagescan_cache_cancellation_token_t *);

/// Create a language cached compilation replay instance with its command-line
/// invocation. Return nullptr when errors occurs and the error message is
/// returned via \c error parameter and its caller needs to free the message
/// using `languagescan_string_dispose`.
LANGUAGESCAN_PUBLIC languagescan_cache_replay_instance_t
languagescan_cache_replay_instance_create(int argc, const char **argv,
                                       languagescan_string_ref_t *error);

/// Dispose language cached compilation replay instance.
LANGUAGESCAN_PUBLIC void
    languagescan_cache_replay_instance_dispose(languagescan_cache_replay_instance_t);

/// Replay the cached compilation using cached compliation replay instance.
/// Returns replay result or nullptr if output not found or error occurs. If
/// error, the error message is returned via \c error parameter and its caller
/// needs to free the message using `languagescan_string_dispose`.
LANGUAGESCAN_PUBLIC languagescan_cache_replay_result_t
languagescan_cache_replay_compilation(languagescan_cache_replay_instance_t,
                                   languagescan_cached_compilation_t,
                                   languagescan_string_ref_t *error);

/// Get stdout from cached replay result. The returning languagescan_string_ref_t
/// is owned by replay result and should not be disposed.
LANGUAGESCAN_PUBLIC
languagescan_string_ref_t
    languagescan_cache_replay_result_get_stdout(languagescan_cache_replay_result_t);

/// Get stderr from cached replay result. The returning languagescan_string_ref_t
/// is owned by replay result and should not be disposed.
LANGUAGESCAN_PUBLIC
languagescan_string_ref_t
    languagescan_cache_replay_result_get_stderr(languagescan_cache_replay_result_t);

/// Dispose a cached replay result.
LANGUAGESCAN_PUBLIC
void languagescan_cache_replay_result_dispose(languagescan_cache_replay_result_t);

//===----------------------------------------------------------------------===//

LANGUAGESCAN_END_DECLS

#endif // LANGUAGE_C_DEPENDENCY_SCAN_H
