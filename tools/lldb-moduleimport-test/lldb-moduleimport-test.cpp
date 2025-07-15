//===--- lldb-moduleimport-test.cpp - LLDB moduleimport tester ------------===//
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
// This program simulates LLDB importing modules from the __apple_ast
// section in Mach-O files. We use it to test for regressions in the
// deserialization API.
//
//===----------------------------------------------------------------------===//

#include "language/ABI/ObjectFile.h"
#include "language/AST/ASTDemangler.h"
#include "language/AST/PrintOptions.h"
#include "language/ASTSectionImporter/ASTSectionImporter.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/Basic/InitializeCodiraModules.h"
#include "language/Frontend/Frontend.h"
#include "language/Serialization/SerializedModuleLoader.h"
#include "language/Serialization/Validation.h"
#include "toolchain/Object/COFF.h"
#include "toolchain/Object/ELFObjectFile.h"
#include "toolchain/Object/MachO.h"
#include "toolchain/Object/ObjectFile.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/ManagedStatic.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/PrettyStackTrace.h"
#include "toolchain/Support/Signals.h"
#include "toolchain/Support/TargetSelect.h"
#include "toolchain/Support/raw_ostream.h"
#include <fstream>
#include <sstream>

void anchorForGetMainExecutable() {}

using namespace toolchain::MachO;

static bool validateModule(
    toolchain::StringRef data, bool Verbose, bool requiresOSSAModules,
    language::serialization::ValidationInfo &info,
    language::serialization::ExtendedValidationInfo &extendedInfo,
    toolchain::SmallVectorImpl<language::serialization::SearchPath> &searchPaths) {
  info = language::serialization::validateSerializedAST(
      data, requiresOSSAModules,
      /*requiredSDK*/ StringRef(), &extendedInfo, /* dependencies*/ nullptr,
      &searchPaths);
  if (info.status != language::serialization::Status::Valid) {
    toolchain::outs() << "error: validateSerializedAST() failed\n";
    return false;
  }

  language::CompilerInvocation CI;
  if (CI.loadFromSerializedAST(data) != language::serialization::Status::Valid) {
    toolchain::outs() << "error: loadFromSerializedAST() failed\n";
    return false;
  }
  CI.getLangOptions().EnableDeserializationSafety = false;

  if (Verbose) {
    if (!info.shortVersion.empty())
      toolchain::outs() << "- Codira Version: " << info.shortVersion << "\n";
    toolchain::outs() << "- Compatibility Version: "
                 << CI.getLangOptions()
                        .EffectiveLanguageVersion.asAPINotesVersionString()
                 << "\n";
    toolchain::outs() << "- Target: " << info.targetTriple << "\n";
    if (!extendedInfo.getSDKPath().empty())
      toolchain::outs() << "- SDK path: " << extendedInfo.getSDKPath() << "\n";
    if (!extendedInfo.getExtraClangImporterOptions().empty()) {
      toolchain::outs() << "- -Xcc options:";
      for (toolchain::StringRef option : extendedInfo.getExtraClangImporterOptions())
        toolchain::outs() << " " << option;
      toolchain::outs() << "\n";
    }
    toolchain::outs() << "- Search Paths:\n";
    for (auto searchPath : searchPaths) {
      toolchain::outs() << "    Path: " << searchPath.Path;
      toolchain::outs() << ", framework="
                   << (searchPath.IsFramework ? "true" : "false");
      toolchain::outs() << ", system=" << (searchPath.IsSystem ? "true" : "false")
                   << "\n";
    }
    toolchain::outs() << "- Plugin Search Options:\n";
    for (auto opt : extendedInfo.getPluginSearchOptions()) {
      StringRef optStr;
      switch (opt.first) {
      case language::PluginSearchOption::Kind::PluginPath:
        optStr = "-plugin-path";
        break;
      case language::PluginSearchOption::Kind::ExternalPluginPath:
        optStr = "-external-plugin-path";
        break;
      case language::PluginSearchOption::Kind::LoadPluginLibrary:
        optStr = "-load-plugin-library";
        break;
      case language::PluginSearchOption::Kind::LoadPluginExecutable:
        optStr = "-load-plugin-executable";
        break;
      case language::PluginSearchOption::Kind::ResolvedPluginConfig:
        optStr = "-load-resolved-plugin";
        break;
      }
      toolchain::outs() << "    " << optStr << " " << opt.second << "\n";
    }
  }

  return true;
}

static void resolveDeclFromMangledNameList(
    language::ASTContext &Ctx, toolchain::ArrayRef<std::string> MangledNames) {
  for (auto &Mangled : MangledNames) {
    language::TypeDecl *ResolvedDecl =
        language::Demangle::getTypeDeclForMangling(Ctx, Mangled);
    if (!ResolvedDecl) {
      toolchain::errs() << "Can't resolve decl of " << Mangled << "\n";
    } else {
      ResolvedDecl->dumpRef(toolchain::outs());
      toolchain::outs() << "\n";
    }
  }
}

static void
resolveTypeFromMangledNameList(language::ASTContext &Ctx,
                               toolchain::ArrayRef<std::string> MangledNames,
                               bool QualifyTypes) {
  for (auto &Mangled : MangledNames) {
    language::Type ResolvedType =
        language::Demangle::getTypeForMangling(Ctx, Mangled);
    if (!ResolvedType) {
      toolchain::outs() << "Can't resolve type of " << Mangled << "\n";
    } else {
      language::PrintOptions PO;
      PO.FullyQualifiedTypesIfAmbiguous = QualifyTypes;
      PO.QualifyImportedTypes = QualifyTypes;
      PO.PrintStorageRepresentationAttrs = true;
      ResolvedType->print(toolchain::outs(), PO);
      toolchain::outs() << "\n";
    }
  }
}

static void
collectMangledNames(const std::string &FilePath,
                    toolchain::SmallVectorImpl<std::string> &MangledNames) {
  std::string Name;
  std::ifstream InputStream(FilePath);
  while (std::getline(InputStream, Name)) {
    if (Name.empty())
      continue;
    MangledNames.push_back(Name);
  }
}

toolchain::BumpPtrAllocator Alloc;

static bool
collectASTModules(toolchain::cl::list<std::string> &InputNames,
                  toolchain::SmallVectorImpl<std::pair<char *, uint64_t>> &Modules) {
  for (auto &name : InputNames) {
    auto OF = toolchain::object::ObjectFile::createObjectFile(name);
    if (!OF) {
      toolchain::outs() << "error: " << name << " "
                   << errorToErrorCode(OF.takeError()).message() << "\n";
      return false;
    }
    auto *Obj = OF->getBinary();
    auto *MachO = toolchain::dyn_cast<toolchain::object::MachOObjectFile>(Obj);
    auto *ELF = toolchain::dyn_cast<toolchain::object::ELFObjectFileBase>(Obj);
    auto *COFF = toolchain::dyn_cast<toolchain::object::COFFObjectFile>(Obj);

    if (MachO) {
      for (auto &Symbol : Obj->symbols()) {
        auto RawSym = Symbol.getRawDataRefImpl();
        toolchain::MachO::nlist nlist = MachO->getSymbolTableEntry(RawSym);
        if (nlist.n_type != N_AST)
          continue;
        auto Path = MachO->getSymbolName(RawSym);
        if (!Path) {
          toolchain::outs() << "Cannot get symbol name\n;";
          return false;
        }

        auto fileBuf = toolchain::MemoryBuffer::getFile(*Path);
        if (!fileBuf) {
          toolchain::outs() << "Cannot read from '" << *Path
                       << "': " << fileBuf.getError().message();
          return false;
        }

        uint64_t Size = fileBuf.get()->getBufferSize();
        char *Module = Alloc.Allocate<char>(Size);
        std::memcpy(Module, (void *)fileBuf.get()->getBufferStart(), Size);
        Modules.push_back({Module, Size});
      }
    }

    for (auto &Section : Obj->sections()) {
      toolchain::Expected<toolchain::StringRef> NameOrErr = Section.getName();
      if (!NameOrErr) {
        toolchain::consumeError(NameOrErr.takeError());
        continue;
      }
      toolchain::StringRef Name = *NameOrErr;
      if ((MachO && Name == language::CodiraObjectFileFormatMachO().getSectionName(
                                language::ReflectionSectionKind::languageast)) ||
          (ELF && Name == language::CodiraObjectFileFormatELF().getSectionName(
                              language::ReflectionSectionKind::languageast)) ||
          (COFF && Name == language::CodiraObjectFileFormatCOFF().getSectionName(
                               language::ReflectionSectionKind::languageast))) {
        uint64_t Size = Section.getSize();

        toolchain::Expected<toolchain::StringRef> ContentsReference =
            Section.getContents();
        if (!ContentsReference) {
          toolchain::errs() << "error: " << name << " "
                       << errorToErrorCode(OF.takeError()).message() << "\n";
          return false;
        }
        char *Module = Alloc.Allocate<char>(Size);
        std::memcpy(Module, (void *)ContentsReference->begin(), Size);
        Modules.push_back({Module, Size});
      }
    }
  }
  return true;
}

int main(int argc, char **argv) {
  PROGRAM_START(argc, argv);
  INITIALIZE_LLVM();
  initializeCodiraModules();

  // Command line handling.
  using namespace toolchain::cl;
  static OptionCategory Visible("Specific Options");
  HideUnrelatedOptions({&Visible});

  list<std::string> InputNames(Positional, desc("compiled_language_file1.o ..."),
                               OneOrMore, cat(Visible));

  opt<bool> DumpModule(
      "dump-module",
      desc("Dump the imported module after checking it imports just fine"),
      cat(Visible));

  opt<bool> Verbose("verbose", desc("Dump information on the loaded module"),
                    cat(Visible));

  opt<std::string> Filter("filter", desc("triple for filtering modules"),
                          cat(Visible));

  opt<std::string> ModuleCachePath(
      "module-cache-path", desc("Clang module cache path"), cat(Visible));

  opt<std::string> DumpDeclFromMangled(
      "decl-from-mangled", desc("dump decl from mangled names list"),
      cat(Visible));

  opt<std::string> DumpTypeFromMangled(
      "type-from-mangled", desc("dump type from mangled names list"),
      cat(Visible));

  opt<std::string> ResourceDir(
      "resource-dir",
      desc("The directory that holds the compiler resource files"),
      cat(Visible));

  opt<bool> DummyDWARFImporter(
      "dummy-dwarfimporter",
      desc("Install a dummy DWARFImporterDelegate"), cat(Visible));

  opt<bool> QualifyTypes("qualify-types", desc("Qualify dumped types"),
                         cat(Visible));

  opt<bool> EnableOSSAModules("enable-ossa-modules", init(false),
                              desc("Serialize modules in OSSA"), cat(Visible));

  ParseCommandLineOptions(argc, argv);

  // Unregister our options so they don't interfere with the command line
  // parsing in CodeGen/BackendUtil.cpp.
  ModuleCachePath.removeArgument();
  DumpModule.removeArgument();
  DumpTypeFromMangled.removeArgument();
  InputNames.removeArgument();

  auto validateInputFile = [](std::string Filename) {
    if (Filename.empty())
      return true;
    if (!toolchain::sys::fs::exists(toolchain::Twine(Filename))) {
      toolchain::errs() << Filename << " does not exist, exiting.\n";
      return false;
    }
    if (!toolchain::sys::fs::is_regular_file(toolchain::Twine(Filename))) {
      toolchain::errs() << Filename << " is not a regular file, exiting.\n";
      return false;
    }
    return true;
  };

  if (!validateInputFile(DumpTypeFromMangled))
    return 1;
  if (!validateInputFile(DumpDeclFromMangled))
    return 1;

  // Fetch the serialized module bitstreams from the Mach-O files and
  // register them with the module loader.
  toolchain::SmallVector<std::pair<char *, uint64_t>, 8> Modules;
  if (!collectASTModules(InputNames, Modules))
    return 1;

  if (Modules.empty())
    return 0;

  language::serialization::ValidationInfo info;
  language::serialization::ExtendedValidationInfo extendedInfo;
  toolchain::SmallVector<language::serialization::SearchPath> searchPaths;
  for (auto &Module : Modules) {
    info = {};
    extendedInfo = {};
    if (!validateModule(StringRef(Module.first, Module.second), Verbose,
                        EnableOSSAModules,
                        info, extendedInfo, searchPaths)) {
      toolchain::errs() << "Malformed module!\n";
      return 1;
    }
  }

  // Create a Codira compiler.
  toolchain::SmallVector<std::string, 4> modules;
  language::CompilerInstance CI;
  language::CompilerInvocation Invocation;

  Invocation.setMainExecutablePath(
      toolchain::sys::fs::getMainExecutable(argv[0],
          reinterpret_cast<void *>(&anchorForGetMainExecutable)));

  // Infer SDK and Target triple from the module.
  if (!extendedInfo.getSDKPath().empty())
    Invocation.setSDKPath(extendedInfo.getSDKPath().str());
  Invocation.setTargetTriple(info.targetTriple);

  Invocation.setModuleName("lldbtest");
  Invocation.getClangImporterOptions().ModuleCachePath = ModuleCachePath;
  Invocation.getLangOptions().EnableMemoryBufferImporter = true;
  Invocation.getSILOptions().EnableOSSAModules = EnableOSSAModules;

  if (!ResourceDir.empty()) {
    Invocation.setRuntimeResourcePath(ResourceDir);
  }

  std::string InstanceSetupError;
  if (CI.setup(Invocation, InstanceSetupError)) {
    toolchain::errs() << InstanceSetupError << '\n';
    return 1;
  }

  language::DWARFImporterDelegate dummyDWARFImporter;
  if (DummyDWARFImporter) {
    auto *ClangImporter = static_cast<language::ClangImporter *>(
        CI.getASTContext().getClangModuleLoader());
    ClangImporter->setDWARFImporterDelegate(dummyDWARFImporter);
 }

  if (Verbose)
    CI.getASTContext().SetPreModuleImportCallback(
        [&](toolchain::StringRef module_name,
            language::ASTContext::ModuleImportKind kind) {
          switch (kind) {
          case language::ASTContext::Module:
            toolchain::outs() << "Loading " << module_name.str() << "\n";
            break;
          case language::ASTContext::Overlay:
            toolchain::outs() << "Loading (overlay) " << module_name.str() << "\n";
            break;
          case language::ASTContext::BridgingHeader:
            toolchain::outs() << "Compiling bridging header: " << module_name.str()
                         << "\n";
            break;
          }
        });

  toolchain::SmallString<0> error;
  toolchain::raw_svector_ostream errs(error);
  toolchain::Triple filter(Filter);
  for (auto &Module : Modules) {
    auto Result = parseASTSection(
        *CI.getMemoryBufferSerializedModuleLoader(),
        StringRef(Module.first, Module.second), filter);
    if (auto E = Result.takeError()) {
      std::string error = toString(std::move(E));
      toolchain::errs() << "error: Failed to parse AST section! " << error << "\n";
      return 1;
    }
    modules.insert(modules.end(), Result->begin(), Result->end());
  }

  // Attempt to import all modules we found.
  for (auto path : modules) {
    if (Verbose)
      toolchain::outs() << "Importing " << path << "...\n";

    language::ImportPath::Module::Builder modulePath;
#ifdef LANGUAGE_SUPPORTS_SUBMODULES
    for (auto i = toolchain::sys::path::begin(path);
         i != toolchain::sys::path::end(path); ++i)
      if (!toolchain::sys::path::is_separator((*i)[0]))
          modulePath.push_back(CI.getASTContext().getIdentifier(*i));
#else
    modulePath.push_back(CI.getASTContext().getIdentifier(path));
#endif

    auto Module = CI.getASTContext().getModule(modulePath.get());
    if (!Module) {
      if (Verbose)
        toolchain::errs() << "FAIL!\n";
      return 1;
    }
    if (Verbose)
      toolchain::outs() << "Import successful!\n";
    if (DumpModule) {
      toolchain::SmallVector<language::Decl*, 10> Decls;
      Module->getTopLevelDecls(Decls);
      for (auto Decl : Decls)
        Decl->dump(toolchain::outs());
    }
    if (!DumpTypeFromMangled.empty()) {
      toolchain::SmallVector<std::string, 8> MangledNames;
      collectMangledNames(DumpTypeFromMangled, MangledNames);
      resolveTypeFromMangledNameList(CI.getASTContext(), MangledNames,
                                     QualifyTypes);
    }
    if (!DumpDeclFromMangled.empty()) {
      toolchain::SmallVector<std::string, 8> MangledNames;
      collectMangledNames(DumpDeclFromMangled, MangledNames);
      resolveDeclFromMangledNameList(CI.getASTContext(), MangledNames);
    }
  }
  return 0;
}
