//===--- ModuleDependencies.h - Module Dependencies -------------*- C++ -*-===//
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
// This file implements data structures for capturing module dependencies.
//
//===----------------------------------------------------------------------===//
#include "language/AST/ModuleDependencies.h"
#include "language/AST/Decl.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/DiagnosticsSema.h"
#include "language/AST/MacroDefinition.h"
#include "language/AST/Module.h"
#include "language/AST/PluginLoader.h"
#include "language/AST/SourceFile.h"
#include "language/Frontend/Frontend.h"
#include "language/Strings.h"
#include "toolchain/Config/config.h"
#include "toolchain/Support/Path.h"
using namespace language;

ModuleDependencyInfoStorageBase::~ModuleDependencyInfoStorageBase() {}

bool ModuleDependencyInfo::isCodiraModule() const {
  return isCodiraInterfaceModule() || isCodiraSourceModule() ||
         isCodiraBinaryModule();
}

bool ModuleDependencyInfo::isTextualCodiraModule() const {
  return isCodiraInterfaceModule() || isCodiraSourceModule();
}

namespace {
  ModuleDependencyKind &operator++(ModuleDependencyKind &e) {
    if (e == ModuleDependencyKind::LastKind) {
      toolchain_unreachable(
                       "Attempting to increment last enum value on ModuleDependencyKind");
    }
    e = ModuleDependencyKind(
                             static_cast<std::underlying_type<ModuleDependencyKind>::type>(e) + 1);
    return e;
  }
}
bool ModuleDependencyInfo::isCodiraInterfaceModule() const {
  return isa<CodiraInterfaceModuleDependenciesStorage>(storage.get());
}

bool ModuleDependencyInfo::isCodiraSourceModule() const {
  return isa<CodiraSourceModuleDependenciesStorage>(storage.get());
}

bool ModuleDependencyInfo::isCodiraBinaryModule() const {
  return isa<CodiraBinaryModuleDependencyStorage>(storage.get());
}

bool ModuleDependencyInfo::isClangModule() const {
  return isa<ClangModuleDependencyStorage>(storage.get());
}

/// Retrieve the dependencies for a Codira textual interface module.
const CodiraInterfaceModuleDependenciesStorage *
ModuleDependencyInfo::getAsCodiraInterfaceModule() const {
  return dyn_cast<CodiraInterfaceModuleDependenciesStorage>(storage.get());
}

const CodiraSourceModuleDependenciesStorage *
ModuleDependencyInfo::getAsCodiraSourceModule() const {
  return dyn_cast<CodiraSourceModuleDependenciesStorage>(storage.get());
}

/// Retrieve the dependencies for a binary Codira dependency module.
const CodiraBinaryModuleDependencyStorage *
ModuleDependencyInfo::getAsCodiraBinaryModule() const {
  return dyn_cast<CodiraBinaryModuleDependencyStorage>(storage.get());
}

/// Retrieve the dependencies for a Clang module.
const ClangModuleDependencyStorage *
ModuleDependencyInfo::getAsClangModule() const {
  return dyn_cast<ClangModuleDependencyStorage>(storage.get());
}

std::string ModuleDependencyInfo::getModuleDefiningPath() const {
  std::string path = "";
  switch (getKind()) {
  case language::ModuleDependencyKind::CodiraInterface:
    path = getAsCodiraInterfaceModule()->languageInterfaceFile;
    break;
  case language::ModuleDependencyKind::CodiraBinary:
    path = getAsCodiraBinaryModule()->compiledModulePath;
    break;
  case language::ModuleDependencyKind::Clang:
    path = getAsClangModule()->moduleMapFile;
    break;
  case language::ModuleDependencyKind::CodiraSource:
    path = getAsCodiraSourceModule()->sourceFiles.front();
    break;
  default:
    toolchain_unreachable("Unexpected dependency kind");
  }

  // Relative to the `module.modulemap` or `.codeinterface` or `.codemodule`,
  // the defininig path is the parent directory of the file.
  return toolchain::sys::path::parent_path(path).str();
}

void ModuleDependencyInfo::addTestableImport(ImportPath::Module module) {
  assert(getAsCodiraSourceModule() && "Expected source module for addTestableImport.");
  dyn_cast<CodiraSourceModuleDependenciesStorage>(storage.get())->addTestableImport(module);
}

bool ModuleDependencyInfo::isTestableImport(StringRef moduleName) const {
  if (auto languageSourceDepStorage = getAsCodiraSourceModule())
    return languageSourceDepStorage->testableImports.contains(moduleName);
  else
    return false;
}

void ModuleDependencyInfo::addOptionalModuleImport(
    StringRef module, bool isExported, AccessLevel accessLevel,
    toolchain::StringSet<> *alreadyAddedModules) {

  if (alreadyAddedModules && alreadyAddedModules->contains(module)) {
    // Find a prior import of this module and add import location
    // and adjust whether or not this module is ever imported as exported
    // as well as the access level
    for (auto &existingImport : storage->optionalModuleImports) {
      if (existingImport.importIdentifier == module) {
        existingImport.isExported |= isExported;
        existingImport.accessLevel = std::max(existingImport.accessLevel,
                                              accessLevel);
        break;
      }
    }
  } else {
    if (alreadyAddedModules)
      alreadyAddedModules->insert(module);

    storage->optionalModuleImports.push_back(
         {module.str(), isExported, accessLevel});
  }
}

void ModuleDependencyInfo::addModuleImport(
    StringRef module, bool isExported, AccessLevel accessLevel,
    toolchain::StringSet<> *alreadyAddedModules, const SourceManager *sourceManager,
    SourceLoc sourceLocation) {
  auto scannerImportLocToDiagnosticLocInfo =
      [&sourceManager](SourceLoc sourceLocation) {
        auto lineAndColumnNumbers =
            sourceManager->getLineAndColumnInBuffer(sourceLocation);
        return ScannerImportStatementInfo::ImportDiagnosticLocationInfo(
            sourceManager->getDisplayNameForLoc(sourceLocation).str(),
            lineAndColumnNumbers.first, lineAndColumnNumbers.second);
      };
  bool validSourceLocation = sourceManager && sourceLocation.isValid() &&
                             sourceManager->isOwning(sourceLocation);

  if (alreadyAddedModules && alreadyAddedModules->contains(module)) {
    // Find a prior import of this module and add import location
    // and adjust whether or not this module is ever imported as exported
    // as well as the access level
    for (auto &existingImport : storage->moduleImports) {
      if (existingImport.importIdentifier == module) {
        if (validSourceLocation) {
          existingImport.addImportLocation(
              scannerImportLocToDiagnosticLocInfo(sourceLocation));
        }
        existingImport.isExported |= isExported;
        existingImport.accessLevel = std::max(existingImport.accessLevel,
                                              accessLevel);
        break;
      }
    }
  } else {
    if (alreadyAddedModules)
      alreadyAddedModules->insert(module);

    if (validSourceLocation)
      storage->moduleImports.push_back(ScannerImportStatementInfo(
          module.str(), isExported, accessLevel,
          scannerImportLocToDiagnosticLocInfo(sourceLocation)));
    else
      storage->moduleImports.push_back(
          ScannerImportStatementInfo(module.str(), isExported, accessLevel));
  }
}

void ModuleDependencyInfo::addModuleImport(
    ImportPath::Module module, bool isExported, AccessLevel accessLevel,
    toolchain::StringSet<> *alreadyAddedModules, const SourceManager *sourceManager,
    SourceLoc sourceLocation) {
  std::string ImportedModuleName = module.front().Item.str().str();
  auto submodulePath = module.getSubmodulePath();
  if (submodulePath.size() > 0 && !submodulePath[0].Item.empty()) {
    auto submoduleComponent = submodulePath[0];
    // Special case: a submodule named "Foo.Private" can be moved to a top-level
    // module named "Foo_Private". ClangImporter has special support for this.
    if (submoduleComponent.Item.str() == "Private")
      addOptionalModuleImport(ImportedModuleName + "_Private",
                              isExported, accessLevel,
                              alreadyAddedModules);
  }

  addModuleImport(ImportedModuleName, isExported, accessLevel,
                  alreadyAddedModules, sourceManager, sourceLocation);
}

void ModuleDependencyInfo::addModuleImports(
    const SourceFile &sourceFile, toolchain::StringSet<> &alreadyAddedModules,
    const SourceManager *sourceManager) {
  // Add all of the module dependencies.
  SmallVector<Decl *, 32> decls;
  sourceFile.getTopLevelDecls(decls);
  for (auto decl : decls) {
    if (auto importDecl = dyn_cast<ImportDecl>(decl)) {
      ImportPath::Builder scratch;
      auto realPath = importDecl->getRealModulePath(scratch);

      // Explicit 'Builtin' import is not a part of the module's
      // dependency set, does not exist on the filesystem,
      // and is resolved within the compiler during compilation.
      SmallString<64> importedModuleName;
      realPath.getString(importedModuleName);
      if (importedModuleName == BUILTIN_NAME)
        continue;

      // Ignore/diagnose tautological imports akin to import resolution
      if (!language::dependencies::checkImportNotTautological(
              realPath, importDecl->getLoc(), sourceFile,
              importDecl->isExported()))
        continue;

      addModuleImport(realPath, importDecl->isExported(),
                      importDecl->getAccessLevel(),
                      &alreadyAddedModules, sourceManager,
                      importDecl->getLoc());

      // Additionally, keep track of which dependencies of a Source
      // module are `@Testable`.
      if (getKind() == language::ModuleDependencyKind::CodiraSource &&
          importDecl->isTestable())
        addTestableImport(realPath);
    } else if (auto macroDecl = dyn_cast<MacroDecl>(decl)) {
      auto macroDef = macroDecl->getDefinition();
      auto &ctx = macroDecl->getASTContext();
      if (macroDef.kind != MacroDefinition::Kind::External)
        continue;
      auto external = macroDef.getExternalMacro();
      PluginLoader &loader = ctx.getPluginLoader();
      auto &entry = loader.lookupPluginByModuleName(external.moduleName);
      if (entry.libraryPath.empty() && entry.executablePath.empty())
        continue;
      addMacroDependency(external.moduleName.str(), entry.libraryPath,
                         entry.executablePath);
    }
  }

  auto fileName = sourceFile.getFilename();
  if (fileName.empty())
    return;

  switch (getKind()) {
  case language::ModuleDependencyKind::CodiraInterface: {
    // If the storage is for an interface file, the only source file we
    // should see is that interface file.
    assert(fileName ==
           cast<CodiraInterfaceModuleDependenciesStorage>(storage.get())->languageInterfaceFile);
    break;
  }
  case language::ModuleDependencyKind::CodiraSource: {
    // Otherwise, record the source file.
    auto languageSourceStorage =
        cast<CodiraSourceModuleDependenciesStorage>(storage.get());
    languageSourceStorage->sourceFiles.push_back(fileName.str());
    break;
  }
  default:
    toolchain_unreachable("Unexpected dependency kind");
  }
}

std::optional<std::string> ModuleDependencyInfo::getBridgingHeader() const {
  switch (getKind()) {
  case language::ModuleDependencyKind::CodiraInterface: {
    auto languageInterfaceStorage =
        cast<CodiraInterfaceModuleDependenciesStorage>(storage.get());
    return languageInterfaceStorage->textualModuleDetails.bridgingHeaderFile;
  }
  case language::ModuleDependencyKind::CodiraSource: {
    auto languageSourceStorage =
        cast<CodiraSourceModuleDependenciesStorage>(storage.get());
    return languageSourceStorage->textualModuleDetails.bridgingHeaderFile;
  }
  default:
    return std::nullopt;
  }
}

std::optional<std::string> ModuleDependencyInfo::getCASFSRootID() const {
  std::string Root;
  switch (getKind()) {
  case language::ModuleDependencyKind::CodiraInterface: {
    auto languageInterfaceStorage =
        cast<CodiraInterfaceModuleDependenciesStorage>(storage.get());
    Root = languageInterfaceStorage->textualModuleDetails.CASFileSystemRootID;
    break;
  }
  case language::ModuleDependencyKind::CodiraSource: {
    auto languageSourceStorage =
        cast<CodiraSourceModuleDependenciesStorage>(storage.get());
    Root = languageSourceStorage->textualModuleDetails.CASFileSystemRootID;
    break;
  }
  case language::ModuleDependencyKind::Clang: {
    auto clangModuleStorage = cast<ClangModuleDependencyStorage>(storage.get());
    Root = clangModuleStorage->CASFileSystemRootID;
    break;
  }
  default:
    return std::nullopt;
  }
  if (Root.empty())
    return std::nullopt;

  return Root;
}

std::optional<std::string> ModuleDependencyInfo::getClangIncludeTree() const {
  std::string Root;
  switch (getKind()) {
  case language::ModuleDependencyKind::Clang: {
    auto clangModuleStorage = cast<ClangModuleDependencyStorage>(storage.get());
    Root = clangModuleStorage->CASClangIncludeTreeRootID;
    break;
  }
  default:
    return std::nullopt;
  }
  if (Root.empty())
    return std::nullopt;

  return Root;
}

std::optional<std::string>
ModuleDependencyInfo::getBridgingHeaderIncludeTree() const {
  std::string Root;
  switch (getKind()) {
  case language::ModuleDependencyKind::CodiraInterface: {
    auto languageInterfaceStorage =
        cast<CodiraInterfaceModuleDependenciesStorage>(storage.get());
    Root = languageInterfaceStorage->textualModuleDetails
               .CASBridgingHeaderIncludeTreeRootID;
    break;
  }
  case language::ModuleDependencyKind::CodiraSource: {
    auto languageSourceStorage =
        cast<CodiraSourceModuleDependenciesStorage>(storage.get());
    Root = languageSourceStorage->textualModuleDetails
               .CASBridgingHeaderIncludeTreeRootID;
    break;
  }
  default:
    return std::nullopt;
  }
  if (Root.empty())
    return std::nullopt;

  return Root;
}

std::string ModuleDependencyInfo::getModuleOutputPath() const {
  switch (getKind()) {
  case language::ModuleDependencyKind::CodiraInterface: {
    auto languageInterfaceStorage =
        cast<CodiraInterfaceModuleDependenciesStorage>(storage.get());
    return languageInterfaceStorage->moduleOutputPath;
  }
  case language::ModuleDependencyKind::CodiraSource: {
    return "<languagemodule>";
  }
  case language::ModuleDependencyKind::Clang: {
    auto clangModuleStorage = cast<ClangModuleDependencyStorage>(storage.get());
    return clangModuleStorage->pcmOutputPath;
  }
  case language::ModuleDependencyKind::CodiraBinary: {
    auto languageBinaryStorage =
        cast<CodiraBinaryModuleDependencyStorage>(storage.get());
    return languageBinaryStorage->compiledModulePath;
  }
  default:
    toolchain_unreachable("Unexpected dependency kind");
  }
}

void ModuleDependencyInfo::addBridgingHeader(StringRef bridgingHeader) {
  switch (getKind()) {
  case language::ModuleDependencyKind::CodiraInterface: {
    auto languageInterfaceStorage =
        cast<CodiraInterfaceModuleDependenciesStorage>(storage.get());
    assert(!languageInterfaceStorage->textualModuleDetails.bridgingHeaderFile);
    languageInterfaceStorage->textualModuleDetails.bridgingHeaderFile = bridgingHeader.str();
    break;
  }
  case language::ModuleDependencyKind::CodiraSource: {
    auto languageSourceStorage =
        cast<CodiraSourceModuleDependenciesStorage>(storage.get());
    assert(!languageSourceStorage->textualModuleDetails.bridgingHeaderFile);
    languageSourceStorage->textualModuleDetails.bridgingHeaderFile = bridgingHeader.str();
    break;
  }
  default:
    toolchain_unreachable("Unexpected dependency kind");
  }
}

/// Add source files that the bridging header depends on.
void ModuleDependencyInfo::setHeaderSourceFiles(
    const std::vector<std::string> &files) {
  switch (getKind()) {
  case language::ModuleDependencyKind::CodiraInterface: {
    auto languageInterfaceStorage =
        cast<CodiraInterfaceModuleDependenciesStorage>(storage.get());
    languageInterfaceStorage->textualModuleDetails.bridgingSourceFiles = files;
    break;
  }
  case language::ModuleDependencyKind::CodiraSource: {
    auto languageSourceStorage =
        cast<CodiraSourceModuleDependenciesStorage>(storage.get());
    languageSourceStorage->textualModuleDetails.bridgingSourceFiles = files;
    break;
  }
  case language::ModuleDependencyKind::CodiraBinary: {
    auto languageBinaryStorage =
        cast<CodiraBinaryModuleDependencyStorage>(storage.get());
    languageBinaryStorage->headerSourceFiles = files;
    break;
  }
  default:
    toolchain_unreachable("Unexpected dependency kind");
  }
}

void ModuleDependencyInfo::addBridgingHeaderIncludeTree(StringRef ID) {
  switch (getKind()) {
  case language::ModuleDependencyKind::CodiraInterface: {
    auto languageInterfaceStorage =
        cast<CodiraInterfaceModuleDependenciesStorage>(storage.get());
    languageInterfaceStorage->textualModuleDetails
        .CASBridgingHeaderIncludeTreeRootID = ID.str();
    break;
  }
  case language::ModuleDependencyKind::CodiraSource: {
    auto languageSourceStorage =
        cast<CodiraSourceModuleDependenciesStorage>(storage.get());
    languageSourceStorage->textualModuleDetails
        .CASBridgingHeaderIncludeTreeRootID = ID.str();
    break;
  }
  default:
    toolchain_unreachable("Unexpected dependency kind");
  }
}

void ModuleDependencyInfo::setChainedBridgingHeaderBuffer(StringRef path,
                                                          StringRef buffer) {
  switch (getKind()) {
  case language::ModuleDependencyKind::CodiraSource: {
    auto languageSourceStorage =
        cast<CodiraSourceModuleDependenciesStorage>(storage.get());
    languageSourceStorage->setChainedBridgingHeaderBuffer(path, buffer);
    break;
  }
  default:
    toolchain_unreachable("Unexpected dependency kind");
  }
}

void ModuleDependencyInfo::addSourceFile(StringRef sourceFile) {
  switch (getKind()) {
  case language::ModuleDependencyKind::CodiraSource: {
    auto languageSourceStorage =
        cast<CodiraSourceModuleDependenciesStorage>(storage.get());
    languageSourceStorage->sourceFiles.push_back(sourceFile.str());
    break;
  }
  default:
    toolchain_unreachable("Unexpected dependency kind");
  }
}

void ModuleDependencyInfo::setOutputPathAndHash(StringRef outputPath,
                                                StringRef hash) {
  switch (getKind()) {
  case language::ModuleDependencyKind::CodiraInterface: {
    auto languageInterfaceStorage =
        cast<CodiraInterfaceModuleDependenciesStorage>(storage.get());
    languageInterfaceStorage->moduleOutputPath = outputPath.str();
    languageInterfaceStorage->contextHash = hash.str();
    break;
  }
  default:
    toolchain_unreachable("Unexpected dependency kind");
  }
}

CodiraDependencyScanningService::CodiraDependencyScanningService() {
  ClangScanningService.emplace(
      clang::tooling::dependencies::ScanningMode::DependencyDirectivesScan,
      clang::tooling::dependencies::ScanningOutputFormat::FullTree,
      clang::CASOptions(),
      /* CAS (toolchain::cas::ObjectStore) */ nullptr,
      /* Cache (toolchain::cas::ActionCache) */ nullptr,
      /* SharedFS */ nullptr,
      // ScanningOptimizations::Default excludes the current working
      // directory optimization. Clang needs to communicate with
      // the build system to handle the optimization safely.
      // Codira can handle the working directory optimizaiton
      // already so it is safe to turn on all optimizations.
      clang::tooling::dependencies::ScanningOptimizations::All);
}

bool
language::dependencies::checkImportNotTautological(const ImportPath::Module modulePath,
                                                const SourceLoc importLoc,
                                                const SourceFile &SF,
                                                bool isExported) {
  if (modulePath.front().Item != SF.getParentModule()->getName() ||
      // Overlays use an @_exported self-import to load their clang module.
      isExported ||
      // Imports of your own submodules are allowed in cross-language libraries.
      modulePath.size() != 1 ||
      // SIL files self-import to get decls from the rest of the module.
      SF.Kind == SourceFileKind::SIL)
    return true;

  ASTContext &ctx = SF.getASTContext();

  StringRef filename = toolchain::sys::path::filename(SF.getFilename());
  if (filename.empty())
    ctx.Diags.diagnose(importLoc, diag::sema_import_current_module,
                       modulePath.front().Item);
  else
    ctx.Diags.diagnose(importLoc, diag::sema_import_current_module_with_file,
                       filename, modulePath.front().Item);

  return false;
}

void language::dependencies::registerCxxInteropLibraries(
    const toolchain::Triple &Target, StringRef mainModuleName, bool hasStaticCxx,
    bool hasStaticCxxStdlib, CXXStdlibKind cxxStdlibKind,
    std::function<void(const LinkLibrary &)> RegistrationCallback) {

  switch (cxxStdlibKind) {
  case CXXStdlibKind::Libcxx:
    RegistrationCallback(
        LinkLibrary{"c++", LibraryKind::Library, /*static=*/false});
    break;
  case CXXStdlibKind::Libstdcxx:
    RegistrationCallback(
        LinkLibrary{"stdc++", LibraryKind::Library, /*static=*/false});
    break;
  case CXXStdlibKind::Msvcprt:
    // FIXME: should we be explicitly linking in msvcprt or will the module do
    // so?
    break;
  case CXXStdlibKind::Unknown:
    // FIXME: we should probably emit a warning or a note here.
    break;
  }

  // Do not try to link Cxx with itself.
  if (mainModuleName != CXX_MODULE_NAME)
    RegistrationCallback(
        LinkLibrary{"languageCxx", LibraryKind::Library, hasStaticCxx});

  // Do not try to link CxxStdlib with the C++ standard library, Cxx or
  // itself.
  if (toolchain::none_of(toolchain::ArrayRef<StringRef>{CXX_MODULE_NAME, "CxxStdlib", "std"},
                    [mainModuleName](StringRef Name) {
                      return mainModuleName == Name;
                    })) {
    // Only link with CxxStdlib on platforms where the overlay is available.
    if (Target.isOSDarwin() || Target.isOSLinux() || Target.isOSWindows() ||
        Target.isOSFreeBSD())
      RegistrationCallback(LinkLibrary{"languageCxxStdlib", LibraryKind::Library,
                                       hasStaticCxxStdlib});
  }
}

void
language::dependencies::registerBackDeployLibraries(
    const IRGenOptions &IRGenOpts,
    std::function<void(const LinkLibrary&)> RegistrationCallback) {
  auto addBackDeployLib = [&](toolchain::VersionTuple version,
                              StringRef libraryName, bool forceLoad) {
    std::optional<toolchain::VersionTuple> compatibilityVersion;
    if (libraryName == "languageCompatibilityDynamicReplacements") {
      compatibilityVersion = IRGenOpts.
          AutolinkRuntimeCompatibilityDynamicReplacementLibraryVersion;
    } else if (libraryName == "languageCompatibilityConcurrency") {
      compatibilityVersion =
          IRGenOpts.AutolinkRuntimeCompatibilityConcurrencyLibraryVersion;
    } else {
      compatibilityVersion = IRGenOpts.
          AutolinkRuntimeCompatibilityLibraryVersion;
    }

    if (!compatibilityVersion)
      return;

    if (*compatibilityVersion > version)
      return;

    RegistrationCallback(
        {libraryName, LibraryKind::Library, /*static=*/true, forceLoad});
  };

#define BACK_DEPLOYMENT_LIB(Version, Filter, LibraryName, ForceLoad) \
    addBackDeployLib(toolchain::VersionTuple Version, LibraryName, ForceLoad);
  #include "language/Frontend/BackDeploymentLibs.def"
}

bool CodiraDependencyScanningService::setupCachingDependencyScanningService(
    CompilerInstance &Instance) {
  if (!Instance.getInvocation().getCASOptions().EnableCaching)
    return false;

  if (CASOpts) {
    // If CASOption matches, the service is initialized already.
    if (*CASOpts == Instance.getInvocation().getCASOptions().CASOpts)
      return false;

    // CASOption mismatch, return error.
    Instance.getDiags().diagnose(SourceLoc(), diag::error_cas_conflict_options);
    return true;
  }

  // Setup CAS.
  CASOpts = Instance.getInvocation().getCASOptions().CASOpts;

  ClangScanningService.emplace(
      clang::tooling::dependencies::ScanningMode::DependencyDirectivesScan,
      clang::tooling::dependencies::ScanningOutputFormat::FullIncludeTree,
      Instance.getInvocation().getCASOptions().CASOpts,
      Instance.getSharedCASInstance(), Instance.getSharedCacheInstance(),
      /*CachingOnDiskFileSystem=*/nullptr,
      // The current working directory optimization (off by default)
      // should not impact CAS. We set the optization to all to be
      // consistent with the non-CAS case.
      clang::tooling::dependencies::ScanningOptimizations::All);

  return false;
}

ModuleDependenciesCache::ModuleDependenciesCache(
    const std::string &mainScanModuleName, const std::string &scannerContextHash)
    : mainScanModuleName(mainScanModuleName),
      scannerContextHash(scannerContextHash),
      scanInitializationTime(std::chrono::system_clock::now()) {
  for (auto kind = ModuleDependencyKind::FirstKind;
       kind != ModuleDependencyKind::LastKind; ++kind)
    ModuleDependenciesMap.insert({kind, ModuleNameToDependencyMap()});
}

const ModuleNameToDependencyMap &
ModuleDependenciesCache::getDependenciesMap(
    ModuleDependencyKind kind) const {
  auto it = ModuleDependenciesMap.find(kind);
  assert(it != ModuleDependenciesMap.end() &&
         "invalid dependency kind");
  return it->second;
}

ModuleNameToDependencyMap &
ModuleDependenciesCache::getDependenciesMap(
    ModuleDependencyKind kind) {
  auto it = ModuleDependenciesMap.find(kind);
  assert(it != ModuleDependenciesMap.end() &&
         "invalid dependency kind");
  return it->second;
}

std::optional<const ModuleDependencyInfo *>
ModuleDependenciesCache::findDependency(
    const ModuleDependencyID moduleID) const {
  return findDependency(moduleID.ModuleName, moduleID.Kind);
}

std::optional<const ModuleDependencyInfo *>
ModuleDependenciesCache::findDependency(
    StringRef moduleName, std::optional<ModuleDependencyKind> kind) const {
  if (!kind) {
    for (auto kind = ModuleDependencyKind::FirstKind;
         kind != ModuleDependencyKind::LastKind; ++kind) {
      auto dep = findDependency(moduleName, kind);
      if (dep.has_value())
        return dep.value();
    }
    return std::nullopt;
  }

  assert(kind.has_value() && "Expected dependencies kind for lookup.");
  std::optional<const ModuleDependencyInfo *> optionalDep = std::nullopt;
  const auto &map = getDependenciesMap(kind.value());
  auto known = map.find(moduleName);
  if (known != map.end())
    optionalDep = &(known->second);

  // During a scan, only produce the cached source module info for the current
  // module under scan.
  if (optionalDep.has_value()) {
    auto dep = optionalDep.value();
    if (dep->getAsCodiraSourceModule() &&
        moduleName != mainScanModuleName &&
        moduleName != "MainModuleCrossImportOverlays") {
      return std::nullopt;
    }
  }

  return optionalDep;
}

std::optional<const ModuleDependencyInfo *>
ModuleDependenciesCache::findCodiraDependency(StringRef moduleName) const {
  if (auto found = findDependency(moduleName, ModuleDependencyKind::CodiraInterface))
    return found;
  if (auto found = findDependency(moduleName, ModuleDependencyKind::CodiraBinary))
    return found;
  if (auto found = findDependency(moduleName, ModuleDependencyKind::CodiraSource))
    return found;
  return std::nullopt;
}

const ModuleDependencyInfo &ModuleDependenciesCache::findKnownDependency(
    const ModuleDependencyID &moduleID) const {
  
  auto dep = findDependency(moduleID);
  assert(dep && "dependency unknown");
  return **dep;
}

bool ModuleDependenciesCache::hasDependency(const ModuleDependencyID &moduleID) const {
  return hasDependency(moduleID.ModuleName, moduleID.Kind);
}

bool ModuleDependenciesCache::hasDependency(
    StringRef moduleName, std::optional<ModuleDependencyKind> kind) const {
  return findDependency(moduleName, kind).has_value();
}

bool ModuleDependenciesCache::hasDependency(StringRef moduleName) const {
  for (auto kind = ModuleDependencyKind::FirstKind;
       kind != ModuleDependencyKind::LastKind; ++kind)
    if (findDependency(moduleName, kind).has_value())
      return true;
  return false;
}

bool ModuleDependenciesCache::hasCodiraDependency(StringRef moduleName) const {
  return findCodiraDependency(moduleName).has_value();
}

void ModuleDependenciesCache::recordDependency(
    StringRef moduleName, ModuleDependencyInfo dependency) {
  auto dependenciesKind = dependency.getKind();
  auto &map = getDependenciesMap(dependenciesKind);
  map.insert({moduleName, dependency});
}

void ModuleDependenciesCache::recordDependencies(
    ModuleDependencyVector dependencies, DiagnosticEngine &diags) {
  for (const auto &dep : dependencies) {
    if (hasDependency(dep.first)) {
      if (dep.first.Kind == ModuleDependencyKind::Clang) {
        auto priorClangModuleDetails =
            findKnownDependency(dep.first).getAsClangModule();
        auto newClangModuleDetails = dep.second.getAsClangModule();
        auto priorContextHash = priorClangModuleDetails->contextHash;
        auto newContextHash = newClangModuleDetails->contextHash;
        if (priorContextHash != newContextHash) {
          // This situation means that within the same scanning action, Clang
          // Dependency Scanner has produced two different variants of the same
          // module. This is not supposed to happen, but we are currently
          // hunting down the rare cases where it does, seemingly due to
          // differences in Clang Scanner direct by-name queries and transitive
          // header lookup queries.
          //
          // Emit a failure diagnostic here that is hopefully more actionable
          // for the time being.
          diags.diagnose(SourceLoc(), diag::dependency_scan_unexpected_variant,
                         dep.first.ModuleName);
          diags.diagnose(
              SourceLoc(),
              diag::dependency_scan_unexpected_variant_context_hash_note,
              priorContextHash, newContextHash);
          diags.diagnose(
              SourceLoc(),
              diag::dependency_scan_unexpected_variant_module_map_note,
              priorClangModuleDetails->moduleMapFile,
              newClangModuleDetails->moduleMapFile);

          auto diagnoseExtraCommandLineFlags =
              [&diags](const ClangModuleDependencyStorage *checkModuleDetails,
                       const ClangModuleDependencyStorage *baseModuleDetails,
                       bool isNewlyDiscovered) -> void {
            std::unordered_set<std::string> baseCommandLineSet(
                baseModuleDetails->buildCommandLine.begin(),
                baseModuleDetails->buildCommandLine.end());
            for (const auto &checkArg : checkModuleDetails->buildCommandLine)
              if (baseCommandLineSet.find(checkArg) == baseCommandLineSet.end())
                diags.diagnose(
                    SourceLoc(),
                    diag::dependency_scan_unexpected_variant_extra_arg_note,
                    isNewlyDiscovered, checkArg);
          };
          diagnoseExtraCommandLineFlags(priorClangModuleDetails,
                                        newClangModuleDetails, true);
          diagnoseExtraCommandLineFlags(newClangModuleDetails,
                                        priorClangModuleDetails, false);
        }
      }
    } else
      recordDependency(dep.first.ModuleName, dep.second);

    if (dep.first.Kind == ModuleDependencyKind::Clang) {
      auto clangModuleDetails = dep.second.getAsClangModule();
      addSeenClangModule(clang::tooling::dependencies::ModuleID{
          dep.first.ModuleName, clangModuleDetails->contextHash});
    }
  }
}

void ModuleDependenciesCache::updateDependency(
    ModuleDependencyID moduleID, ModuleDependencyInfo dependencyInfo) {
  auto &map = getDependenciesMap(moduleID.Kind);
  assert(map.find(moduleID.ModuleName) != map.end() && "Not yet added to map");
  map.insert_or_assign(moduleID.ModuleName, std::move(dependencyInfo));
}

void ModuleDependenciesCache::removeDependency(ModuleDependencyID moduleID) {
  auto &map = getDependenciesMap(moduleID.Kind);
  map.erase(moduleID.ModuleName);
}

void
ModuleDependenciesCache::setImportedCodiraDependencies(ModuleDependencyID moduleID,
                                                      const ArrayRef<ModuleDependencyID> dependencyIDs) {
  auto dependencyInfo = findKnownDependency(moduleID);
  assert(dependencyInfo.getImportedCodiraDependencies().empty());
#ifndef NDEBUG
  for (const auto &depID : dependencyIDs)
    assert(depID.Kind != ModuleDependencyKind::Clang);
#endif
  // Copy the existing info to a mutable one we can then replace it with, after setting its overlay dependencies.
  auto updatedDependencyInfo = dependencyInfo;
  updatedDependencyInfo.setImportedCodiraDependencies(dependencyIDs);
  updateDependency(moduleID, updatedDependencyInfo);
}
void
ModuleDependenciesCache::setImportedClangDependencies(ModuleDependencyID moduleID,
                                                      const ArrayRef<ModuleDependencyID> dependencyIDs) {
  auto dependencyInfo = findKnownDependency(moduleID);
  assert(dependencyInfo.getImportedClangDependencies().empty());
#ifndef NDEBUG
  for (const auto &depID : dependencyIDs)
    assert(depID.Kind == ModuleDependencyKind::Clang);
#endif
  // Copy the existing info to a mutable one we can then replace it with, after setting its overlay dependencies.
  auto updatedDependencyInfo = dependencyInfo;
  updatedDependencyInfo.setImportedClangDependencies(dependencyIDs);
  updateDependency(moduleID, updatedDependencyInfo);
}
void
ModuleDependenciesCache::setHeaderClangDependencies(ModuleDependencyID moduleID,
                                                    const ArrayRef<ModuleDependencyID> dependencyIDs) {
  auto dependencyInfo = findKnownDependency(moduleID);
#ifndef NDEBUG
  for (const auto &depID : dependencyIDs)
    assert(depID.Kind == ModuleDependencyKind::Clang);
#endif
  // Copy the existing info to a mutable one we can then replace it with, after setting its overlay dependencies.
  auto updatedDependencyInfo = dependencyInfo;
  updatedDependencyInfo.setHeaderClangDependencies(dependencyIDs);
  updateDependency(moduleID, updatedDependencyInfo);
}
void ModuleDependenciesCache::setCodiraOverlayDependencies(ModuleDependencyID moduleID,
                                                          const ArrayRef<ModuleDependencyID> dependencyIDs) {
  auto dependencyInfo = findKnownDependency(moduleID);
  assert(dependencyInfo.getCodiraOverlayDependencies().empty());
#ifndef NDEBUG
  for (const auto &depID : dependencyIDs)
    assert(depID.Kind != ModuleDependencyKind::Clang);
#endif
  // Copy the existing info to a mutable one we can then replace it with, after setting its overlay dependencies.
  auto updatedDependencyInfo = dependencyInfo;
  updatedDependencyInfo.setCodiraOverlayDependencies(dependencyIDs);
  updateDependency(moduleID, updatedDependencyInfo);
}
void
ModuleDependenciesCache::setCrossImportOverlayDependencies(ModuleDependencyID moduleID,
                                                           const ArrayRef<ModuleDependencyID> dependencyIDs) {
  auto dependencyInfo = findKnownDependency(moduleID);
  assert(dependencyInfo.getCrossImportOverlayDependencies().empty());
  // Copy the existing info to a mutable one we can then replace it with, after setting its overlay dependencies.
  auto updatedDependencyInfo = dependencyInfo;
  updatedDependencyInfo.setCrossImportOverlayDependencies(dependencyIDs);
  updateDependency(moduleID, updatedDependencyInfo);
}

ModuleDependencyIDSetVector
ModuleDependenciesCache::getAllDependencies(const ModuleDependencyID &moduleID) const {
  const auto &moduleInfo = findKnownDependency(moduleID);
  ModuleDependencyIDSetVector result;
  if (moduleInfo.isCodiraModule()) {
    auto languageImportedDepsRef = moduleInfo.getImportedCodiraDependencies();
    auto headerClangDepsRef = moduleInfo.getHeaderClangDependencies();
    auto overlayDependenciesRef = moduleInfo.getCodiraOverlayDependencies();
    result.insert(languageImportedDepsRef.begin(),
                  languageImportedDepsRef.end());
    result.insert(headerClangDepsRef.begin(),
                  headerClangDepsRef.end());
    result.insert(overlayDependenciesRef.begin(),
                  overlayDependenciesRef.end());
  }

  if (moduleInfo.isCodiraSourceModule()) {
    auto crossImportOverlayDepsRef = moduleInfo.getCrossImportOverlayDependencies();
    result.insert(crossImportOverlayDepsRef.begin(),
                  crossImportOverlayDepsRef.end());
  }

  auto clangImportedDepsRef = moduleInfo.getImportedClangDependencies();
  result.insert(clangImportedDepsRef.begin(),
                clangImportedDepsRef.end());

  return result;
}

ModuleDependencyIDSetVector
ModuleDependenciesCache::getClangDependencies(const ModuleDependencyID &moduleID) const {
  const auto &moduleInfo = findKnownDependency(moduleID);
  ModuleDependencyIDSetVector result;
  auto clangImportedDepsRef = moduleInfo.getImportedClangDependencies();
  result.insert(clangImportedDepsRef.begin(),
                clangImportedDepsRef.end());
  if (moduleInfo.isCodiraSourceModule() || moduleInfo.isCodiraBinaryModule()) {
    auto headerClangDepsRef = moduleInfo.getHeaderClangDependencies();
    result.insert(headerClangDepsRef.begin(),
                  headerClangDepsRef.end());
  }
  return result;
}

toolchain::ArrayRef<ModuleDependencyID>
ModuleDependenciesCache::getImportedCodiraDependencies(const ModuleDependencyID &moduleID) const {
  const auto &moduleInfo = findKnownDependency(moduleID);
  assert(moduleInfo.isCodiraModule());
  return moduleInfo.getImportedCodiraDependencies();
}

toolchain::ArrayRef<ModuleDependencyID>
ModuleDependenciesCache::getImportedClangDependencies(const ModuleDependencyID &moduleID) const {
  const auto &moduleInfo = findKnownDependency(moduleID);
  return moduleInfo.getImportedClangDependencies();
}

toolchain::ArrayRef<ModuleDependencyID>
ModuleDependenciesCache::getHeaderClangDependencies(const ModuleDependencyID &moduleID) const {
  const auto &moduleInfo = findKnownDependency(moduleID);
  assert(moduleInfo.isCodiraModule());
  return moduleInfo.getHeaderClangDependencies();
}

toolchain::ArrayRef<ModuleDependencyID>
ModuleDependenciesCache::getCodiraOverlayDependencies(const ModuleDependencyID &moduleID) const {
  const auto &moduleInfo = findKnownDependency(moduleID);
  assert(moduleInfo.isCodiraModule());
  return moduleInfo.getCodiraOverlayDependencies();
}

toolchain::ArrayRef<ModuleDependencyID>
ModuleDependenciesCache::getCrossImportOverlayDependencies(const ModuleDependencyID &moduleID) const {
  const auto &moduleInfo = findKnownDependency(moduleID);
  assert(moduleInfo.isCodiraSourceModule());
  return moduleInfo.getCrossImportOverlayDependencies();
}

