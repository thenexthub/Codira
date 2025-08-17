//===--- CrossTranslationUnit.h - -------------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
//  This file provides an interface to load binary AST dumps on demand. This
//  feature can be utilized for tools that require cross translation unit
//  support.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_CROSSTU_CROSSTRANSLATIONUNIT_H
#define LANGUAGE_CORE_CROSSTU_CROSSTRANSLATIONUNIT_H

#include "language/Core/AST/ASTImporterSharedState.h"
#include "language/Core/Analysis/MacroExpansionContext.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/Path.h"
#include <optional>

namespace language::Core {
class CompilerInstance;
class ASTContext;
class ASTImporter;
class ASTUnit;
class DeclContext;
class FunctionDecl;
class VarDecl;
class NamedDecl;
class TranslationUnitDecl;

namespace cross_tu {

enum class index_error_code {
  success = 0,
  unspecified = 1,
  missing_index_file,
  invalid_index_format,
  multiple_definitions,
  missing_definition,
  failed_import,
  failed_to_get_external_ast,
  failed_to_generate_usr,
  triple_mismatch,
  lang_mismatch,
  lang_dialect_mismatch,
  load_threshold_reached,
  invocation_list_ambiguous,
  invocation_list_file_not_found,
  invocation_list_empty,
  invocation_list_wrong_format,
  invocation_list_lookup_unsuccessful
};

class IndexError : public toolchain::ErrorInfo<IndexError> {
public:
  static char ID;
  IndexError(index_error_code C) : Code(C), LineNo(0) {}
  IndexError(index_error_code C, std::string FileName, int LineNo = 0)
      : Code(C), FileName(std::move(FileName)), LineNo(LineNo) {}
  IndexError(index_error_code C, std::string FileName, std::string TripleToName,
             std::string TripleFromName)
      : Code(C), FileName(std::move(FileName)),
        TripleToName(std::move(TripleToName)),
        TripleFromName(std::move(TripleFromName)) {}
  void log(raw_ostream &OS) const override;
  std::error_code convertToErrorCode() const override;
  index_error_code getCode() const { return Code; }
  int getLineNum() const { return LineNo; }
  std::string getFileName() const { return FileName; }
  std::string getTripleToName() const { return TripleToName; }
  std::string getTripleFromName() const { return TripleFromName; }

private:
  index_error_code Code;
  std::string FileName;
  int LineNo;
  std::string TripleToName;
  std::string TripleFromName;
};

/// This function parses an index file that determines which
/// translation unit contains which definition. The IndexPath is not prefixed
/// with CTUDir, so an absolute path is expected for consistent results.
///
/// The index file format is the following:
/// each line consists of an USR and a filepath separated by a space.
///
/// \return Returns a map where the USR is the key and the filepath is the value
///         or an error.
toolchain::Expected<toolchain::StringMap<std::string>>
parseCrossTUIndex(StringRef IndexPath);

std::string createCrossTUIndexString(const toolchain::StringMap<std::string> &Index);

using InvocationListTy = toolchain::StringMap<toolchain::SmallVector<std::string, 32>>;
/// Parse the YAML formatted invocation list file content \p FileContent.
/// The format is expected to be a mapping from absolute source file
/// paths in the filesystem to a list of command-line parts, which
/// constitute the invocation needed to compile that file. That invocation
/// will be used to produce the AST of the TU.
toolchain::Expected<InvocationListTy> parseInvocationList(
    StringRef FileContent,
    toolchain::sys::path::Style PathStyle = toolchain::sys::path::Style::posix);

/// Returns true if it makes sense to import a foreign variable definition.
/// For instance, we don't want to import variables that have non-trivial types
/// because the constructor might have side-effects.
bool shouldImport(const VarDecl *VD, const ASTContext &ACtx);

/// This class is used for tools that requires cross translation
///        unit capability.
///
/// This class can load definitions from external AST sources.
/// The loaded definition will be merged back to the original AST using the
/// AST Importer.
/// In order to use this class, an index file is required that describes
/// the locations of the AST files for each definition.
///
/// Note that this class also implements caching.
class CrossTranslationUnitContext {
public:
  CrossTranslationUnitContext(CompilerInstance &CI);
  ~CrossTranslationUnitContext();

  /// This function loads a function or variable definition from an
  ///        external AST file and merges it into the original AST.
  ///
  /// This method should only be used on functions that have no definitions or
  /// variables that have no initializer in
  /// the current translation unit. A function definition with the same
  /// declaration will be looked up in the index file which should be in the
  /// \p CrossTUDir directory, called \p IndexName. In case the declaration is
  /// found in the index the corresponding AST will be loaded and the
  /// definition will be merged into the original AST using the AST Importer.
  ///
  /// \return The declaration with the definition will be returned.
  /// If no suitable definition is found in the index file or multiple
  /// definitions found error will be returned.
  ///
  /// Note that the AST files should also be in the \p CrossTUDir.
  toolchain::Expected<const FunctionDecl *>
  getCrossTUDefinition(const FunctionDecl *FD, StringRef CrossTUDir,
                       StringRef IndexName, bool DisplayCTUProgress = false);
  toolchain::Expected<const VarDecl *>
  getCrossTUDefinition(const VarDecl *VD, StringRef CrossTUDir,
                       StringRef IndexName, bool DisplayCTUProgress = false);

  /// This function loads a definition from an external AST file.
  ///
  /// A definition with the same declaration will be looked up in the
  /// index file which should be in the \p CrossTUDir directory, called
  /// \p IndexName. In case the declaration is found in the index the
  /// corresponding AST will be loaded. If the number of TUs imported
  /// reaches \p CTULoadTreshold, no loading is performed.
  ///
  /// \return Returns a pointer to the ASTUnit that contains the definition of
  /// the looked up name or an Error.
  /// The returned pointer is never a nullptr.
  ///
  /// Note that the AST files should also be in the \p CrossTUDir.
  toolchain::Expected<ASTUnit *> loadExternalAST(StringRef LookupName,
                                            StringRef CrossTUDir,
                                            StringRef IndexName,
                                            bool DisplayCTUProgress = false);

  /// This function merges a definition from a separate AST Unit into
  ///        the current one which was created by the compiler instance that
  ///        was passed to the constructor.
  ///
  /// \return Returns the resulting definition or an error.
  toolchain::Expected<const FunctionDecl *> importDefinition(const FunctionDecl *FD,
                                                        ASTUnit *Unit);
  toolchain::Expected<const VarDecl *> importDefinition(const VarDecl *VD,
                                                   ASTUnit *Unit);

  /// Get a name to identify a named decl.
  static std::optional<std::string> getLookupName(const NamedDecl *ND);

  /// Emit diagnostics for the user for potential configuration errors.
  void emitCrossTUDiagnostics(const IndexError &IE);

  /// Returns the MacroExpansionContext for the imported TU to which the given
  /// source-location corresponds.
  /// \p ToLoc Source location in the imported-to AST.
  /// \note If any error happens such as \p ToLoc is a non-imported
  ///       source-location, empty is returned.
  /// \note Macro expansion tracking for imported TUs is not implemented yet.
  ///       It returns empty unconditionally.
  std::optional<language::Core::MacroExpansionContext>
  getMacroExpansionContextForSourceLocation(
      const language::Core::SourceLocation &ToLoc) const;

  /// Returns true if the given Decl is newly created during the import.
  bool isImportedAsNew(const Decl *ToDecl) const;

  /// Returns true if the given Decl is mapped (or created) during an import
  /// but there was an unrecoverable error (the AST node cannot be erased, it
  /// is marked with an Error object in this case).
  bool hasError(const Decl *ToDecl) const;

private:
  void lazyInitImporterSharedSt(TranslationUnitDecl *ToTU);
  ASTImporter &getOrCreateASTImporter(ASTUnit *Unit);
  template <typename T>
  toolchain::Expected<const T *> getCrossTUDefinitionImpl(const T *D,
                                                     StringRef CrossTUDir,
                                                     StringRef IndexName,
                                                     bool DisplayCTUProgress);
  template <typename T>
  const T *findDefInDeclContext(const DeclContext *DC,
                                StringRef LookupName);
  template <typename T>
  toolchain::Expected<const T *> importDefinitionImpl(const T *D, ASTUnit *Unit);

  using ImporterMapTy =
      toolchain::DenseMap<TranslationUnitDecl *, std::unique_ptr<ASTImporter>>;

  ImporterMapTy ASTUnitImporterMap;

  ASTContext &Context;
  std::shared_ptr<ASTImporterSharedState> ImporterSharedSt;

  using LoadResultTy = toolchain::Expected<std::unique_ptr<ASTUnit>>;

  /// Loads ASTUnits from AST-dumps or source-files.
  class ASTLoader {
  public:
    ASTLoader(CompilerInstance &CI, StringRef CTUDir,
              StringRef InvocationListFilePath);

    /// Load the ASTUnit by its identifier found in the index file. If the
    /// identifier is suffixed with '.ast' it is considered a dump. Otherwise
    /// it is treated as source-file, and on-demand parsed. Relative paths are
    /// prefixed with CTUDir.
    LoadResultTy load(StringRef Identifier);

    /// Lazily initialize the invocation list information, which is needed for
    /// on-demand parsing.
    toolchain::Error lazyInitInvocationList();

  private:
    /// The style used for storage and lookup of filesystem paths.
    /// Defaults to posix.
    const toolchain::sys::path::Style PathStyle = toolchain::sys::path::Style::posix;

    /// Loads an AST from a pch-dump.
    LoadResultTy loadFromDump(StringRef Identifier);
    /// Loads an AST from a source-file.
    LoadResultTy loadFromSource(StringRef Identifier);

    CompilerInstance &CI;
    StringRef CTUDir;
    /// The path to the file containing the invocation list, which is in YAML
    /// format, and contains a mapping from source files to compiler invocations
    /// that produce the AST used for analysis.
    StringRef InvocationListFilePath;
    /// In case of on-demand parsing, the invocations for parsing the source
    /// files is stored.
    std::optional<InvocationListTy> InvocationList;
    index_error_code PreviousParsingResult = index_error_code::success;
  };

  /// Maintain number of AST loads and check for reaching the load limit.
  class ASTLoadGuard {
  public:
    ASTLoadGuard(unsigned Limit) : Limit(Limit) {}

    /// Indicates, whether a new load operation is permitted, it is within the
    /// threshold.
    operator bool() const { return Count < Limit; }

    /// Tell that a new AST was loaded successfully.
    void indicateLoadSuccess() { ++Count; }

  private:
    /// The number of ASTs actually imported.
    unsigned Count{0u};
    /// The limit (threshold) value for number of loaded ASTs.
    const unsigned Limit;
  };

  /// Storage and load of ASTUnits, cached access, and providing searchability
  /// are the concerns of ASTUnitStorage class.
  class ASTUnitStorage {
  public:
    ASTUnitStorage(CompilerInstance &CI);
    /// Loads an ASTUnit for a function.
    ///
    /// \param FunctionName USR name of the function.
    /// \param CrossTUDir Path to the directory used to store CTU related files.
    /// \param IndexName Name of the file inside \p CrossTUDir which maps
    /// function USR names to file paths. These files contain the corresponding
    /// AST-dumps.
    /// \param DisplayCTUProgress Display a message about loading new ASTs.
    ///
    /// \return An Expected instance which contains the ASTUnit pointer or the
    /// error occurred during the load.
    toolchain::Expected<ASTUnit *> getASTUnitForFunction(StringRef FunctionName,
                                                    StringRef CrossTUDir,
                                                    StringRef IndexName,
                                                    bool DisplayCTUProgress);
    /// Identifies the path of the file which can be used to load the ASTUnit
    /// for a given function.
    ///
    /// \param FunctionName USR name of the function.
    /// \param CrossTUDir Path to the directory used to store CTU related files.
    /// \param IndexName Name of the file inside \p CrossTUDir which maps
    /// function USR names to file paths. These files contain the corresponding
    /// AST-dumps.
    ///
    /// \return An Expected instance containing the filepath.
    toolchain::Expected<std::string> getFileForFunction(StringRef FunctionName,
                                                   StringRef CrossTUDir,
                                                   StringRef IndexName);

  private:
    toolchain::Error ensureCTUIndexLoaded(StringRef CrossTUDir, StringRef IndexName);
    toolchain::Expected<ASTUnit *> getASTUnitForFile(StringRef FileName,
                                                bool DisplayCTUProgress);

    template <typename... T> using BaseMapTy = toolchain::StringMap<T...>;
    using OwningMapTy = BaseMapTy<std::unique_ptr<language::Core::ASTUnit>>;
    using NonOwningMapTy = BaseMapTy<language::Core::ASTUnit *>;

    OwningMapTy FileASTUnitMap;
    NonOwningMapTy NameASTUnitMap;

    using IndexMapTy = BaseMapTy<std::string>;
    IndexMapTy NameFileMap;

    /// Loads the AST based on the identifier found in the index.
    ASTLoader Loader;

    /// Limit the number of loaded ASTs. It is used to limit the  memory usage
    /// of the CrossTranslationUnitContext. The ASTUnitStorage has the
    /// information whether the AST to load is actually loaded or returned from
    /// cache. This information is needed to maintain the counter.
    ASTLoadGuard LoadGuard;
  };

  ASTUnitStorage ASTStorage;
};

} // namespace cross_tu
} // namespace language::Core

#endif // LANGUAGE_CORE_CROSSTU_CROSSTRANSLATIONUNIT_H
