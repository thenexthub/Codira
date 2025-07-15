//===--- DependencyScanJSON.cpp -- JSON output for dependencies -----------===//
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
#include "language/DependencyScan/DependencyScanJSON.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/DependencyScan/DependencyScanImpl.h"
#include "language/DependencyScan/StringUtils.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringRef.h"

using namespace language;
using namespace language::dependencies;
using namespace language::c_string_utils;
using namespace toolchain;

namespace {
std::string quote(StringRef unquoted) {
  toolchain::SmallString<128> buffer;
  toolchain::raw_svector_ostream os(buffer);
  for (const auto ch : unquoted) {
    if (ch == '\\')
      os << '\\';
    if (ch == '"')
      os << '\\';
    os << ch;
  }
  return buffer.str().str();
}
} // namespace


/// Write a string value as JSON.
void writeJSONValue(toolchain::raw_ostream &out, StringRef value,
                    unsigned indentLevel) {
  out << "\"";
  out << quote(value);
  out << "\"";
}

void writeJSONValue(toolchain::raw_ostream &out, languagescan_string_ref_t value,
                    unsigned indentLevel) {
  out << "\"";
  out << quote(get_C_string(value));
  out << "\"";
}

void writeJSONValue(toolchain::raw_ostream &out, languagescan_string_set_t *value_set,
                    unsigned indentLevel) {
  out << "[\n";

  for (size_t i = 0; i < value_set->count; ++i) {
    out.indent((indentLevel + 1) * 2);

    writeJSONValue(out, value_set->strings[i], indentLevel + 1);

    if (i != value_set->count - 1) {
      out << ",";
    }
    out << "\n";
  }

  out.indent(indentLevel * 2);
  out << "]";
}

/// Write a boolean value as JSON.
void writeJSONValue(toolchain::raw_ostream &out, bool value, unsigned indentLevel) {
  out.write_escaped(value ? "true" : "false");
}

/// Write a boolean value as JSON.
void writeJSONValue(toolchain::raw_ostream &out, uint32_t value, unsigned indentLevel) {
  out.write_escaped(std::to_string(value));
}

/// Write a JSON array.
template <typename T>
void writeJSONValue(toolchain::raw_ostream &out, ArrayRef<T> values,
                    unsigned indentLevel) {
  out << "[\n";

  for (const auto &value : values) {

    out.indent((indentLevel + 1) * 2);

    writeJSONValue(out, value, indentLevel + 1);

    if (&value != &values.back()) {
      out << ",";
    }
    out << "\n";
  }

  out.indent(indentLevel * 2);
  out << "]";
}

/// Write a JSON array.
template <typename T>
void writeJSONValue(toolchain::raw_ostream &out, const std::vector<T> &values,
                    unsigned indentLevel) {
  writeJSONValue(out, toolchain::ArrayRef(values), indentLevel);
}

/// Write a single JSON field.
template <typename T>
void writeJSONSingleField(toolchain::raw_ostream &out, StringRef fieldName,
                          const T &value, unsigned indentLevel,
                          bool trailingComma) {
  out.indent(indentLevel * 2);
  writeJSONValue(out, fieldName, indentLevel);
  out << ": ";
  auto updatedIndentLevel = indentLevel;

  writeJSONValue(out, value, updatedIndentLevel);

  if (trailingComma)
    out << ",";
  out << "\n";
}

void writeEncodedModuleIdJSONValue(toolchain::raw_ostream &out,
                                   languagescan_string_ref_t value,
                                   unsigned indentLevel) {
  out << "{\n";
  static const std::string textualPrefix("languageTextual");
  static const std::string binaryPrefix("languageBinary");
  static const std::string clangPrefix("clang");
  std::string valueStr = get_C_string(value);
  std::string moduleKind;
  std::string moduleName;
  if (!valueStr.compare(0, textualPrefix.size(), textualPrefix)) {
    moduleKind = "language";
    moduleName = valueStr.substr(textualPrefix.size() + 1);
  } else if (!valueStr.compare(0, binaryPrefix.size(), binaryPrefix)) {
    // FIXME: rename to be consistent in the clients (language-driver)
    moduleKind = "languagePrebuiltExternal";
    moduleName = valueStr.substr(binaryPrefix.size() + 1);
  } else {
    moduleKind = "clang";
    moduleName = valueStr.substr(clangPrefix.size() + 1);
  }
  writeJSONSingleField(out, moduleKind, moduleName, indentLevel + 1,
                       /*trailingComma=*/false);
  out.indent(indentLevel * 2);
  out << "}";
}


static void writeDependencies(toolchain::raw_ostream &out,
                              const languagescan_string_set_t *dependencies,
                              std::string dependenciesKind,
                              unsigned indentLevel, bool trailingComma) {
  out.indent(indentLevel * 2);
  out << "\"" + dependenciesKind + "\": ";
  out << "[\n";

  for (size_t i = 0; i < dependencies->count; ++i) {
    out.indent((indentLevel + 1) * 2);
    writeEncodedModuleIdJSONValue(out, dependencies->strings[i],
                                  indentLevel + 1);
    if (i != dependencies->count - 1) {
      out << ",";
    }
    out << "\n";
  }

  out.indent(indentLevel * 2);
  out << "]";

  if (trailingComma)
    out << ",";
  out << "\n";
}

void writeLinkLibraries(toolchain::raw_ostream &out,
                        const languagescan_link_library_set_t *link_libraries,
                        unsigned indentLevel, bool trailingComma) {
  out.indent(indentLevel * 2);
  out << "\"linkLibraries\": ";
  out << "[\n";

  for (size_t i = 0; i < link_libraries->count; ++i) {
    const auto &llInfo = *link_libraries->link_libraries[i];
    out.indent((indentLevel + 1) * 2);
    out << "{\n";
    auto entryIndentLevel = ((indentLevel + 2) * 2);
    out.indent(entryIndentLevel);
    out << "\"linkName\": ";
    writeJSONValue(out, llInfo.name, indentLevel);
    out << ",\n";
    out.indent(entryIndentLevel);
    out << "\"isStatic\": ";
    writeJSONValue(out, llInfo.isStatic, entryIndentLevel);
    out << ",\n";
    out.indent(entryIndentLevel);
    out << "\"isFramework\": ";
    writeJSONValue(out, llInfo.isFramework, entryIndentLevel);
    out << ",\n";
    out.indent(entryIndentLevel);
    out << "\"shouldForceLoad\": ";
    writeJSONValue(out, llInfo.forceLoad, entryIndentLevel);
    out << "\n";
    out.indent((indentLevel + 1) * 2);
    out << "}";
    if (i != link_libraries->count - 1) {
      out << ",";
    }
    out << "\n";
  }

  out.indent(indentLevel * 2);
  out << "]";

  if (trailingComma)
    out << ",";
  out << "\n";
}

void writeImportInfos(toolchain::raw_ostream &out,
                      const languagescan_import_info_set_t *imports,
                      unsigned indentLevel, bool trailingComma) {
  out.indent(indentLevel * 2);
  out << "\"imports\": ";
  out << "[\n";

  for (size_t i = 0; i < imports->count; ++i) {
    const auto &iInfo = *imports->imports[i];
    out.indent((indentLevel + 1) * 2);
    out << "{\n";
    auto entryIndentLevel = ((indentLevel + 2) * 2);
    out.indent(entryIndentLevel);
    out << "\"identifier\": ";
    writeJSONValue(out, iInfo.import_identifier, indentLevel);
    out << ",\n";
    out.indent(entryIndentLevel);
    out << "\"accessLevel\": ";
    switch (iInfo.access_level) {
      case LANGUAGESCAN_ACCESS_LEVEL_PRIVATE:
        writeJSONValue(out, StringRef("private"), entryIndentLevel);
        break;
      case LANGUAGESCAN_ACCESS_LEVEL_FILEPRIVATE:
        writeJSONValue(out, StringRef("fileprivate"), entryIndentLevel);
        break;
      case LANGUAGESCAN_ACCESS_LEVEL_INTERNAL:
        writeJSONValue(out, StringRef("internal"), entryIndentLevel);
        break;
      case LANGUAGESCAN_ACCESS_LEVEL_PACKAGE:
        writeJSONValue(out, StringRef("package"), entryIndentLevel);
        break;
      case LANGUAGESCAN_ACCESS_LEVEL_PUBLIC:
        writeJSONValue(out, StringRef("public"), entryIndentLevel);
        break;
    }

    if (iInfo.source_locations->count) {
      out << ",\n";
      out.indent(entryIndentLevel);
      out << "\"importLocations\": ";
      out << "[\n";
      auto slIndentLevel = ((entryIndentLevel + 4));
      for (size_t i = 0; i < iInfo.source_locations->count; ++i) {
        out.indent(entryIndentLevel + 2);
        out << "{\n";
        const auto &sl = *iInfo.source_locations->source_locations[i];
        out.indent(slIndentLevel);
        out << "\"bufferIdentifier\": ";
        writeJSONValue(out, sl.buffer_identifier, indentLevel);
        out << ",\n";
        out.indent(slIndentLevel);
        out << "\"linuNumber\": ";
        writeJSONValue(out, sl.line_number, indentLevel);
        out << ",\n";
        out.indent(slIndentLevel);
        out << "\"columnNumber\": ";
        writeJSONValue(out, sl.column_number, indentLevel);
        out << "\n";
        out.indent(entryIndentLevel + 2);
        out << "}";
        if (i != iInfo.source_locations->count - 1)
          out << ",";
        out << "\n";
      }
      out.indent(entryIndentLevel);
      out << "]\n";
    } else {
      out << "\n";
    }
    out.indent((indentLevel + 1) * 2);
    out << "}";
    if (i != imports->count - 1)
      out << ",";
    out << "\n";
  }

  out.indent(indentLevel * 2);
  out << "]";

  if (trailingComma)
    out << ",";
  out << "\n";
}

static void
writeMacroDependencies(toolchain::raw_ostream &out,
                       const languagescan_macro_dependency_set_t *macro_deps,
                       unsigned indentLevel, bool trailingComma) {
  if (!macro_deps)
    return;

  out.indent(indentLevel * 2);
  out << "\"macroDependencies\": ";
  out << "[\n";
  for (size_t i = 0; i < macro_deps->count; ++i) {
    const auto &macroInfo = *macro_deps->macro_dependencies[i];
    out.indent((indentLevel + 1) * 2);
    out << "{\n";
    auto entryIndentLevel = ((indentLevel + 2) * 2);
    out.indent(entryIndentLevel);
    out << "\"moduleName\": ";
    writeJSONValue(out, macroInfo.module_name, indentLevel);
    out << ",\n";
    out.indent(entryIndentLevel);
    out << "\"libraryPath\": ";
    writeJSONValue(out, macroInfo.library_path, entryIndentLevel);
    out << ",\n";
    out.indent(entryIndentLevel);
    out << "\"executablePath\": ";
    writeJSONValue(out, macroInfo.executable_path, entryIndentLevel);
    out << "\n";
    out.indent((indentLevel + 1) * 2);
    out << "}";
    if (i != macro_deps->count - 1) {
      out << ",";
    }
    out << "\n";
  }

  out.indent(indentLevel * 2);
  out << "]";

  if (trailingComma)
    out << ",";
  out << "\n";
}

static const languagescan_language_textual_details_t *
getAsTextualDependencyModule(languagescan_module_details_t details) {
  if (details->kind == LANGUAGESCAN_DEPENDENCY_INFO_LANGUAGE_TEXTUAL)
    return &details->language_textual_details;
  return nullptr;
}

static const languagescan_language_binary_details_t *
getAsBinaryDependencyModule(languagescan_module_details_t details) {
  if (details->kind == LANGUAGESCAN_DEPENDENCY_INFO_LANGUAGE_BINARY)
    return &details->language_binary_details;
  return nullptr;
}

static const languagescan_clang_details_t *
getAsClangDependencyModule(languagescan_module_details_t details) {
  if (details->kind == LANGUAGESCAN_DEPENDENCY_INFO_CLANG)
    return &details->clang_details;
  return nullptr;
}

namespace language::dependencies {
void writePrescanJSON(toolchain::raw_ostream &out,
                      languagescan_import_set_t importSet) {
  // Write out a JSON containing all main module imports.
  out << "{\n";
  LANGUAGE_DEFER { out << "}\n"; };

  writeJSONSingleField(out, "imports", importSet->imports, 0, false);
}

void writeJSON(toolchain::raw_ostream &out,
               languagescan_dependency_graph_t fullDependencies) {
  // Write out a JSON description of all of the dependencies.
  out << "{\n";
  LANGUAGE_DEFER { out << "}\n"; };
  // Name of the main module.
  writeJSONSingleField(out, "mainModuleName",
                       fullDependencies->main_module_name,
                       /*indentLevel=*/1, /*trailingComma=*/true);
  // Write out all of the modules.
  out << "  \"modules\": [\n";
  LANGUAGE_DEFER { out << "  ]\n"; };
  const auto module_set = fullDependencies->dependencies;
  for (size_t mi = 0; mi < module_set->count; ++mi) {
    const auto &moduleInfo = *module_set->modules[mi];
    auto &directDependencies = moduleInfo.direct_dependencies;
    // The module we are describing.
    out.indent(2 * 2);
    writeEncodedModuleIdJSONValue(out, moduleInfo.module_name, 2);
    out << ",\n";
    out.indent(2 * 2);
    out << "{\n";
    auto languageTextualDeps = getAsTextualDependencyModule(moduleInfo.details);
    auto languageBinaryDeps = getAsBinaryDependencyModule(moduleInfo.details);
    auto clangDeps = getAsClangDependencyModule(moduleInfo.details);

    // Module path.
    const char *modulePathSuffix = clangDeps ? ".pcm" : ".codemodule";

    std::string modulePath;
    std::string moduleKindAndName =
        std::string(get_C_string(moduleInfo.module_name));
    std::string moduleName =
        moduleKindAndName.substr(moduleKindAndName.find(":") + 1);
    if (languageBinaryDeps)
      modulePath = get_C_string(languageBinaryDeps->compiled_module_path);
    else if (clangDeps || languageTextualDeps)
      modulePath = get_C_string(moduleInfo.module_path);
    else
      modulePath = moduleName + modulePathSuffix;

    writeJSONSingleField(out, "modulePath", modulePath, /*indentLevel=*/3,
                         /*trailingComma=*/true);

    // Source files.
    if (languageTextualDeps || clangDeps) {
      writeJSONSingleField(out, "sourceFiles", moduleInfo.source_files, 3,
                           /*trailingComma=*/true);
    }

    // Direct dependencies.
    if (languageTextualDeps || languageBinaryDeps || clangDeps) {
      writeDependencies(out, directDependencies,
                        "directDependencies", 3,
                        /*trailingComma=*/true);
      writeLinkLibraries(out, moduleInfo.link_libraries,
                         3, /*trailingComma=*/true);
      writeImportInfos(out, moduleInfo.imports,
                       3, /*trailingComma=*/true);
    }
    // Codira and Clang-specific details.
    out.indent(3 * 2);
    out << "\"details\": {\n";
    out.indent(4 * 2);
    if (languageTextualDeps) {
      out << "\"language\": {\n";
      /// Codira interface file, if there is one. The main module, for
      /// example, will not have an interface file.
      std::string moduleInterfacePath =
          languageTextualDeps->module_interface_path.data
              ? get_C_string(languageTextualDeps->module_interface_path)
              : "";
      if (!moduleInterfacePath.empty()) {
        writeJSONSingleField(out, "moduleInterfacePath", moduleInterfacePath, 5,
                             /*trailingComma=*/true);
        out.indent(5 * 2);
        out << "\"compiledModuleCandidates\": [\n";
        for (int i = 0,
                 count = languageTextualDeps->compiled_module_candidates->count;
             i < count; ++i) {
          const auto &candidate = get_C_string(
              languageTextualDeps->compiled_module_candidates->strings[i]);
          out.indent(6 * 2);
          out << "\"" << quote(candidate) << "\"";
          if (i != count - 1)
            out << ",";
          out << "\n";
        }
        out.indent(5 * 2);
        out << "],\n";
      }
      out.indent(5 * 2);
      out << "\"commandLine\": [\n";
      for (int i = 0, count = languageTextualDeps->command_line->count; i < count;
           ++i) {
        const auto &arg =
            get_C_string(languageTextualDeps->command_line->strings[i]);
        out.indent(6 * 2);
        out << "\"" << quote(arg) << "\"";
        if (i != count - 1)
          out << ",";
        out << "\n";
      }
      out.indent(5 * 2);
      out << "],\n";
      writeJSONSingleField(out, "contextHash", languageTextualDeps->context_hash,
                           5,
                           /*trailingComma=*/true);
      bool hasBridgingHeader =
          (languageTextualDeps->bridging_header_path.data &&
           get_C_string(languageTextualDeps->bridging_header_path)[0] != '\0') ||
          languageTextualDeps->bridging_pch_command_line->count != 0;
      bool hasOverlayDependencies =
          languageTextualDeps->language_overlay_module_dependencies &&
          languageTextualDeps->language_overlay_module_dependencies->count > 0;
      bool hasSourceImportedDependencies =
          languageTextualDeps->source_import_module_dependencies &&
          languageTextualDeps->source_import_module_dependencies->count > 0;
      bool commaAfterBridgingHeaderPath = hasOverlayDependencies;
      bool commaAfterFramework =
          hasBridgingHeader || commaAfterBridgingHeaderPath;

      if (languageTextualDeps->cas_fs_root_id.length != 0) {
        writeJSONSingleField(out, "casFSRootID",
                             languageTextualDeps->cas_fs_root_id, 5,
                             /*trailingComma=*/true);
      }
      if (languageTextualDeps->module_cache_key.length != 0) {
        writeJSONSingleField(out, "moduleCacheKey",
                             languageTextualDeps->module_cache_key, 5,
                             /*trailingComma=*/true);
      }
      if (languageTextualDeps->chained_bridging_header_path.length != 0) {
        writeJSONSingleField(out, "chainedBridgingHeaderPath",
                             languageTextualDeps->chained_bridging_header_path, 5,
                             /*trailingComma=*/true);
      }
      writeJSONSingleField(out, "userModuleVersion",
                           languageTextualDeps->user_module_version, 5,
                           /*trailingComma=*/true);
      writeMacroDependencies(out, languageTextualDeps->macro_dependencies, 5,
                             /*trailingComma=*/true);
      if (hasSourceImportedDependencies) {
        writeDependencies(out, languageTextualDeps->source_import_module_dependencies,
                          "sourceImportedDependencies", 5,
                          /*trailingComma=*/true);
      }
      writeJSONSingleField(out, "isFramework", languageTextualDeps->is_framework,
                           5, commaAfterFramework);
      /// Bridging header and its source file dependencies, if any.
      if (hasBridgingHeader) {
        out.indent(5 * 2);
        out << "\"bridgingHeader\": {\n";
        writeJSONSingleField(out, "path",
                             languageTextualDeps->bridging_header_path, 6,
                             /*trailingComma=*/true);
        writeJSONSingleField(out, "sourceFiles",
                             languageTextualDeps->bridging_source_files, 6,
                             /*trailingComma=*/true);
        if (languageTextualDeps->bridging_header_include_tree.length != 0) {
          writeJSONSingleField(out, "includeTree",
                               languageTextualDeps->bridging_header_include_tree,
                               6, /*trailingComma=*/true);
        }
        writeJSONSingleField(out, "moduleDependencies",
                             languageTextualDeps->bridging_module_dependencies, 6,
                             /*trailingComma=*/true);
        out.indent(6 * 2);
        out << "\"commandLine\": [\n";
        for (int i = 0,
                 count = languageTextualDeps->bridging_pch_command_line->count;
             i < count; ++i) {
          const auto &arg = get_C_string(
              languageTextualDeps->bridging_pch_command_line->strings[i]);
          out.indent(7 * 2);
          out << "\"" << quote(arg) << "\"";
          if (i != count - 1)
            out << ",";
          out << "\n";
        }
        out.indent(6 * 2);
        out << "]\n";
        out.indent(5 * 2);
        out << (commaAfterBridgingHeaderPath ? "},\n" : "}\n");
      }
      if (hasOverlayDependencies) {
        writeDependencies(out, languageTextualDeps->language_overlay_module_dependencies,
                          "languageOverlayDependencies", 5,
                          /*trailingComma=*/false);
      }
    } else if (languageBinaryDeps) {
      bool hasOverlayDependencies =
        languageBinaryDeps->language_overlay_module_dependencies &&
        languageBinaryDeps->language_overlay_module_dependencies->count > 0;

      out << "\"languagePrebuiltExternal\": {\n";
      assert(languageBinaryDeps->compiled_module_path.data &&
             get_C_string(languageBinaryDeps->compiled_module_path)[0] != '\0' &&
             "Expected .codemodule for a Binary Codira Module Dependency.");

      writeJSONSingleField(out, "compiledModulePath",
                           languageBinaryDeps->compiled_module_path,
                           /*indentLevel=*/5,
                           /*trailingComma=*/true);
      writeJSONSingleField(out, "userModuleVersion",
                           languageBinaryDeps->user_module_version,
                           /*indentLevel=*/5,
                           /*trailingComma=*/true);
      // Module doc file
      if (languageBinaryDeps->module_doc_path.data &&
          get_C_string(languageBinaryDeps->module_doc_path)[0] != '\0')
        writeJSONSingleField(out, "moduleDocPath",
                             languageBinaryDeps->module_doc_path,
                             /*indentLevel=*/5,
                             /*trailingComma=*/true);
      // Module Source Info file
      if (languageBinaryDeps->module_source_info_path.data &&
          get_C_string(languageBinaryDeps->module_source_info_path)[0] != '\0')
        writeJSONSingleField(out, "moduleSourceInfoPath",
                             languageBinaryDeps->module_source_info_path,
                             /*indentLevel=*/5,
                             /*trailingComma=*/true);
      if (languageBinaryDeps->module_cache_key.length != 0) {
        writeJSONSingleField(out, "moduleCacheKey",
                             languageBinaryDeps->module_cache_key, 5,
                             /*trailingComma=*/true);
      }

      // Module Header Dependencies
      if (languageBinaryDeps->header_dependency.length != 0)
        writeJSONSingleField(out, "headerDependency",
                             languageBinaryDeps->header_dependency, 5,
                             /*trailingComma=*/true);

      // Module Header Module Dependencies
      if (languageBinaryDeps->header_dependencies_module_dependnecies->count != 0)
        writeJSONSingleField(out, "headerModuleDependencies",
                             languageBinaryDeps->header_dependencies_module_dependnecies, 5,
                             /*trailingComma=*/true);

      // Module Header Source Files
      if (languageBinaryDeps->header_dependencies_source_files->count != 0)
        writeJSONSingleField(out, "headerDependenciesSourceFiles",
                             languageBinaryDeps->header_dependencies_source_files, 5,
                             /*trailingComma=*/true);

      if (hasOverlayDependencies) {
        writeDependencies(out, languageBinaryDeps->language_overlay_module_dependencies,
                          "languageOverlayDependencies", 5,
                          /*trailingComma=*/true);
      }

      writeMacroDependencies(out, languageBinaryDeps->macro_dependencies, 5,
                             /*trailingComma=*/true);
      writeJSONSingleField(out, "isFramework", languageBinaryDeps->is_framework,
                           5, /*trailingComma=*/false);
    } else {
      out << "\"clang\": {\n";

      // Module map file.
      writeJSONSingleField(out, "moduleMapPath", clangDeps->module_map_path, 5,
                           /*trailingComma=*/true);

      // Context hash.
      writeJSONSingleField(out, "contextHash", clangDeps->context_hash, 5,
                           /*trailingComma=*/true);

      // Command line.
      writeJSONSingleField(out, "commandLine", clangDeps->command_line, 5,
                           clangDeps->cas_fs_root_id.length != 0 ||
                           clangDeps->clang_include_tree.length != 0 ||
                           clangDeps->module_cache_key.length != 0);

      if (clangDeps->cas_fs_root_id.length != 0)
        writeJSONSingleField(out, "casFSRootID", clangDeps->cas_fs_root_id, 5,
                             clangDeps->clang_include_tree.length != 0 ||
                             clangDeps->module_cache_key.length != 0);
      if (clangDeps->clang_include_tree.length != 0)
        writeJSONSingleField(out, "clangIncludeTree",
                             clangDeps->clang_include_tree, 5,
                             clangDeps->module_cache_key.length != 0);
      if (clangDeps->module_cache_key.length != 0)
        writeJSONSingleField(out, "moduleCacheKey", clangDeps->module_cache_key,
                             5,
                             /*trailingComma=*/false);
    }

    out.indent(4 * 2);
    out << "}\n";
    out.indent(3 * 2);
    out << "}\n";
    out.indent(2 * 2);
    out << "}";

    if (mi != module_set->count - 1)
      out << ",";
    out << "\n";
  }
}
} // namespace language::dependencies
