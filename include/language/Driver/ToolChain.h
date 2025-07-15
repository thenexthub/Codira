//===--- ToolChain.h - Collections of tools for one platform ----*- C++ -*-===//
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

#ifndef LANGUAGE_DRIVER_TOOLCHAIN_H
#define LANGUAGE_DRIVER_TOOLCHAIN_H

#include "language/Basic/FileTypes.h"
#include "language/Basic/Toolchain.h"
#include "language/Driver/Action.h"
#include "language/Driver/Job.h"
#include "language/Option/Options.h"
#include "toolchain/TargetParser/Triple.h"
#include "toolchain/Option/Option.h"

#include <memory>

namespace language {
class DiagnosticEngine;

namespace driver {
class CommandOutput;
class Compilation;
class Driver;
class Job;
class OutputInfo;

/// A ToolChain is responsible for turning abstract Actions into concrete,
/// runnable Jobs.
///
/// The primary purpose of a ToolChain is built around the
/// \c constructInvocation family of methods. This is a set of callbacks
/// following the Visitor pattern for the various JobAction subclasses, which
/// returns an executable name and arguments for the Job to be run. The base
/// ToolChain knows how to perform most operations, but some (like linking)
/// require platform-specific knowledge, provided in subclasses.
class ToolChain {
  const Driver &D;
  const toolchain::Triple Triple;
  mutable toolchain::StringMap<std::string> ProgramLookupCache;

protected:
  ToolChain(const Driver &D, const toolchain::Triple &T) : D(D), Triple(T) {}

  /// A special name used to identify the Codira executable itself.
  constexpr static const char *const LANGUAGE_EXECUTABLE_NAME = "language";

  /// Packs together the supplementary information about the job being created.
  class JobContext {
  private:
    Compilation &C;

  public:
    ArrayRef<const Job *> Inputs;
    ArrayRef<const Action *> InputActions;
    const CommandOutput &Output;
    const OutputInfo &OI;

    /// The arguments to the driver. Can also be used to create new strings with
    /// the same lifetime.
    ///
    /// This just caches C.getArgs().
    const toolchain::opt::ArgList &Args;

  public:
    JobContext(Compilation &C, ArrayRef<const Job *> Inputs,
               ArrayRef<const Action *> InputActions,
               const CommandOutput &Output, const OutputInfo &OI);

    /// Forwards to Compilation::getInputFiles.
    ArrayRef<InputPair> getTopLevelInputFiles() const;

    /// Forwards to Compilation::getAllSourcesPath.
    const char *getAllSourcesPath() const;

    /// Creates a new temporary file for use by a job.
    ///
    /// The returned string already has its lifetime extended to match other
    /// arguments.
    const char *getTemporaryFilePath(const toolchain::Twine &name,
                                     StringRef suffix = "") const;

    /// For frontend, merge-module, and link invocations.
    bool shouldUseInputFileList() const;

    bool shouldUsePrimaryInputFileListInFrontendInvocation() const;

    bool shouldUseMainOutputFileListInFrontendInvocation() const;

    bool shouldUseSupplementaryOutputFileMapInFrontendInvocation() const;

    /// Reify the existing behavior that SingleCompile compile actions do not
    /// filter, single-file compilations do. Some clients are
    /// relying on this (i.e., they pass inputs that don't have ".code" as an
    /// extension.) It would be nice to eliminate this distinction someday.
    bool shouldFilterFrontendInputsByType() const;

    const char *computeFrontendModeForCompile() const;

    void addFrontendInputAndOutputArguments(
        toolchain::opt::ArgStringList &Arguments,
        std::vector<FilelistInfo> &FilelistInfos) const;

  private:
    void addFrontendCommandLineInputArguments(
        bool mayHavePrimaryInputs, bool useFileList, bool usePrimaryFileList,
        bool filterByType, toolchain::opt::ArgStringList &arguments) const;
    void addFrontendSupplementaryOutputArguments(
        toolchain::opt::ArgStringList &arguments) const;
  };

  /// Packs together information chosen by toolchains to create jobs.
  struct InvocationInfo {
    const char *ExecutableName;
    toolchain::opt::ArgStringList Arguments;
    std::vector<std::pair<const char *, const char *>> ExtraEnvironment;
    std::vector<FilelistInfo> FilelistInfos;

    // Not all platforms and jobs support the use of response files, so assume
    // "false" by default. If the executable specified in the InvocationInfo
    // constructor supports response files, this can be overridden and set to
    // "true".
    bool allowsResponseFiles = false;

    InvocationInfo(const char *name, toolchain::opt::ArgStringList args = {},
                   decltype(ExtraEnvironment) extraEnv = {})
        : ExecutableName(name), Arguments(std::move(args)),
          ExtraEnvironment(std::move(extraEnv)) {}
  };

  /// Handle arguments common to all invocations of the frontend (compilation,
  /// module-merging, LLDB's REPL, etc).
  virtual void addCommonFrontendArgs(const OutputInfo &OI,
                                     const CommandOutput &output,
                                     const toolchain::opt::ArgList &inputArgs,
                                     toolchain::opt::ArgStringList &arguments) const;

  virtual void addPlatformSpecificPluginFrontendArgs(
      const OutputInfo &OI,
      const CommandOutput &output,
      const toolchain::opt::ArgList &inputArgs,
      toolchain::opt::ArgStringList &arguments) const;
  virtual InvocationInfo constructInvocation(const CompileJobAction &job,
                                             const JobContext &context) const;
  virtual InvocationInfo constructInvocation(const InterpretJobAction &job,
                                             const JobContext &context) const;
  virtual InvocationInfo constructInvocation(const BackendJobAction &job,
                                             const JobContext &context) const;
  virtual InvocationInfo constructInvocation(const MergeModuleJobAction &job,
                                             const JobContext &context) const;
  virtual InvocationInfo constructInvocation(const ModuleWrapJobAction &job,
                                             const JobContext &context) const;

  virtual InvocationInfo constructInvocation(const REPLJobAction &job,
                                             const JobContext &context) const;

  virtual InvocationInfo constructInvocation(const GenerateDSYMJobAction &job,
                                             const JobContext &context) const;
  virtual InvocationInfo
  constructInvocation(const VerifyDebugInfoJobAction &job,
                      const JobContext &context) const;
  virtual InvocationInfo
  constructInvocation(const VerifyModuleInterfaceJobAction &job,
                      const JobContext &context) const;
  virtual InvocationInfo constructInvocation(const GeneratePCHJobAction &job,
                                             const JobContext &context) const;
  virtual InvocationInfo
  constructInvocation(const AutolinkExtractJobAction &job,
                      const JobContext &context) const;
  virtual InvocationInfo constructInvocation(const DynamicLinkJobAction &job,
                                             const JobContext &context) const;

  virtual InvocationInfo constructInvocation(const StaticLinkJobAction &job,
                                             const JobContext &context) const;

  /// Searches for the given executable in appropriate paths relative to the
  /// Codira binary.
  ///
  /// This method caches its results.
  ///
  /// \sa findProgramRelativeToCodiraImpl
  std::string findProgramRelativeToCodira(StringRef name) const;

  /// An override point for platform-specific subclasses to customize how to
  /// do relative searches for programs.
  ///
  /// This method is invoked by findProgramRelativeToCodira().
  virtual std::string findProgramRelativeToCodiraImpl(StringRef name) const;

  void addInputsOfType(toolchain::opt::ArgStringList &Arguments,
                       ArrayRef<const Action *> Inputs,
                       file_types::ID InputType,
                       const char *PrefixArgument = nullptr) const;

  void addInputsOfType(toolchain::opt::ArgStringList &Arguments,
                       ArrayRef<const Job *> Jobs,
                       const toolchain::opt::ArgList &Args, file_types::ID InputType,
                       const char *PrefixArgument = nullptr) const;

  void addPrimaryInputsOfType(toolchain::opt::ArgStringList &Arguments,
                              ArrayRef<const Job *> Jobs,
                              const toolchain::opt::ArgList &Args,
                              file_types::ID InputType,
                              const char *PrefixArgument = nullptr) const;

  /// Get the resource dir link path, which is platform-specific and found
  /// relative to the compiler.
  void getResourceDirPath(SmallVectorImpl<char> &runtimeLibPath,
                          const toolchain::opt::ArgList &args, bool shared) const;

  /// Get the secondary runtime library link path given the primary path.
  void getSecondaryResourceDirPath(
      SmallVectorImpl<char> &secondaryResourceDirPath,
      StringRef primaryPath) const;

  /// Get the runtime library link paths, which typically include the resource
  /// dir path and the SDK.
  void getRuntimeLibraryPaths(SmallVectorImpl<std::string> &runtimeLibPaths,
                              const toolchain::opt::ArgList &args,
                              StringRef SDKPath, bool shared) const;

  void addPathEnvironmentVariableIfNeeded(Job::EnvironmentVector &env,
                                          const char *name,
                                          const char *separator,
                                          options::ID optionID,
                                          const toolchain::opt::ArgList &args,
                                          ArrayRef<std::string> extraEntries = {}) const;

  /// Specific toolchains should override this to provide additional conditions
  /// under which the compiler invocation should be written into debug info. For
  /// example, Darwin does this if the RC_DEBUG_OPTIONS environment variable is
  /// set to match the behavior of Clang.
  virtual bool shouldStoreInvocationInDebugInfo() const { return false; }

  /// Specific toolchains should override this to provide additional
  /// -debug-prefix-map entries. For example, Darwin has an RC_DEBUG_PREFIX_MAP
  /// environment variable that is also understood by Clang.
  virtual std::string getGlobalDebugPathRemapping() const { return {}; }
  
  /// Gets the response file path and command line argument for an invocation
  /// if the tool supports response files and if the command line length would
  /// exceed system limits.
  std::optional<Job::ResponseFileInfo>
  getResponseFileInfo(const Compilation &C, const char *executablePath,
                      const InvocationInfo &invocationInfo,
                      const JobContext &context) const;

  void addPluginArguments(const toolchain::opt::ArgList &Args,
                          toolchain::opt::ArgStringList &Arguments) const;

public:
  virtual ~ToolChain() = default;

  const Driver &getDriver() const { return D; }
  const toolchain::Triple &getTriple() const { return Triple; }

  /// Special handling for passing down '-l' arguments.
  ///
  /// Not all downstream tools (lldb, ld etc.) consistently accept
  /// a space between the '-l' flag and its argument, so we remove
  /// the extra space if it was present in \c Args.
  static void addLinkedLibArgs(const toolchain::opt::ArgList &Args,
                               toolchain::opt::ArgStringList &FrontendArgs);

  /// Construct a Job for the action \p JA, taking the given information into
  /// account.
  ///
  /// This method dispatches to the various \c constructInvocation methods,
  /// which may be overridden by platform-specific subclasses.
  std::unique_ptr<Job> constructJob(const JobAction &JA, Compilation &C,
                                    SmallVectorImpl<const Job *> &&inputs,
                                    ArrayRef<const Action *> inputActions,
                                    std::unique_ptr<CommandOutput> output,
                                    const OutputInfo &OI) const;

  /// Return the default language type to use for the given extension.
  /// If the extension is empty or is otherwise not recognized, return
  /// the invalid type \c TY_INVALID.
  file_types::ID lookupTypeForExtension(StringRef Ext) const;

  /// Copies the path for the directory clang libraries would be stored in on
  /// the current toolchain.
  void getClangLibraryPath(const toolchain::opt::ArgList &Args,
                           SmallString<128> &LibPath) const;

  // Returns the Clang driver executable to use for linking.
  const char *getClangLinkerDriver(const toolchain::opt::ArgList &Args) const;

  /// Returns the name the clang library for a given sanitizer would have on
  /// the current toolchain.
  ///
  /// \param Sanitizer Sanitizer name.
  /// \param shared Whether the library is shared
  virtual std::string sanitizerRuntimeLibName(StringRef Sanitizer,
                                              bool shared = true) const = 0;

  /// Returns whether a given sanitizer exists for the current toolchain.
  ///
  /// \param sanitizer Sanitizer name.
  /// \param shared Whether the library is shared
  bool sanitizerRuntimeLibExists(const toolchain::opt::ArgList &args,
                                 StringRef sanitizer, bool shared = true) const;

  /// Adds a runtime library to the arguments list for linking.
  ///
  /// \param LibName The library name
  /// \param Arguments The arguments list to append to
  void addLinkRuntimeLib(const toolchain::opt::ArgList &Args,
                         toolchain::opt::ArgStringList &Arguments,
                         StringRef LibName) const;

  /// Validates arguments passed to the toolchain.
  ///
  /// An override point for platform-specific subclasses to customize the
  /// validations that should be performed.
  virtual void validateArguments(DiagnosticEngine &diags,
                                 const toolchain::opt::ArgList &args,
                                 StringRef defaultTarget) const {}

  /// Validate the output information.
  ///
  /// An override point for platform-specific subclasses to customize their
  /// behavior once the outputs are known.
  virtual void validateOutputInfo(DiagnosticEngine &diags,
                                  const OutputInfo &outputInfo) const { }

  toolchain::Expected<file_types::ID>
  remarkFileTypeFromArgs(const toolchain::opt::ArgList &Args) const;
};
} // end namespace driver
} // end namespace language

#endif
