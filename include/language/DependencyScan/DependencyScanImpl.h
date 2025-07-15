//===-------------- DependencyScanImpl.h - Codira Compiler -----------------===//
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
// Implementation details of the dependency scanning C API
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_C_DEPENDENCY_SCAN_IMPL_H
#define LANGUAGE_C_DEPENDENCY_SCAN_IMPL_H

#include "language-c/DependencyScan/DependencyScan.h"

namespace language {
namespace dependencies {
class DependencyScanningTool;
} // namespace dependencies
} // namespace language

struct languagescan_dependency_graph_s {
  /// The name of the main module for this dependency graph (root node)
  languagescan_string_ref_t main_module_name;

  /// The complete list of modules discovered
  languagescan_dependency_set_t *dependencies;

  /// Diagnostics produced during this scan
  languagescan_diagnostic_set_t *diagnostics;
};

struct languagescan_dependency_info_s {
  /// The module's name
  /// The format is:
  /// `<module-kind>:<module-name>`
  /// where `module-kind` is one of:
  /// "languageInterface"
  /// "languageSource"
  /// "languageBinary"
  /// "clang""
  languagescan_string_ref_t module_name;

  /// The path for the module.
  languagescan_string_ref_t module_path;

  /// The source files used to build this module.
  languagescan_string_set_t *source_files;

  /**
   * The list of modules which this module direct depends on.
   * The format is:
   * `<module-kind>:<module-name>`
   */
  languagescan_string_set_t *direct_dependencies;

  /// The list of link libraries for this module.
  languagescan_link_library_set_t *link_libraries;

  /// The list of source import infos.
  languagescan_import_info_set_t *imports;

  /// Specific details of a particular kind of module.
  languagescan_module_details_t details;
};

struct languagescan_link_library_info_s {
  languagescan_string_ref_t name;
  bool isStatic;
  bool isFramework;
  bool forceLoad;
};

struct languagescan_import_info_s {
  languagescan_string_ref_t import_identifier;
  languagescan_source_location_set_t *source_locations;
  languagescan_access_level_t access_level;
};

struct languagescan_macro_dependency_s {
  languagescan_string_ref_t module_name;
  languagescan_string_ref_t library_path;
  languagescan_string_ref_t executable_path;
};

/// Codira modules to be built from a module interface, may have a bridging
/// header.
typedef struct {
  /// The module interface from which this module was built, if any.
  languagescan_string_ref_t module_interface_path;

  /// The paths of potentially ready-to-use compiled modules for the interface.
  languagescan_string_set_t *compiled_module_candidates;

  /// The bridging header, if any.
  languagescan_string_ref_t bridging_header_path;

  /// The source files referenced by the bridging header.
  languagescan_string_set_t *bridging_source_files;

  /// (Clang) modules on which the bridging header depends.
  languagescan_string_set_t *bridging_module_dependencies;

  /// (Codira) module dependencies by means of being overlays of
  /// Clang module dependencies
  languagescan_string_set_t *language_overlay_module_dependencies;

  /// Directly-imported in source module dependencies
  languagescan_string_set_t *source_import_module_dependencies;

  /// Options to the compile command required to build this module interface
  languagescan_string_set_t *command_line;

  /// Options to the compile command required to build bridging header.
  languagescan_string_set_t *bridging_pch_command_line;

  /// The hash value that will be used for the generated module
  languagescan_string_ref_t context_hash;

  /// A flag to indicate whether or not this module is a framework.
  bool is_framework;

  /// A flag that indicates this dependency is associated with a static archive
  bool is_static;

  /// The CASID for CASFileSystemRoot
  languagescan_string_ref_t cas_fs_root_id;

  /// The CASID for bridging header include tree
  languagescan_string_ref_t bridging_header_include_tree;

  /// ModuleCacheKey
  languagescan_string_ref_t module_cache_key;

  /// Macro dependecies.
  languagescan_macro_dependency_set_t *macro_dependencies;

  /// User module version
  languagescan_string_ref_t user_module_version;

  /// Chained bridging header path.
  languagescan_string_ref_t chained_bridging_header_path;

  /// Chained bridging header content.
  languagescan_string_ref_t chained_bridging_header_content;

} languagescan_language_textual_details_t;

/// Codira modules with only a binary module file.
typedef struct {
  /// The path to the pre-compiled binary module
  languagescan_string_ref_t compiled_module_path;

  /// The path to the .codeModuleDoc file.
  languagescan_string_ref_t module_doc_path;

  /// The path to the .codeSourceInfo file.
  languagescan_string_ref_t module_source_info_path;

  /// (Codira) module dependencies by means of being overlays of
  /// Clang module dependencies
  languagescan_string_set_t *language_overlay_module_dependencies;

  /// (Clang) header (.h) dependency of this binary module.
  languagescan_string_ref_t header_dependency;

  /// (Clang) module dependencies of the textual header inputs
  languagescan_string_set_t *header_dependencies_module_dependnecies;

  /// Source files included by the header dependencies of this binary module
  languagescan_string_set_t *header_dependencies_source_files;

  /// A flag to indicate whether or not this module is a framework.
  bool is_framework;

  /// A flag that indicates this dependency is associated with a static archive
  bool is_static;

  /// Macro dependecies.
  languagescan_macro_dependency_set_t *macro_dependencies;

  /// ModuleCacheKey
  languagescan_string_ref_t module_cache_key;

  /// User module version
  languagescan_string_ref_t user_module_version;
} languagescan_language_binary_details_t;

/// Clang modules are built from a module map file.
typedef struct {
  /// The path to the module map used to build this module.
  languagescan_string_ref_t module_map_path;

  /// clang-generated context hash
  languagescan_string_ref_t context_hash;

  /// Options to the compile command required to build this clang modulemap
  languagescan_string_set_t *command_line;

  /// The CASID for CASFileSystemRoot
  languagescan_string_ref_t cas_fs_root_id;

  /// The CASID for CASFileSystemRoot
  languagescan_string_ref_t clang_include_tree;

  /// ModuleCacheKey
  languagescan_string_ref_t module_cache_key;
} languagescan_clang_details_t;

struct languagescan_module_details_s {
  languagescan_dependency_info_kind_t kind;
  union {
    languagescan_language_textual_details_t language_textual_details;
    languagescan_language_binary_details_t language_binary_details;
    languagescan_clang_details_t clang_details;
  };
};

struct languagescan_import_set_s {
  /// The complete list of imports discovered
  languagescan_string_set_t *imports;
  /// Diagnostics produced during this import scan
  languagescan_diagnostic_set_t *diagnostics;
};

struct languagescan_scan_invocation_s {
  languagescan_string_ref_t working_directory;
  languagescan_string_set_t *argv;
};

struct languagescan_diagnostic_info_s {
  languagescan_string_ref_t message;
  languagescan_diagnostic_severity_t severity;
  languagescan_source_location_t source_location;
};

struct languagescan_source_location_s {
  languagescan_string_ref_t buffer_identifier;
  uint32_t line_number;
  uint32_t column_number;
};

#endif // LANGUAGE_C_DEPENDENCY_SCAN_IMPL_H
