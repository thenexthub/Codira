//===--- SerializedModuleLoader.cpp - Import Codira modules ----------------===//
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

#include "language/Serialization/SerializedModuleLoader.h"
#include "ModuleFile.h"
#include "ModuleFileSharedCore.h"
#include "language/AST/ASTContext.h"
#include "language/AST/DiagnosticsSema.h"
#include "language/AST/ImportCache.h"
#include "language/AST/Module.h"
#include "language/AST/ModuleDependencies.h"
#include "language/AST/PluginLoader.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/Basic/FileTypes.h"
#include "language/Basic/Platform.h"
#include "language/Basic/STLExtras.h"
#include "language/Basic/SourceManager.h"
#include "language/Basic/Version.h"
#include "language/Frontend/ModuleInterfaceLoader.h"
#include "language/Option/Options.h"
#include "language/Serialization/Validation.h"

#include "toolchain/Option/OptTable.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringSet.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/VersionTuple.h"
#include "toolchain/TargetParser/Host.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/CommandLine.h"
#include <memory>
#include <optional>
#include <system_error>

using namespace language;
using language::version::Version;

namespace {

/// Apply \c body for each target-specific module file base name to search from
/// most to least desirable.
void forEachTargetModuleBasename(const ASTContext &Ctx,
                                 toolchain::function_ref<void(StringRef)> body) {
  auto normalizedTarget = getTargetSpecificModuleTriple(Ctx.LangOpts.Target);

  // An arm64 module can import an arm64e module.
  std::optional<toolchain::Triple> normalizedAltTarget;
  if ((normalizedTarget.getArch() == toolchain::Triple::ArchType::aarch64) &&
      (normalizedTarget.getSubArch() !=
       toolchain::Triple::SubArchType::AArch64SubArch_arm64e)) {
    auto altTarget = normalizedTarget;
    altTarget.setArchName("arm64e");
    normalizedAltTarget = getTargetSpecificModuleTriple(altTarget);
  }

  body(normalizedTarget.str());

  if (normalizedAltTarget) {
    body(normalizedAltTarget->str());
  }

  // We used the un-normalized architecture as a target-specific
  // module name. Fall back to that behavior.
  body(Ctx.LangOpts.Target.getArchName());

  // FIXME: We used to use "major architecture" names for these files---the
  // names checked in "#if arch(...)". Fall back to that name in the one case
  // where it's different from what Codira 4.2 supported:
  // - 32-bit ARM platforms (formerly "arm")
  // We should be able to drop this once there's an Xcode that supports the
  // new names.
  if (Ctx.LangOpts.Target.getArch() == toolchain::Triple::ArchType::arm) {
    body("arm");
  }

  if (normalizedAltTarget) {
    body(normalizedAltTarget->getArchName());
  }
}

/// Apply \p body for each module search path in \p Ctx until \p body returns
/// non-None value. Returns the return value from \p body, or \c None.
std::optional<bool> forEachModuleSearchPath(
    const ASTContext &Ctx,
    toolchain::function_ref<std::optional<bool>(StringRef, ModuleSearchPathKind,
                                           bool isSystem)>
        callback) {
  for (const auto &path : Ctx.SearchPathOpts.getImportSearchPaths())
    if (auto result =
            callback(path.Path, ModuleSearchPathKind::Import, path.IsSystem))
      return result;

  for (const auto &path : Ctx.SearchPathOpts.getFrameworkSearchPaths())
    if (auto result =
            callback(path.Path, ModuleSearchPathKind::Framework, path.IsSystem))
      return result;

  for (const auto &path :
       Ctx.SearchPathOpts.getImplicitFrameworkSearchPaths()) {
    if (auto result = callback(path, ModuleSearchPathKind::ImplicitFramework,
                               /*isSystem=*/true))
      return result;
  }

  for (const auto &importPath :
       Ctx.SearchPathOpts.getRuntimeLibraryImportPaths()) {
    if (auto result = callback(importPath, ModuleSearchPathKind::RuntimeLibrary,
                               /*isSystem=*/true))
      return result;
  }

  return std::nullopt;
}
} // end unnamed namespace

// Defined out-of-line so that we can see ~ModuleFile.
SerializedModuleLoaderBase::SerializedModuleLoaderBase(
    ASTContext &ctx, DependencyTracker *tracker, ModuleLoadingMode loadMode,
    bool IgnoreCodiraSourceInfoFile)
    : ModuleLoader(tracker), Ctx(ctx), LoadMode(loadMode),
      IgnoreCodiraSourceInfoFile(IgnoreCodiraSourceInfoFile) {}

SerializedModuleLoaderBase::~SerializedModuleLoaderBase() = default;
ImplicitSerializedModuleLoader::~ImplicitSerializedModuleLoader() = default;
MemoryBufferSerializedModuleLoader::~MemoryBufferSerializedModuleLoader() =
    default;

void SerializedModuleLoaderBase::collectVisibleTopLevelModuleNamesImpl(
    SmallVectorImpl<Identifier> &names, StringRef extension) const {
  toolchain::SmallString<16> moduleSuffix;
  moduleSuffix += '.';
  moduleSuffix += file_types::getExtension(file_types::TY_CodiraModuleFile);

  toolchain::SmallString<16> suffix;
  suffix += '.';
  suffix += extension;

  SmallVector<SmallString<64>, 2> targetFiles;
  forEachTargetModuleBasename(Ctx, [&](StringRef targetName) {
    targetFiles.emplace_back(targetName);
    targetFiles.back() += suffix;
  });

  auto &fs = *Ctx.SourceMgr.getFileSystem();

  // Apply \p body for each directory entry in \p dirPath.
  auto forEachDirectoryEntryPath =
      [&](StringRef dirPath, toolchain::function_ref<void(StringRef)> body) {
        std::error_code errorCode;
        toolchain::vfs::directory_iterator DI = fs.dir_begin(dirPath, errorCode);
        toolchain::vfs::directory_iterator End;
        for (; !errorCode && DI != End; DI.increment(errorCode))
          body(DI->path());
      };

  // Check whether target specific module file exists or not in given directory.
  // $PATH/{arch}.{extension}
  auto checkTargetFiles = [&](StringRef path) -> bool {
    toolchain::SmallString<256> scratch;
    for (auto targetFile : targetFiles) {
      scratch.clear();
      toolchain::sys::path::append(scratch, path, targetFile);
      // If {arch}.{extension} exists, consider it's visible. Technically, we
      // should check the file type, permission, format, etc., but it's too
      // heavy to do that for each files.
      if (fs.exists(scratch))
        return true;
    }
    return false;
  };

  forEachModuleSearchPath(Ctx, [&](StringRef searchPath,
                                   ModuleSearchPathKind Kind, bool isSystem) {
    switch (Kind) {
    case ModuleSearchPathKind::Import: {
      // Look for:
      // $PATH/{name}.codemodule/{arch}.{extension} or
      // $PATH/{name}.{extension}
      forEachDirectoryEntryPath(searchPath, [&](StringRef path) {
        auto pathExt = toolchain::sys::path::extension(path);
        if (pathExt != moduleSuffix && pathExt != suffix)
          return;

        auto stat = fs.status(path);
        if (!stat)
          return;
        if (pathExt == moduleSuffix && stat->isDirectory()) {
          if (!checkTargetFiles(path))
            return;
        } else if (pathExt != suffix || stat->isDirectory()) {
          return;
        }
        // Extract module name.
        auto name = toolchain::sys::path::filename(path).drop_back(pathExt.size());
        names.push_back(Ctx.getIdentifier(name));
      });
      return std::nullopt;
    }
    case ModuleSearchPathKind::RuntimeLibrary: {
      // Look for:
      // (Darwin OS) $PATH/{name}.codemodule/{arch}.{extension}
      // (Other OS)  $PATH/{name}.{extension}
      bool requireTargetSpecificModule = Ctx.LangOpts.Target.isOSDarwin();
      forEachDirectoryEntryPath(searchPath, [&](StringRef path) {
        auto pathExt = toolchain::sys::path::extension(path);

        if (pathExt != moduleSuffix)
          if (requireTargetSpecificModule || pathExt != suffix)
            return;

        if (!checkTargetFiles(path)) {
          if (requireTargetSpecificModule)
            return;

          auto stat = fs.status(path);
          if (!stat || stat->isDirectory())
            return;
        }

        // Extract module name.
        auto name = toolchain::sys::path::filename(path).drop_back(pathExt.size());
        names.push_back(Ctx.getIdentifier(name));
      });
      return std::nullopt;
    }
    case ModuleSearchPathKind::Framework:
    case ModuleSearchPathKind::ImplicitFramework: {
      // Look for:
      // $PATH/{name}.framework/Modules/{name}.codemodule/{arch}.{extension}
      forEachDirectoryEntryPath(searchPath, [&](StringRef path) {
        if (toolchain::sys::path::extension(path) != ".framework")
          return;

        // Extract Framework name.
        auto name = toolchain::sys::path::filename(path).drop_back(
            StringLiteral(".framework").size());

        SmallString<256> moduleDir;
        toolchain::sys::path::append(moduleDir, path, "Modules",
                                name + moduleSuffix);
        if (!checkTargetFiles(moduleDir))
          return;

        names.push_back(Ctx.getIdentifier(name));
      });
      return std::nullopt;
    }
    }
    toolchain_unreachable("covered switch");
  });
}

void ImplicitSerializedModuleLoader::collectVisibleTopLevelModuleNames(
    SmallVectorImpl<Identifier> &names) const {
  collectVisibleTopLevelModuleNamesImpl(
      names, file_types::getExtension(file_types::TY_CodiraModuleFile));
}

std::error_code SerializedModuleLoaderBase::openModuleDocFileIfPresent(
  ImportPath::Element ModuleID,
  const SerializedModuleBaseName &BaseName,
  std::unique_ptr<toolchain::MemoryBuffer> *ModuleDocBuffer) {

  if (!ModuleDocBuffer)
    return std::error_code();

  toolchain::vfs::FileSystem &FS = *Ctx.SourceMgr.getFileSystem();

  // Try to open the module documentation file.  If it does not exist, ignore
  // the error.  However, pass though all other errors.
  SmallString<256>
  ModuleDocPath{BaseName.getName(file_types::TY_CodiraModuleDocFile)};

  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> ModuleDocOrErr =
    FS.getBufferForFile(ModuleDocPath);
  if (ModuleDocOrErr) {
    *ModuleDocBuffer = std::move(*ModuleDocOrErr);
  } else if (ModuleDocOrErr.getError() !=
               std::errc::no_such_file_or_directory) {
    return ModuleDocOrErr.getError();
  }

  return std::error_code();
}

std::unique_ptr<toolchain::MemoryBuffer>
SerializedModuleLoaderBase::getModuleName(ASTContext &Ctx, StringRef modulePath,
                                          std::string &Name) {
  return ModuleFile::getModuleName(Ctx, modulePath, Name);
}

std::optional<std::string> SerializedModuleLoaderBase::invalidModuleReason(serialization::Status status) {
  using namespace serialization;
  switch (status) {
  case Status::FormatTooOld:
    return "compiled with an older version of the compiler";
  case Status::FormatTooNew:
    return "compiled with a newer version of the compiler";
  case Status::RevisionIncompatible:
    return "compiled with a different version of the compiler";
  case Status::ChannelIncompatible:
    return "compiled for a different distribution channel";
  case Status::NotInOSSA:
    return "module was not built with OSSA";
  case Status::MissingDependency:
    return "missing dependency";
  case Status::MissingUnderlyingModule:
    return "missing underlying module";
  case Status::CircularDependency:
    return "circular dependency";
  case Status::FailedToLoadBridgingHeader:
    return "failed to load bridging header";
  case Status::Malformed:
    return "malformed";
  case Status::MalformedDocumentation:
    return "malformed documentation";
  case Status::NameMismatch:
    return "name mismatch";
  case Status::TargetIncompatible:
    return "compiled for a different target platform";
  case Status::TargetTooNew:
    return "target platform newer than current platform";
  case Status::SDKMismatch:
    return "SDK does not match";
  case Status::Valid:
    return std::nullopt;
  }
  toolchain_unreachable("bad status");
}

toolchain::ErrorOr<std::vector<ScannerImportStatementInfo>>
SerializedModuleLoaderBase::getMatchingPackageOnlyImportsOfModule(
    Twine modulePath, bool isFramework, bool isRequiredOSSAModules,
    StringRef SDKName, StringRef packageName, toolchain::vfs::FileSystem *fileSystem,
    PathObfuscator &recoverer) {
  auto moduleBuf = fileSystem->getBufferForFile(modulePath);
  if (!moduleBuf)
    return moduleBuf.getError();

  std::vector<ScannerImportStatementInfo> importedModuleNames;
  // Load the module file without validation.
  std::shared_ptr<const ModuleFileSharedCore> loadedModuleFile;
  serialization::ValidationInfo loadInfo = ModuleFileSharedCore::load(
      "", "", std::move(moduleBuf.get()), nullptr, nullptr, isFramework,
      isRequiredOSSAModules, SDKName, recoverer, loadedModuleFile);

  if (loadedModuleFile->getModulePackageName() != packageName)
    return importedModuleNames;

  for (const auto &dependency : loadedModuleFile->getDependencies()) {
    if (dependency.isHeader())
      continue;
    if (!dependency.isPackageOnly())
      continue;

    // Find the top-level module name.
    auto modulePathStr = dependency.getPrettyPrintedPath();
    StringRef moduleName = modulePathStr;
    auto dotPos = moduleName.find('.');
    if (dotPos != std::string::npos)
      moduleName = moduleName.slice(0, dotPos);

    importedModuleNames.push_back({moduleName.str(), dependency.isExported(),
      dependency.isInternalOrBelow() ? AccessLevel::Internal : AccessLevel::Public});
  }

  return importedModuleNames;
}


std::error_code
SerializedModuleLoaderBase::openModuleSourceInfoFileIfPresent(
    ImportPath::Element ModuleID,
    const SerializedModuleBaseName &BaseName,
    std::unique_ptr<toolchain::MemoryBuffer> *ModuleSourceInfoBuffer) {
  if (IgnoreCodiraSourceInfoFile || !ModuleSourceInfoBuffer)
    return std::error_code();

  toolchain::vfs::FileSystem &FS = *Ctx.SourceMgr.getFileSystem();

  toolchain::SmallString<128>
  PathWithoutProjectDir{BaseName.getName(file_types::TY_CodiraSourceInfoFile)};

  toolchain::SmallString<128> PathWithProjectDir = PathWithoutProjectDir;

  // Insert "Project" before the filename in PathWithProjectDir.
  StringRef FileName = toolchain::sys::path::filename(PathWithoutProjectDir);
  toolchain::sys::path::remove_filename(PathWithProjectDir);
  toolchain::sys::path::append(PathWithProjectDir, "Project");
  toolchain::sys::path::append(PathWithProjectDir, FileName);

  // Try to open the module source info file from the "Project" directory.
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>>
  ModuleSourceInfoOrErr = FS.getBufferForFile(PathWithProjectDir);

  // If it does not exist, try to open the module source info file adjacent to
  // the .codemodule file.
  if (ModuleSourceInfoOrErr.getError() == std::errc::no_such_file_or_directory)
    ModuleSourceInfoOrErr = FS.getBufferForFile(PathWithoutProjectDir);

  // If we ended up with a different file system error, return it.
  if (ModuleSourceInfoOrErr)
    *ModuleSourceInfoBuffer = std::move(*ModuleSourceInfoOrErr);
  else if (ModuleSourceInfoOrErr.getError() !=
              std::errc::no_such_file_or_directory)
    return ModuleSourceInfoOrErr.getError();

  return std::error_code();
}

std::error_code SerializedModuleLoaderBase::openModuleFile(
    ImportPath::Element ModuleID, const SerializedModuleBaseName &BaseName,
    std::unique_ptr<toolchain::MemoryBuffer> *ModuleBuffer) {
  toolchain::vfs::FileSystem &FS = *Ctx.SourceMgr.getFileSystem();

  // Try to open the module file first.  If we fail, don't even look for the
  // module documentation file.
  SmallString<256> ModulePath{BaseName.getName(file_types::TY_CodiraModuleFile)};

  // If there's no buffer to load into, simply check for the existence of
  // the module file.
  if (!ModuleBuffer) {
    toolchain::ErrorOr<toolchain::vfs::Status> statResult = FS.status(ModulePath);
    if (!statResult)
      return statResult.getError();

    if (!statResult->exists())
      return std::make_error_code(std::errc::no_such_file_or_directory);

    // FIXME: toolchain::vfs::FileSystem doesn't give us information on whether or
    // not we can /read/ the file without actually trying to do so.
    return std::error_code();
  }

  // Actually load the file and error out if necessary.
  //
  // Use the default arguments except for IsVolatile that is set by the
  // frontend option -enable-volatile-modules. If set, we avoid the use of
  // mmap to workaround issues on NFS when the languagemodule file loaded changes
  // on disk while it's in use.
  //
  // In practice, a languagemodule file can chane when a client uses a
  // languagemodule file from a framework while the framework is recompiled and
  // installed over existing files. Or when many processes rebuild the same
  // module interface.
  //
  // We have seen these scenarios leading to deserialization errors that on
  // the surface look like memory corruption.
  //
  // rdar://63755989
  bool enableVolatileModules = Ctx.LangOpts.EnableVolatileModules;
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> ModuleOrErr =
      FS.getBufferForFile(ModulePath,
                          /*FileSize=*/-1,
                          /*RequiresNullTerminator=*/true,
                          /*IsVolatile=*/enableVolatileModules);
  if (!ModuleOrErr)
    return ModuleOrErr.getError();

  *ModuleBuffer = std::move(ModuleOrErr.get());
  return std::error_code();
}

toolchain::ErrorOr<SerializedModuleLoaderBase::BinaryModuleImports>
SerializedModuleLoaderBase::getImportsOfModule(
    const ModuleFileSharedCore &loadedModuleFile,
    ModuleLoadingBehavior transitiveBehavior, StringRef packageName,
    bool isTestableImport) {
  std::vector<ScannerImportStatementInfo> moduleImports;
  std::string importedHeader = "";
  for (const auto &dependency : loadedModuleFile.getDependencies()) {
    if (dependency.isHeader()) {
      assert(importedHeader.empty() &&
             "Unexpected more than one header dependency");
      importedHeader = dependency.RawPath;
      continue;
    }

    ModuleLoadingBehavior dependencyTransitiveBehavior =
        loadedModuleFile.getTransitiveLoadingBehavior(
            dependency,
            /*importPrivateDependencies*/ false,
            /*isPartialModule*/ false, packageName,
            /*resolveInPackageModuleDependencies */ true,
            isTestableImport);
    if (dependencyTransitiveBehavior > transitiveBehavior)
      continue;

    // Find the top-level module name.
    auto modulePathStr = dependency.getPrettyPrintedPath();
    StringRef moduleName = modulePathStr;
    auto dotPos = moduleName.find('.');
    if (dotPos != std::string::npos)
      moduleName = moduleName.slice(0, dotPos);

    // Reverse rewrite of user-specified C++ standard
    // library module name to one used in the modulemap.
    // TODO: If we are going to do this for more than this module,
    // we will need a centralized system for doing module import name remap.
    if (moduleName == Ctx.Id_CxxStdlib.str())
      moduleName = "std";

    moduleImports.push_back(ScannerImportStatementInfo(
        moduleName.str(), dependency.isExported(),
        dependency.isInternalOrBelow() ? AccessLevel::Internal
                                       : AccessLevel::Public));
  }

  return SerializedModuleLoaderBase::BinaryModuleImports{moduleImports,
                                                         importedHeader};
}

std::optional<MacroPluginDependency>
SerializedModuleLoaderBase::resolveMacroPlugin(const ExternalMacroPlugin &macro,
                                               StringRef packageName) {
  if (macro.MacroAccess == ExternalMacroPlugin::Access::Internal)
    return std::nullopt;

  if (macro.MacroAccess == ExternalMacroPlugin::Access::Package &&
      packageName != Ctx.LangOpts.PackageName)
    return std::nullopt;

  auto &loader = Ctx.getPluginLoader();
  auto &entry =
      loader.lookupPluginByModuleName(Ctx.getIdentifier(macro.ModuleName));
  if (entry.libraryPath.empty() && entry.executablePath.empty())
    return std::nullopt;

  return MacroPluginDependency{entry.libraryPath.str(),
                               entry.executablePath.str()};
}

std::error_code ImplicitSerializedModuleLoader::findModuleFilesInDirectory(
    ImportPath::Element ModuleID, const SerializedModuleBaseName &BaseName,
    SmallVectorImpl<char> *ModuleInterfacePath,
    SmallVectorImpl<char> *ModuleInterfaceSourcePath,
    std::unique_ptr<toolchain::MemoryBuffer> *ModuleBuffer,
    std::unique_ptr<toolchain::MemoryBuffer> *ModuleDocBuffer,
    std::unique_ptr<toolchain::MemoryBuffer> *ModuleSourceInfoBuffer,
    bool skipBuildingInterface, bool IsFramework, bool IsTestableDependencyLookup) {
  if (LoadMode == ModuleLoadingMode::OnlyInterface ||
      Ctx.IgnoreAdjacentModules)
    return std::make_error_code(std::errc::not_supported);

  auto ModuleErr = openModuleFile(ModuleID, BaseName, ModuleBuffer);
  if (ModuleErr)
    return ModuleErr;

  if (ModuleInterfaceSourcePath) {
    if (auto InterfacePath =
        BaseName.findInterfacePath(*Ctx.SourceMgr.getFileSystem(), Ctx))
      ModuleInterfaceSourcePath->assign(InterfacePath->begin(),
                                        InterfacePath->end());
  }

  if (auto ModuleSourceInfoError = openModuleSourceInfoFileIfPresent(
          ModuleID, BaseName, ModuleSourceInfoBuffer))
    return ModuleSourceInfoError;

  if (auto ModuleDocErr =
          openModuleDocFileIfPresent(ModuleID, BaseName, ModuleDocBuffer))
    return ModuleDocErr;

  return std::error_code();
}

bool ImplicitSerializedModuleLoader::maybeDiagnoseTargetMismatch(
    SourceLoc sourceLocation, StringRef moduleName,
    const SerializedModuleBaseName &absoluteBaseName) {
  toolchain::vfs::FileSystem &fs = *Ctx.SourceMgr.getFileSystem();

  // Get the last component of the base name, which is the target-specific one.
  auto target = toolchain::sys::path::filename(absoluteBaseName.baseName);

  // Strip off the last component to get the .codemodule folder.
  auto dir = absoluteBaseName.baseName;
  toolchain::sys::path::remove_filename(dir);

  std::error_code errorCode;
  std::string foundArchs;
  for (toolchain::vfs::directory_iterator directoryIterator =
           fs.dir_begin(dir, errorCode), endIterator;
       directoryIterator != endIterator;
       directoryIterator.increment(errorCode)) {
    if (errorCode)
      return false;
    StringRef filePath = directoryIterator->path();
    StringRef extension = toolchain::sys::path::extension(filePath);
    if (file_types::lookupTypeForExtension(extension) ==
          file_types::TY_CodiraModuleFile) {
      if (!foundArchs.empty())
        foundArchs += ", ";
      foundArchs += toolchain::sys::path::stem(filePath).str();
    }
  }

  if (foundArchs.empty()) {
    // Maybe this languagemodule directory only contains languageinterfaces, or
    // maybe something else is going on. Regardless, we shouldn't emit a
    // possibly incorrect diagnostic.
    return false;
  }

  Ctx.Diags.diagnose(sourceLocation, diag::sema_no_import_target, moduleName,
                     target, foundArchs, dir);
  return true;
}

SerializedModuleBaseName::SerializedModuleBaseName(
    StringRef parentDir, const SerializedModuleBaseName &name)
    : baseName(parentDir) {
  toolchain::sys::path::append(baseName, name.baseName);
}

std::string SerializedModuleBaseName::getName(file_types::ID fileTy) const {
  auto result = baseName;
  result += '.';
  result += file_types::getExtension(fileTy);

  return std::string(result.str());
}

std::optional<std::string>
SerializedModuleBaseName::getPackageInterfacePathIfInSamePackage(
    toolchain::vfs::FileSystem &fs, ASTContext &ctx) const {
  std::string packagePath{
      getName(file_types::TY_PackageCodiraModuleInterfaceFile)};

  if (fs.exists(packagePath)) {
    // Read the interface file and extract its package-name argument value
    if (auto packageName = getPackageNameFromInterface(packagePath, fs)) {
      // Return the .package.codeinterface path if the package name applies to
      // the importer module.
      if (*packageName == ctx.LangOpts.PackageName)
        return packagePath;
    }
  }
  return std::nullopt;
}

std::optional<std::string>
SerializedModuleBaseName::getPackageNameFromInterface(
    StringRef interfacePath, toolchain::vfs::FileSystem &fs) const {
  std::optional<std::string> result;
  if (auto interfaceFile = fs.getBufferForFile(interfacePath)) {
    toolchain::BumpPtrAllocator alloc;
    toolchain::StringSaver argSaver(alloc);
    SmallVector<const char *, 8> args;
    (void)extractCompilerFlagsFromInterface(
        interfacePath, (*interfaceFile)->getBuffer(), argSaver, args);
    for (unsigned I = 0, N = args.size(); I + 1 < N; I++) {
      StringRef current(args[I]), next(args[I + 1]);
      if (current == "-package-name") {
        // Instead of `break` here, continue to get the last value in case of
        // dupes, to be consistent with the default parsing logic.
        result = next;
      }
    }
  }
  return result;
}

std::optional<std::string>
SerializedModuleBaseName::findInterfacePath(toolchain::vfs::FileSystem &fs,
                                            ASTContext &ctx) const {
  std::string interfacePath{getName(file_types::TY_CodiraModuleInterfaceFile)};
  // Ensure the public languageinterface already exists, otherwise bail early
  // as it's considered the module doesn't exist.
  if (!fs.exists(interfacePath))
    return std::nullopt;

  // If both -package-name and -experimental-package-interface-load
  // are passed to the client, try to look for the package interface
  // to load; if either flag is missing, fall back to loading private
  // or public interface.
  if (!ctx.LangOpts.PackageName.empty() &&
      ctx.LangOpts.EnablePackageInterfaceLoad) {
    if (auto found =
            getPackageInterfacePathIfInSamePackage(fs, ctx))
      return *found;

    // If package interface is not found, check if we can load the
    // public/private interface file by checking:
    // * if AllowNonPackageInterfaceImportFromSamePackage is true
    // * if the package name is not equal so not in the same package.
    if (!ctx.LangOpts.AllowNonPackageInterfaceImportFromSamePackage) {
      if (auto packageName = getPackageNameFromInterface(interfacePath, fs)) {
        if (*packageName == ctx.LangOpts.PackageName)
          return std::nullopt;
      }
    }
  }

  // Otherwise, use the private interface instead of the public one.
  std::string privatePath{
      getName(file_types::TY_PrivateCodiraModuleInterfaceFile)};
  if (fs.exists(privatePath))
    return privatePath;

  // Otherwise return the public .codeinterface path
  return interfacePath;
}

bool SerializedModuleLoaderBase::findModule(
    ImportPath::Element moduleID, SmallVectorImpl<char> *moduleInterfacePath,
    SmallVectorImpl<char> *moduleInterfaceSourcePath,
    std::unique_ptr<toolchain::MemoryBuffer> *moduleBuffer,
    std::unique_ptr<toolchain::MemoryBuffer> *moduleDocBuffer,
    std::unique_ptr<toolchain::MemoryBuffer> *moduleSourceInfoBuffer,
    bool skipBuildingInterface, bool isTestableDependencyLookup,
    bool &isFramework, bool &isSystemModule) {
  // Find a module with an actual, physical name on disk, in case
  // -module-alias is used (otherwise same).
  //
  // For example, if '-module-alias Foo=Bar' is passed in to the frontend,
  // and a source file has 'import Foo', a module called Bar (real name)
  // should be searched.
  StringRef moduleNameRef = Ctx.getRealModuleName(moduleID.Item).str();
  SmallString<32> moduleName(moduleNameRef);
  SerializedModuleBaseName genericBaseName(moduleName);

  auto genericModuleFileName =
      genericBaseName.getName(file_types::TY_CodiraModuleFile);

  SmallVector<SerializedModuleBaseName, 4> targetSpecificBaseNames;
  forEachTargetModuleBasename(Ctx, [&](StringRef targetName) {
    // Construct a base name like ModuleName.codemodule/arch-vendor-os
    SmallString<64> targetBaseName{genericModuleFileName};
    toolchain::sys::path::append(targetBaseName, targetName);

    targetSpecificBaseNames.emplace_back(targetBaseName.str());
  });

  auto &fs = *Ctx.SourceMgr.getFileSystem();

  toolchain::SmallString<256> currPath;

  enum class SearchResult { Found, NotFound, Error };

  /// Returns true if a target-specific module file was found, false if an error
  /// was diagnosed, or None if neither one happened and the search should
  /// continue.
  auto findTargetSpecificModuleFiles = [&](bool IsFramework) -> SearchResult {
    std::optional<SerializedModuleBaseName> firstAbsoluteBaseName;

    for (const auto &targetSpecificBaseName : targetSpecificBaseNames) {
      SerializedModuleBaseName absoluteBaseName{currPath,
                                                targetSpecificBaseName};

      if (!firstAbsoluteBaseName.has_value())
        firstAbsoluteBaseName.emplace(absoluteBaseName);

      auto result = findModuleFilesInDirectory(
          moduleID, absoluteBaseName, moduleInterfacePath,
          moduleInterfaceSourcePath, moduleBuffer, moduleDocBuffer,
          moduleSourceInfoBuffer, skipBuildingInterface, IsFramework,
          isTestableDependencyLookup);
      if (!result)
        return SearchResult::Found;
      if (result == std::errc::not_supported)
        return SearchResult::Error;
      if (result != std::errc::no_such_file_or_directory)
        return SearchResult::NotFound;
    }

    // We can only get here if all targetFileNamePairs failed with
    // 'std::errc::no_such_file_or_directory'.
    if (firstAbsoluteBaseName &&
        maybeDiagnoseTargetMismatch(moduleID.Loc, moduleName,
                                    *firstAbsoluteBaseName))
      return SearchResult::Error;

    return SearchResult::NotFound;
  };

  SmallVector<std::string, 4> InterestingFilenames = {
      (moduleName + ".framework").str(),
      genericBaseName.getName(file_types::TY_CodiraModuleInterfaceFile),
      genericBaseName.getName(file_types::TY_PrivateCodiraModuleInterfaceFile),
      genericBaseName.getName(file_types::TY_PackageCodiraModuleInterfaceFile),
      genericBaseName.getName(file_types::TY_CodiraModuleFile)};

  auto searchPaths = Ctx.SearchPathOpts.moduleSearchPathsContainingFile(
      InterestingFilenames, Ctx.SourceMgr.getFileSystem().get(),
      Ctx.LangOpts.Target.isOSDarwin());
  for (const auto &searchPath : searchPaths) {
    currPath = searchPath->getPath();
    isSystemModule = searchPath->isSystem();

    switch (searchPath->getKind()) {
    case ModuleSearchPathKind::Import:
    case ModuleSearchPathKind::RuntimeLibrary: {
      isFramework = false;

      // On Apple platforms, we can assume that the runtime libraries use
      // target-specific module files within a `.codemodule` directory.
      // This was not always true on non-Apple platforms, and in order to
      // ease the transition, check both layouts.
      bool checkTargetSpecificModule = true;
      if (searchPath->getKind() != ModuleSearchPathKind::RuntimeLibrary ||
          !Ctx.LangOpts.Target.isOSDarwin()) {
        auto modulePath = currPath;
        toolchain::sys::path::append(modulePath, genericModuleFileName);
        toolchain::ErrorOr<toolchain::vfs::Status> statResult = fs.status(modulePath);

        // Even if stat fails, we can't just return the error; the path
        // we're looking for might not be "Foo.codemodule".
        checkTargetSpecificModule = statResult && statResult->isDirectory();
      }

      if (checkTargetSpecificModule) {
        // A .codemodule directory contains architecture-specific files.
        switch (findTargetSpecificModuleFiles(isFramework)) {
        case SearchResult::Found:
          return true;
        case SearchResult::NotFound:
          continue;
        case SearchResult::Error:
          return false;
        }
      }

      SerializedModuleBaseName absoluteBaseName{currPath, genericBaseName};

      auto result = findModuleFilesInDirectory(
          moduleID, absoluteBaseName, moduleInterfacePath,
          moduleInterfaceSourcePath, moduleBuffer, moduleDocBuffer,
          moduleSourceInfoBuffer, skipBuildingInterface, isFramework,
          isTestableDependencyLookup);
      if (!result)
        return true;
      if (result == std::errc::not_supported)
        return false;
      continue;
    }
    case ModuleSearchPathKind::Framework:
    case ModuleSearchPathKind::ImplicitFramework: {
      isFramework = true;
      toolchain::sys::path::append(currPath, moduleName + ".framework");

      // Check if the framework directory exists.
      if (!fs.exists(currPath)) {
        continue;
      }

      // Frameworks always use architecture-specific files within a
      // .codemodule directory.
      toolchain::sys::path::append(currPath, "Modules");
      switch (findTargetSpecificModuleFiles(isFramework)) {
      case SearchResult::Found:
        return true;
      case SearchResult::NotFound:
        continue;
      case SearchResult::Error:
        return false;
      }
    }
    }
    toolchain_unreachable("covered switch");
  }
  return false;
}

static std::pair<StringRef, clang::VersionTuple>
getOSAndVersionForDiagnostics(const toolchain::Triple &triple) {
  StringRef osName;
  toolchain::VersionTuple osVersion;
  if (triple.isMacOSX()) {
    // macOS triples represent their versions differently, so we have to use the
    // special accessor.
    triple.getMacOSXVersion(osVersion);
    osName = language::prettyPlatformString(PlatformKind::macOS);
  } else {
    osVersion = triple.getOSVersion();
    if (triple.isWatchOS()) {
      osName = language::prettyPlatformString(PlatformKind::watchOS);
    } else if (triple.isTvOS()) {
      assert(triple.isiOS() &&
             "LLVM treats tvOS as a kind of iOS, so tvOS is checked first");
      osName = language::prettyPlatformString(PlatformKind::tvOS);
    } else if (triple.isiOS()) {
      osName = language::prettyPlatformString(PlatformKind::iOS);
    }  else if (triple.isXROS()) {
      osName = language::prettyPlatformString(PlatformKind::visionOS);
    } else {
      assert(!triple.isOSDarwin() && "unknown Apple OS");
      // Fallback to the LLVM triple name. This isn't great (it won't be
      // capitalized or anything), but it's better than nothing.
      osName = triple.getOSName();
    }
  }

  assert(!osName.empty());
  return {osName, osVersion};
}

LoadedFile *SerializedModuleLoaderBase::loadAST(
    ModuleDecl &M, std::optional<SourceLoc> diagLoc,
    StringRef moduleInterfacePath, StringRef moduleInterfaceSourcePath,
    std::unique_ptr<toolchain::MemoryBuffer> moduleInputBuffer,
    std::unique_ptr<toolchain::MemoryBuffer> moduleDocInputBuffer,
    std::unique_ptr<toolchain::MemoryBuffer> moduleSourceInfoInputBuffer,
    bool isFramework) {
  assert(moduleInputBuffer);

  // The buffers are moved into the shared core, so grab their IDs now in case
  // they're needed for diagnostics later.
  StringRef moduleBufferID = moduleInputBuffer->getBufferIdentifier();
  StringRef moduleDocBufferID;
  if (moduleDocInputBuffer)
    moduleDocBufferID = moduleDocInputBuffer->getBufferIdentifier();
  StringRef moduleSourceInfoID;
  if (moduleSourceInfoInputBuffer)
    moduleSourceInfoID = moduleSourceInfoInputBuffer->getBufferIdentifier();

  if (moduleInputBuffer->getBufferSize() % 4 != 0) {
    if (diagLoc)
      Ctx.Diags.diagnose(*diagLoc, diag::serialization_malformed_module,
                         moduleBufferID);
    return nullptr;
  }

  std::unique_ptr<ModuleFile> loadedModuleFile;
  std::shared_ptr<const ModuleFileSharedCore> loadedModuleFileCore;
  serialization::ValidationInfo loadInfo = ModuleFileSharedCore::load(
      moduleInterfacePath, moduleInterfaceSourcePath,
      std::move(moduleInputBuffer), std::move(moduleDocInputBuffer),
      std::move(moduleSourceInfoInputBuffer), isFramework,
      isRequiredOSSAModules(),
      Ctx.LangOpts.SDKName,
      Ctx.SearchPathOpts.DeserializedPathRecoverer, loadedModuleFileCore);
  SerializedASTFile *fileUnit = nullptr;

  if (loadInfo.status == serialization::Status::Valid) {
    loadedModuleFile =
        std::make_unique<ModuleFile>(std::move(loadedModuleFileCore));
    M.setResilienceStrategy(loadedModuleFile->getResilienceStrategy());

    // We've loaded the file. Now try to bring it into the AST.
    fileUnit = new (Ctx) SerializedASTFile(M, *loadedModuleFile);
    M.setStaticLibrary(loadedModuleFile->isStaticLibrary());
    M.setHasHermeticSealAtLink(loadedModuleFile->hasHermeticSealAtLink());
    M.setIsEmbeddedCodiraModule(loadedModuleFile->isEmbeddedCodiraModule());
    if (loadedModuleFile->isTestable())
      M.setTestingEnabled();
    if (loadedModuleFile->arePrivateImportsEnabled())
      M.setPrivateImportsEnabled();
    if (loadedModuleFile->isImplicitDynamicEnabled())
      M.setImplicitDynamicEnabled();
    if (loadedModuleFile->hasIncrementalInfo())
      M.setHasIncrementalInfo();
    if (loadedModuleFile->isBuiltFromInterface())
      M.setIsBuiltFromInterface();
    if (loadedModuleFile->allowNonResilientAccess())
      M.setAllowNonResilientAccess();
    if (loadedModuleFile->serializePackageEnabled())
      M.setSerializePackageEnabled();
    if (!loadedModuleFile->getModuleABIName().empty())
      M.setABIName(Ctx.getIdentifier(loadedModuleFile->getModuleABIName()));
    if (!loadedModuleFile->getPublicModuleName().empty())
      M.setPublicModuleName(Ctx.getIdentifier(loadedModuleFile->getPublicModuleName()));
    if (loadedModuleFile->isConcurrencyChecked())
      M.setIsConcurrencyChecked();
    if (loadedModuleFile->strictMemorySafety())
      M.setStrictMemorySafety();
    if (loadedModuleFile->hasCxxInteroperability()) {
      M.setHasCxxInteroperability();
      M.setCXXStdlibKind(loadedModuleFile->getCXXStdlibKind());
    }
    if (!loadedModuleFile->getModulePackageName().empty()) {
      M.setPackageName(Ctx.getIdentifier(loadedModuleFile->getModulePackageName()));
    }
    M.setUserModuleVersion(loadedModuleFile->getUserModuleVersion());
    M.setCodiraInterfaceCompilerVersion(
        loadedModuleFile->getCodiraInterfaceCompilerVersion());
    for (auto name: loadedModuleFile->getAllowableClientNames()) {
      M.addAllowableClientName(Ctx.getIdentifier(name));
    }
    if (Ctx.LangOpts.BypassResilienceChecks)
      M.setBypassResilience();
    auto diagLocOrInvalid = diagLoc.value_or(SourceLoc());
    loadInfo.status = loadedModuleFile->associateWithFileContext(
        fileUnit, diagLocOrInvalid, Ctx.LangOpts.AllowModuleWithCompilerErrors);

    // FIXME: This seems wrong. Overlay for system Clang module doesn't
    // necessarily mean it's "system" module. User can make their own overlay
    // in non-system directory.
    // Remove this block after we fix the test suite.
    if (auto shadowed = loadedModuleFile->getUnderlyingModule())
      if (shadowed->isSystemModule())
        M.setIsSystemModule(true);

    if (loadInfo.status == serialization::Status::Valid ||
        (Ctx.LangOpts.AllowModuleWithCompilerErrors &&
         (loadInfo.status == serialization::Status::TargetTooNew ||
          loadInfo.status == serialization::Status::TargetIncompatible))) {
      if (loadedModuleFile->hasSourceInfoFile() &&
          !loadedModuleFile->hasSourceInfo())
        Ctx.Diags.diagnose(diagLocOrInvalid,
                           diag::serialization_malformed_sourceinfo,
                           moduleSourceInfoID);

      Ctx.bumpGeneration();
      LoadedModuleFiles.emplace_back(std::move(loadedModuleFile),
                                     Ctx.getCurrentGeneration());
      findOverlayFiles(diagLoc.value_or(SourceLoc()), &M, fileUnit);
    } else {
      fileUnit = nullptr;
    }
  }

  if (loadInfo.status != serialization::Status::Valid) {
    if (diagLoc)
      serialization::diagnoseSerializedASTLoadFailure(
          Ctx, *diagLoc, loadInfo, moduleBufferID, moduleDocBufferID,
          loadedModuleFile.get(), M.getName());

    // Even though the module failed to load, it's possible its contents
    // include a source buffer that need to survive because it's already been
    // used for diagnostics.
    // Note this is only necessary in case a bridging header failed to load
    // during the `associateWithFileContext()` call.
    if (loadedModuleFile &&
        loadedModuleFile->mayHaveDiagnosticsPointingAtBuffer())
      OrphanedModuleFiles.push_back(std::move(loadedModuleFile));
  } else {
    // Report non-fatal compiler tag mismatch on stderr only to avoid
    // polluting the IDE UI.
    if (!loadInfo.problematicRevision.empty()) {
      toolchain::errs() << "remark: compiled module was created by a different " <<
                      "version of the compiler '" <<
                      loadInfo.problematicRevision <<
                      "': " << moduleBufferID << "\n";
    }
  }

  // The -experimental-hermetic-seal-at-link flag turns on dead-stripping
  // optimizations assuming library code can be optimized against client code.
  // If the imported module was built with -experimental-hermetic-seal-at-link
  // but the current module isn't, error out.
  if (M.hasHermeticSealAtLink() && !Ctx.LangOpts.HermeticSealAtLink) {
    Ctx.Diags.diagnose(diagLoc.value_or(SourceLoc()),
                       diag::need_hermetic_seal_to_import_module, M.getName());
  }

  if (M.isEmbeddedCodiraModule() &&
      !Ctx.LangOpts.hasFeature(Feature::Embedded)) {
    Ctx.Diags.diagnose(diagLoc.value_or(SourceLoc()),
                       diag::cannot_import_embedded_module, M.getName());
  }

  if (!M.isEmbeddedCodiraModule() &&
      Ctx.LangOpts.hasFeature(Feature::Embedded)) {
    Ctx.Diags.diagnose(diagLoc.value_or(SourceLoc()),
                       diag::cannot_import_non_embedded_module, M.getName());
  }

  // Non-resilient modules built with C++ interoperability enabled
  // are typically incompatible with clients that do not enable
  // C++ interoperability.
  if (M.hasCxxInteroperability() &&
      M.getResilienceStrategy() != ResilienceStrategy::Resilient &&
      !Ctx.LangOpts.EnableCXXInterop &&
      Ctx.LangOpts.RequireCxxInteropToImportCxxInteropModule &&
      M.getName().str() != CXX_MODULE_NAME) {
    auto loc = diagLoc.value_or(SourceLoc());
    Ctx.Diags.diagnose(loc, diag::need_cxx_interop_to_import_module,
                       M.getName());
    Ctx.Diags.diagnose(loc, diag::enable_cxx_interop_docs);
  }
  // Modules built with libc++ cannot be imported into modules that are built
  // with libstdc++, and vice versa. Make an exception for Cxx.codemodule since
  // it doesn't refer to any C++ stdlib symbols, and for CxxStdlib.codemodule
  // since we skipped loading the overlay for the module.
  if (M.hasCxxInteroperability() && Ctx.LangOpts.EnableCXXInterop &&
      M.getCXXStdlibKind() != Ctx.LangOpts.CXXStdlib &&
      M.getName() != Ctx.Id_Cxx && M.getName() != Ctx.Id_CxxStdlib) {
    auto loc = diagLoc.value_or(SourceLoc());
    Ctx.Diags.diagnose(loc, diag::cxx_stdlib_kind_mismatch, M.getName(),
                       to_string(M.getCXXStdlibKind()),
                       to_string(Ctx.LangOpts.CXXStdlib));
  }

  return fileUnit;
}

bool SerializedModuleLoaderBase::isRequiredOSSAModules() const {
  return Ctx.SILOpts.EnableOSSAModules;
}

void language::serialization::diagnoseSerializedASTLoadFailure(
    ASTContext &Ctx, SourceLoc diagLoc,
    const serialization::ValidationInfo &loadInfo,
    StringRef moduleBufferID, StringRef moduleDocBufferID,
    ModuleFile *loadedModuleFile, Identifier ModuleName) {
  auto diagnoseDifferentLanguageVersion = [&](StringRef shortVersion) -> bool {
    if (shortVersion.empty())
      return false;

    SmallString<32> versionBuf;
    toolchain::raw_svector_ostream versionString(versionBuf);
    versionString << Version::getCurrentLanguageVersion();
    if (versionString.str() == shortVersion)
      return false;

    Ctx.Diags.diagnose(
        diagLoc, diag::serialization_module_language_version_mismatch,
        loadInfo.shortVersion, versionString.str(), moduleBufferID);
    return true;
  };

  switch (loadInfo.status) {
  case serialization::Status::Valid:
    toolchain_unreachable("At this point we know loading has failed");

  case serialization::Status::FormatTooNew:
    if (diagnoseDifferentLanguageVersion(loadInfo.shortVersion))
      break;
    Ctx.Diags.diagnose(diagLoc, diag::serialization_module_too_new,
                       moduleBufferID);
    break;
  case serialization::Status::FormatTooOld:
    if (diagnoseDifferentLanguageVersion(loadInfo.shortVersion))
      break;
    Ctx.Diags.diagnose(diagLoc, diag::serialization_module_too_old, ModuleName,
                       moduleBufferID);
    break;
  case serialization::Status::NotInOSSA:
    if (Ctx.SerializationOpts.ExplicitModuleBuild ||
        Ctx.SILOpts.EnableOSSAModules) {
      Ctx.Diags.diagnose(diagLoc,
                         diag::serialization_non_ossa_module_incompatible,
                         ModuleName);
    }
    break;
  case serialization::Status::RevisionIncompatible:
    Ctx.Diags.diagnose(diagLoc, diag::serialization_module_incompatible_revision,
                       loadInfo.problematicRevision, ModuleName, moduleBufferID);
    break;
  case serialization::Status::ChannelIncompatible:
    Ctx.Diags.diagnose(diagLoc, diag::serialization_module_incompatible_channel,
                       loadInfo.problematicChannel,
                       version::getCurrentCompilerChannel(),
                       ModuleName, moduleBufferID);
    break;
  case serialization::Status::Malformed:
    Ctx.Diags.diagnose(diagLoc, diag::serialization_malformed_module,
                       moduleBufferID);
    break;

  case serialization::Status::MalformedDocumentation:
    assert(!moduleDocBufferID.empty());
    Ctx.Diags.diagnose(diagLoc, diag::serialization_malformed_module,
                       moduleDocBufferID);
    break;

  case serialization::Status::MissingDependency:
  case serialization::Status::CircularDependency:
  case serialization::Status::MissingUnderlyingModule:
    serialization::diagnoseSerializedASTLoadFailureTransitive(
      Ctx, diagLoc, loadInfo.status,
      loadedModuleFile, ModuleName, /*forTestable*/false);
    break;

  case serialization::Status::FailedToLoadBridgingHeader:
    // We already emitted a diagnostic about the bridging header. Just emit
    // a generic message here.
    Ctx.Diags.diagnose(diagLoc, diag::serialization_load_failed,
                       ModuleName.str());
    break;

  case serialization::Status::NameMismatch: {
    // FIXME: This doesn't handle a non-debugger REPL, which should also treat
    // this as a non-fatal error.
    auto diagKind = diag::serialization_name_mismatch;
    if (Ctx.LangOpts.DebuggerSupport)
      diagKind = diag::serialization_name_mismatch_repl;
    Ctx.Diags.diagnose(diagLoc, diagKind, loadInfo.name, ModuleName.str());
    break;
  }

  case serialization::Status::TargetIncompatible: {
    // FIXME: This doesn't handle a non-debugger REPL, which should also treat
    // this as a non-fatal error.
    auto diagKind = diag::serialization_target_incompatible;
    if (Ctx.LangOpts.DebuggerSupport ||
        Ctx.LangOpts.AllowModuleWithCompilerErrors)
      diagKind = diag::serialization_target_incompatible_repl;
    Ctx.Diags.diagnose(diagLoc, diagKind, ModuleName, loadInfo.targetTriple,
                       moduleBufferID);
    break;
  }

  case serialization::Status::TargetTooNew: {
    toolchain::Triple moduleTarget(toolchain::Triple::normalize(loadInfo.targetTriple));

    std::pair<StringRef, clang::VersionTuple> moduleOSInfo =
        getOSAndVersionForDiagnostics(moduleTarget);
    std::pair<StringRef, clang::VersionTuple> compilationOSInfo =
        getOSAndVersionForDiagnostics(Ctx.LangOpts.Target);

    // FIXME: This doesn't handle a non-debugger REPL, which should also treat
    // this as a non-fatal error.
    auto diagKind = diag::serialization_target_too_new;
    if (Ctx.LangOpts.DebuggerSupport ||
        Ctx.LangOpts.AllowModuleWithCompilerErrors)
      diagKind = diag::serialization_target_too_new_repl;
    Ctx.Diags.diagnose(diagLoc, diagKind, compilationOSInfo.first,
                       compilationOSInfo.second, ModuleName,
                       moduleOSInfo.second, moduleBufferID);
    break;
  }

  case serialization::Status::SDKMismatch:
    auto currentSDK = Ctx.LangOpts.SDKName;
    auto moduleSDK = loadInfo.sdkName;
    Ctx.Diags.diagnose(diagLoc, diag::serialization_sdk_mismatch,
                       ModuleName, moduleSDK, currentSDK, moduleBufferID);
    break;
  }
}

void language::serialization::diagnoseSerializedASTLoadFailureTransitive(
    ASTContext &Ctx, SourceLoc diagLoc, const serialization::Status status,
    ModuleFile *loadedModuleFile, Identifier ModuleName, bool forTestable) {
  switch (status) {
  case serialization::Status::Valid:
  case serialization::Status::FormatTooNew:
  case serialization::Status::FormatTooOld:
  case serialization::Status::NotInOSSA:
  case serialization::Status::RevisionIncompatible:
  case serialization::Status::ChannelIncompatible:
  case serialization::Status::Malformed:
  case serialization::Status::MalformedDocumentation:
  case serialization::Status::FailedToLoadBridgingHeader:
  case serialization::Status::NameMismatch:
  case serialization::Status::TargetIncompatible:
  case serialization::Status::TargetTooNew:
  case serialization::Status::SDKMismatch:
    toolchain_unreachable("status not handled by "
        "diagnoseSerializedASTLoadFailureTransitive");

  case serialization::Status::MissingDependency: {
    // Figure out /which/ dependencies are missing.
    // FIXME: Dependencies should be de-duplicated at serialization time,
    // not now.
    toolchain::StringSet<> duplicates;
    toolchain::SmallVector<ModuleFile::Dependency, 4> missing;
    std::copy_if(
        loadedModuleFile->getDependencies().begin(),
        loadedModuleFile->getDependencies().end(), std::back_inserter(missing),
        [&duplicates, &loadedModuleFile, forTestable](
            const ModuleFile::Dependency &dependency) -> bool {
          if (dependency.isLoaded() || dependency.isHeader() ||
              loadedModuleFile->getTransitiveLoadingBehavior(dependency,
                                                             forTestable)
                != ModuleLoadingBehavior::Required) {
            return false;
          }
          return duplicates.insert(dependency.Core.RawPath).second;
        });

    // FIXME: only show module part of RawAccessPath
    assert(!missing.empty() && "unknown missing dependency?");
    if (missing.size() == 1) {
      Ctx.Diags.diagnose(diagLoc, diag::serialization_missing_single_dependency,
                         missing.front().Core.getPrettyPrintedPath());
    } else {
      toolchain::SmallString<64> missingNames;
      missingNames += '\'';
      interleave(missing,
                 [&](const ModuleFile::Dependency &next) {
                   missingNames += next.Core.getPrettyPrintedPath();
                 },
                 [&] { missingNames += "', '"; });
      missingNames += '\'';

      Ctx.Diags.diagnose(diagLoc, diag::serialization_missing_dependencies,
                         missingNames);
    }

    if (Ctx.SearchPathOpts.getSDKPath().empty() &&
        toolchain::Triple(toolchain::sys::getProcessTriple()).isMacOSX()) {
      Ctx.Diags.diagnose(SourceLoc(), diag::sema_no_import_no_sdk);
      Ctx.Diags.diagnose(SourceLoc(), diag::sema_no_import_no_sdk_xcrun);
    }
    break;
  }

  case serialization::Status::CircularDependency: {
    auto circularDependencyIter = toolchain::find_if(
        loadedModuleFile->getDependencies(),
        [](const ModuleFile::Dependency &next) {
          return next.isLoaded() &&
                 !(next.Import.has_value() &&
                   next.Import->importedModule->hasResolvedImports());
        });
    assert(circularDependencyIter !=
               loadedModuleFile->getDependencies().end() &&
           "circular dependency reported, but no module with unresolved "
           "imports found");

    // FIXME: We should include the path of the circularity as well, but that's
    // hard because we're discovering this /while/ resolving imports, which
    // means the problematic modules haven't been recorded yet.
    Ctx.Diags.diagnose(diagLoc, diag::serialization_circular_dependency,
                       circularDependencyIter->Core.getPrettyPrintedPath(),
                       ModuleName);
    break;
  }

  case serialization::Status::MissingUnderlyingModule: {
    Ctx.Diags.diagnose(diagLoc, diag::serialization_missing_underlying_module,
                       ModuleName);
    if (Ctx.SearchPathOpts.getSDKPath().empty() &&
        toolchain::Triple(toolchain::sys::getProcessTriple()).isMacOSX()) {
      Ctx.Diags.diagnose(SourceLoc(), diag::sema_no_import_no_sdk);
      Ctx.Diags.diagnose(SourceLoc(), diag::sema_no_import_no_sdk_xcrun);
    }
    break;
  }
  }
}

static std::optional<StringRef> getFlagsFromInterfaceFile(StringRef &file,
                                                          StringRef prefix) {
  StringRef line, buffer = file;
  while (!buffer.empty()) {
    std::tie(line, buffer) = buffer.split('\n');
    // If the line is no longer comments, return not found.
    if (!line.consume_front("// "))
      return std::nullopt;

    if (line.consume_front(prefix) && line.consume_front(":")) {
      file = buffer;
      return line;
    }
  }

  return std::nullopt;
}

bool language::extractCompilerFlagsFromInterface(
    StringRef interfacePath, StringRef buffer, toolchain::StringSaver &ArgSaver,
    SmallVectorImpl<const char *> &SubArgs,
    std::optional<toolchain::Triple> PreferredTarget, DiagnosticEngine *Diag) {
  auto FlagMatch = getFlagsFromInterfaceFile(buffer, LANGUAGE_MODULE_FLAGS_KEY);
  if (!FlagMatch)
    return true;
  toolchain::cl::TokenizeGNUCommandLine(*FlagMatch, ArgSaver, SubArgs);

  for (unsigned I = 1; I < SubArgs.size(); ++I) {
    if (strcmp(SubArgs[I - 1], "-target") != 0 &&
        strcmp(SubArgs[I - 1], "-target-variant") != 0)
      continue;

    toolchain::Triple triple(SubArgs[I]);
    bool shouldModify = false;
    // If the target triple parsed from the languageinterface file differs
    // only in subarchitecture from the compatible target triple, then
    // we have loaded a Codira interface from a different-but-compatible
    // architecture slice. Use the compatible subarchitecture.
    if (PreferredTarget && triple.getArch() == PreferredTarget->getArch() &&
        triple.getSubArch() != PreferredTarget->getSubArch()) {
      triple.setArch(PreferredTarget->getArch(), PreferredTarget->getSubArch());
      shouldModify = true;
    }

    // Canonicalize the version in the target triple parsed from the
    // languageinterface.
    auto canonicalTriple = getCanonicalTriple(triple);
    if (!canonicalTriple.has_value()) {
      if (Diag) {
        const toolchain::VersionTuple OSVersion = triple.getOSVersion();
        const bool isOSVersionInValidRange =
            toolchain::Triple::isValidVersionForOS(triple.getOS(), OSVersion);
        const toolchain::VersionTuple canonicalVersion =
            toolchain::Triple::getCanonicalVersionForOS(
                triple.getOS(), triple.getOSVersion(), isOSVersionInValidRange);
        Diag->diagnose(SourceLoc(),
                       diag::map_os_version_from_textual_interface_failed,
                       OSVersion.getAsString(), canonicalVersion.getAsString(),
                       interfacePath);
      }
      break;
    }
    // Update the triple to use if it differs.
    if (!areTriplesStrictlyEqual(triple, *canonicalTriple)) {
      triple = *canonicalTriple;
      shouldModify = true;
    }
    if (shouldModify)
      SubArgs[I] = ArgSaver.save(triple.str()).data();
  }

  auto IgnFlagMatch =
      getFlagsFromInterfaceFile(buffer, LANGUAGE_MODULE_FLAGS_IGNORABLE_KEY);
  auto IgnPrivateFlagMatch = getFlagsFromInterfaceFile(
      buffer, LANGUAGE_MODULE_FLAGS_IGNORABLE_PRIVATE_KEY);

  // It's OK the interface doesn't have the ignorable list (private or not), we just
  // ignore them all.
  if (!IgnFlagMatch && !IgnPrivateFlagMatch)
    return false;

  SmallVector<const char *, 8> IgnSubArgs;
  if (IgnFlagMatch)
    toolchain::cl::TokenizeGNUCommandLine(*IgnFlagMatch, ArgSaver, IgnSubArgs);
  if (IgnPrivateFlagMatch)
    toolchain::cl::TokenizeGNUCommandLine(*IgnPrivateFlagMatch, ArgSaver,
                                     IgnSubArgs);

  std::unique_ptr<toolchain::opt::OptTable> table = language::createCodiraOptTable();
  unsigned missingArgIdx = 0;
  unsigned missingArgCount = 0;
  auto parsedIgns = table->ParseArgs(IgnSubArgs, missingArgIdx, missingArgCount);
  for (auto parse: parsedIgns) {
    // Check if the option is a frontend option. This will filter out unknown
    // options and input-like options.
    if (!parse->getOption().hasFlag(options::FrontendOption))
      continue;
    auto spelling = ArgSaver.save(parse->getSpelling());
    auto &values = parse->getValues();
    if (spelling.ends_with("=")) {
      // Handle the case like -tbd-install_name=Foo. This should be rare because
      // most equal-separated arguments are alias to the separate form.
      assert(values.size() == 1);
      SubArgs.push_back(ArgSaver.save((toolchain::Twine(spelling) + values[0]).str()).data());
    } else {
      // Push the supported option and its value to the list.
      SubArgs.push_back(spelling.data());
      for (auto value: values)
        SubArgs.push_back(value);
    }
  }

  return false;
}

toolchain::VersionTuple
language::extractUserModuleVersionFromInterface(StringRef moduleInterfacePath) {
  toolchain::VersionTuple result;
  // Read the interface file and extract its compiler arguments line
  if (auto file = toolchain::MemoryBuffer::getFile(moduleInterfacePath)) {
    toolchain::BumpPtrAllocator alloc;
    toolchain::StringSaver argSaver(alloc);
    SmallVector<const char*, 8> args;
    (void)extractCompilerFlagsFromInterface(moduleInterfacePath,
                                            (*file)->getBuffer(), argSaver, args);
    for (unsigned I = 0, N = args.size(); I + 1 < N; I++) {
      // Check the version number specified via -user-module-version.
      StringRef current(args[I]), next(args[I + 1]);
      if (current == "-user-module-version") {
        // Sanitize versions that are too long
        while(next.count('.') > 3) {
          next = next.rsplit('.').first;
        }
        result.tryParse(next);
        break;
      }
    }
  }
  return result;
}

std::string language::extractEmbeddedBridgingHeaderContent(
    std::unique_ptr<toolchain::MemoryBuffer> file, ASTContext &Context) {
  std::shared_ptr<const ModuleFileSharedCore> loadedModuleFile;
  serialization::ValidationInfo loadInfo = ModuleFileSharedCore::load(
      "", "", std::move(file), nullptr, nullptr, false,
      Context.SILOpts.EnableOSSAModules, Context.LangOpts.SDKName,
      Context.SearchPathOpts.DeserializedPathRecoverer,
      loadedModuleFile);

  if (loadInfo.status != serialization::Status::Valid)
    return {};

  return loadedModuleFile->getEmbeddedHeader();
}

bool SerializedModuleLoaderBase::canImportModule(
    ImportPath::Module path, SourceLoc loc, ModuleVersionInfo *versionInfo,
    bool isTestableDependencyLookup) {
  // FIXME: Codira submodules?
  if (path.hasSubmodule())
    return false;

  // Look on disk.
  SmallString<256> moduleInterfaceSourcePath;
  std::unique_ptr<toolchain::MemoryBuffer> moduleInputBuffer;
  bool isFramework = false;
  bool isSystemModule = false;

  auto mID = path[0];
  auto found = findModule(
      mID, /*moduleInterfacePath=*/nullptr, &moduleInterfaceSourcePath,
      &moduleInputBuffer,
      /*moduleDocBuffer=*/nullptr, /*moduleSourceInfoBuffer=*/nullptr,
      /*skipBuildingInterface=*/true, isTestableDependencyLookup,
      isFramework, isSystemModule);
  // If we cannot find the module, don't continue.
  if (!found)
    return false;

  if (!moduleInterfaceSourcePath.empty()) {
    // If we found interface and version is not requested, we're done.
    if (!versionInfo)
      return true;

    auto moduleVersion =
        extractUserModuleVersionFromInterface(moduleInterfaceSourcePath);

    // If version is requested and found in interface, return the version.
    // Otherwise fallback to binary module handling.
    if (!moduleVersion.empty()) {
      versionInfo->setVersion(moduleVersion,
                              ModuleVersionSourceKind::CodiraInterface);
      return true;
    }
  }

  if (moduleInputBuffer) {
    auto metaData = serialization::validateSerializedAST(
        moduleInputBuffer->getBuffer(), Ctx.SILOpts.EnableOSSAModules,
        Ctx.LangOpts.SDKName);

    // If we only found binary module, make sure that is valid.
    if (metaData.status != serialization::Status::Valid &&
        moduleInterfaceSourcePath.empty()) {
      // Emit warning if the canImport check location is known.
      if (loc.isValid())
        Ctx.Diags.diagnose(loc, diag::can_import_invalid_languagemodule,
                           moduleInputBuffer->getBufferIdentifier());

      return false;
    }

    if (versionInfo)
      versionInfo->setVersion(metaData.userModuleVersion,
                              ModuleVersionSourceKind::CodiraBinaryModule);
  }

  if (versionInfo && !versionInfo->isValid()) {
    // If no version is found, set it to empty version.
    versionInfo->setVersion(toolchain::VersionTuple(),
                            ModuleVersionSourceKind::CodiraBinaryModule);
  }

  return true;
}

bool MemoryBufferSerializedModuleLoader::canImportModule(
    ImportPath::Module path, SourceLoc loc, ModuleVersionInfo *versionInfo,
    bool isTestableDependencyLookup) {
  // FIXME: Codira submodules?
  if (path.hasSubmodule())
    return false;

  auto mID = path[0];
  auto mIt = MemoryBuffers.find(mID.Item.str());
  if (mIt == MemoryBuffers.end())
    return false;

  if (!versionInfo)
    return true;

  versionInfo->setVersion(mIt->second.userVersion,
                          ModuleVersionSourceKind::CodiraBinaryModule);
  return true;
}

ModuleDecl *
SerializedModuleLoaderBase::loadModule(SourceLoc importLoc,
                                       ImportPath::Module path,
                                       bool AllowMemoryCache) {
  // FIXME: Codira submodules?
  if (path.size() > 1)
    return nullptr;

  auto moduleID = path[0];
  bool isFramework = false;
  bool isSystemModule = false;

  toolchain::SmallString<256> moduleInterfacePath;
  toolchain::SmallString<256> moduleInterfaceSourcePath;
  std::unique_ptr<toolchain::MemoryBuffer> moduleInputBuffer;
  std::unique_ptr<toolchain::MemoryBuffer> moduleDocInputBuffer;
  std::unique_ptr<toolchain::MemoryBuffer> moduleSourceInfoInputBuffer;

  // Look on disk.
  if (!findModule(moduleID, &moduleInterfacePath, &moduleInterfaceSourcePath,
                  &moduleInputBuffer, &moduleDocInputBuffer,
                  &moduleSourceInfoInputBuffer,
                  /*skipBuildingInterface=*/false,
                  /*isTestableDependencyLookup=*/false,
                  isFramework,
                  isSystemModule)) {
    return nullptr;
  }

  assert(moduleInputBuffer);

  LoadedFile *file = nullptr;
  auto *M = ModuleDecl::create(moduleID.Item, Ctx,
                               [&](ModuleDecl *M, auto addFile) {
    M->setIsSystemModule(isSystemModule);
    if (AllowMemoryCache)
      Ctx.addLoadedModule(M);

    toolchain::sys::path::native(moduleInterfacePath);
    file = loadAST(*M, moduleID.Loc, moduleInterfacePath,
                   moduleInterfaceSourcePath, std::move(moduleInputBuffer),
                   std::move(moduleDocInputBuffer),
                   std::move(moduleSourceInfoInputBuffer), isFramework);
    if (file) {
      addFile(file);
    } else {
      M->setFailedToLoad();
    }
    M->setHasResolvedImports();
  });

  if (dependencyTracker && file) {
    auto DepPath = file->getFilename();
    // Don't record cached artifacts as dependencies.
    if (!isCached(DepPath)) {
      if (M->hasIncrementalInfo()) {
        dependencyTracker->addIncrementalDependency(DepPath,
                                                    M->getFingerprint());
      } else {
        dependencyTracker->addDependency(DepPath, /*isSystem=*/false);
      }
    }
  }
  return M;
}

ModuleDecl *
MemoryBufferSerializedModuleLoader::loadModule(SourceLoc importLoc,
                                               ImportPath::Module path,
                                               bool AllowMemoryCache) {
  // FIXME: Codira submodules?
  if (path.size() > 1)
    return nullptr;

  auto moduleID = path[0];

  // See if we find it in the registered memory buffers.

  // FIXME: Right now this works only with access paths of length 1.
  // Once submodules are designed, this needs to support suffix
  // matching and a search path.
  auto bufIter = MemoryBuffers.find(moduleID.Item.str());
  if (bufIter == MemoryBuffers.end())
    return nullptr;

  bool isFramework = false;
  std::unique_ptr<toolchain::MemoryBuffer> moduleInputBuffer;
  moduleInputBuffer = std::move(bufIter->second.buffer);
  MemoryBuffers.erase(bufIter);
  assert(moduleInputBuffer);

  auto *M = ModuleDecl::create(moduleID.Item, Ctx,
                               [&](ModuleDecl *M, auto addFile) {
    if (AllowMemoryCache)
      Ctx.addLoadedModule(M);

    auto *file = loadAST(*M, moduleID.Loc, /*moduleInterfacePath=*/"",
                         /*moduleInterfaceSourcePath=*/"",
                         std::move(moduleInputBuffer), {}, {}, isFramework);
    if (!file) {
      M->setFailedToLoad();
      return;
    }

    addFile(file);
    M->setHasResolvedImports();
  });
  if (M->failedToLoad()) {
    Ctx.removeLoadedModule(moduleID.Item);
    return nullptr;
  }
  // The MemoryBuffer loader is used by LLDB during debugging. Modules
  // imported from .code_ast sections are not typically produced from
  // textual interfaces. By disabling resilience, the debugger can
  // directly access private members.
  if (BypassResilience && !M->isBuiltFromInterface())
    M->setBypassResilience();

  return M;
}

void SerializedModuleLoaderBase::loadExtensions(NominalTypeDecl *nominal,
                                                unsigned previousGeneration) {
  for (auto &modulePair : LoadedModuleFiles) {
    if (modulePair.second <= previousGeneration)
      continue;
    modulePair.first->loadExtensions(nominal);
  }
}

void SerializedModuleLoaderBase::loadObjCMethods(
       NominalTypeDecl *typeDecl,
       ObjCSelector selector,
       bool isInstanceMethod,
       unsigned previousGeneration,
       toolchain::TinyPtrVector<AbstractFunctionDecl *> &methods) {
  for (auto &modulePair : LoadedModuleFiles) {
    if (modulePair.second <= previousGeneration)
      continue;
    modulePair.first->loadObjCMethods(typeDecl, selector, isInstanceMethod,
                                      methods);
  }
}

void SerializedModuleLoaderBase::loadDerivativeFunctionConfigurations(
    AbstractFunctionDecl *originalAFD, unsigned int previousGeneration,
    toolchain::SetVector<AutoDiffConfig> &results) {
  for (auto &modulePair : LoadedModuleFiles) {
    if (modulePair.second <= previousGeneration)
      continue;
    modulePair.first->loadDerivativeFunctionConfigurations(originalAFD,
                                                           results);
  }
}

std::error_code MemoryBufferSerializedModuleLoader::findModuleFilesInDirectory(
    ImportPath::Element ModuleID, const SerializedModuleBaseName &BaseName,
    SmallVectorImpl<char> *ModuleInterfacePath,
    SmallVectorImpl<char> *ModuleInterfaceSourcePath,
    std::unique_ptr<toolchain::MemoryBuffer> *ModuleBuffer,
    std::unique_ptr<toolchain::MemoryBuffer> *ModuleDocBuffer,
    std::unique_ptr<toolchain::MemoryBuffer> *ModuleSourceInfoBuffer,
    bool skipBuildingInterface, bool IsFramework,
    bool isTestableDependencyLookup) {
  // This is a soft error instead of an toolchain_unreachable because this API is
  // primarily used by LLDB which makes it more likely that unwitting changes to
  // the Codira compiler accidentally break the contract.
  assert(false && "not supported");
  return std::make_error_code(std::errc::not_supported);
}

bool MemoryBufferSerializedModuleLoader::maybeDiagnoseTargetMismatch(
    SourceLoc sourceLocation, StringRef moduleName,
    const SerializedModuleBaseName &absoluteBaseName) {
  return false;
}

void SerializedModuleLoaderBase::verifyAllModules() {
#ifndef NDEBUG
  for (const LoadedModulePair &loaded : LoadedModuleFiles)
    loaded.first->verify();
#endif
}

//-----------------------------------------------------------------------------
// SerializedASTFile implementation
//-----------------------------------------------------------------------------

void SerializedASTFile::getImportedModules(
    SmallVectorImpl<ImportedModule> &imports,
    ModuleDecl::ImportFilter filter) const {
  File.getImportedModules(imports, filter);
}

void SerializedASTFile::getExternalMacros(
    SmallVectorImpl<ExternalMacroPlugin> &macros) const {
  File.getExternalMacros(macros);
}

void SerializedASTFile::collectLinkLibrariesFromImports(
    ModuleDecl::LinkLibraryCallback callback) const {
  toolchain::SmallVector<ImportedModule, 8> Imports;
  File.getImportedModules(Imports, {ModuleDecl::ImportFilterKind::Exported,
                                    ModuleDecl::ImportFilterKind::Default});

  for (auto Import : Imports)
    Import.importedModule->collectLinkLibraries(callback);
}

void SerializedASTFile::collectLinkLibraries(
    ModuleDecl::LinkLibraryCallback callback) const {
  if (isSIB()) {
    collectLinkLibrariesFromImports(callback);
  } else {
    File.collectLinkLibraries(callback);
  }
}

void SerializedASTFile::loadDependenciesForTestable(SourceLoc diagLoc) const {
  serialization::Status status =
    File.loadDependenciesForFileContext(this, diagLoc, /*forTestable=*/true);

  if (status != serialization::Status::Valid) {
    serialization::diagnoseSerializedASTLoadFailureTransitive(
        getASTContext(), diagLoc, status, &File,
        getParentModule()->getName(), /*forTestable*/true);
  }
}

bool SerializedASTFile::isSIB() const {
  return File.isSIB();
}

bool SerializedASTFile::hadLoadError() const {
  return File.hasError();
}

bool SerializedASTFile::isSystemModule() const {
  if (auto Mod = File.getUnderlyingModule()) {
    return Mod->isSystemModule();
  }
  return false;
}

void SerializedASTFile::lookupValue(DeclName name, NLKind lookupKind,
                                    OptionSet<ModuleLookupFlags> Flags,
                                    SmallVectorImpl<ValueDecl*> &results) const{
  File.lookupValue(name, Flags, results);
}

StringRef
SerializedASTFile::getFilenameForPrivateDecl(const Decl *decl) const {
  return File.FilenamesForPrivateValues.lookup(decl);
}

TypeDecl *SerializedASTFile::lookupLocalType(toolchain::StringRef MangledName) const{
  return File.lookupLocalType(MangledName);
}

OpaqueTypeDecl *
SerializedASTFile::lookupOpaqueResultType(StringRef MangledName) {
  return File.lookupOpaqueResultType(MangledName);
}

TypeDecl *
SerializedASTFile::lookupNestedType(Identifier name,
                                    const NominalTypeDecl *parent) const {
  return File.lookupNestedType(name, parent);
}

void SerializedASTFile::lookupOperatorDirect(
    Identifier name, OperatorFixity fixity,
    TinyPtrVector<OperatorDecl *> &results) const {
  if (auto *op = File.lookupOperator(name, fixity))
    results.push_back(op);
}

void SerializedASTFile::lookupPrecedenceGroupDirect(
    Identifier name, TinyPtrVector<PrecedenceGroupDecl *> &results) const {
  if (auto *group = File.lookupPrecedenceGroup(name))
    results.push_back(group);
}

void SerializedASTFile::lookupVisibleDecls(ImportPath::Access accessPath,
                                           VisibleDeclConsumer &consumer,
                                           NLKind lookupKind) const {
  File.lookupVisibleDecls(accessPath, consumer, lookupKind);
}

void SerializedASTFile::lookupClassMembers(ImportPath::Access accessPath,
                                           VisibleDeclConsumer &consumer) const{
  File.lookupClassMembers(accessPath, consumer);
}

void
SerializedASTFile::lookupClassMember(ImportPath::Access accessPath,
                                     DeclName name,
                                     SmallVectorImpl<ValueDecl*> &decls) const {
  File.lookupClassMember(accessPath, name, decls);
}

void SerializedASTFile::lookupObjCMethods(
       ObjCSelector selector,
       SmallVectorImpl<AbstractFunctionDecl *> &results) const {
  File.lookupObjCMethods(selector, results);
}

std::optional<Fingerprint>
SerializedASTFile::loadFingerprint(const IterableDeclContext *IDC) const {
  return File.loadFingerprint(IDC);
}

void SerializedASTFile::lookupImportedSPIGroups(
                        const ModuleDecl *importedModule,
                        toolchain::SmallSetVector<Identifier, 4> &spiGroups) const {
  auto M = getParentModule();
  auto &imports = M->getASTContext().getImportCache();
  for (auto &dep : File.Dependencies) {
    if (!dep.Import.has_value())
      continue;

    if (dep.Import->importedModule == importedModule ||
        (imports.isImportedBy(importedModule, dep.Import->importedModule) &&
         imports.isImportedByViaCodiraOnly(importedModule,
                                          dep.Import->importedModule))) {
      spiGroups.insert(dep.spiGroups.begin(), dep.spiGroups.end());
    }
  }
}

std::optional<CommentInfo>
SerializedASTFile::getCommentForDecl(const Decl *D) const {
  return File.getCommentForDecl(D);
}

bool SerializedASTFile::hasLoadedCodiraDoc() const {
  return File.hasLoadedCodiraDoc();
}

std::optional<ExternalSourceLocs::RawLocs>
SerializedASTFile::getExternalRawLocsForDecl(const Decl *D) const {
  return File.getExternalRawLocsForDecl(D);
}

std::optional<StringRef>
SerializedASTFile::getGroupNameForDecl(const Decl *D) const {
  return File.getGroupNameForDecl(D);
}

std::optional<StringRef>
SerializedASTFile::getSourceFileNameForDecl(const Decl *D) const {
  return File.getSourceFileNameForDecl(D);
}

std::optional<unsigned>
SerializedASTFile::getSourceOrderForDecl(const Decl *D) const {
  return File.getSourceOrderForDecl(D);
}

void SerializedASTFile::collectAllGroups(
    SmallVectorImpl<StringRef> &Names) const {
  File.collectAllGroups(Names);
}

std::optional<StringRef>
SerializedASTFile::getGroupNameByUSR(StringRef USR) const {
  return File.getGroupNameByUSR(USR);
}

void
SerializedASTFile::getTopLevelDecls(SmallVectorImpl<Decl*> &results) const {
  File.getTopLevelDecls(results);
}

void SerializedASTFile::getExportedPrespecializations(
    SmallVectorImpl<Decl *> &results) const {
  File.getExportedPrespecializations(results);
}
void SerializedASTFile::getTopLevelDeclsWhereAttributesMatch(
              SmallVectorImpl<Decl*> &results,
              toolchain::function_ref<bool(DeclAttributes)> matchAttributes) const {
  File.getTopLevelDecls(results, matchAttributes);
}

void SerializedASTFile::getOperatorDecls(
    SmallVectorImpl<OperatorDecl *> &results) const {
  File.getOperatorDecls(results);
}

void SerializedASTFile::getPrecedenceGroups(
       SmallVectorImpl<PrecedenceGroupDecl*> &results) const {
  File.getPrecedenceGroups(results);
}

void
SerializedASTFile::getLocalTypeDecls(SmallVectorImpl<TypeDecl*> &results) const{
  File.getLocalTypeDecls(results);
}

void
SerializedASTFile::getOpaqueReturnTypeDecls(
                              SmallVectorImpl<OpaqueTypeDecl*> &results) const {
  File.getOpaqueReturnTypeDecls(results);
}

void
SerializedASTFile::getDisplayDecls(SmallVectorImpl<Decl*> &results, bool recursive) const {
  File.getDisplayDecls(results, recursive);
}

StringRef SerializedASTFile::getFilename() const {
  return File.getModuleFilename();
}

StringRef SerializedASTFile::getLoadedFilename() const {
  return File.getModuleLoadedFilename();
}

StringRef SerializedASTFile::getSourceFilename() const {
  return File.getModuleSourceFilename();
}

StringRef SerializedASTFile::getTargetTriple() const {
  return File.getTargetTriple();
}

ModuleDecl *SerializedASTFile::getUnderlyingModuleIfOverlay() const {
  return File.getUnderlyingModule();
}

const clang::Module *SerializedASTFile::getUnderlyingClangModule() const {
  if (auto *UnderlyingModule = File.getUnderlyingModule())
    return UnderlyingModule->findUnderlyingClangModule();
  return nullptr;
}

Identifier
SerializedASTFile::getDiscriminatorForPrivateDecl(const Decl *D) const {
  Identifier discriminator = File.getDiscriminatorForPrivateDecl(D);
  assert(!discriminator.empty() && "no discriminator found for value");
  return discriminator;
}

void SerializedASTFile::collectBasicSourceFileInfo(
    toolchain::function_ref<void(const BasicSourceFileInfo &)> callback) const {
  File.collectBasicSourceFileInfo(callback);
}

void SerializedASTFile::collectSerializedSearchPath(
    toolchain::function_ref<void(StringRef)> callback) const {
  File.collectSerializedSearchPath(callback);
}
