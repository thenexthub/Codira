//===- CompilerInvocation.h - Compiler Invocation Helper Data ---*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_FRONTEND_COMPILERINVOCATION_H
#define LANGUAGE_CORE_FRONTEND_COMPILERINVOCATION_H

#include "language/Core/APINotes/APINotesOptions.h"
#include "language/Core/Basic/CodeGenOptions.h"
#include "language/Core/Basic/DiagnosticOptions.h"
#include "language/Core/Basic/FileSystemOptions.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/LangOptions.h"
#include "language/Core/Basic/LangStandard.h"
#include "language/Core/Frontend/DependencyOutputOptions.h"
#include "language/Core/Frontend/FrontendOptions.h"
#include "language/Core/Frontend/MigratorOptions.h"
#include "language/Core/Frontend/PreprocessorOutputOptions.h"
#include "language/Core/StaticAnalyzer/Core/AnalyzerOptions.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/ArrayRef.h"
#include <memory>
#include <string>

namespace toolchain {

class Triple;

namespace opt {

class ArgList;

} // namespace opt

namespace vfs {

class FileSystem;

} // namespace vfs

} // namespace toolchain

namespace language::Core {

class DiagnosticsEngine;
class HeaderSearchOptions;
class PreprocessorOptions;
class TargetOptions;

// This lets us create the DiagnosticsEngine with a properly-filled-out
// DiagnosticOptions instance.
std::unique_ptr<DiagnosticOptions>
CreateAndPopulateDiagOpts(ArrayRef<const char *> Argv);

/// Fill out Opts based on the options given in Args.
///
/// Args must have been created from the OptTable returned by
/// createCC1OptTable().
///
/// When errors are encountered, return false and, if Diags is non-null,
/// report the error(s).
bool ParseDiagnosticArgs(DiagnosticOptions &Opts, toolchain::opt::ArgList &Args,
                         DiagnosticsEngine *Diags = nullptr,
                         bool DefaultDiagColor = true);

/// The base class of CompilerInvocation. It keeps individual option objects
/// behind reference-counted pointers, which is useful for clients that want to
/// keep select option objects alive (even after CompilerInvocation gets
/// destroyed) without making a copy.
class CompilerInvocationBase {
protected:
  /// Options controlling the language variant.
  std::shared_ptr<LangOptions> LangOpts;

  /// Options controlling the target.
  std::shared_ptr<TargetOptions> TargetOpts;

  /// Options controlling the diagnostic engine.
  std::shared_ptr<DiagnosticOptions> DiagnosticOpts;

  /// Options controlling the \#include directive.
  std::shared_ptr<HeaderSearchOptions> HSOpts;

  /// Options controlling the preprocessor (aside from \#include handling).
  std::shared_ptr<PreprocessorOptions> PPOpts;

  /// Options controlling the static analyzer.
  std::shared_ptr<AnalyzerOptions> AnalyzerOpts;

  std::shared_ptr<MigratorOptions> MigratorOpts;

  /// Options controlling API notes.
  std::shared_ptr<APINotesOptions> APINotesOpts;

  /// Options controlling IRgen and the backend.
  std::shared_ptr<CodeGenOptions> CodeGenOpts;

  /// Options controlling file system operations.
  std::shared_ptr<FileSystemOptions> FSOpts;

  /// Options controlling the frontend itself.
  std::shared_ptr<FrontendOptions> FrontendOpts;

  /// Options controlling dependency output.
  std::shared_ptr<DependencyOutputOptions> DependencyOutputOpts;

  /// Options controlling preprocessed output.
  std::shared_ptr<PreprocessorOutputOptions> PreprocessorOutputOpts;

  /// Dummy tag type whose instance can be passed into the constructor to
  /// prevent creation of the reference-counted option objects.
  struct EmptyConstructor {};

  CompilerInvocationBase();
  CompilerInvocationBase(EmptyConstructor) {}
  CompilerInvocationBase(const CompilerInvocationBase &X) = delete;
  CompilerInvocationBase(CompilerInvocationBase &&X) = default;
  CompilerInvocationBase &operator=(const CompilerInvocationBase &X) = delete;
  CompilerInvocationBase &deep_copy_assign(const CompilerInvocationBase &X);
  CompilerInvocationBase &shallow_copy_assign(const CompilerInvocationBase &X);
  CompilerInvocationBase &operator=(CompilerInvocationBase &&X) = default;
  ~CompilerInvocationBase() = default;

public:
  /// Const getters.
  /// @{
  const LangOptions &getLangOpts() const { return *LangOpts; }
  const TargetOptions &getTargetOpts() const { return *TargetOpts; }
  const DiagnosticOptions &getDiagnosticOpts() const { return *DiagnosticOpts; }
  const HeaderSearchOptions &getHeaderSearchOpts() const { return *HSOpts; }
  const PreprocessorOptions &getPreprocessorOpts() const { return *PPOpts; }
  const AnalyzerOptions &getAnalyzerOpts() const { return *AnalyzerOpts; }
  const MigratorOptions &getMigratorOpts() const { return *MigratorOpts; }
  const APINotesOptions &getAPINotesOpts() const { return *APINotesOpts; }
  const CodeGenOptions &getCodeGenOpts() const { return *CodeGenOpts; }
  const FileSystemOptions &getFileSystemOpts() const { return *FSOpts; }
  const FrontendOptions &getFrontendOpts() const { return *FrontendOpts; }
  const DependencyOutputOptions &getDependencyOutputOpts() const {
    return *DependencyOutputOpts;
  }
  const PreprocessorOutputOptions &getPreprocessorOutputOpts() const {
    return *PreprocessorOutputOpts;
  }
  /// @}

  /// Command line generation.
  /// @{
  using StringAllocator = toolchain::function_ref<const char *(const Twine &)>;
  /// Generate cc1-compatible command line arguments from this instance.
  ///
  /// \param [out] Args - The generated arguments. Note that the caller is
  /// responsible for inserting the path to the clang executable and "-cc1" if
  /// desired.
  /// \param SA - A function that given a Twine can allocate storage for a given
  /// command line argument and return a pointer to the newly allocated string.
  /// The returned pointer is what gets appended to Args.
  void generateCC1CommandLine(toolchain::SmallVectorImpl<const char *> &Args,
                              StringAllocator SA) const {
    generateCC1CommandLine([&](const Twine &Arg) {
      // No need to allocate static string literals.
      Args.push_back(Arg.isSingleStringLiteral()
                         ? Arg.getSingleStringRef().data()
                         : SA(Arg));
    });
  }

  using ArgumentConsumer = toolchain::function_ref<void(const Twine &)>;
  /// Generate cc1-compatible command line arguments from this instance.
  ///
  /// \param Consumer - Callback that gets invoked for every single generated
  /// command line argument.
  void generateCC1CommandLine(ArgumentConsumer Consumer) const;

  /// Generate cc1-compatible command line arguments from this instance,
  /// wrapping the result as a std::vector<std::string>.
  ///
  /// This is a (less-efficient) wrapper over generateCC1CommandLine().
  std::vector<std::string> getCC1CommandLine() const;

private:
  /// Generate command line options from DiagnosticOptions.
  static void GenerateDiagnosticArgs(const DiagnosticOptions &Opts,
                                     ArgumentConsumer Consumer,
                                     bool DefaultDiagColor);

  /// Generate command line options from LangOptions.
  static void GenerateLangArgs(const LangOptions &Opts,
                               ArgumentConsumer Consumer, const toolchain::Triple &T,
                               InputKind IK);

  // Generate command line options from CodeGenOptions.
  static void GenerateCodeGenArgs(const CodeGenOptions &Opts,
                                  ArgumentConsumer Consumer,
                                  const toolchain::Triple &T,
                                  const std::string &OutputFile,
                                  const LangOptions *LangOpts);
  /// @}
};

class CowCompilerInvocation;

/// Helper class for holding the data necessary to invoke the compiler.
///
/// This class is designed to represent an abstract "invocation" of the
/// compiler, including data such as the include paths, the code generation
/// options, the warning flags, and so on.
class CompilerInvocation : public CompilerInvocationBase {
public:
  CompilerInvocation() = default;
  CompilerInvocation(const CompilerInvocation &X)
      : CompilerInvocationBase(EmptyConstructor{}) {
    deep_copy_assign(X);
  }
  CompilerInvocation(CompilerInvocation &&) = default;
  CompilerInvocation &operator=(const CompilerInvocation &X) {
    deep_copy_assign(X);
    return *this;
  }
  ~CompilerInvocation() = default;

  explicit CompilerInvocation(const CowCompilerInvocation &X);
  CompilerInvocation &operator=(const CowCompilerInvocation &X);

  /// Const getters.
  /// @{
  // Note: These need to be pulled in manually. Otherwise, they get hidden by
  // the mutable getters with the same names.
  using CompilerInvocationBase::getLangOpts;
  using CompilerInvocationBase::getTargetOpts;
  using CompilerInvocationBase::getDiagnosticOpts;
  using CompilerInvocationBase::getHeaderSearchOpts;
  using CompilerInvocationBase::getPreprocessorOpts;
  using CompilerInvocationBase::getAnalyzerOpts;
  using CompilerInvocationBase::getMigratorOpts;
  using CompilerInvocationBase::getAPINotesOpts;
  using CompilerInvocationBase::getCodeGenOpts;
  using CompilerInvocationBase::getFileSystemOpts;
  using CompilerInvocationBase::getFrontendOpts;
  using CompilerInvocationBase::getDependencyOutputOpts;
  using CompilerInvocationBase::getPreprocessorOutputOpts;
  /// @}

  /// Mutable getters.
  /// @{
  LangOptions &getLangOpts() { return *LangOpts; }
  TargetOptions &getTargetOpts() { return *TargetOpts; }
  DiagnosticOptions &getDiagnosticOpts() { return *DiagnosticOpts; }
  HeaderSearchOptions &getHeaderSearchOpts() { return *HSOpts; }
  PreprocessorOptions &getPreprocessorOpts() { return *PPOpts; }
  AnalyzerOptions &getAnalyzerOpts() { return *AnalyzerOpts; }
  MigratorOptions &getMigratorOpts() { return *MigratorOpts; }
  APINotesOptions &getAPINotesOpts() { return *APINotesOpts; }
  CodeGenOptions &getCodeGenOpts() { return *CodeGenOpts; }
  FileSystemOptions &getFileSystemOpts() { return *FSOpts; }
  FrontendOptions &getFrontendOpts() { return *FrontendOpts; }
  DependencyOutputOptions &getDependencyOutputOpts() {
    return *DependencyOutputOpts;
  }
  PreprocessorOutputOptions &getPreprocessorOutputOpts() {
    return *PreprocessorOutputOpts;
  }
  /// @}

  /// Create a compiler invocation from a list of input options.
  /// \returns true on success.
  ///
  /// \returns false if an error was encountered while parsing the arguments
  /// and attempts to recover and continue parsing the rest of the arguments.
  /// The recovery is best-effort and only guarantees that \p Res will end up in
  /// one of the vaild-to-access (albeit arbitrary) states.
  ///
  /// \param [out] Res - The resulting invocation.
  /// \param [in] CommandLineArgs - Array of argument strings, this must not
  /// contain "-cc1".
  static bool CreateFromArgs(CompilerInvocation &Res,
                             ArrayRef<const char *> CommandLineArgs,
                             DiagnosticsEngine &Diags,
                             const char *Argv0 = nullptr);

  /// Get the directory where the compiler headers
  /// reside, relative to the compiler binary (found by the passed in
  /// arguments).
  ///
  /// \param Argv0 - The program path (from argv[0]), for finding the builtin
  /// compiler path.
  /// \param MainAddr - The address of main (or some other function in the main
  /// executable), for finding the builtin compiler path.
  static std::string GetResourcesPath(const char *Argv0, void *MainAddr);

  /// Populate \p Opts with the default set of pointer authentication-related
  /// options given \p LangOpts and \p Triple.
  ///
  /// Note: This is intended to be used by tools which must be aware of
  /// pointer authentication-related code generation, e.g. lldb.
  static void setDefaultPointerAuthOptions(PointerAuthOptions &Opts,
                                           const LangOptions &LangOpts,
                                           const toolchain::Triple &Triple);

  /// Retrieve a module hash string that is suitable for uniquely
  /// identifying the conditions under which the module was built.
  std::string getModuleHash() const;

  /// Check that \p Args can be parsed and re-serialized without change,
  /// emiting diagnostics for any differences.
  ///
  /// This check is only suitable for command-lines that are expected to already
  /// be canonical.
  ///
  /// \return false if there are any errors.
  static bool checkCC1RoundTrip(ArrayRef<const char *> Args,
                                DiagnosticsEngine &Diags,
                                const char *Argv0 = nullptr);

  /// Reset all of the options that are not considered when building a
  /// module.
  void resetNonModularOptions();

  /// Disable implicit modules and canonicalize options that are only used by
  /// implicit modules.
  void clearImplicitModuleBuildOptions();

private:
  static bool CreateFromArgsImpl(CompilerInvocation &Res,
                                 ArrayRef<const char *> CommandLineArgs,
                                 DiagnosticsEngine &Diags, const char *Argv0);

  /// Parse command line options that map to LangOptions.
  static bool ParseLangArgs(LangOptions &Opts, toolchain::opt::ArgList &Args,
                            InputKind IK, const toolchain::Triple &T,
                            std::vector<std::string> &Includes,
                            DiagnosticsEngine &Diags);

  /// Parse command line options that map to CodeGenOptions.
  static bool ParseCodeGenArgs(CodeGenOptions &Opts, toolchain::opt::ArgList &Args,
                               InputKind IK, DiagnosticsEngine &Diags,
                               const toolchain::Triple &T,
                               const std::string &OutputFile,
                               const LangOptions &LangOptsRef);
};

/// Same as \c CompilerInvocation, but with copy-on-write optimization.
class CowCompilerInvocation : public CompilerInvocationBase {
public:
  CowCompilerInvocation() = default;
  CowCompilerInvocation(const CowCompilerInvocation &X)
      : CompilerInvocationBase(EmptyConstructor{}) {
    shallow_copy_assign(X);
  }
  CowCompilerInvocation(CowCompilerInvocation &&) = default;
  CowCompilerInvocation &operator=(const CowCompilerInvocation &X) {
    shallow_copy_assign(X);
    return *this;
  }
  ~CowCompilerInvocation() = default;

  CowCompilerInvocation(const CompilerInvocation &X)
      : CompilerInvocationBase(EmptyConstructor{}) {
    deep_copy_assign(X);
  }

  CowCompilerInvocation(CompilerInvocation &&X)
      : CompilerInvocationBase(std::move(X)) {}

  // Const getters are inherited from the base class.

  /// Mutable getters.
  /// @{
  LangOptions &getMutLangOpts();
  TargetOptions &getMutTargetOpts();
  DiagnosticOptions &getMutDiagnosticOpts();
  HeaderSearchOptions &getMutHeaderSearchOpts();
  PreprocessorOptions &getMutPreprocessorOpts();
  AnalyzerOptions &getMutAnalyzerOpts();
  MigratorOptions &getMutMigratorOpts();
  APINotesOptions &getMutAPINotesOpts();
  CodeGenOptions &getMutCodeGenOpts();
  FileSystemOptions &getMutFileSystemOpts();
  FrontendOptions &getMutFrontendOpts();
  DependencyOutputOptions &getMutDependencyOutputOpts();
  PreprocessorOutputOptions &getMutPreprocessorOutputOpts();
  /// @}
};

IntrusiveRefCntPtr<toolchain::vfs::FileSystem>
createVFSFromCompilerInvocation(const CompilerInvocation &CI,
                                DiagnosticsEngine &Diags);

IntrusiveRefCntPtr<toolchain::vfs::FileSystem> createVFSFromCompilerInvocation(
    const CompilerInvocation &CI, DiagnosticsEngine &Diags,
    IntrusiveRefCntPtr<toolchain::vfs::FileSystem> BaseFS);

IntrusiveRefCntPtr<toolchain::vfs::FileSystem>
createVFSFromOverlayFiles(ArrayRef<std::string> VFSOverlayFiles,
                          DiagnosticsEngine &Diags,
                          IntrusiveRefCntPtr<toolchain::vfs::FileSystem> BaseFS);

} // namespace language::Core

#endif // LANGUAGE_CORE_FRONTEND_COMPILERINVOCATION_H
