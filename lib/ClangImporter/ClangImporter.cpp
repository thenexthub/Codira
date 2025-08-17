//===--- ClangImporter.cpp - Import Clang Modules -------------------------===//
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
// This file implements support for loading Clang modules into Codira.
//
//===----------------------------------------------------------------------===//
#include "language/ClangImporter/ClangImporter.h"
#include "CFTypeInfo.h"
#include "ClangDerivedConformances.h"
#include "ClangDiagnosticConsumer.h"
#include "ClangIncludePaths.h"
#include "ImporterImpl.h"
#include "CodiraDeclSynthesizer.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Builtins.h"
#include "language/AST/ClangModuleLoader.h"
#include "language/AST/ConcreteDeclRef.h"
#include "language/AST/Decl.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsClangImporter.h"
#include "language/AST/DiagnosticsSema.h"
#include "language/AST/Evaluator.h"
#include "language/AST/IRGenOptions.h"
#include "language/AST/ImportCache.h"
#include "language/AST/LinkLibrary.h"
#include "language/AST/Module.h"
#include "language/AST/ModuleNameLookup.h"
#include "language/AST/NameLookup.h"
#include "language/AST/NameLookupRequests.h"
#include "language/AST/PrettyStackTrace.h"
#include "language/AST/SourceFile.h"
#include "language/AST/Type.h"
#include "language/AST/TypeCheckRequests.h"
#include "language/AST/Types.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/Platform.h"
#include "language/Basic/Range.h"
#include "language/Basic/SourceLoc.h"
#include "language/Basic/StringExtras.h"
#include "language/Basic/Version.h"
#include "language/ClangImporter/ClangImporterRequests.h"
#include "language/ClangImporter/ClangModule.h"
#include "language/Frontend/CompileJobCacheKey.h"
#include "language/Parse/ParseVersion.h"
#include "language/Strings.h"
#include "language/Subsystems.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclBase.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/DeclTemplate.h"
#include "language/Core/AST/Mangle.h"
#include "language/Core/AST/TemplateBase.h"
#include "language/Core/AST/Type.h"
#include "language/Core/Basic/DiagnosticOptions.h"
#include "language/Core/Basic/FileEntry.h"
#include "language/Core/Basic/IdentifierTable.h"
#include "language/Core/Basic/LangStandard.h"
#include "language/Core/Basic/MacroBuilder.h"
#include "language/Core/Basic/Module.h"
#include "language/Core/Basic/Specifiers.h"
#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/CAS/CASOptions.h"
#include "language/Core/CAS/IncludeTree.h"
#include "language/Core/CodeGen/ObjectFilePCHContainerWriter.h"
#include "language/Core/Frontend/CompilerInvocation.h"
#include "language/Core/Frontend/FrontendActions.h"
#include "language/Core/Frontend/FrontendOptions.h"
#include "language/Core/Frontend/IncludeTreePPActions.h"
#include "language/Core/Frontend/TextDiagnosticPrinter.h"
#include "language/Core/Frontend/Utils.h"
#include "language/Core/Index/IndexingAction.h"
#include "language/Core/Lex/Preprocessor.h"
#include "language/Core/Lex/PreprocessorOptions.h"
#include "language/Core/Parse/Parser.h"
#include "language/Core/Rewrite/Frontend/Rewriters.h"
#include "language/Core/Sema/DelayedDiagnostic.h"
#include "language/Core/Sema/Lookup.h"
#include "language/Core/Sema/Sema.h"
#include "language/Core/Serialization/ASTReader.h"
#include "language/Core/Serialization/ASTWriter.h"
#include "language/Core/Serialization/ObjectFilePCHContainerReader.h"
#include "language/Core/Tooling/DependencyScanning/ModuleDepCollector.h"
#include "language/Core/Tooling/DependencyScanning/ScanAndUpdateArgs.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/TypeSwitch.h"
#include "toolchain/CAS/CASReference.h"
#include "toolchain/CAS/ObjectStore.h"
#include "toolchain/Support/Casting.h"
#include "toolchain/Support/CrashRecoveryContext.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/FileCollector.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Memory.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/PrefixMapper.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include "toolchain/TextAPI/InterfaceFile.h"
#include "toolchain/TextAPI/TextAPIReader.h"
#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <utility>

using namespace language;
using namespace importer;

// Commonly-used Clang classes.
using language::Core::CompilerInstance;
using language::Core::CompilerInvocation;

#pragma mark Internal data structures

namespace {
  class HeaderImportCallbacks : public language::Core::PPCallbacks {
    ClangImporter::Implementation &Impl;
  public:
    HeaderImportCallbacks(ClangImporter::Implementation &impl)
      : Impl(impl) {}

    void handleImport(const language::Core::Module *imported) {
      if (!imported)
        return;
      Impl.ImportedHeaderExports.push_back(
          const_cast<language::Core::Module *>(imported));
    }

    void InclusionDirective(
        language::Core::SourceLocation HashLoc, const language::Core::Token &IncludeTok,
        StringRef FileName, bool IsAngled, language::Core::CharSourceRange FilenameRange,
        language::Core::OptionalFileEntryRef File, StringRef SearchPath,
        StringRef RelativePath, const language::Core::Module *SuggestedModule,
        bool ModuleImported,
        language::Core::SrcMgr::CharacteristicKind FileType) override {
      handleImport(ModuleImported ? SuggestedModule : nullptr);
    }

    void moduleImport(language::Core::SourceLocation ImportLoc,
                              language::Core::ModuleIdPath Path,
                              const language::Core::Module *Imported) override {
      handleImport(Imported);
    }
  };

  class PCHDeserializationCallbacks : public language::Core::ASTDeserializationListener {
    ClangImporter::Implementation &Impl;
  public:
    explicit PCHDeserializationCallbacks(ClangImporter::Implementation &impl)
      : Impl(impl) {}
    void ModuleImportRead(language::Core::serialization::SubmoduleID ID,
                          language::Core::SourceLocation ImportLoc) override {
      if (Impl.IsReadingBridgingPCH) {
        Impl.PCHImportedSubmodules.push_back(ID);
      }
    }
  };

  class HeaderParsingASTConsumer : public language::Core::ASTConsumer {
    SmallVector<language::Core::DeclGroupRef, 4> DeclGroups;
    PCHDeserializationCallbacks PCHCallbacks;
  public:
    explicit HeaderParsingASTConsumer(ClangImporter::Implementation &impl)
      : PCHCallbacks(impl) {}
    void
    HandleTopLevelDeclInObjCContainer(language::Core::DeclGroupRef decls) override {
      DeclGroups.push_back(decls);
    }

    ArrayRef<language::Core::DeclGroupRef> getAdditionalParsedDecls() {
      return DeclGroups;
    }

    language::Core::ASTDeserializationListener *GetASTDeserializationListener() override {
      return &PCHCallbacks;
    }

    void reset() {
      DeclGroups.clear();
    }
  };

  class ParsingAction : public language::Core::ASTFrontendAction {
    ClangImporter &Importer;
    ClangImporter::Implementation &Impl;
    const ClangImporterOptions &ImporterOpts;
    std::string CodiraPCHHash;
  public:
    explicit ParsingAction(ClangImporter &importer,
                           ClangImporter::Implementation &impl,
                           const ClangImporterOptions &importerOpts,
                           std::string languagePCHHash)
      : Importer(importer), Impl(impl), ImporterOpts(importerOpts),
        CodiraPCHHash(languagePCHHash) {}
    std::unique_ptr<language::Core::ASTConsumer>
    CreateASTConsumer(language::Core::CompilerInstance &CI, StringRef InFile) override {
      return std::make_unique<HeaderParsingASTConsumer>(Impl);
    }
    bool BeginSourceFileAction(language::Core::CompilerInstance &CI) override {
      auto PCH =
          Importer.getOrCreatePCH(ImporterOpts, CodiraPCHHash, /*Cached=*/true);
      if (PCH.has_value()) {
        Impl.getClangInstance()->getPreprocessorOpts().ImplicitPCHInclude =
            PCH.value();
        Impl.IsReadingBridgingPCH = true;
        Impl.setSinglePCHImport(PCH.value());
      }

      return true;
    }
  };

  class StdStringMemBuffer : public toolchain::MemoryBuffer {
    const std::string storage;
    const std::string name;
  public:
    StdStringMemBuffer(std::string &&source, StringRef name)
        : storage(std::move(source)), name(name.str()) {
      init(storage.data(), storage.data() + storage.size(),
           /*null-terminated=*/true);
    }

    StringRef getBufferIdentifier() const override {
      return name;
    }

    BufferKind getBufferKind() const override {
      return MemoryBuffer_Malloc;
    }
  };

  class ZeroFilledMemoryBuffer : public toolchain::MemoryBuffer {
    const std::string name;
  public:
    explicit ZeroFilledMemoryBuffer(size_t size, StringRef name)
        : name(name.str()) {
      assert(size > 0);
      std::error_code error;
      toolchain::sys::MemoryBlock memory =
          toolchain::sys::Memory::allocateMappedMemory(size, nullptr,
                                                  toolchain::sys::Memory::MF_READ,
                                                  error);
      assert(!error && "failed to allocated read-only zero-filled memory");
      init(static_cast<char *>(memory.base()),
           static_cast<char *>(memory.base()) + memory.allocatedSize() - 1,
           /*null-terminated*/true);
    }

    ~ZeroFilledMemoryBuffer() override {
      toolchain::sys::MemoryBlock memory{const_cast<char *>(getBufferStart()),
        getBufferSize()};
      std::error_code error = toolchain::sys::Memory::releaseMappedMemory(memory);
      assert(!error && "failed to deallocate read-only zero-filled memory");
      (void)error;
    }

    ZeroFilledMemoryBuffer(const ZeroFilledMemoryBuffer &) = delete;
    ZeroFilledMemoryBuffer(ZeroFilledMemoryBuffer &&) = delete;
    void operator=(const ZeroFilledMemoryBuffer &) = delete;
    void operator=(ZeroFilledMemoryBuffer &&) = delete;

    StringRef getBufferIdentifier() const override {
      return name;
    }
    BufferKind getBufferKind() const override {
      return MemoryBuffer_MMap;
    }
  };
} // end anonymous namespace

namespace {
class BridgingPPTracker : public language::Core::PPCallbacks {
  ClangImporter::Implementation &Impl;

public:
  BridgingPPTracker(ClangImporter::Implementation &Impl)
    : Impl(Impl) {}

private:
  static unsigned getNumModuleIdentifiers(const language::Core::Module *Mod) {
    unsigned Result = 1;
    while (Mod->Parent) {
      Mod = Mod->Parent;
      ++Result;
    }
    return Result;
  }

  void InclusionDirective(language::Core::SourceLocation HashLoc,
                          const language::Core::Token &IncludeTok, StringRef FileName,
                          bool IsAngled, language::Core::CharSourceRange FilenameRange,
                          language::Core::OptionalFileEntryRef File,
                          StringRef SearchPath, StringRef RelativePath,
                          const language::Core::Module *SuggestedModule,
                          bool ModuleImported,
                          language::Core::SrcMgr::CharacteristicKind FileType) override {
    if (!ModuleImported) {
      if (File)
        Impl.BridgeHeaderFiles.insert(*File);
      return;
    }
    // Synthesize identifier locations.
    SmallVector<language::Core::SourceLocation, 4> IdLocs;
    for (unsigned I = 0, E = getNumModuleIdentifiers(SuggestedModule); I != E; ++I)
      IdLocs.push_back(HashLoc);
    handleImport(HashLoc, IdLocs, SuggestedModule);
  }

  void moduleImport(language::Core::SourceLocation ImportLoc,
                    language::Core::ModuleIdPath Path,
                    const language::Core::Module *Imported) override {
    if (!Imported)
      return;
    SmallVector<language::Core::SourceLocation, 4> IdLocs;
    for (auto &P : Path)
      IdLocs.push_back(P.second);
    handleImport(ImportLoc, IdLocs, Imported);
  }

  void handleImport(language::Core::SourceLocation ImportLoc,
                    ArrayRef<language::Core::SourceLocation> IdLocs,
                    const language::Core::Module *Imported) {
    language::Core::ASTContext &ClangCtx = Impl.getClangASTContext();
    language::Core::ImportDecl *ClangImport = language::Core::ImportDecl::Create(ClangCtx,
                                            ClangCtx.getTranslationUnitDecl(),
                                            ImportLoc,
                                           const_cast<language::Core::Module*>(Imported),
                                            IdLocs);
    Impl.BridgeHeaderTopLevelImports.push_back(ClangImport);
  }

  void MacroDefined(const language::Core::Token &MacroNameTok,
                    const language::Core::MacroDirective *MD) override {
    Impl.BridgeHeaderMacros.push_back(MacroNameTok.getIdentifierInfo());
  }
};

class ClangImporterDependencyCollector : public language::Core::DependencyCollector
{
  toolchain::StringSet<> ExcludedPaths;
  /// The FileCollector is used by LLDB to generate reproducers. It's not used
  /// by Codira to track dependencies.
  std::shared_ptr<toolchain::FileCollectorBase> FileCollector;
  const IntermoduleDepTrackingMode Mode;

public:
  ClangImporterDependencyCollector(
      IntermoduleDepTrackingMode Mode,
      std::shared_ptr<toolchain::FileCollectorBase> FileCollector)
      : FileCollector(FileCollector), Mode(Mode) {}

  void excludePath(StringRef filename) {
    ExcludedPaths.insert(filename);
  }

  bool isClangImporterSpecialName(StringRef Filename) {
    using ImporterImpl = ClangImporter::Implementation;
    return (Filename == ImporterImpl::moduleImportBufferName
            || Filename == ImporterImpl::bridgingHeaderBufferName);
  }

  bool needSystemDependencies() override {
    return Mode == IntermoduleDepTrackingMode::IncludeSystem;
  }

  bool sawDependency(StringRef Filename, bool FromClangModule,
                     bool IsSystem, bool IsClangModuleFile,
                     bool IsMissing) override {
    if (!language::Core::DependencyCollector::sawDependency(Filename, FromClangModule,
                                                   IsSystem, IsClangModuleFile,
                                                   IsMissing))
      return false;
    // Currently preserving older ClangImporter behavior of ignoring .pcm
    // file dependencies, but possibly revisit?
    if (IsClangModuleFile
        || isClangImporterSpecialName(Filename)
        || ExcludedPaths.count(Filename))
      return false;
    return true;
  }

  void maybeAddDependency(StringRef Filename, bool FromModule, bool IsSystem,
                          bool IsModuleFile, bool IsMissing) override {
    if (FileCollector)
      FileCollector->addFile(Filename);
    language::Core::DependencyCollector::maybeAddDependency(
        Filename, FromModule, IsSystem, IsModuleFile, IsMissing);
  }
};
} // end anonymous namespace

std::shared_ptr<language::Core::DependencyCollector>
ClangImporter::createDependencyCollector(
    IntermoduleDepTrackingMode Mode,
    std::shared_ptr<toolchain::FileCollectorBase> FileCollector) {
  return std::make_shared<ClangImporterDependencyCollector>(Mode,
                                                            FileCollector);
}

bool ClangImporter::isKnownCFTypeName(toolchain::StringRef name) {
  return CFPointeeInfo::isKnownCFTypeName(name);
}

void ClangImporter::Implementation::addBridgeHeaderTopLevelDecls(
    language::Core::Decl *D) {
  if (shouldIgnoreBridgeHeaderTopLevelDecl(D))
    return;

  BridgeHeaderTopLevelDecls.push_back(D);
}

bool importer::isForwardDeclOfType(const language::Core::Decl *D) {
  if (auto *ID = dyn_cast<language::Core::ObjCInterfaceDecl>(D)) {
    if (!ID->isThisDeclarationADefinition())
      return true;
  } else if (auto PD = dyn_cast<language::Core::ObjCProtocolDecl>(D)) {
    if (!PD->isThisDeclarationADefinition())
      return true;
  } else if (auto TD = dyn_cast<language::Core::TagDecl>(D)) {
    if (!TD->isThisDeclarationADefinition())
      return true;
  }
  return false;
}

bool ClangImporter::Implementation::shouldIgnoreBridgeHeaderTopLevelDecl(
    language::Core::Decl *D) {
  return importer::isForwardDeclOfType(D);
}

ClangImporter::ClangImporter(ASTContext &ctx,
                             DependencyTracker *tracker,
                             DWARFImporterDelegate *dwarfImporterDelegate)
    : ClangModuleLoader(tracker),
      Impl(*new Implementation(ctx, tracker, dwarfImporterDelegate)) {
}

ClangImporter::~ClangImporter() {
  delete &Impl;
}

#pragma mark Module loading

static bool clangSupportsPragmaAttributeWithCodiraAttr() {
  language::Core::AttributeCommonInfo languageAttrInfo(language::Core::SourceRange(),
     language::Core::AttributeCommonInfo::AT_CodiraAttr,
     language::Core::AttributeCommonInfo::Form::GNU());
  auto languageAttrParsedInfo = language::Core::ParsedAttrInfo::get(languageAttrInfo);
  return languageAttrParsedInfo.IsSupportedByPragmaAttribute;
}

static inline bool isPCHFilenameExtension(StringRef path) {
  return toolchain::sys::path::extension(path)
    .ends_with(file_types::getExtension(file_types::TY_PCH));
}

void importer::getNormalInvocationArguments(
    std::vector<std::string> &invocationArgStrs, ASTContext &ctx,
    bool ignoreClangTarget) {
  const auto &LangOpts = ctx.LangOpts;
  toolchain::Triple triple = LangOpts.Target;
  // Use clang specific target triple if given.
  if (LangOpts.ClangTarget.has_value() && !ignoreClangTarget) {
    triple = LangOpts.ClangTarget.value();
  }
  auto canonicalTriple = getCanonicalTriple(triple);
  if (canonicalTriple.has_value() &&
      !areTriplesStrictlyEqual(*canonicalTriple, triple))
    triple = *canonicalTriple;

  SearchPathOptions &searchPathOpts = ctx.SearchPathOpts;
  ClangImporterOptions &importerOpts = ctx.ClangImporterOpts;
  auto languageVersion = ctx.LangOpts.EffectiveLanguageVersion;

  auto bridgingPCH = importerOpts.getPCHInputPath();
  if (!bridgingPCH.empty())
    invocationArgStrs.insert(invocationArgStrs.end(),
                             {"-include-pch", bridgingPCH});

  // If there are no shims in the resource dir, add a search path in the SDK.
  SmallString<128> shimsPath(searchPathOpts.RuntimeResourcePath);
  toolchain::sys::path::append(shimsPath, "shims");
  if (!toolchain::sys::fs::exists(shimsPath)) {
    shimsPath = searchPathOpts.getSDKPath();
    toolchain::sys::path::append(shimsPath, "usr", "lib", "language", "shims");
    invocationArgStrs.insert(invocationArgStrs.end(),
                             {"-isystem", std::string(shimsPath.str())});
  }

  // Construct the invocation arguments for the current target.
  // Add target-independent options first.
  invocationArgStrs.insert(invocationArgStrs.end(), {
      // Don't emit LLVM IR.
      "-fsyntax-only",

      // Enable block support.
      "-fblocks",

      languageVersion.preprocessorDefinition("__language__", {10000, 100, 1}),

      "-fretain-comments-from-system-headers",

      "-isystem", searchPathOpts.RuntimeResourcePath,
  });

  if (LangOpts.hasFeature(Feature::Embedded)) {
    invocationArgStrs.insert(invocationArgStrs.end(), {"-D__language_embedded__"});
  }

  // Enable Position Independence.  `-fPIC` is not supported on Windows, which
  // is implicitly position independent.
  if (!triple.isOSWindows())
    invocationArgStrs.insert(invocationArgStrs.end(), {"-fPIC"});

  // Enable modules.
  invocationArgStrs.insert(invocationArgStrs.end(), {
      "-fmodules",
      "-Xclang", "-fmodule-feature", "-Xclang", "language"
  });

  bool EnableCXXInterop = LangOpts.EnableCXXInterop;

  if (LangOpts.EnableObjCInterop) {
    invocationArgStrs.insert(invocationArgStrs.end(), {"-fobjc-arc"});
    // TODO: Investigate whether 7.0 is a suitable default version.
    if (!triple.isOSDarwin())
      invocationArgStrs.insert(invocationArgStrs.end(),
                               {"-fobjc-runtime=ios-7.0"});

    invocationArgStrs.insert(invocationArgStrs.end(), {
      "-x", EnableCXXInterop ? "objective-c++" : "objective-c",
    });
  } else {
    invocationArgStrs.insert(invocationArgStrs.end(), {
      "-x", EnableCXXInterop ? "c++" : "c",
    });
  }

  {
    const language::Core::LangStandard &stdcxx =
#if defined(CLANG_DEFAULT_STD_CXX)
        *language::Core::LangStandard::getLangStandardForName(CLANG_DEFAULT_STD_CXX);
#else
        language::Core::LangStandard::getLangStandardForKind(
            language::Core::LangStandard::lang_gnucxx17);
#endif

    const language::Core::LangStandard &stdc =
#if defined(CLANG_DEFAULT_STD_C)
        *language::Core::LangStandard::getLangStandardForName(CLANG_DEFAULT_STD_C);
#else
        language::Core::LangStandard::getLangStandardForKind(
            language::Core::LangStandard::lang_gnu11);
#endif

    invocationArgStrs.insert(invocationArgStrs.end(), {
      (Twine("-std=") + StringRef(EnableCXXInterop ? stdcxx.getName()
                                                   : stdc.getName())).str()
    });
  }

  if (LangOpts.EnableCXXInterop) {
    if (auto path = getCxxShimModuleMapPath(searchPathOpts, LangOpts, triple)) {
      invocationArgStrs.push_back((Twine("-fmodule-map-file=") + *path).str());
    }
  }

  if (LangOpts.hasFeature(Feature::SafeInteropWrappers))
    invocationArgStrs.push_back("-fexperimental-bounds-safety-attributes");

  // Set C language options.
  if (triple.isOSDarwin()) {
    invocationArgStrs.insert(invocationArgStrs.end(), {
      // Avoid including the iso646.h header because some headers from OS X
      // frameworks are broken by it.
      "-D_ISO646_H_", "-D__ISO646_H",

      // Request new APIs from AppKit.
      "-DLANGUAGE_SDK_OVERLAY_APPKIT_EPOCH=2",

      // Request new APIs from Foundation.
      "-DLANGUAGE_SDK_OVERLAY_FOUNDATION_EPOCH=8",

      // Request new APIs from SceneKit.
      "-DLANGUAGE_SDK_OVERLAY2_SCENEKIT_EPOCH=3",

      // Request new APIs from GameplayKit.
      "-DLANGUAGE_SDK_OVERLAY_GAMEPLAYKIT_EPOCH=1",

      // Request new APIs from SpriteKit.
      "-DLANGUAGE_SDK_OVERLAY_SPRITEKIT_EPOCH=1",

      // Request new APIs from CoreImage.
      "-DLANGUAGE_SDK_OVERLAY_COREIMAGE_EPOCH=2",

      // Request new APIs from libdispatch.
      "-DLANGUAGE_SDK_OVERLAY_DISPATCH_EPOCH=2",

      // Request new APIs from libpthread
      "-DLANGUAGE_SDK_OVERLAY_PTHREAD_EPOCH=1",

      // Request new APIs from CoreGraphics.
      "-DLANGUAGE_SDK_OVERLAY_COREGRAPHICS_EPOCH=0",

      // Request new APIs from UIKit.
      "-DLANGUAGE_SDK_OVERLAY_UIKIT_EPOCH=2",

      // Backwards compatibility for headers that were checking this instead of
      // '__language__'.
      "-DLANGUAGE_CLASS_EXTRA=",
    });

    // Indicate that using '__attribute__((language_attr))' with '@Sendable' and
    // '@_nonSendable' on Clang declarations is fully supported, including the
    // 'attribute push' pragma.
    if (clangSupportsPragmaAttributeWithCodiraAttr())
      invocationArgStrs.push_back("-D__LANGUAGE_ATTR_SUPPORTS_SENDABLE_DECLS=1");

    if (triple.isXROS()) {
      // FIXME: This is a gnarly hack until some macros get adjusted in the SDK.
      invocationArgStrs.insert(invocationArgStrs.end(), {
        "-DOS_OBJECT_HAVE_OBJC_SUPPORT=1",
      });
    }

    // Get the version of this compiler and pass it to C/Objective-C
    // declarations.
    auto V = version::getCurrentCompilerVersion();
    if (!V.empty()) {
      // Note: Prior to Codira 5.7, the "Y" version component was omitted and the
      // "X" component resided in its digits.
      invocationArgStrs.insert(invocationArgStrs.end(), {
        V.preprocessorDefinition("__LANGUAGE_COMPILER_VERSION",
                                 {1000000000000,   // X
                                     1000000000,   // Y
                                        1000000,   // Z
                                           1000,   // a
                                              1}), // b
      });
    }
  } else {
    // Ideally we should turn this on for all Glibc targets that are actually
    // using Glibc or a libc that respects that flag. This will cause some
    // source breakage however (specifically with strerror_r()) on Linux
    // without a workaround.
    if (triple.isOSFuchsia() || triple.isAndroid() || triple.isMusl()) {
      // Many of the modern libc features are hidden behind feature macros like
      // _GNU_SOURCE or _XOPEN_SOURCE.
      invocationArgStrs.insert(invocationArgStrs.end(), {
        "-D_GNU_SOURCE",
      });
    }

    if (triple.isOSWindows()) {
      switch (triple.getArch()) {
      default: toolchain_unreachable("unsupported Windows architecture");
      case toolchain::Triple::arm:
      case toolchain::Triple::thumb:
        invocationArgStrs.insert(invocationArgStrs.end(), {"-D_ARM_"});
        break;
      case toolchain::Triple::aarch64:
      case toolchain::Triple::aarch64_32:
        invocationArgStrs.insert(invocationArgStrs.end(), {"-D_ARM64_"});
        break;
      case toolchain::Triple::x86:
        invocationArgStrs.insert(invocationArgStrs.end(), {"-D_X86_"});
        break;
      case toolchain::Triple::x86_64:
        invocationArgStrs.insert(invocationArgStrs.end(), {"-D_AMD64_"});
        break;
      }
    }
  }

  if (LangOpts.UseStaticStandardLibrary)
    invocationArgStrs.push_back("-DLANGUAGE_STATIC_STDLIB");

  // If we support SendingArgsAndResults, set the -D flag to signal that it
  // is supported.
  if (LangOpts.hasFeature(Feature::SendingArgsAndResults))
    invocationArgStrs.push_back("-D__LANGUAGE_ATTR_SUPPORTS_SENDING=1");

  // Indicate that the compiler will respect macros applied to imported
  // declarations via '__attribute__((language_attr("@...")))'.
  if (LangOpts.hasFeature(Feature::MacrosOnImports))
    invocationArgStrs.push_back("-D__LANGUAGE_ATTR_SUPPORTS_MACROS=1");

  if (searchPathOpts.getSDKPath().empty()) {
    invocationArgStrs.push_back("-Xclang");
    invocationArgStrs.push_back("-nostdsysteminc");
  } else {
    if (triple.isWindowsMSVCEnvironment()) {
      toolchain::SmallString<261> path; // MAX_PATH + 1
      path = searchPathOpts.getSDKPath();
      toolchain::sys::path::append(path, "usr", "include");
      toolchain::sys::path::native(path);

      invocationArgStrs.push_back("-isystem");
      invocationArgStrs.push_back(std::string(path.str()));
    } else {
      // On Darwin, Clang uses -isysroot to specify the include
      // system root. On other targets, it seems to use --sysroot.
      if (triple.isOSDarwin()) {
        invocationArgStrs.push_back("-isysroot");
        invocationArgStrs.push_back(searchPathOpts.getSDKPath().str());
      } else {
        if (auto sysroot = searchPathOpts.getSysRoot()) {
          invocationArgStrs.push_back("--sysroot");
          invocationArgStrs.push_back(sysroot->str());
        } else {
          invocationArgStrs.push_back("--sysroot");
          invocationArgStrs.push_back(searchPathOpts.getSDKPath().str());
        }
      }
    }
  }

  const std::string &moduleCachePath = importerOpts.ModuleCachePath;
  const std::string &scannerCachePath = importerOpts.ClangScannerModuleCachePath;
  // If a scanner cache is specified, this must be a scanning action. Prefer this
  // path for the Clang scanner to cache its Scanning PCMs.
  if (!scannerCachePath.empty()) {
    invocationArgStrs.push_back("-fmodules-cache-path=");
    invocationArgStrs.back().append(scannerCachePath);
  } else if (!moduleCachePath.empty() && !importerOpts.DisableImplicitClangModules) {
    invocationArgStrs.push_back("-fmodules-cache-path=");
    invocationArgStrs.back().append(moduleCachePath);
  }

  if (importerOpts.DisableImplicitClangModules) {
    invocationArgStrs.push_back("-fno-implicit-modules");
    invocationArgStrs.push_back("-fno-implicit-module-maps");
  }

  if (ctx.SearchPathOpts.DisableModulesValidateSystemDependencies) {
    invocationArgStrs.push_back("-fno-modules-validate-system-headers");
  } else {
    invocationArgStrs.push_back("-fmodules-validate-system-headers");
  }

  if (importerOpts.DetailedPreprocessingRecord) {
    invocationArgStrs.insert(invocationArgStrs.end(), {
      "-Xclang", "-detailed-preprocessing-record",
      "-Xclang", "-fmodule-format=raw",
    });
  } else {
    invocationArgStrs.insert(invocationArgStrs.end(), {
      "-Xclang", "-fmodule-format=obj",
    });
  }

  // Enable API notes alongside headers/in frameworks.
  invocationArgStrs.push_back("-fapinotes-modules");
  invocationArgStrs.push_back("-fapinotes-language-version=" +
                              languageVersion.asAPINotesVersionString());

  // Prefer `-sdk` paths.
  if (!searchPathOpts.getSDKPath().empty()) {
    toolchain::SmallString<261> path{searchPathOpts.getSDKPath()};
    toolchain::sys::path::append(path, "usr", "lib", "language", "apinotes");

    invocationArgStrs.push_back("-iapinotes-modules");
    invocationArgStrs.push_back(path.str().str());
  }

  // Fallback to "legacy" `-resource-dir` paths.
  {
    toolchain::SmallString<261> path{searchPathOpts.RuntimeResourcePath};
    toolchain::sys::path::append(path, "apinotes");

    invocationArgStrs.push_back("-iapinotes-modules");
    invocationArgStrs.push_back(path.str().str());
  }
}

static void
getEmbedBitcodeInvocationArguments(std::vector<std::string> &invocationArgStrs,
                                   ASTContext &ctx) {
  invocationArgStrs.insert(invocationArgStrs.end(), {
    // Backend mode.
    "-fembed-bitcode",

    // ...but Clang isn't doing the emission.
    "-fsyntax-only",

    "-x", "ir",
  });
}

void
importer::addCommonInvocationArguments(
    std::vector<std::string> &invocationArgStrs,
    ASTContext &ctx, bool requiresBuiltinHeadersInSystemModules,
    bool ignoreClangTarget) {
  using ImporterImpl = ClangImporter::Implementation;
  toolchain::Triple triple = ctx.LangOpts.Target;
  // Use clang specific target triple if given.
  if (ctx.LangOpts.ClangTarget.has_value() && !ignoreClangTarget) {
    triple = ctx.LangOpts.ClangTarget.value();
  }
  auto canonicalTriple = getCanonicalTriple(triple);
  if (canonicalTriple.has_value() &&
      !areTriplesStrictlyEqual(*canonicalTriple, triple))
    triple = *canonicalTriple;

  SearchPathOptions &searchPathOpts = ctx.SearchPathOpts;
  const ClangImporterOptions &importerOpts = ctx.ClangImporterOpts;

  invocationArgStrs.push_back("-target");
  invocationArgStrs.push_back(triple.str());

  if (ctx.LangOpts.SDKVersion) {
    invocationArgStrs.push_back("-Xclang");
    invocationArgStrs.push_back(
        "-target-sdk-version=" + ctx.LangOpts.SDKVersion->getAsString());
  }

  invocationArgStrs.push_back(ImporterImpl::moduleImportBufferName);

  if (ctx.LangOpts.EnableAppExtensionRestrictions) {
    invocationArgStrs.push_back("-fapplication-extension");
  }

  if (!importerOpts.TargetCPU.empty()) {
    switch (triple.getArch()) {
    case toolchain::Triple::x86:
    case toolchain::Triple::x86_64:
      // For x86, `-mcpu` is deprecated and an alias of `-mtune`. We need to
      // pass `-march` and `-mtune` to behave like `-mcpu` on other targets.
      invocationArgStrs.push_back("-march=" + importerOpts.TargetCPU);
      invocationArgStrs.push_back("-mtune=" + importerOpts.TargetCPU);
      break;
    default:
      invocationArgStrs.push_back("-mcpu=" + importerOpts.TargetCPU);
      break;
    }
  } else if (triple.getArch() == toolchain::Triple::systemz) {
    invocationArgStrs.push_back("-march=z13");
  }

  if (triple.getArch() == toolchain::Triple::x86_64) {
    // Enable double wide atomic intrinsics on every x86_64 target.
    // (This is the default on Darwin, but not so on other platforms.)
    invocationArgStrs.push_back("-mcx16");
  }

  if (triple.isOSDarwin()) {
    if (auto variantTriple = ctx.LangOpts.TargetVariant) {
      // Passing the -target-variant along to clang causes clang's
      // CodeGenerator to emit zippered .o files.
      invocationArgStrs.push_back("-darwin-target-variant");
      if (ctx.LangOpts.ClangTargetVariant.has_value() && !ignoreClangTarget)
        variantTriple = ctx.LangOpts.ClangTargetVariant.value();

      auto canonicalVariantTriple = getCanonicalTriple(*variantTriple);
      if (canonicalVariantTriple.has_value() &&
          !areTriplesStrictlyEqual(*canonicalVariantTriple, *variantTriple))
        *variantTriple = *canonicalVariantTriple;

      invocationArgStrs.push_back(variantTriple->str());
    }

    if (ctx.LangOpts.VariantSDKVersion) {
      invocationArgStrs.push_back("-Xclang");
      invocationArgStrs.push_back(
        ("-darwin-target-variant-sdk-version=" +
         ctx.LangOpts.VariantSDKVersion->getAsString()));
    }
  }

  if (std::optional<StringRef> R = searchPathOpts.getWinSDKRoot()) {
    invocationArgStrs.emplace_back("-Xmicrosoft-windows-sdk-root");
    invocationArgStrs.emplace_back(*R);
  }
  if (std::optional<StringRef> V = searchPathOpts.getWinSDKVersion()) {
    invocationArgStrs.emplace_back("-Xmicrosoft-windows-sdk-version");
    invocationArgStrs.emplace_back(*V);
  }
  if (std::optional<StringRef> R = searchPathOpts.getVCToolsRoot()) {
    invocationArgStrs.emplace_back("-Xmicrosoft-visualc-tools-root");
    invocationArgStrs.emplace_back(*R);
  }
  if (std::optional<StringRef> V = searchPathOpts.getVCToolsVersion()) {
    invocationArgStrs.emplace_back("-Xmicrosoft-visualc-tools-version");
    invocationArgStrs.emplace_back(*V);
  }

  if (!importerOpts.Optimization.empty()) {
    invocationArgStrs.push_back(importerOpts.Optimization);
  }

  const std::string &overrideResourceDir = importerOpts.OverrideResourceDir;
  if (overrideResourceDir.empty()) {
    toolchain::SmallString<128> resourceDir(searchPathOpts.RuntimeResourcePath);

    // Adjust the path to refer to our copy of the Clang resource directory
    // under 'lib/language/clang', which is either a real resource directory or a
    // symlink to one inside of a full Clang installation.
    //
    // The rationale for looking under the Codira resource directory and not
    // assuming that the Clang resource directory is located next to it is that
    // Codira, when installed separately, should not need to install files in
    // directories that are not "owned" by it.
    toolchain::sys::path::append(resourceDir, "clang");

    // Set the Clang resource directory to the path we computed.
    invocationArgStrs.push_back("-resource-dir");
    invocationArgStrs.push_back(std::string(resourceDir.str()));
  } else {
    invocationArgStrs.push_back("-resource-dir");
    invocationArgStrs.push_back(overrideResourceDir);
  }

  if (!importerOpts.IndexStorePath.empty()) {
    invocationArgStrs.push_back("-index-store-path");
    invocationArgStrs.push_back(importerOpts.IndexStorePath);
  }

  invocationArgStrs.push_back("-fansi-escape-codes");

  if (importerOpts.ValidateModulesOnce) {
    invocationArgStrs.push_back("-fmodules-validate-once-per-build-session");
    invocationArgStrs.push_back("-fbuild-session-file=" + importerOpts.BuildSessionFilePath);
  }

  for (auto extraArg : importerOpts.ExtraArgs) {
    invocationArgStrs.push_back(extraArg);
  }

  for (const auto &framepath : searchPathOpts.getFrameworkSearchPaths()) {
    if (!framepath.Path.empty()) {
      if (framepath.IsSystem) {
        invocationArgStrs.push_back("-iframework");
        invocationArgStrs.push_back(framepath.Path);
      } else {
        invocationArgStrs.push_back("-F" + framepath.Path);
      }
    }
  }

  for (const auto &path : searchPathOpts.getImportSearchPaths()) {
    if (!path.Path.empty()) {
      if (path.IsSystem) {
        invocationArgStrs.push_back("-isystem");
        invocationArgStrs.push_back(path.Path);
      } else {
        invocationArgStrs.push_back("-I" + path.Path);
      }
    }
  }

  for (auto &overlay : searchPathOpts.VFSOverlayFiles) {
    invocationArgStrs.push_back("-ivfsoverlay");
    invocationArgStrs.push_back(overlay);
  }

  if (requiresBuiltinHeadersInSystemModules) {
    invocationArgStrs.push_back("-Xclang");
    invocationArgStrs.push_back("-fbuiltin-headers-in-system-modules");
  }
}

bool ClangImporter::canReadPCH(StringRef PCHFilename) {
  if (!toolchain::sys::fs::exists(PCHFilename))
    return false;

  // FIXME: The following attempts to do an initial ReadAST invocation to verify
  // the PCH, without causing trouble for the existing CompilerInstance.
  // Look into combining creating the ASTReader along with verification + update
  // if necessary, so that we can create and use one ASTReader in the common case
  // when there is no need for update.
  language::Core::CompilerInstance CI(Impl.Instance->getPCHContainerOperations(),
                             &Impl.Instance->getModuleCache());
  auto invocation =
      std::make_shared<language::Core::CompilerInvocation>(*Impl.Invocation);
  invocation->getPreprocessorOpts().DisablePCHOrModuleValidation =
      language::Core::DisableValidationForModuleKind::None;
  invocation->getHeaderSearchOpts().ModulesValidateSystemHeaders = true;
  invocation->getLangOpts().NeededByPCHOrCompilationUsesPCH = true;
  invocation->getLangOpts().CacheGeneratedPCH = true;

  // ClangImporter::create adds a remapped MemoryBuffer that we don't need
  // here.  Moreover, it's a raw pointer owned by the preprocessor options; if
  // we don't clear the range then both the original and new CompilerInvocation
  // will try to free it.
  invocation->getPreprocessorOpts().RemappedFileBuffers.clear();

  CI.setInvocation(std::move(invocation));
  CI.setTarget(&Impl.Instance->getTarget());
  CI.setDiagnostics(
      &*language::Core::CompilerInstance::createDiagnostics(new language::Core::DiagnosticOptions()));

  // Note: Reusing the file manager is safe; this is a component that's already
  // reused when building PCM files for the module cache.
  CI.createSourceManager(Impl.Instance->getFileManager());
  auto &clangSrcMgr = CI.getSourceManager();
  auto FID = clangSrcMgr.createFileID(
                        std::make_unique<ZeroFilledMemoryBuffer>(1, "<main>"));
  clangSrcMgr.setMainFileID(FID);
  auto &diagConsumer = CI.getDiagnosticClient();
  diagConsumer.BeginSourceFile(CI.getLangOpts());
  LANGUAGE_DEFER {
    diagConsumer.EndSourceFile();
  };

  // Pass in TU_Complete, which is the default mode for the Preprocessor
  // constructor and the right one for reading a PCH.
  CI.createPreprocessor(language::Core::TU_Complete);
  CI.createASTContext();
  CI.createASTReader();
  language::Core::ASTReader &Reader = *CI.getASTReader();

  auto failureCapabilities =
    language::Core::ASTReader::ARR_Missing |
    language::Core::ASTReader::ARR_OutOfDate |
    language::Core::ASTReader::ARR_VersionMismatch;

  // If a PCH was output with errors, it may not have serialized all its
  // inputs. If there was a change to the search path or a headermap now
  // exists where it didn't previously, it's possible those inputs will now be
  // found. Ideally we would only rebuild in this particular case rather than
  // any error in general, but explicit module builds are the real solution
  // there. For now, just treat PCH with errors as out of date.
  failureCapabilities |= language::Core::ASTReader::ARR_TreatModuleWithErrorsAsOutOfDate;

  auto result = Reader.ReadAST(PCHFilename, language::Core::serialization::MK_PCH,
                               language::Core::SourceLocation(), failureCapabilities);
  switch (result) {
  case language::Core::ASTReader::Success:
    return true;
  case language::Core::ASTReader::Failure:
  case language::Core::ASTReader::Missing:
  case language::Core::ASTReader::OutOfDate:
  case language::Core::ASTReader::VersionMismatch:
    return false;
  case language::Core::ASTReader::ConfigurationMismatch:
  case language::Core::ASTReader::HadErrors:
    assert(0 && "unexpected ASTReader failure for PCH validation");
    return false;
  }
  toolchain_unreachable("unhandled result");
}

std::string ClangImporter::getOriginalSourceFile(StringRef PCHFilename) {
  return language::Core::ASTReader::getOriginalSourceFile(
      PCHFilename.str(), Impl.Instance->getFileManager(),
      Impl.Instance->getPCHContainerReader(), Impl.Instance->getDiagnostics());
}

std::optional<std::string>
ClangImporter::getPCHFilename(const ClangImporterOptions &ImporterOptions,
                              StringRef CodiraPCHHash, bool &isExplicit) {
  auto bridgingPCH = ImporterOptions.getPCHInputPath();
  if (!bridgingPCH.empty()) {
    isExplicit = true;
    return bridgingPCH;
  }
  isExplicit = false;

  const auto &BridgingHeader = ImporterOptions.BridgingHeader;
  const auto &PCHOutputDir = ImporterOptions.PrecompiledHeaderOutputDir;
  if (CodiraPCHHash.empty() || BridgingHeader.empty() || PCHOutputDir.empty()) {
    return std::nullopt;
  }

  SmallString<256> PCHBasename { toolchain::sys::path::filename(BridgingHeader) };
  toolchain::sys::path::replace_extension(PCHBasename, "");
  PCHBasename.append("-language_");
  PCHBasename.append(CodiraPCHHash);
  PCHBasename.append("-clang_");
  PCHBasename.append(getClangModuleHash());
  PCHBasename.append(".pch");
  SmallString<256> PCHFilename { PCHOutputDir };
  toolchain::sys::path::append(PCHFilename, PCHBasename);
  return PCHFilename.str().str();
}

std::optional<std::string>
ClangImporter::getOrCreatePCH(const ClangImporterOptions &ImporterOptions,
                              StringRef CodiraPCHHash, bool Cached) {
  bool isExplicit;
  auto PCHFilename = getPCHFilename(ImporterOptions, CodiraPCHHash,
                                    isExplicit);
  if (!PCHFilename.has_value()) {
    return std::nullopt;
  }
  if (!isExplicit && !ImporterOptions.PCHDisableValidation &&
      !canReadPCH(PCHFilename.value())) {
    StringRef parentDir = toolchain::sys::path::parent_path(PCHFilename.value());
    std::error_code EC = toolchain::sys::fs::create_directories(parentDir);
    if (EC) {
      toolchain::errs() << "failed to create directory '" << parentDir << "': "
        << EC.message();
      return std::nullopt;
    }
    auto FailedToEmit = emitBridgingPCH(ImporterOptions.BridgingHeader,
                                        PCHFilename.value(), Cached);
    if (FailedToEmit) {
      return std::nullopt;
    }
  }

  return PCHFilename.value();
}

std::vector<std::string>
ClangImporter::getClangDriverArguments(ASTContext &ctx, bool ignoreClangTarget) {
  assert(!ctx.ClangImporterOpts.DirectClangCC1ModuleBuild &&
         "direct-clang-cc1-module-build should not call this function");
  std::vector<std::string> invocationArgStrs;
  // When creating from driver commands, clang expects this to be like an actual
  // command line. So we need to pass in "clang" for argv[0]
  invocationArgStrs.push_back(ctx.ClangImporterOpts.clangPath);
  switch (ctx.ClangImporterOpts.Mode) {
  case ClangImporterOptions::Modes::Normal:
  case ClangImporterOptions::Modes::PrecompiledModule:
    getNormalInvocationArguments(invocationArgStrs, ctx, ignoreClangTarget);
    break;
  case ClangImporterOptions::Modes::EmbedBitcode:
    getEmbedBitcodeInvocationArguments(invocationArgStrs, ctx);
    break;
  }
  addCommonInvocationArguments(invocationArgStrs, ctx,
      requiresBuiltinHeadersInSystemModules, ignoreClangTarget);
  return invocationArgStrs;
}

std::optional<std::vector<std::string>> ClangImporter::getClangCC1Arguments(
    ASTContext &ctx, toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> VFS,
    bool ignoreClangTarget) {
  std::unique_ptr<language::Core::CompilerInvocation> CI;

  // Set up a temporary diagnostic client to report errors from parsing the
  // command line, which may be important for Codira clients if, for example,
  // they're using -Xcc options. Unfortunately this diagnostic engine has to
  // use the default options because the /actual/ options haven't been parsed
  // yet.
  //
  // The long-term client for Clang diagnostics is set up afterwards, after the
  // language::Core::CompilerInstance is created.
  toolchain::IntrusiveRefCntPtr<language::Core::DiagnosticOptions> tempDiagOpts{
      new language::Core::DiagnosticOptions};
  auto *tempDiagClient =
      new ClangDiagnosticConsumer(Impl, *tempDiagOpts,
                                  ctx.ClangImporterOpts.DumpClangDiagnostics);
  auto clangDiags = language::Core::CompilerInstance::createDiagnostics(
      tempDiagOpts.get(), tempDiagClient,
      /*owned*/ true);

  // If using direct cc1 module build, use extra args to setup ClangImporter.
  if (ctx.ClangImporterOpts.DirectClangCC1ModuleBuild) {
    toolchain::SmallVector<const char *> clangArgs;
    clangArgs.reserve(ctx.ClangImporterOpts.ExtraArgs.size());
    toolchain::for_each(
        ctx.ClangImporterOpts.ExtraArgs,
        [&](const std::string &Arg) { clangArgs.push_back(Arg.c_str()); });

    // Try parse extra args, if failed, return nullopt.
    CI = std::make_unique<language::Core::CompilerInvocation>();
    if (!language::Core::CompilerInvocation::CreateFromArgs(*CI, clangArgs,
                                                   *clangDiags))
      return std::nullopt;

    // Forwards some options from language to clang even using direct mode. This is
    // to reduce the number of argument passing on the command-line and language
    // compiler can be more efficient to compute language cache key without having
    // the knowledge about clang command-line options.
    if (ctx.CASOpts.EnableCaching || ctx.CASOpts.ImportModuleFromCAS) {
      CI->getCASOpts() = ctx.CASOpts.CASOpts;
      // When clangImporter is used to compile (generate .pcm or .pch), need to
      // inherit the include tree from language args (last one wins) and clear the
      // input file.
      if ((CI->getFrontendOpts().ProgramAction ==
               language::Core::frontend::ActionKind::GenerateModule ||
           CI->getFrontendOpts().ProgramAction ==
               language::Core::frontend::ActionKind::GeneratePCH) &&
          !ctx.CASOpts.ClangIncludeTree.empty()) {
        CI->getFrontendOpts().CASIncludeTreeID = ctx.CASOpts.ClangIncludeTree;
        CI->getFrontendOpts().Inputs.clear();
      }
    }

    // If clang target is ignored, using language target.
    if (ignoreClangTarget) {
      CI->getTargetOpts().Triple = ctx.LangOpts.Target.str();
      if (ctx.LangOpts.TargetVariant.has_value())
        CI->getTargetOpts().DarwinTargetVariantTriple = ctx.LangOpts.TargetVariant->str();
    }

    // Forward the index store path. That information is not passed to scanner
    // and it is cached invariant so we don't want to re-scan if that changed.
    CI->getFrontendOpts().IndexStorePath = ctx.ClangImporterOpts.IndexStorePath;
  } else {
    // Otherwise, create cc1 arguments from driver args.
    auto driverArgs = getClangDriverArguments(ctx, ignoreClangTarget);

    toolchain::SmallVector<const char *> invocationArgs;
    invocationArgs.reserve(driverArgs.size());
    toolchain::for_each(driverArgs, [&](const std::string &Arg) {
      invocationArgs.push_back(Arg.c_str());
    });

    if (ctx.ClangImporterOpts.DumpClangDiagnostics) {
      toolchain::errs() << "clang importer driver args: '";
      toolchain::interleave(
          invocationArgs, [](StringRef arg) { toolchain::errs() << arg; },
          [] { toolchain::errs() << "' '"; });
      toolchain::errs() << "'\n\n";
    }

    language::Core::CreateInvocationOptions CIOpts;
    CIOpts.VFS = VFS;
    CIOpts.Diags = clangDiags;
    CIOpts.RecoverOnError = false;
    CIOpts.ProbePrecompiled = true;
    CI = language::Core::createInvocation(invocationArgs, std::move(CIOpts));
    if (!CI)
      return std::nullopt;
  }

  // FIXME: clang fails to generate a module if there is a `-fmodule-map-file`
  // argument pointing to a missing file.
  // Such missing module files occur frequently in SourceKit. If the files are
  // missing, SourceKit fails to build CodiraShims (which wouldn't have required
  // the missing module file), thus fails to load the stdlib and hence looses
  // all semantic functionality.
  // To work around this issue, drop all `-fmodule-map-file` arguments pointing
  // to missing files and report the error that clang would throw manually.
  // rdar://77516546 is tracking that the clang importer should be more
  // resilient and provide a module even if there were building it.
  auto TempVFS = language::Core::createVFSFromCompilerInvocation(
      *CI, *clangDiags,
      VFS ? VFS : Impl.CodiraContext.SourceMgr.getFileSystem());

  std::vector<std::string> FilteredModuleMapFiles;
  for (auto ModuleMapFile : CI->getFrontendOpts().ModuleMapFiles) {
    if (ctx.CASOpts.HasImmutableFileSystem) {
      // There is no need to add any module map file here. Issue a warning and
      // drop the option.
      Impl.diagnose(SourceLoc(), diag::module_map_ignored, ModuleMapFile);
    } else if (TempVFS->exists(ModuleMapFile)) {
      FilteredModuleMapFiles.push_back(ModuleMapFile);
    } else {
      Impl.diagnose(SourceLoc(), diag::module_map_not_found, ModuleMapFile);
    }
  }
  CI->getFrontendOpts().ModuleMapFiles = FilteredModuleMapFiles;

  // Clear clang debug flags.
  CI->getCodeGenOpts().DwarfDebugFlags.clear();

  return CI->getCC1CommandLine();
}

std::unique_ptr<language::Core::CompilerInvocation> ClangImporter::createClangInvocation(
    ClangImporter *importer, const ClangImporterOptions &importerOpts,
    toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> VFS,
    const std::vector<std::string> &CC1Args) {
  std::vector<const char *> invocationArgs;
  invocationArgs.reserve(CC1Args.size());
  toolchain::for_each(CC1Args, [&](const std::string &Arg) {
    invocationArgs.push_back(Arg.c_str());
  });

  // Create a diagnostics engine for creating clang compiler invocation. The
  // option here is either generated by dependency scanner or just round tripped
  // from `getClangCC1Arguments` so we don't expect it to fail. Use a simple
  // printing diagnostics consumer for debugging any unexpected error.
  auto diagOpts = toolchain::makeIntrusiveRefCnt<language::Core::DiagnosticOptions>();
  language::Core::DiagnosticsEngine clangDiags(
      new language::Core::DiagnosticIDs(), diagOpts,
      new language::Core::TextDiagnosticPrinter(toolchain::errs(), diagOpts.get()));

  // Finally, use the CC1 command-line and the diagnostic engine
  // to instantiate our Invocation.
  auto CI = std::make_unique<language::Core::CompilerInvocation>();
  if (!language::Core::CompilerInvocation::CreateFromArgs(
          *CI, invocationArgs, clangDiags, importerOpts.clangPath.c_str()))
    return nullptr;

  return CI;
}

std::unique_ptr<ClangImporter>
ClangImporter::create(ASTContext &ctx,
                      std::string languagePCHHash, DependencyTracker *tracker,
                      DWARFImporterDelegate *dwarfImporterDelegate,
                      bool ignoreFileMapping) {
  std::unique_ptr<ClangImporter> importer{
      new ClangImporter(ctx, tracker, dwarfImporterDelegate)};
  auto &importerOpts = ctx.ClangImporterOpts;

  auto bridgingPCH = importerOpts.getPCHInputPath();
  if (!bridgingPCH.empty()) {
    importer->Impl.setSinglePCHImport(bridgingPCH);
    importer->Impl.IsReadingBridgingPCH = true;
    if (tracker) {
      // Currently ignoring dependency on bridging .pch files because they are
      // temporaries; if and when they are no longer temporaries, this condition
      // should be removed.
      auto &coll = static_cast<ClangImporterDependencyCollector &>(
        *tracker->getClangCollector());
      coll.excludePath(bridgingPCH);
    }
  }

  toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> VFS =
      ctx.SourceMgr.getFileSystem();

  ClangInvocationFileMapping fileMapping =
    applyClangInvocationMapping(ctx, nullptr, VFS, ignoreFileMapping);

  importer->requiresBuiltinHeadersInSystemModules =
      fileMapping.requiresBuiltinHeadersInSystemModules;

  // Create a new Clang compiler invocation.
  {
    if (auto ClangArgs = importer->getClangCC1Arguments(ctx, VFS))
      importer->Impl.ClangArgs = *ClangArgs;
    else
      return nullptr;

    ArrayRef<std::string> invocationArgStrs = importer->Impl.ClangArgs;
    if (importerOpts.DumpClangDiagnostics) {
      toolchain::errs() << "clang importer cc1 args: '";
      toolchain::interleave(
                       invocationArgStrs, [](StringRef arg) { toolchain::errs() << arg; },
                       [] { toolchain::errs() << "' '"; });
      toolchain::errs() << "'\n";
    }
    importer->Impl.Invocation = createClangInvocation(
        importer.get(), importerOpts, VFS, importer->Impl.ClangArgs);
    if (!importer->Impl.Invocation)
      return nullptr;
  }

  {
    // Create an almost-empty memory buffer.
    auto sourceBuffer = toolchain::MemoryBuffer::getMemBuffer(
      "extern int __language __attribute__((unavailable));",
      Implementation::moduleImportBufferName);
    language::Core::PreprocessorOptions &ppOpts =
        importer->Impl.Invocation->getPreprocessorOpts();
    ppOpts.addRemappedFile(Implementation::moduleImportBufferName,
                           sourceBuffer.release());
  }

  // Install a Clang module file extension to build Codira name lookup tables.
  importer->Impl.Invocation->getFrontendOpts().ModuleFileExtensions.push_back(
      std::make_shared<CodiraNameLookupExtension>(
          importer->Impl.BridgingHeaderLookupTable, importer->Impl.LookupTables,
          importer->Impl.CodiraContext,
          importer->Impl.getBufferImporterForDiagnostics(),
          importer->Impl.platformAvailability, &importer->Impl));

  // Create a compiler instance.
  {
    // The Clang modules produced by ClangImporter are always embedded in an
    // ObjectFilePCHContainer and contain -gmodules debug info.
    importer->Impl.Invocation->getCodeGenOpts().DebugTypeExtRefs = true;

    auto PCHContainerOperations =
      std::make_shared<language::Core::PCHContainerOperations>();
    PCHContainerOperations->registerWriter(
        std::make_unique<language::Core::ObjectFilePCHContainerWriter>());
    PCHContainerOperations->registerReader(
        std::make_unique<language::Core::ObjectFilePCHContainerReader>());
    importer->Impl.Instance.reset(
        new language::Core::CompilerInstance(std::move(PCHContainerOperations)));
  }
  auto &instance = *importer->Impl.Instance;
  instance.setInvocation(importer->Impl.Invocation);

  if (tracker)
    instance.addDependencyCollector(tracker->getClangCollector());

  {
    // Now set up the real client for Clang diagnostics---configured with proper
    // options---as opposed to the temporary one we made above.
    auto actualDiagClient = std::make_unique<ClangDiagnosticConsumer>(
        importer->Impl, instance.getDiagnosticOpts(),
        importerOpts.DumpClangDiagnostics);
    instance.createDiagnostics(actualDiagClient.release());
  }

  // Set up the file manager.
  {
    VFS = language::Core::createVFSFromCompilerInvocation(
        instance.getInvocation(), instance.getDiagnostics(), std::move(VFS));
    instance.createFileManager(VFS);
  }

  // Don't stop emitting messages if we ever can't load a module.
  // FIXME: This is actually a general problem: any "fatal" error could mess up
  // the CompilerInvocation when we're not in "show diagnostics after fatal
  // error" mode.
  language::Core::DiagnosticsEngine &clangDiags = instance.getDiagnostics();
  clangDiags.setSeverity(language::Core::diag::err_module_not_found,
                         language::Core::diag::Severity::Error,
                         language::Core::SourceLocation());
  clangDiags.setSeverity(language::Core::diag::err_module_not_built,
                         language::Core::diag::Severity::Error,
                         language::Core::SourceLocation());
  clangDiags.setFatalsAsError(ctx.Diags.getShowDiagnosticsAfterFatalError());

  // Use Clang to configure/save options for Codira IRGen/CodeGen
  if (ctx.LangOpts.ClangTarget.has_value()) {
    // If '-clang-target' is set, create a mock invocation with the Codira triple
    // to configure CodeGen and Target options for Codira compilation.
    auto languageTargetClangArgs = importer->getClangCC1Arguments(ctx, VFS, true);
    if (!languageTargetClangArgs)
      return nullptr;
    auto languageTargetClangInvocation = createClangInvocation(
        importer.get(), importerOpts, VFS, *languageTargetClangArgs);
    if (!languageTargetClangInvocation)
      return nullptr;
    auto targetInfo = language::Core::TargetInfo::CreateTargetInfo(
        clangDiags, languageTargetClangInvocation->TargetOpts);
    // Ensure the target info has configured target-specific defines
    std::string defineBuffer;
    toolchain::raw_string_ostream predefines(defineBuffer);
    language::Core::MacroBuilder builder(predefines);
    targetInfo->getTargetDefines(instance.getLangOpts(), builder);
    importer->Impl.setCodiraTargetInfo(targetInfo);
    importer->Impl.setCodiraCodeGenOptions(new language::Core::CodeGenOptions(
        languageTargetClangInvocation->getCodeGenOpts()));
  } else {
    // Just use the existing Invocation's directly
    importer->Impl.setCodiraTargetInfo(language::Core::TargetInfo::CreateTargetInfo(
        clangDiags, importer->Impl.Invocation->TargetOpts));
    importer->Impl.setCodiraCodeGenOptions(
        new language::Core::CodeGenOptions(importer->Impl.Invocation->getCodeGenOpts()));
  }

  // Create the associated action.
  importer->Impl.Action.reset(new ParsingAction(*importer,
                                                importer->Impl,
                                                importerOpts,
                                                languagePCHHash));
  auto *action = importer->Impl.Action.get();

  // Execute the action. We effectively inline most of
  // CompilerInstance::ExecuteAction here, because we need to leave the AST
  // open for future module loading.
  // FIXME: This has to be cleaned up on the Clang side before we can improve
  // things here.

  // Create the target instance.
  instance.setTarget(
    language::Core::TargetInfo::CreateTargetInfo(clangDiags,
                                        instance.getInvocation().TargetOpts));
  if (!instance.hasTarget())
    return nullptr;

  // Inform the target of the language options.
  //
  // FIXME: We shouldn't need to do this, the target should be immutable once
  // created. This complexity should be lifted elsewhere.
  instance.getTarget().adjust(clangDiags, instance.getLangOpts());

  if (importerOpts.Mode == ClangImporterOptions::Modes::EmbedBitcode)
    return importer;

  // ClangImporter always sets this in Normal mode, so we need to make sure to
  // set it before bailing out early when configuring ClangImporter for
  // precompiled modules. This is not a benign langopt, so forgetting this (for
  // example, if we combined the early exit below with the one above) would make
  // the compiler instance used to emit PCMs incompatible with the one used to
  // read them later.
  instance.getLangOpts().NeededByPCHOrCompilationUsesPCH = true;

  // Clang implicitly enables this by default in C++20 mode.
  instance.getLangOpts().ModulesLocalVisibility = false;

  if (importerOpts.Mode == ClangImporterOptions::Modes::PrecompiledModule)
    return importer;

  instance.initializeDelayedInputFileFromCAS();
  if (instance.getFrontendOpts().Inputs.empty())
    return nullptr; // no inputs available.

  bool canBegin = action->BeginSourceFile(instance,
                                          instance.getFrontendOpts().Inputs[0]);
  if (!canBegin)
    return nullptr; // there was an error related to the compiler arguments.

  language::Core::Preprocessor &clangPP = instance.getPreprocessor();
  clangPP.enableIncrementalProcessing();

  // Setup Preprocessor callbacks before initialing the parser to make sure
  // we catch implicit includes.
  auto ppTracker = std::make_unique<BridgingPPTracker>(importer->Impl);
  clangPP.addPPCallbacks(std::move(ppTracker));

  instance.createASTReader();

  // Manually run the action, so that the TU stays open for additional parsing.
  instance.createSema(action->getTranslationUnitKind(), nullptr);
  importer->Impl.Parser.reset(new language::Core::Parser(clangPP, instance.getSema(),
                                                /*SkipFunctionBodies=*/false));

  clangPP.EnterMainSourceFile();
  importer->Impl.Parser->Initialize();

  importer->Impl.nameImporter.reset(new NameImporter(
      importer->Impl.CodiraContext, importer->Impl.platformAvailability,
      importer->Impl.getClangSema(), &importer->Impl));

  // FIXME: These decls are not being parsed correctly since (a) some of the
  // callbacks are still being added, and (b) the logic to parse them has
  // changed.
  language::Core::Parser::DeclGroupPtrTy parsed;
  language::Core::Sema::ModuleImportState importState =
      language::Core::Sema::ModuleImportState::NotACXX20Module;
  while (!importer->Impl.Parser->ParseTopLevelDecl(parsed, importState)) {
    for (auto *D : parsed.get()) {
      importer->Impl.addBridgeHeaderTopLevelDecls(D);

      if (auto named = dyn_cast<language::Core::NamedDecl>(D)) {
        addEntryToLookupTable(*importer->Impl.BridgingHeaderLookupTable, named,
                              *importer->Impl.nameImporter);
      }
    }
  }

  // FIXME: This is missing implicit includes.
  auto *CB = new HeaderImportCallbacks(importer->Impl);
  clangPP.addPPCallbacks(std::unique_ptr<language::Core::PPCallbacks>(CB));

  // Create the selectors we'll be looking for.
  auto &clangContext = importer->Impl.Instance->getASTContext();
  importer->Impl.objectAtIndexedSubscript
    = clangContext.Selectors.getUnarySelector(
        &clangContext.Idents.get("objectAtIndexedSubscript"));
  const language::Core::IdentifierInfo *setObjectAtIndexedSubscriptIdents[2] = {
      &clangContext.Idents.get("setObject"),
      &clangContext.Idents.get("atIndexedSubscript"),
  };
  importer->Impl.setObjectAtIndexedSubscript
    = clangContext.Selectors.getSelector(2, setObjectAtIndexedSubscriptIdents);
  importer->Impl.objectForKeyedSubscript
    = clangContext.Selectors.getUnarySelector(
        &clangContext.Idents.get("objectForKeyedSubscript"));
  const language::Core::IdentifierInfo *setObjectForKeyedSubscriptIdents[2] = {
      &clangContext.Idents.get("setObject"),
      &clangContext.Idents.get("forKeyedSubscript"),
  };
  importer->Impl.setObjectForKeyedSubscript
    = clangContext.Selectors.getSelector(2, setObjectForKeyedSubscriptIdents);

  // Set up the imported header module.
  auto *importedHeaderModule = ModuleDecl::create(
      ctx.getIdentifier(CLANG_HEADER_MODULE_NAME), ctx,
      [&](ModuleDecl *importedHeaderModule, auto addFile) {
        importer->Impl.ImportedHeaderUnit = new (ctx)
            ClangModuleUnit(*importedHeaderModule, importer->Impl, nullptr);
        addFile(importer->Impl.ImportedHeaderUnit);
      });

  importedHeaderModule->setHasResolvedImports();
  importedHeaderModule->setIsNonCodiraModule(true);

  importer->Impl.IsReadingBridgingPCH = false;

  return importer;
}

bool ClangImporter::addSearchPath(StringRef newSearchPath, bool isFramework,
                                  bool isSystem) {
  language::Core::FileManager &fileMgr = Impl.Instance->getFileManager();
  auto optionalEntry = fileMgr.getOptionalDirectoryRef(newSearchPath);
  if (!optionalEntry)
    return true;
  auto entry = *optionalEntry;

  auto &headerSearchInfo = Impl.getClangPreprocessor().getHeaderSearchInfo();
  auto exists = std::any_of(headerSearchInfo.search_dir_begin(),
                            headerSearchInfo.search_dir_end(),
                            [&](const language::Core::DirectoryLookup &lookup) -> bool {
    if (isFramework)
      return lookup.getFrameworkDir() == &entry.getDirEntry();
    return lookup.getDir() == &entry.getDirEntry();
  });
  if (exists) {
    // Don't bother adding a search path that's already there. Clang would have
    // removed it via deduplication at the time the search path info gets built.
    return false;
  }

  auto kind = isSystem ? language::Core::SrcMgr::C_System : language::Core::SrcMgr::C_User;
  headerSearchInfo.AddSearchPath({entry, kind, isFramework},
                                 /*isAngled=*/true);

  // In addition to changing the current preprocessor directly, we still need
  // to change the options structure for future module-building.
  Impl.Instance->getHeaderSearchOpts().AddPath(newSearchPath,
                   isSystem ? language::Core::frontend::System : language::Core::frontend::Angled,
                                               isFramework,
                                               /*IgnoreSysRoot=*/true);
  return false;
}

language::Core::SourceLocation
ClangImporter::Implementation::getNextIncludeLoc() {
  language::Core::SourceManager &srcMgr = getClangInstance()->getSourceManager();

  if (!DummyIncludeBuffer.isValid()) {
    language::Core::SourceLocation includeLoc =
        srcMgr.getLocForStartOfFile(srcMgr.getMainFileID());
    // Picking the beginning of the main FileID as include location is also what
    // the clang PCH mechanism is doing (see
    // language::Core::ASTReader::getImportLocation()). Choose the next source location
    // here to avoid having the exact same import location as the clang PCH.
    // Otherwise, if we are using a PCH for bridging header, we'll have
    // problems with source order comparisons of clang source locations not
    // being deterministic.
    includeLoc = includeLoc.getLocWithOffset(1);
    DummyIncludeBuffer = srcMgr.createFileID(
        std::make_unique<ZeroFilledMemoryBuffer>(
          256*1024, StringRef(moduleImportBufferName)),
        language::Core::SrcMgr::C_User, /*LoadedID*/0, /*LoadedOffset*/0, includeLoc);
  }

  language::Core::SourceLocation clangImportLoc =
      srcMgr.getLocForStartOfFile(DummyIncludeBuffer)
            .getLocWithOffset(IncludeCounter++);
  assert(srcMgr.isInFileID(clangImportLoc, DummyIncludeBuffer) &&
         "confused Clang's source manager with our fake locations");
  return clangImportLoc;
}

bool ClangImporter::Implementation::importHeader(
    ModuleDecl *adapter, StringRef headerName, SourceLoc diagLoc,
    bool trackParsedSymbols,
    std::unique_ptr<toolchain::MemoryBuffer> sourceBuffer,
    bool implicitImport) {

  // Progress update for the debugger.
  CodiraContext.PreModuleImportHook(
      headerName, ASTContext::ModuleImportKind::BridgingHeader);

  // Don't even try to load the bridging header if the Clang AST is in a bad
  // state. It could cause a crash.
  auto &clangDiags = getClangASTContext().getDiagnostics();
  if (clangDiags.hasUnrecoverableErrorOccurred() &&
      !getClangInstance()->getPreprocessorOpts().AllowPCHWithCompilerErrors)
    return true;

  assert(adapter);
  ImportedHeaderOwners.push_back(adapter);

  bool hadError = clangDiags.hasErrorOccurred();

  language::Core::SourceManager &sourceMgr = getClangInstance()->getSourceManager();
  language::Core::FileID bufferID = sourceMgr.createFileID(std::move(sourceBuffer),
                                                  language::Core::SrcMgr::C_User,
                                                  /*LoadedID=*/0,
                                                  /*LoadedOffset=*/0,
                                                  getNextIncludeLoc());
  auto &consumer =
      static_cast<HeaderParsingASTConsumer &>(Instance->getASTConsumer());
  consumer.reset();

  language::Core::Preprocessor &pp = getClangPreprocessor();
  pp.EnterSourceFile(bufferID, /*Dir=*/nullptr, /*Loc=*/{});
  // Force the import to occur.
  pp.LookAhead(0);

  SmallVector<language::Core::DeclGroupRef, 16> allParsedDecls;
  auto handleParsed = [&](language::Core::DeclGroupRef parsed) {
    if (trackParsedSymbols) {
      for (auto *D : parsed) {
        addBridgeHeaderTopLevelDecls(D);
      }
    }

    allParsedDecls.push_back(parsed);
  };

  language::Core::Parser::DeclGroupPtrTy parsed;
  language::Core::Sema::ModuleImportState importState =
      language::Core::Sema::ModuleImportState::NotACXX20Module;
  while (!Parser->ParseTopLevelDecl(parsed, importState)) {
    if (parsed)
      handleParsed(parsed.get());
    for (auto additionalParsedGroup : consumer.getAdditionalParsedDecls())
      handleParsed(additionalParsedGroup);
    consumer.reset();
  }

  // We're trying to discourage (and eventually deprecate) the use of implicit
  // bridging-header imports triggered by IMPORTED_HEADER blocks in
  // modules. There are two sub-cases to consider:
  //
  //   #1 The implicit import actually occurred.
  //
  //   #2 The user explicitly -import-objc-header'ed some header or PCH that
  //      makes the implicit import redundant.
  //
  // It's not obvious how to exactly differentiate these cases given the
  // interface clang gives us, but we only want to warn on case #1, and the
  // non-emptiness of allParsedDecls is a _definite_ sign that we're in case
  // #1. So we treat that as an approximation of the condition we're after, and
  // accept that we might fail to warn in the odd case where "the import
  // occurred" but didn't introduce any new decls.
  //
  // We also want to limit (for now) the warning in case #1 to invocations that
  // requested an explicit bridging header, because otherwise the warning will
  // complain in a very common scenario (unit test w/o bridging header imports
  // application w/ bridging header) that we don't yet have Xcode automation
  // to correct. The fix would be explicitly importing on the command line.
  if (implicitImport && !allParsedDecls.empty() &&
    BridgingHeaderExplicitlyRequested) {
    diagnose(
      diagLoc, diag::implicit_bridging_header_imported_from_module,
      toolchain::sys::path::filename(headerName), adapter->getName());
  }

  // We can't do this as we're parsing because we may want to resolve naming
  // conflicts between the things we've parsed.

  std::function<void(language::Core::Decl *)> visit = [&](language::Core::Decl *decl) {
    // Iterate into extern "C" {} type declarations.
    if (auto linkageDecl = dyn_cast<language::Core::LinkageSpecDecl>(decl)) {
      for (auto *decl : linkageDecl->noload_decls()) {
        visit(decl);
      }
    }
    if (auto named = dyn_cast<language::Core::NamedDecl>(decl)) {
      addEntryToLookupTable(*BridgingHeaderLookupTable, named,
                              getNameImporter());
    }
  };
  for (auto group : allParsedDecls) {
    for (auto *D : group) {
      visit(D);
    }
  }

  pp.EndSourceFile();
  bumpGeneration();

  // Add any defined macros to the bridging header lookup table.
  addMacrosToLookupTable(*BridgingHeaderLookupTable, getNameImporter());

  // Finish loading any extra modules that were (transitively) imported.
  handleDeferredImports(diagLoc);

  // Wrap all Clang imports under a Codira import decl.
  for (auto &Import : BridgeHeaderTopLevelImports) {
    if (auto *ClangImport = Import.dyn_cast<language::Core::ImportDecl*>()) {
      Import = createImportDecl(CodiraContext, adapter, ClangImport, {});
    }
  }

  // Finalize the lookup table, which may fail.
  finalizeLookupTable(*BridgingHeaderLookupTable, getNameImporter(),
                      getBufferImporterForDiagnostics());

  // FIXME: What do we do if there was already an error?
  if (!hadError && clangDiags.hasErrorOccurred() &&
      !getClangInstance()->getPreprocessorOpts().AllowPCHWithCompilerErrors) {
    diagnose(diagLoc, diag::bridging_header_error, headerName);
    return true;
  }

  return false;
}

bool ClangImporter::importHeader(StringRef header, ModuleDecl *adapter,
                                 off_t expectedSize, time_t expectedModTime,
                                 StringRef cachedContents, SourceLoc diagLoc) {
  language::Core::FileManager &fileManager = Impl.Instance->getFileManager();
  auto headerFile = fileManager.getFile(header, /*OpenFile=*/true);
  // Prefer importing the header directly if the header content matches by
  // checking size and mod time. This allows correct import if some no-modular
  // headers are already imported into clang importer. If mod time is zero, then
  // the module should be built from CAS and there is no mod time to verify.
  if (headerFile && (*headerFile)->getSize() == expectedSize &&
      (expectedModTime == 0 ||
       (*headerFile)->getModificationTime() == expectedModTime)) {
    return importBridgingHeader(header, adapter, diagLoc, false, true);
  }

  // If we've made it to here, this is some header other than the bridging
  // header, which means we can no longer rely on one file's modification time
  // to invalidate code completion caches. :-(
  Impl.setSinglePCHImport(std::nullopt);

  if (!cachedContents.empty() && cachedContents.back() == '\0')
    cachedContents = cachedContents.drop_back();
  std::unique_ptr<toolchain::MemoryBuffer> sourceBuffer{
    toolchain::MemoryBuffer::getMemBuffer(cachedContents, header)
  };
  return Impl.importHeader(adapter, header, diagLoc, /*trackParsedSymbols=*/false,
                           std::move(sourceBuffer), true);
}

bool ClangImporter::importBridgingHeader(StringRef header, ModuleDecl *adapter,
                                         SourceLoc diagLoc,
                                         bool trackParsedSymbols,
                                         bool implicitImport) {
  if (isPCHFilenameExtension(header)) {
    return bindBridgingHeader(adapter, diagLoc);
  }

  language::Core::FileManager &fileManager = Impl.Instance->getFileManager();
  auto headerFile = fileManager.getFile(header, /*OpenFile=*/true);
  if (!headerFile) {
    Impl.diagnose(diagLoc, diag::bridging_header_missing, header);
    return true;
  }

  toolchain::SmallString<128> importLine;
  if (Impl.CodiraContext.LangOpts.EnableObjCInterop)
    importLine = "#import \"";
  else
    importLine = "#include \"";

  importLine += header;
  importLine += "\"\n";

  std::unique_ptr<toolchain::MemoryBuffer> sourceBuffer{
    toolchain::MemoryBuffer::getMemBufferCopy(
      importLine, Implementation::bridgingHeaderBufferName)
  };
  return Impl.importHeader(adapter, header, diagLoc, trackParsedSymbols,
                           std::move(sourceBuffer), implicitImport);
}

bool ClangImporter::bindBridgingHeader(ModuleDecl *adapter, SourceLoc diagLoc) {
  Impl.ImportedHeaderOwners.push_back(adapter);
  // We already imported this with -include-pch above, so we should have
  // collected a bunch of PCH-encoded module imports that we just need to
  // replay in handleDeferredImports.
  Impl.handleDeferredImports(diagLoc);
  return false;
}

static toolchain::Expected<toolchain::cas::ObjectRef>
setupIncludeTreeInput(language::Core::CompilerInvocation &invocation,
                      StringRef headerPath, StringRef pchIncludeTree) {
  auto DB = invocation.getCASOpts().getOrCreateDatabases();
  if (!DB)
    return DB.takeError();
  auto CAS = DB->first;
  auto Cache = DB->second;
  auto ID = CAS->parseID(pchIncludeTree);
  if (!ID)
    return ID.takeError();
  auto Ref = CAS->getReference(*ID);
  if (!Ref)
    return toolchain::cas::ObjectStore::createUnknownObjectError(*ID);
  auto Key = ClangImporter::createEmbeddedBridgingHeaderCacheKey(*CAS, *Ref);
  if (!Key)
    return Key.takeError();
  auto Lookup = Cache->get(CAS->getID(*Key));
  if (!Lookup)
    return Lookup.takeError();

  std::optional<toolchain::cas::ObjectRef> includeTreeRef;
  if (*Lookup) {
    includeTreeRef = CAS->getReference(**Lookup);
    if (!includeTreeRef)
      return toolchain::cas::ObjectStore::createUnknownObjectError(**Lookup);
  } else
    // Failed to look up. This is from a caching build that doesn't use bridging
    // header chaining due to an older language-driver. Just use the include tree
    // for PCH directly.
    includeTreeRef = *Ref;

  invocation.getFrontendOpts().Inputs.push_back(language::Core::FrontendInputFile(
      *includeTreeRef, headerPath, language::Core::Language::ObjC));

  return *includeTreeRef;
}

std::string ClangImporter::getBridgingHeaderContents(
    StringRef headerPath, off_t &fileSize, time_t &fileModTime,
    StringRef pchIncludeTree) {
  auto invocation =
      std::make_shared<language::Core::CompilerInvocation>(*Impl.Invocation);

  invocation->getFrontendOpts().DisableFree = false;
  invocation->getFrontendOpts().Inputs.clear();

  std::optional<toolchain::cas::ObjectRef> includeTreeRef;
  if (pchIncludeTree.empty())
    invocation->getFrontendOpts().Inputs.push_back(
        language::Core::FrontendInputFile(headerPath, language::Core::Language::ObjC));
  else if (auto err =
               setupIncludeTreeInput(*invocation, headerPath, pchIncludeTree)
                   .moveInto(includeTreeRef)) {
    Impl.diagnose({}, diag::err_rewrite_bridging_header,
                  toString(std::move(err)));
    return "";
  }

  invocation->getPreprocessorOpts().resetNonModularOptions();

  language::Core::CompilerInstance rewriteInstance(
    Impl.Instance->getPCHContainerOperations(),
    &Impl.Instance->getModuleCache());
  rewriteInstance.setInvocation(invocation);
  rewriteInstance.createDiagnostics(new language::Core::IgnoringDiagConsumer);

  language::Core::FileManager &fileManager = Impl.Instance->getFileManager();
  rewriteInstance.setFileManager(&fileManager);
  rewriteInstance.createSourceManager(fileManager);
  rewriteInstance.setTarget(&Impl.Instance->getTarget());

  std::string result;
  bool success = toolchain::CrashRecoveryContext().RunSafelyOnThread([&] {
    // A much simpler version of language::Core::RewriteIncludesAction that lets us
    // write to an in-memory buffer.
    class RewriteIncludesAction : public language::Core::PreprocessorFrontendAction {
      raw_ostream &OS;
      std::optional<toolchain::cas::ObjectRef> includeTreeRef;

      void ExecuteAction() override {
        language::Core::CompilerInstance &compiler = getCompilerInstance();
        // If the input is include tree, setup the IncludeTreePPAction.
        if (includeTreeRef) {
          auto IncludeTreeRoot = language::Core::cas::IncludeTreeRoot::get(
              compiler.getOrCreateObjectStore(), *includeTreeRef);
          if (!IncludeTreeRoot)
            toolchain::report_fatal_error(IncludeTreeRoot.takeError());
          auto PPCachedAct =
              language::Core::createPPActionsFromIncludeTree(*IncludeTreeRoot);
          if (!PPCachedAct)
            toolchain::report_fatal_error(PPCachedAct.takeError());
          compiler.getPreprocessor().setPPCachedActions(
              std::move(*PPCachedAct));
        }

        language::Core::RewriteIncludesInInput(compiler.getPreprocessor(), &OS,
                                      compiler.getPreprocessorOutputOpts());
      }

    public:
      explicit RewriteIncludesAction(
          raw_ostream &os, std::optional<toolchain::cas::ObjectRef> includeTree)
          : OS(os), includeTreeRef(includeTree) {}
    };

    toolchain::raw_string_ostream os(result);
    RewriteIncludesAction action(os, includeTreeRef);
    rewriteInstance.ExecuteAction(action);
  });

  success |= !rewriteInstance.getDiagnostics().hasErrorOccurred();
  if (!success) {
    Impl.diagnose({}, diag::could_not_rewrite_bridging_header);
    return "";
  }

  if (auto fileInfo = fileManager.getFile(headerPath)) {
    fileSize = (*fileInfo)->getSize();
    fileModTime = (*fileInfo)->getModificationTime();
  }
  return result;
}

/// Returns the appropriate source input language based on language options.
static language::Core::Language getLanguageFromOptions(
    const language::Core::LangOptions &LangOpts) {
  if (LangOpts.OpenCL)
    return language::Core::Language::OpenCL;
  if (LangOpts.CUDA)
    return language::Core::Language::CUDA;
  if (LangOpts.ObjC)
    return LangOpts.CPlusPlus ?
        language::Core::Language::ObjCXX : language::Core::Language::ObjC;
  return LangOpts.CPlusPlus ? language::Core::Language::CXX : language::Core::Language::C;
}

/// Wraps the given frontend action in an index data recording action if the
/// frontend options have an index store path specified.
static
std::unique_ptr<language::Core::FrontendAction> wrapActionForIndexingIfEnabled(
    const language::Core::FrontendOptions &FrontendOpts,
    std::unique_ptr<language::Core::FrontendAction> action) {
  if (!FrontendOpts.IndexStorePath.empty()) {
    return language::Core::index::createIndexDataRecordingAction(
        FrontendOpts, std::move(action));
  }
  return action;
}

std::unique_ptr<language::Core::CompilerInstance>
ClangImporter::cloneCompilerInstanceForPrecompiling() {
  auto invocation =
      std::make_shared<language::Core::CompilerInvocation>(*Impl.Invocation);

  auto &PPOpts = invocation->getPreprocessorOpts();
  PPOpts.resetNonModularOptions();

  auto &FrontendOpts = invocation->getFrontendOpts();
  FrontendOpts.DisableFree = false;
  if (FrontendOpts.CASIncludeTreeID.empty())
    FrontendOpts.Inputs.clear();

  // Share the CASOption and the underlying CAS.
  invocation->setCASOption(Impl.Invocation->getCASOptsPtr());

  auto clonedInstance = std::make_unique<language::Core::CompilerInstance>(
    Impl.Instance->getPCHContainerOperations(),
    &Impl.Instance->getModuleCache());
  clonedInstance->setInvocation(std::move(invocation));
  clonedInstance->createDiagnostics(&Impl.Instance->getDiagnosticClient(),
                                    /*ShouldOwnClient=*/false);

  language::Core::FileManager &fileManager = Impl.Instance->getFileManager();
  clonedInstance->setFileManager(&fileManager);
  clonedInstance->createSourceManager(fileManager);
  clonedInstance->setTarget(&Impl.Instance->getTarget());
  clonedInstance->setOutputBackend(Impl.CodiraContext.OutputBackend);

  return clonedInstance;
}

bool ClangImporter::emitBridgingPCH(
    StringRef headerPath, StringRef outputPCHPath, bool cached) {
  auto emitInstance = cloneCompilerInstanceForPrecompiling();
  auto &invocation = emitInstance->getInvocation();

  auto &LangOpts = invocation.getLangOpts();
  LangOpts.NeededByPCHOrCompilationUsesPCH = true;
  LangOpts.CacheGeneratedPCH = cached;

  auto language = getLanguageFromOptions(LangOpts);
  auto inputFile = language::Core::FrontendInputFile(headerPath, language);

  auto &FrontendOpts = invocation.getFrontendOpts();
  if (invocation.getFrontendOpts().CASIncludeTreeID.empty())
    FrontendOpts.Inputs = {inputFile};
  FrontendOpts.OutputFile = outputPCHPath.str();
  FrontendOpts.ProgramAction = language::Core::frontend::GeneratePCH;

  auto action = wrapActionForIndexingIfEnabled(
      FrontendOpts, std::make_unique<language::Core::GeneratePCHAction>());
  emitInstance->ExecuteAction(*action);

  if (emitInstance->getDiagnostics().hasErrorOccurred() &&
      !emitInstance->getPreprocessorOpts().AllowPCHWithCompilerErrors) {
    Impl.diagnose({}, diag::bridging_header_pch_error,
                  outputPCHPath, headerPath);
    return true;
  }
  return false;
}

bool ClangImporter::runPreprocessor(
    StringRef inputPath, StringRef outputPath) {
  auto emitInstance = cloneCompilerInstanceForPrecompiling();
  auto &invocation = emitInstance->getInvocation();
  auto &LangOpts = invocation.getLangOpts();
  auto &OutputOpts = invocation.getPreprocessorOutputOpts();
  OutputOpts.ShowCPP = 1;
  OutputOpts.ShowComments = 0;
  OutputOpts.ShowLineMarkers = 0;
  OutputOpts.ShowMacros = 0;
  OutputOpts.ShowMacroComments = 0;
  auto language = getLanguageFromOptions(LangOpts);
  auto inputFile = language::Core::FrontendInputFile(inputPath, language);

  auto &FrontendOpts = invocation.getFrontendOpts();
  if (invocation.getFrontendOpts().CASIncludeTreeID.empty())
    FrontendOpts.Inputs = {inputFile};
  FrontendOpts.OutputFile = outputPath.str();
  FrontendOpts.ProgramAction = language::Core::frontend::PrintPreprocessedInput;

  auto action = wrapActionForIndexingIfEnabled(
      FrontendOpts, std::make_unique<language::Core::PrintPreprocessedAction>());
  emitInstance->ExecuteAction(*action);
  return emitInstance->getDiagnostics().hasErrorOccurred();
}

bool ClangImporter::emitPrecompiledModule(
    StringRef moduleMapPath, StringRef moduleName, StringRef outputPath) {
  auto emitInstance = cloneCompilerInstanceForPrecompiling();
  auto &invocation = emitInstance->getInvocation();

  auto &LangOpts = invocation.getLangOpts();
  LangOpts.setCompilingModule(language::Core::LangOptions::CMK_ModuleMap);
  LangOpts.ModuleName = moduleName.str();
  LangOpts.CurrentModule = LangOpts.ModuleName;

  auto language = getLanguageFromOptions(LangOpts);

  auto &FrontendOpts = invocation.getFrontendOpts();
  if (invocation.getFrontendOpts().CASIncludeTreeID.empty()) {
    auto inputFile = language::Core::FrontendInputFile(
        moduleMapPath,
        language::Core::InputKind(language, language::Core::InputKind::ModuleMap, false),
        FrontendOpts.IsSystemModule);
    FrontendOpts.Inputs = {inputFile};
  }
  FrontendOpts.OriginalModuleMap = moduleMapPath.str();
  FrontendOpts.OutputFile = outputPath.str();
  FrontendOpts.ProgramAction = language::Core::frontend::GenerateModule;

  auto action = wrapActionForIndexingIfEnabled(
      FrontendOpts,
      std::make_unique<language::Core::GenerateModuleFromModuleMapAction>());
  emitInstance->ExecuteAction(*action);

  if (emitInstance->getDiagnostics().hasErrorOccurred() &&
      !FrontendOpts.AllowPCMWithCompilerErrors) {
    Impl.diagnose({}, diag::emit_pcm_error, outputPath, moduleMapPath);
    return true;
  }
  return false;
}

bool ClangImporter::dumpPrecompiledModule(
    StringRef modulePath, StringRef outputPath) {
  auto dumpInstance = cloneCompilerInstanceForPrecompiling();
  auto &invocation = dumpInstance->getInvocation();

  auto inputFile = language::Core::FrontendInputFile(
      modulePath, language::Core::InputKind(
          language::Core::Language::Unknown, language::Core::InputKind::Precompiled, false));

  auto &FrontendOpts = invocation.getFrontendOpts();
  if (invocation.getFrontendOpts().CASIncludeTreeID.empty())
    FrontendOpts.Inputs = {inputFile};
  FrontendOpts.OutputFile = outputPath.str();

  auto action = std::make_unique<language::Core::DumpModuleInfoAction>();
  dumpInstance->ExecuteAction(*action);

  if (dumpInstance->getDiagnostics().hasErrorOccurred()) {
    Impl.diagnose({}, diag::dump_pcm_error, modulePath);
    return true;
  }
  return false;
}

void ClangImporter::collectVisibleTopLevelModuleNames(
    SmallVectorImpl<Identifier> &names) const {
  SmallVector<language::Core::Module *, 32> Modules;
  Impl.getClangPreprocessor().getHeaderSearchInfo().collectAllModules(Modules);
  for (auto &M : Modules) {
    if (!M->isAvailable())
      continue;

    names.push_back(
        Impl.CodiraContext.getIdentifier(M->getTopLevelModuleName()));
  }
}

void ClangImporter::collectSubModuleNames(
    ImportPath::Module path,
    std::vector<std::string> &names) const {
  auto &clangHeaderSearch = Impl.getClangPreprocessor().getHeaderSearchInfo();

  // Look up the top-level module first.
  language::Core::Module *clangModule = clangHeaderSearch.lookupModule(
      path.front().Item.str(), /*ImportLoc=*/language::Core::SourceLocation(),
      /*AllowSearch=*/true, /*AllowExtraModuleMapSearch=*/true);
  if (!clangModule)
    return;
  language::Core::Module *submodule = clangModule;
  for (auto component : path.getSubmodulePath()) {
    submodule = submodule->findSubmodule(component.Item.str());
    if (!submodule)
      return;
  }
  for (auto sub : submodule->submodules())
    names.push_back(sub->Name);
}

bool ClangImporter::isModuleImported(const language::Core::Module *M) {
  return M->NameVisibility == language::Core::Module::NameVisibilityKind::AllVisible;
}

static toolchain::VersionTuple getCurrentVersionFromTBD(toolchain::vfs::FileSystem &FS,
                                                   StringRef path,
                                                   StringRef moduleName) {
  std::string fwName = (moduleName + ".framework").str();
  auto pos = path.find(fwName);
  if (pos == StringRef::npos)
    return {};
  toolchain::SmallString<256> buffer(path.substr(0, pos + fwName.size()));
  toolchain::sys::path::append(buffer, moduleName + ".tbd");
  auto tbdPath = buffer.str();
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> tbdBufOrErr =
      FS.getBufferForFile(tbdPath);
  // .tbd file doesn't exist, exit.
  if (!tbdBufOrErr)
    return {};
  auto tbdFileOrErr =
      toolchain::MachO::TextAPIReader::get(tbdBufOrErr.get()->getMemBufferRef());
  if (auto err = tbdFileOrErr.takeError()) {
    consumeError(std::move(err));
    return {};
  }
  auto tbdCV = (*tbdFileOrErr)->getCurrentVersion();
  return toolchain::VersionTuple(tbdCV.getMajor(), tbdCV.getMinor(),
                            tbdCV.getSubminor());
}

bool ClangImporter::canImportModule(ImportPath::Module modulePath,
                                    SourceLoc loc,
                                    ModuleVersionInfo *versionInfo,
                                    bool isTestableDependencyLookup) {
  // Look up the top-level module to see if it exists.
  auto topModule = modulePath.front();
  language::Core::Module *clangModule = Impl.lookupModule(topModule.Item.str());
  if (!clangModule) {
    return false;
  }

  language::Core::Module::Requirement r;
  language::Core::Module::UnresolvedHeaderDirective mh;
  language::Core::Module *m;
  auto &ctx = Impl.getClangASTContext();
  auto &lo = ctx.getLangOpts();
  auto &ti = getModuleAvailabilityTarget();

  auto available = clangModule->isAvailable(lo, ti, r, mh, m);
  if (!available)
    return false;

  if (modulePath.hasSubmodule()) {
    for (auto &component : modulePath.getSubmodulePath()) {
      clangModule = clangModule->findSubmodule(component.Item.str());

      // Special case: a submodule named "Foo.Private" can be moved to a
      // top-level module named "Foo_Private". Clang has special support for
      // this.
      if (!clangModule && component.Item.str() == "Private" &&
          (&component) == (&modulePath.getRaw()[1])) {
        clangModule =
            Impl.lookupModule((topModule.Item.str() + "_Private").str());
      }
      if (!clangModule || !clangModule->isAvailable(lo, ti, r, mh, m)) {
        return false;
      }
    }
  }

  if (!versionInfo)
    return true;

  assert(available);
  StringRef path = getClangASTContext().getSourceManager()
    .getFilename(clangModule->DefinitionLoc);

  // Look for the .tbd file inside .framework dir to get the project version
  // number.
  toolchain::VersionTuple currentVersion = getCurrentVersionFromTBD(
      Impl.Instance->getVirtualFileSystem(), path, topModule.Item.str());
  versionInfo->setVersion(currentVersion,
                          ModuleVersionSourceKind::ClangModuleTBD);
  return true;
}

language::Core::Module *
ClangImporter::Implementation::lookupModule(StringRef moduleName) {
  auto &clangHeaderSearch = getClangPreprocessor().getHeaderSearchInfo();

  // Explicit module. Try load from modulemap.
  auto &PP = Instance->getPreprocessor();
  auto &MM = PP.getHeaderSearchInfo().getModuleMap();
  auto loadFromMM = [&]() -> language::Core::Module * {
    auto *II = PP.getIdentifierInfo(moduleName);
    if (auto clangModule = MM.getCachedModuleLoad(*II))
      return *clangModule;
    return nullptr;
  };
  // Check if it is already loaded.
  if (auto *clangModule = loadFromMM())
    return clangModule;

  // If not, try load it.
  auto &PrebuiltModules = Instance->getHeaderSearchOpts().PrebuiltModuleFiles;
  auto moduleFile = PrebuiltModules.find(moduleName);
  if (moduleFile == PrebuiltModules.end()) {
    if (getClangASTContext().getLangOpts().ImplicitModules)
      return clangHeaderSearch.lookupModule(
          moduleName, /*ImportLoc=*/language::Core::SourceLocation(),
          /*AllowSearch=*/true, /*AllowExtraModuleMapSearch=*/true);
    return nullptr;
  }

  language::Core::serialization::ModuleFile *Loaded = nullptr;
  if (!Instance->loadModuleFile(moduleFile->second, Loaded))
    return nullptr; // error loading, return not found.
  return loadFromMM();
}

ModuleDecl *ClangImporter::Implementation::loadModuleClang(
    SourceLoc importLoc, ImportPath::Module path) {
  auto realModuleName = CodiraContext.getRealModuleName(path.front().Item).str();

  // Convert the Codira import path over to a Clang import path.
  SmallVector<std::pair<language::Core::IdentifierInfo *, language::Core::SourceLocation>, 4>
      clangPath;
  bool isTopModuleComponent = true;
  for (auto component : path) {
    StringRef item = isTopModuleComponent? realModuleName:
                                           component.Item.str();
    isTopModuleComponent = false;

    clangPath.emplace_back(
        getClangPreprocessor().getIdentifierInfo(item),
        exportSourceLoc(component.Loc));
  }

  auto &diagEngine = Instance->getDiagnostics();
  auto &rawDiagClient = *diagEngine.getClient();
  auto &diagClient = static_cast<ClangDiagnosticConsumer &>(rawDiagClient);

  auto loadModule = [&](language::Core::ModuleIdPath path,
                        language::Core::Module::NameVisibilityKind visibility)
      -> language::Core::ModuleLoadResult {
    auto importRAII =
        diagClient.handleImport(clangPath.front().first, diagEngine,
                                importLoc);

    std::string preservedIndexStorePathOption;
    auto &clangFEOpts = Instance->getFrontendOpts();
    if (!clangFEOpts.IndexStorePath.empty()) {
      StringRef moduleName = path[0].first->getName();
      // Ignore the CodiraShims module for the index data.
      if (moduleName == CodiraContext.CodiraShimsModuleName.str()) {
        preservedIndexStorePathOption = clangFEOpts.IndexStorePath;
        clangFEOpts.IndexStorePath.clear();
      }
    }

    language::Core::SourceLocation clangImportLoc = getNextIncludeLoc();
    language::Core::ModuleLoadResult result =
        Instance->loadModule(clangImportLoc, path, visibility,
                             /*IsInclusionDirective=*/false);

    if (!preservedIndexStorePathOption.empty()) {
      // Restore the -index-store-path option.
      clangFEOpts.IndexStorePath = preservedIndexStorePathOption;
    }

    if (result && (visibility == language::Core::Module::AllVisible)) {
      getClangPreprocessor().makeModuleVisible(result, clangImportLoc);
    }
    return result;
  };

  // Now load the top-level module, so that we can check if the submodule
  // exists without triggering a fatal error.
  auto clangModule = loadModule(clangPath.front(), language::Core::Module::AllVisible);
  if (!clangModule)
    return nullptr;

  // If we're asked to import the top-level module then we're done here.
  auto *topCodiraModule = finishLoadingClangModule(clangModule, importLoc);
  if (path.size() == 1) {
    return topCodiraModule;
  }

  // Verify that the submodule exists.
  language::Core::Module *submodule = clangModule;
  for (auto &component : path.getSubmodulePath()) {
    submodule = submodule->findSubmodule(component.Item.str());

    // Special case: a submodule named "Foo.Private" can be moved to a top-level
    // module named "Foo_Private". Clang has special support for this.
    // We're limiting this to just submodules named "Private" because this will
    // put the Clang AST in a fatal error state if it /doesn't/ exist.
    if (!submodule && component.Item.str() == "Private" &&
        (&component) == (&path.getRaw()[1])) {
      submodule = loadModule(toolchain::ArrayRef(clangPath).slice(0, 2),
                             language::Core::Module::Hidden);
    }

    if (!submodule) {
      // FIXME: Specialize the error for a missing submodule?
      return nullptr;
    }
  }

  // Finally, load the submodule and make it visible.
  clangModule = loadModule(clangPath, language::Core::Module::AllVisible);
  if (!clangModule)
    return nullptr;

  return finishLoadingClangModule(clangModule, importLoc);
}

ModuleDecl *
ClangImporter::loadModule(SourceLoc importLoc,
                          ImportPath::Module path,
                          bool AllowMemoryCache) {
  return Impl.loadModule(importLoc, path);
}

ModuleDecl *ClangImporter::Implementation::loadModule(
    SourceLoc importLoc, ImportPath::Module path) {
  ModuleDecl *MD = nullptr;
  ASTContext &ctx = getNameImporter().getContext();

  // `CxxStdlib` is the only accepted spelling of the C++ stdlib module name.
  if (path.front().Item.is("std") ||
      path.front().Item.str().starts_with("std_"))
    return nullptr;
  if (path.front().Item == ctx.Id_CxxStdlib) {
    ImportPath::Builder adjustedPath(ctx.getIdentifier("std"), importLoc);
    adjustedPath.append(path.getSubmodulePath());
    path = adjustedPath.copyTo(ctx).getModulePath(ImportKind::Module);
  }

  if (!DisableSourceImport)
    MD = loadModuleClang(importLoc, path);
  if (!MD)
    MD = loadModuleDWARF(importLoc, path);
  return MD;
}

ModuleDecl *ClangImporter::Implementation::finishLoadingClangModule(
    const language::Core::Module *clangModule, SourceLoc importLoc) {
  assert(clangModule);

  // Bump the generation count.
  bumpGeneration();

  // Force load overlays for all imported modules.
  // FIXME: This forces the creation of wrapper modules for all imports as
  // well, and may do unnecessary work.
  ClangModuleUnit *wrapperUnit = getWrapperForModule(clangModule, importLoc);
  ModuleDecl *result = wrapperUnit->getParentModule();
  auto &moduleWrapper = ModuleWrappers[clangModule];
  if (!moduleWrapper.getInt()) {
    moduleWrapper.setInt(true);
    (void) namelookup::getAllImports(result);
  }

  // Register '.h' inputs of each Clang module dependency with
  // the dependency tracker. In implicit builds such dependencies are registered
  // during the on-demand construction of Clang module. In Explicit Module
  // Builds, since we load pre-built PCMs directly, we do not get to do so. So
  // instead, manually register all `.h` inputs of Clang module dependnecies.
  if (CodiraDependencyTracker &&
      !Instance->getInvocation().getLangOpts().ImplicitModules) {
    if (auto moduleRef = clangModule->getASTFile()) {
      auto *moduleFile = Instance->getASTReader()->getModuleManager().lookup(
          *moduleRef);
      toolchain::SmallString<0> pathBuf;
      pathBuf.reserve(256);
      Instance->getASTReader()->visitInputFileInfos(
          *moduleFile, /*IncludeSystem=*/true,
          [&](const language::Core::serialization::InputFileInfo &IFI, bool isSystem) {
            auto Filename = language::Core::ASTReader::ResolveImportedPath(
                pathBuf, IFI.UnresolvedImportedFilename, *moduleFile);
            CodiraDependencyTracker->addDependency(*Filename, isSystem);
          });
    }
  }

  if (clangModule->isSubModule()) {
    finishLoadingClangModule(clangModule->getTopLevelModule(), importLoc);
  } else {

    if (!CodiraContext.getLoadedModule(result->getName()))
      CodiraContext.addLoadedModule(result);
  }

  return result;
}

// Run through the set of deferred imports -- either those referenced by
// submodule ID from a bridging PCH, or those already loaded as language::Core::Modules
// in response to an import directive in a bridging header -- and call
// finishLoadingClangModule on each.
void ClangImporter::Implementation::handleDeferredImports(SourceLoc diagLoc) {
  language::Core::ASTReader &R = *Instance->getASTReader();
  toolchain::SmallSet<language::Core::serialization::SubmoduleID, 32> seenSubmodules;
  for (language::Core::serialization::SubmoduleID ID : PCHImportedSubmodules) {
    if (!seenSubmodules.insert(ID).second)
      continue;
    ImportedHeaderExports.push_back(R.getSubmodule(ID));
  }
  PCHImportedSubmodules.clear();

  // Avoid a for-in loop because in unusual situations we can end up pulling in
  // another bridging header while we finish loading the modules that are
  // already here. This is a brittle situation but it's outside what's
  // officially supported with bridging headers: app targets and unit tests
  // only. Unfortunately that's not enforced.
  for (size_t i = 0; i < ImportedHeaderExports.size(); ++i) {
    (void)finishLoadingClangModule(ImportedHeaderExports[i], diagLoc);
  }
}

ModuleDecl *ClangImporter::getImportedHeaderModule() const {
  return Impl.ImportedHeaderUnit->getParentModule();
}

ModuleDecl *
ClangImporter::getWrapperForModule(const language::Core::Module *mod,
                                   bool returnOverlayIfPossible) const {
  auto clangUnit = Impl.getWrapperForModule(mod);
  if (returnOverlayIfPossible && clangUnit->getOverlayModule())
    return clangUnit->getOverlayModule();
  return clangUnit->getParentModule();
}

PlatformAvailability::PlatformAvailability(const LangOptions &langOpts)
    : platformKind(targetPlatform(langOpts)) {
  switch (platformKind) {
  case PlatformKind::iOS:
  case PlatformKind::iOSApplicationExtension:
  case PlatformKind::macCatalyst:
  case PlatformKind::macCatalystApplicationExtension:
  case PlatformKind::tvOS:
  case PlatformKind::tvOSApplicationExtension:
    deprecatedAsUnavailableMessage =
        "APIs deprecated as of iOS 7 and earlier are unavailable in Codira";
    asyncDeprecatedAsUnavailableMessage =
      "APIs deprecated as of iOS 12 and earlier are not imported as 'async'";
    break;

  case PlatformKind::watchOS:
  case PlatformKind::watchOSApplicationExtension:
    deprecatedAsUnavailableMessage = "";
    asyncDeprecatedAsUnavailableMessage =
      "APIs deprecated as of watchOS 5 and earlier are not imported as "
      "'async'";
    break;

  case PlatformKind::macOS:
  case PlatformKind::macOSApplicationExtension:
    deprecatedAsUnavailableMessage =
        "APIs deprecated as of macOS 10.9 and earlier are unavailable in Codira";
    asyncDeprecatedAsUnavailableMessage =
      "APIs deprecated as of macOS 10.14 and earlier are not imported as "
      "'async'";
    break;

  case PlatformKind::visionOS:
  case PlatformKind::visionOSApplicationExtension:
    break;

  case PlatformKind::FreeBSD:
    deprecatedAsUnavailableMessage = "";
    break;

  case PlatformKind::OpenBSD:
    deprecatedAsUnavailableMessage = "";
    break;

  case PlatformKind::Windows:
    deprecatedAsUnavailableMessage = "";
    break;

  case PlatformKind::none:
    break;
  }
}

bool PlatformAvailability::isPlatformRelevant(StringRef name) const {
  switch (platformKind) {
  case PlatformKind::macOS:
    return name == "macos";
  case PlatformKind::macOSApplicationExtension:
    return name == "macos" || name == "macos_app_extension";

  case PlatformKind::iOS:
    return name == "ios";
  case PlatformKind::iOSApplicationExtension:
    return name == "ios" || name == "ios_app_extension";

  case PlatformKind::macCatalyst:
    return name == "ios" || name == "maccatalyst";
  case PlatformKind::macCatalystApplicationExtension:
    return name == "ios" || name == "ios_app_extension" ||
           name == "maccatalyst" || name == "maccatalyst_app_extension";

  case PlatformKind::tvOS:
    return name == "tvos";
  case PlatformKind::tvOSApplicationExtension:
    return name == "tvos" || name == "tvos_app_extension";

  case PlatformKind::watchOS:
    return name == "watchos";
  case PlatformKind::watchOSApplicationExtension:
    return name == "watchos" || name == "watchos_app_extension";

  case PlatformKind::visionOS:
    return name == "xros" || name == "visionos";
  case PlatformKind::visionOSApplicationExtension:
    return name == "xros" || name == "xros_app_extension" ||
           name == "visionos" || name == "visionos_app_extension";

  case PlatformKind::FreeBSD:
    return name == "freebsd";

  case PlatformKind::OpenBSD:
    return name == "openbsd";

  case PlatformKind::Windows:
    return name == "windows";

  case PlatformKind::none:
    return false;
  }

  toolchain_unreachable("Unexpected platform");
}

bool PlatformAvailability::treatDeprecatedAsUnavailable(
    const language::Core::Decl *clangDecl, const toolchain::VersionTuple &version,
    bool isAsync) const {
  assert(!version.empty() && "Must provide version when deprecated");
  unsigned major = version.getMajor();
  std::optional<unsigned> minor = version.getMinor();

  switch (platformKind) {
  case PlatformKind::none:
    toolchain_unreachable("version but no platform?");

  case PlatformKind::macOS:
  case PlatformKind::macOSApplicationExtension:
    // Anything deprecated by macOS 10.14 is unavailable for async import
    // in Codira.
    if (isAsync && !clangDecl->hasAttr<language::Core::CodiraAsyncAttr>()) {
      return major < 10 ||
          (major == 10 && (!minor.has_value() || minor.value() <= 14));
    }

    // Anything deprecated in OSX 10.9.x and earlier is unavailable in Codira.
    return major < 10 ||
           (major == 10 && (!minor.has_value() || minor.value() <= 9));

  case PlatformKind::iOS:
  case PlatformKind::iOSApplicationExtension:
  case PlatformKind::tvOS:
  case PlatformKind::tvOSApplicationExtension:
    // Anything deprecated by iOS 12 is unavailable for async import
    // in Codira.
    if (isAsync && !clangDecl->hasAttr<language::Core::CodiraAsyncAttr>()) {
      return major <= 12;
    }

    // Anything deprecated in iOS 7.x and earlier is unavailable in Codira.
    return major <= 7;

  case PlatformKind::macCatalyst:
  case PlatformKind::macCatalystApplicationExtension:
    // ClangImporter does not yet support macCatalyst.
    return false;

  case PlatformKind::watchOS:
  case PlatformKind::watchOSApplicationExtension:
    // Anything deprecated by watchOS 5.0 is unavailable for async import
    // in Codira.
    if (isAsync && !clangDecl->hasAttr<language::Core::CodiraAsyncAttr>()) {
      return major <= 5;
    }

    // No deprecation filter on watchOS
    return false;

  case PlatformKind::visionOS:
  case PlatformKind::visionOSApplicationExtension:
    // No deprecation filter on xrOS
    return false;

  case PlatformKind::FreeBSD:
    // No deprecation filter on FreeBSD
    return false;

  case PlatformKind::OpenBSD:
    // No deprecation filter on OpenBSD
    return false;

  case PlatformKind::Windows:
    // No deprecation filter on Windows
    return false;
  }

  toolchain_unreachable("Unexpected platform");
}

ClangImporter::Implementation::Implementation(
    ASTContext &ctx, DependencyTracker *dependencyTracker,
    DWARFImporterDelegate *dwarfImporterDelegate)
    : CodiraContext(ctx), ImportForwardDeclarations(
                             ctx.ClangImporterOpts.ImportForwardDeclarations),
      DisableCodiraBridgeAttr(ctx.ClangImporterOpts.DisableCodiraBridgeAttr),
      BridgingHeaderExplicitlyRequested(
          !ctx.ClangImporterOpts.BridgingHeader.empty()),
      DisableOverlayModules(ctx.ClangImporterOpts.DisableOverlayModules),
      EnableClangSPI(ctx.ClangImporterOpts.EnableClangSPI),
      IsReadingBridgingPCH(false),
      CurrentVersion(ImportNameVersion::fromOptions(ctx.LangOpts)),
      Walker(DiagnosticWalker(*this)), BuffersForDiagnostics(ctx.SourceMgr),
      BridgingHeaderLookupTable(new CodiraLookupTable(nullptr)),
      platformAvailability(ctx.LangOpts), nameImporter(),
      DisableSourceImport(ctx.ClangImporterOpts.DisableSourceImport),
      CodiraDependencyTracker(dependencyTracker),
      DWARFImporter(dwarfImporterDelegate) {}

ClangImporter::Implementation::~Implementation() {
#ifndef NDEBUG
  CodiraContext.SourceMgr.verifyAllBuffers();
#endif
}

ClangImporter::Implementation::DiagnosticWalker::DiagnosticWalker(
    ClangImporter::Implementation &Impl)
    : Impl(Impl) {}

bool ClangImporter::Implementation::DiagnosticWalker::TraverseDecl(
    language::Core::Decl *D) {
  if (!D)
    return true;
  // In some cases, diagnostic notes about types (ex: built-in types) do not
  // have an obvious source location at which to display diagnostics. We
  // provide the location of the closest decl as a reasonable choice.
  toolchain::SaveAndRestore<language::Core::SourceLocation> sar{TypeReferenceSourceLocation,
                                                  D->getBeginLoc()};
  return language::Core::RecursiveASTVisitor<DiagnosticWalker>::TraverseDecl(D);
}

bool ClangImporter::Implementation::DiagnosticWalker::TraverseParmVarDecl(
    language::Core::ParmVarDecl *D) {
  // When the ClangImporter imports functions / methods, the return
  // type is first imported, followed by parameter types in order of
  // declaration. If any type fails to import, the import of the function /
  // method is aborted. This means any parameters after the first to fail to
  // import (the first could be the return type) will not have diagnostics
  // attached. Even though these remaining parameters may have unimportable
  // types, we avoid diagnosing these types as a type diagnosis without a
  // "parameter not imported" note on the referencing param decl is inconsistent
  // behaviour and could be confusing.
  if (Impl.ImportDiagnostics[D].size()) {
    // Since the parameter decl in question has been diagnosed (we didn't bail
    // before importing this param) continue the traversal as normal.
    return language::Core::RecursiveASTVisitor<DiagnosticWalker>::TraverseParmVarDecl(D);
  }

  // If the decl in question has not been diagnosed, traverse "as normal" except
  // avoid traversing to the referenced typed. Note the traversal has been
  // simplified greatly and may need to be modified to support some future
  // diagnostics.
  if (!getDerived().shouldTraversePostOrder())
    if (!WalkUpFromParmVarDecl(D))
      return false;

  if (language::Core::DeclContext *declContext = dyn_cast<language::Core::DeclContext>(D)) {
    for (auto *Child : declContext->decls()) {
      if (!canIgnoreChildDeclWhileTraversingDeclContext(Child))
        if (!TraverseDecl(Child))
          return false;
    }
  }
  if (getDerived().shouldTraversePostOrder())
    if (!WalkUpFromParmVarDecl(D))
      return false;
  return true;
}

bool ClangImporter::Implementation::DiagnosticWalker::VisitDecl(
    language::Core::Decl *D) {
  Impl.emitDiagnosticsForTarget(D);
  return true;
}

bool ClangImporter::Implementation::DiagnosticWalker::VisitMacro(
    const language::Core::MacroInfo *MI) {
  Impl.emitDiagnosticsForTarget(MI);
  for (const language::Core::Token &token : MI->tokens()) {
    Impl.emitDiagnosticsForTarget(&token);
  }
  return true;
}

bool ClangImporter::Implementation::DiagnosticWalker::
    VisitObjCObjectPointerType(language::Core::ObjCObjectPointerType *T) {
  // If an ObjCInterface is pointed to, diagnose it.
  if (const language::Core::ObjCInterfaceDecl *decl = T->getInterfaceDecl()) {
    Impl.emitDiagnosticsForTarget(decl);
  }
  // Diagnose any protocols the pointed to type conforms to.
  for (auto cp = T->qual_begin(), cpEnd = T->qual_end(); cp != cpEnd; ++cp) {
    Impl.emitDiagnosticsForTarget(*cp);
  }
  return true;
}

bool ClangImporter::Implementation::DiagnosticWalker::VisitType(
    language::Core::Type *T) {
  if (TypeReferenceSourceLocation.isValid())
    Impl.emitDiagnosticsForTarget(T, TypeReferenceSourceLocation);
  return true;
}

ClangModuleUnit *ClangImporter::Implementation::getWrapperForModule(
    const language::Core::Module *underlying, SourceLoc diagLoc) {
  auto &cacheEntry = ModuleWrappers[underlying];
  if (ClangModuleUnit *cached = cacheEntry.getPointer())
    return cached;

  // FIXME: Handle hierarchical names better.
  Identifier name = underlying->Name == "std"
                        ? CodiraContext.Id_CxxStdlib
                        : CodiraContext.getIdentifier(underlying->Name);
  ImplicitImportInfo implicitImportInfo;
  if (auto mainModule = CodiraContext.MainModule) {
    implicitImportInfo = mainModule->getImplicitImportInfo();
  }
  ClangModuleUnit *file = nullptr;
  auto wrapper = ModuleDecl::create(name, CodiraContext, implicitImportInfo,
                                    [&](ModuleDecl *wrapper, auto addFile) {
    file = new (CodiraContext) ClangModuleUnit(*wrapper, *this, underlying);
    addFile(file);
  });
  wrapper->setIsSystemModule(underlying->IsSystem);
  wrapper->setIsNonCodiraModule();
  wrapper->setHasResolvedImports();
  if (!underlying->ExportAsModule.empty())
    wrapper->setExportAsName(
        CodiraContext.getIdentifier(underlying->ExportAsModule));

  CodiraContext.getClangModuleLoader()->findOverlayFiles(diagLoc, wrapper, file);
  cacheEntry.setPointer(file);

  return file;
}

ClangModuleUnit *ClangImporter::Implementation::getClangModuleForDecl(
    const language::Core::Decl *D,
    bool allowForwardDeclaration) {
  auto maybeModule = getClangSubmoduleForDecl(D, allowForwardDeclaration);
  if (!maybeModule)
    return nullptr;
  if (!maybeModule.value())
    return ImportedHeaderUnit;

  // Get the parent module because currently we don't represent submodules with
  // ClangModuleUnit.
  auto *M = maybeModule.value()->getTopLevelModule();

  return getWrapperForModule(M);
}

void ClangImporter::Implementation::addImportDiagnostic(
    ImportDiagnosticTarget target, Diagnostic &&diag,
    language::Core::SourceLocation loc) {
  ImportDiagnostic importDiag = ImportDiagnostic(target, diag, loc);
  if (CodiraContext.LangOpts.DisableExperimentalClangImporterDiagnostics)
    return;
  auto [_, inserted] = CollectedDiagnostics.insert(importDiag);
  if (!inserted)
    return;
  ImportDiagnostics[target].push_back(importDiag);
}

#pragma mark Source locations
language::Core::SourceLocation
ClangImporter::Implementation::exportSourceLoc(SourceLoc loc) {
  // FIXME: Implement!
  return language::Core::SourceLocation();
}

SourceLoc
ClangImporter::Implementation::importSourceLoc(language::Core::SourceLocation loc) {
  return BuffersForDiagnostics.resolveSourceLocation(Instance->getSourceManager(), loc);
}

SourceRange
ClangImporter::Implementation::importSourceRange(language::Core::SourceRange range) {
  return SourceRange(importSourceLoc(range.getBegin()), importSourceLoc(range.getEnd()));
}

#pragma mark Importing names

language::Core::DeclarationName
ClangImporter::Implementation::exportName(Identifier name) {
  // FIXME: When we start dealing with C++, we can map over some operator
  // names.
  if (name.empty() || name.isOperator())
    return language::Core::DeclarationName();

  // Map the identifier. If it's some kind of keyword, it can't be mapped.
  auto ident = &Instance->getASTContext().Idents.get(name.str());
  if (ident->getTokenID() != language::Core::tok::identifier)
    return language::Core::DeclarationName();

  return ident;
}

Identifier
ClangImporter::Implementation::importIdentifier(
  const language::Core::IdentifierInfo *identifier,
  StringRef removePrefix)
{
  if (!identifier) return Identifier();

  StringRef name = identifier->getName();
  // Remove the prefix, if any.
  if (!removePrefix.empty()) {
    if (name.starts_with(removePrefix)) {
      name = name.slice(removePrefix.size(), name.size());
    }
  }

  // Get the Codira identifier.
  return CodiraContext.getIdentifier(name);
}

ObjCSelector ClangImporter::Implementation::importSelector(
               language::Core::Selector selector) {
  auto &ctx = CodiraContext;

  // Handle zero-argument selectors directly.
  if (selector.isUnarySelector()) {
    Identifier name;
    if (auto id = selector.getIdentifierInfoForSlot(0))
      name = ctx.getIdentifier(id->getName());
    return ObjCSelector(ctx, 0, name);
  }

  SmallVector<Identifier, 2> pieces;
  for (auto i = 0u, n = selector.getNumArgs(); i != n; ++i) {
    Identifier piece;
    if (auto id = selector.getIdentifierInfoForSlot(i))
      piece = ctx.getIdentifier(id->getName());
    pieces.push_back(piece);
  }

  return ObjCSelector(ctx, pieces.size(), pieces);
}

language::Core::Selector
ClangImporter::Implementation::exportSelector(DeclName name,
                                              bool allowSimpleName) {
  if (!allowSimpleName && name.isSimpleName())
    return {};

  language::Core::ASTContext &ctx = getClangASTContext();

  SmallVector<const language::Core::IdentifierInfo *, 8> pieces;
  pieces.push_back(exportName(name.getBaseIdentifier()).getAsIdentifierInfo());

  auto argNames = name.getArgumentNames();
  if (argNames.empty())
    return ctx.Selectors.getNullarySelector(pieces.front());

  if (!argNames.front().empty())
    return {};
  argNames = argNames.slice(1);

  for (Identifier argName : argNames)
    pieces.push_back(exportName(argName).getAsIdentifierInfo());

  return ctx.Selectors.getSelector(pieces.size(), pieces.data());
}

language::Core::Selector
ClangImporter::Implementation::exportSelector(ObjCSelector selector) {
  SmallVector<const language::Core::IdentifierInfo *, 4> pieces;
  for (auto piece : selector.getSelectorPieces())
    pieces.push_back(exportName(piece).getAsIdentifierInfo());
  return getClangASTContext().Selectors.getSelector(selector.getNumArgs(),
                                                    pieces.data());
}

/// Determine whether the given method potentially conflicts with the
/// setter for a property in the given protocol.
static bool
isPotentiallyConflictingSetter(const language::Core::ObjCProtocolDecl *proto,
                               const language::Core::ObjCMethodDecl *method) {
  auto sel = method->getSelector();
  if (sel.getNumArgs() != 1)
    return false;

  const language::Core::IdentifierInfo *setterID = sel.getIdentifierInfoForSlot(0);
  if (!setterID || !setterID->getName().starts_with("set"))
    return false;

  for (auto *prop : proto->properties()) {
    if (prop->getSetterName() == sel)
      return true;
  }

  return false;
}

bool importer::shouldSuppressDeclImport(const language::Core::Decl *decl) {
  if (auto objcMethod = dyn_cast<language::Core::ObjCMethodDecl>(decl)) {
    // First check if we're actually in a Codira class.
    auto dc = decl->getDeclContext();
    if (hasNativeCodiraDecl(cast<language::Core::ObjCContainerDecl>(dc)))
      return true;

    // If this member is a method that is a getter or setter for a
    // property, don't add it into the table. property names and
    // getter names (by choosing to only have a property).
    //
    // Note that this is suppressed for certain accessibility declarations,
    // which are imported as getter/setter pairs and not properties.
    if (objcMethod->isPropertyAccessor()) {
      // Suppress the import of this method when the corresponding
      // property is not suppressed.
      return !shouldSuppressDeclImport(
               objcMethod->findPropertyDecl(/*CheckOverrides=*/false));
    }

    // If the method was declared within a protocol, check that it
    // does not conflict with the setter of a property.
    if (auto proto = dyn_cast<language::Core::ObjCProtocolDecl>(dc))
      return isPotentiallyConflictingSetter(proto, objcMethod);


    return false;
  }

  if (auto objcProperty = dyn_cast<language::Core::ObjCPropertyDecl>(decl)) {
    // First check if we're actually in a Codira class.
    auto dc = objcProperty->getDeclContext();
    if (hasNativeCodiraDecl(cast<language::Core::ObjCContainerDecl>(dc)))
      return true;

    // Suppress certain properties; import them as getter/setter pairs instead.
    if (shouldImportPropertyAsAccessors(objcProperty))
      return true;

    // Check whether there is a superclass method for the getter that
    // is *not* suppressed, in which case we will need to suppress
    // this property.
    auto objcClass = dyn_cast<language::Core::ObjCInterfaceDecl>(dc);
    if (!objcClass) {
      if (auto objcCategory = dyn_cast<language::Core::ObjCCategoryDecl>(dc)) {
        // If the enclosing category is invalid, suppress this declaration.
        if (objcCategory->isInvalidDecl()) return true;

        objcClass = objcCategory->getClassInterface();
      }
    }

    if (objcClass) {
      if (auto objcSuperclass = objcClass->getSuperClass()) {
        auto getterMethod =
            objcSuperclass->lookupMethod(objcProperty->getGetterName(),
                                         objcProperty->isInstanceProperty());
        if (getterMethod && !shouldSuppressDeclImport(getterMethod))
          return true;
      }
    }

    return false;
  }

  if (isa<language::Core::BuiltinTemplateDecl>(decl)) {
    return true;
  }

  return false;
}

#pragma mark Name lookup
const language::Core::TypedefNameDecl *
ClangImporter::Implementation::lookupTypedef(language::Core::DeclarationName name) {
  language::Core::Sema &sema = Instance->getSema();
  language::Core::LookupResult lookupResult(sema, name,
                                   language::Core::SourceLocation(),
                                   language::Core::Sema::LookupOrdinaryName);

  if (sema.LookupName(lookupResult, sema.TUScope)) {
    for (auto decl : lookupResult) {
      if (auto typedefDecl =
          dyn_cast<language::Core::TypedefNameDecl>(decl->getUnderlyingDecl()))
        return typedefDecl;
    }
  }

  return nullptr;
}

static bool isDeclaredInModule(const ClangModuleUnit *ModuleFilter,
                               const Decl *VD) {
  // Sometimes imported decls get put into the clang header module. If we
  // found one of these decls, don't filter it out.
  if (VD->getModuleContext()->getName().str() == CLANG_HEADER_MODULE_NAME) {
    return true;
  }
  // Because the ClangModuleUnit saved as a decl context will be saved as the top-level module, but
  // the ModuleFilter we're given might be a submodule (if a submodule was passed to
  // getTopLevelDecls, for example), we should compare the underlying Clang modules to determine
  // module membership.
  if (auto ClangNode = VD->getClangNode()) {
    if (auto *ClangModule = ClangNode.getOwningClangModule()) {
      return ModuleFilter->getClangModule() == ClangModule;
    }
  }
  auto ContainingUnit = VD->getDeclContext()->getModuleScopeContext();
  return ModuleFilter == ContainingUnit;
}

static const language::Core::Module *
getClangOwningModule(ClangNode Node, const language::Core::ASTContext &ClangCtx) {
  assert(!Node.getAsModule() && "not implemented for modules");

  if (const language::Core::Decl *D = Node.getAsDecl()) {
    auto ExtSource = ClangCtx.getExternalSource();
    assert(ExtSource);

    auto originalDecl = D;
    if (auto functionDecl = dyn_cast<language::Core::FunctionDecl>(D)) {
      if (auto pattern = functionDecl->getTemplateInstantiationPattern()) {
        // Function template instantiations don't have an owning Clang module.
        // Let's use the owning module of the template pattern.
        originalDecl = pattern;
      }
    }
    if (!originalDecl->hasOwningModule()) {
      if (auto cxxRecordDecl = dyn_cast<language::Core::CXXRecordDecl>(D)) {
        if (auto pattern = cxxRecordDecl->getTemplateInstantiationPattern()) {
          // Class template instantiations sometimes don't have an owning Clang
          // module, if the instantiation is not typedef-ed.
          originalDecl = pattern;
        }
      }
    }

    return ExtSource->getModule(originalDecl->getOwningModuleID());
  }

  if (const language::Core::ModuleMacro *M = Node.getAsModuleMacro())
    return M->getOwningModule();

  // A locally-defined MacroInfo does not have an owning module.
  assert(Node.getAsMacroInfo());
  return nullptr;
}

static const language::Core::Module *
getClangTopLevelOwningModule(ClangNode Node,
                             const language::Core::ASTContext &ClangCtx) {
  const language::Core::Module *OwningModule = getClangOwningModule(Node, ClangCtx);
  if (!OwningModule)
    return nullptr;
  return OwningModule->getTopLevelModule();
}

static bool isVisibleFromModule(const ClangModuleUnit *ModuleFilter,
                                ValueDecl *VD) {
  assert(ModuleFilter);

  auto ContainingUnit = VD->getDeclContext()->getModuleScopeContext();
  if (ModuleFilter == ContainingUnit)
    return true;

  // The rest of this function is looking to see if the Clang entity that
  // caused VD to be imported has redeclarations in the filter module.
  auto Wrapper = dyn_cast<ClangModuleUnit>(ContainingUnit);
  if (!Wrapper)
    return false;

  ASTContext &Ctx = ContainingUnit->getASTContext();
  auto *Importer = static_cast<ClangImporter *>(Ctx.getClangModuleLoader());
  auto ClangNode = Importer->getEffectiveClangNode(VD);

  // Macros can be "redeclared" by putting an equivalent definition in two
  // different modules. (We don't actually check the equivalence.)
  // FIXME: We're also not checking if the redeclaration is in /this/ module.
  if (ClangNode.getAsMacro())
    return true;

  const language::Core::Decl *D = ClangNode.castAsDecl();
  auto &ClangASTContext = ModuleFilter->getClangASTContext();
  // We don't handle Clang submodules; pop everything up to the top-level
  // module.
  auto OwningClangModule = getClangTopLevelOwningModule(ClangNode,
                                                        ClangASTContext);
  if (OwningClangModule == ModuleFilter->getClangModule())
    return true;

  // If this decl was implicitly synthesized by the compiler, and is not
  // supposed to be owned by any module, return true.
  if (Importer->isSynthesizedAndVisibleFromAllModules(D)) {
    return true;
  }

  // Friends from class templates don't have an owning module. Just return true.
  if (isa<language::Core::FunctionDecl>(D) &&
      cast<language::Core::FunctionDecl>(D)->isThisDeclarationInstantiatedFromAFriendDefinition())
    return true;

  // Handle redeclarable Clang decls by checking each redeclaration.
  bool IsTagDecl = isa<language::Core::TagDecl>(D);
  if (!(IsTagDecl || isa<language::Core::FunctionDecl>(D) || isa<language::Core::VarDecl>(D) ||
        isa<language::Core::TypedefNameDecl>(D) || isa<language::Core::NamespaceDecl>(D))) {
    return false;
  }

  for (auto Redeclaration : D->redecls()) {
    if (Redeclaration == D)
      continue;

    // For enums, structs, and unions, only count definitions when looking to
    // see what other modules they appear in.
    if (IsTagDecl) {
      auto TD = cast<language::Core::TagDecl>(Redeclaration);
      if (!TD->isCompleteDefinition() &&
          !TD->isThisDeclarationADemotedDefinition())
        continue;
    }

    auto OwningClangModule = getClangTopLevelOwningModule(Redeclaration,
                                                          ClangASTContext);
    if (OwningClangModule == ModuleFilter->getClangModule())
      return true;
  }

  return false;
}


namespace {
class ClangVectorDeclConsumer : public language::Core::VisibleDeclConsumer {
  std::vector<language::Core::NamedDecl *> results;
public:
  ClangVectorDeclConsumer() = default;

  void FoundDecl(language::Core::NamedDecl *ND, language::Core::NamedDecl *Hiding,
                 language::Core::DeclContext *Ctx, bool InBaseClass) override {
    if (!ND->getIdentifier())
      return;

    if (ND->isModulePrivate())
      return;

    results.push_back(ND);
  }

  toolchain::MutableArrayRef<language::Core::NamedDecl *> getResults() {
    return results;
  }
};

class FilteringVisibleDeclConsumer : public language::VisibleDeclConsumer {
  language::VisibleDeclConsumer &NextConsumer;
  const ClangModuleUnit *ModuleFilter;

public:
  FilteringVisibleDeclConsumer(language::VisibleDeclConsumer &consumer,
                               const ClangModuleUnit *CMU)
      : NextConsumer(consumer), ModuleFilter(CMU) {
    assert(CMU);
  }

  void foundDecl(ValueDecl *VD, DeclVisibilityKind Reason,
                 DynamicLookupInfo dynamicLookupInfo) override {
    if (!VD->hasClangNode() || isVisibleFromModule(ModuleFilter, VD))
      NextConsumer.foundDecl(VD, Reason, dynamicLookupInfo);
  }
};

class FilteringDeclaredDeclConsumer : public language::VisibleDeclConsumer {
  language::VisibleDeclConsumer &NextConsumer;
  const ClangModuleUnit *ModuleFilter;

public:
  FilteringDeclaredDeclConsumer(language::VisibleDeclConsumer &consumer,
                                const ClangModuleUnit *CMU)
      : NextConsumer(consumer), ModuleFilter(CMU) {
    assert(CMU);
  }

  void foundDecl(ValueDecl *VD, DeclVisibilityKind Reason,
                 DynamicLookupInfo dynamicLookupInfo) override {
    if (isDeclaredInModule(ModuleFilter, VD)) {
      NextConsumer.foundDecl(VD, Reason, dynamicLookupInfo);
    }
  }
};

/// A hack to hide particular types in the "Darwin" module on Apple platforms.
class DarwinLegacyFilterDeclConsumer : public language::VisibleDeclConsumer {
  language::VisibleDeclConsumer &NextConsumer;
  language::Core::ASTContext &ClangASTContext;

  bool shouldDiscard(ValueDecl *VD) {
    if (!VD->hasClangNode())
      return false;

    const language::Core::Module *clangModule = getClangOwningModule(VD->getClangNode(),
                                                            ClangASTContext);
    if (!clangModule)
      return false;

    if (clangModule->Name == "MacTypes") {
      if (!VD->hasName() || VD->getBaseName().isSpecial())
        return true;
      return toolchain::StringSwitch<bool>(VD->getBaseName().userFacingName())
          .Cases("OSErr", "OSStatus", "OptionBits", false)
          .Cases("FourCharCode", "OSType", false)
          .Case("Boolean", false)
          .Case("kUnknownType", false)
          .Cases("UTF32Char", "UniChar", "UTF16Char", "UTF8Char", false)
          .Case("ProcessSerialNumber", false)
          .Default(true);
    }

    if (clangModule->Parent &&
        clangModule->Parent->Name == "CarbonCore") {
      return toolchain::StringSwitch<bool>(clangModule->Name)
          .Cases("BackupCore", "DiskSpaceRecovery", "MacErrors", false)
          .Case("UnicodeUtilities", false)
          .Default(true);
    }

    if (clangModule->Parent &&
        clangModule->Parent->Name == "OSServices") {
      // Note that this is a list of things to /drop/ rather than to /keep/.
      // We're more likely to see new, modern headers added to OSServices.
      return toolchain::StringSwitch<bool>(clangModule->Name)
          .Cases("IconStorage", "KeychainCore", "Power", true)
          .Cases("SecurityCore", "SystemSound", true)
          .Cases("WSMethodInvocation", "WSProtocolHandler", "WSTypes", true)
          .Default(false);
    }

    return false;
  }

public:
  DarwinLegacyFilterDeclConsumer(language::VisibleDeclConsumer &consumer,
                                 language::Core::ASTContext &clangASTContext)
      : NextConsumer(consumer), ClangASTContext(clangASTContext) {}

  static bool needsFiltering(const language::Core::Module *topLevelModule) {
    return topLevelModule && (topLevelModule->Name == "Darwin" ||
                              topLevelModule->Name == "CoreServices");
  }

  void foundDecl(ValueDecl *VD, DeclVisibilityKind Reason,
                 DynamicLookupInfo dynamicLookupInfo) override {
    if (!shouldDiscard(VD))
      NextConsumer.foundDecl(VD, Reason, dynamicLookupInfo);
  }
};

} // unnamed namespace

/// Translate a MacroDefinition to a ClangNode, either a ModuleMacro for
/// a definition imported from a module or a MacroInfo for a macro defined
/// locally.
ClangNode getClangNodeForMacroDefinition(language::Core::MacroDefinition &M) {
  if (!M.getModuleMacros().empty())
    return ClangNode(M.getModuleMacros().back()->getMacroInfo());
  if (auto *MD = M.getLocalDirective())
    return ClangNode(MD->getMacroInfo());
  return ClangNode();
}

void ClangImporter::lookupBridgingHeaderDecls(
                              toolchain::function_ref<bool(ClangNode)> filter,
                              toolchain::function_ref<void(Decl*)> receiver) const {
  for (auto &Import : Impl.BridgeHeaderTopLevelImports) {
    auto ImportD = Import.get<ImportDecl*>();
    if (filter(ImportD->getClangDecl()))
      receiver(ImportD);
  }
  for (auto *ClangD : Impl.BridgeHeaderTopLevelDecls) {
    if (filter(ClangD)) {
      if (auto *ND = dyn_cast<language::Core::NamedDecl>(ClangD)) {
        if (Decl *imported = Impl.importDeclReal(ND, Impl.CurrentVersion))
          receiver(imported);
      }
    }
  }

  auto &ClangPP = Impl.getClangPreprocessor();
  for (language::Core::IdentifierInfo *II : Impl.BridgeHeaderMacros) {
    auto MD = ClangPP.getMacroDefinition(II);
    if (auto macroNode = getClangNodeForMacroDefinition(MD)) {
      if (filter(macroNode)) {
        auto MI = macroNode.getAsMacro();
        Identifier Name = Impl.getNameImporter().importMacroName(II, MI);
        if (Decl *imported = Impl.importMacro(Name, macroNode))
          receiver(imported);
      }
    }
  }
}

bool ClangImporter::lookupDeclsFromHeader(StringRef Filename,
                              toolchain::function_ref<bool(ClangNode)> filter,
                              toolchain::function_ref<void(Decl*)> receiver) const {
  toolchain::Expected<language::Core::FileEntryRef> ExpectedFile =
      getClangPreprocessor().getFileManager().getFileRef(Filename);
  if (!ExpectedFile)
    return true;
  language::Core::FileEntryRef File = *ExpectedFile;

  auto &ClangCtx = getClangASTContext();
  auto &ClangSM = ClangCtx.getSourceManager();
  auto &ClangPP = getClangPreprocessor();

  // Look up the header in the includes of the bridging header.
  if (Impl.BridgeHeaderFiles.count(File)) {
    auto headerFilter = [&](ClangNode ClangN) -> bool {
      if (ClangN.isNull())
        return false;

      auto ClangLoc = ClangSM.getFileLoc(ClangN.getLocation());
      if (ClangLoc.isInvalid())
        return false;

      language::Core::OptionalFileEntryRef LocRef =
          ClangSM.getFileEntryRefForID(ClangSM.getFileID(ClangLoc));
      if (!LocRef || *LocRef != File)
        return false;

      return filter(ClangN);
    };

    lookupBridgingHeaderDecls(headerFilter, receiver);
    return false;
  }

  language::Core::FileID FID = ClangSM.translateFile(File);
  if (FID.isInvalid())
    return false;

  // Look up the header in the ASTReader.
  if (ClangSM.isLoadedFileID(FID)) {
    // Decls.
    SmallVector<language::Core::Decl *, 32> Decls;
    unsigned Length = ClangSM.getFileIDSize(FID);
    ClangCtx.getExternalSource()->FindFileRegionDecls(FID, 0, Length, Decls);
    for (auto *ClangD : Decls) {
      if (Impl.shouldIgnoreBridgeHeaderTopLevelDecl(ClangD))
        continue;
      if (filter(ClangD)) {
        if (auto *ND = dyn_cast<language::Core::NamedDecl>(ClangD)) {
          if (Decl *imported = Impl.importDeclReal(ND, Impl.CurrentVersion))
            receiver(imported);
        }
      }
    }

    // Macros.
    for (const auto &Iter : ClangPP.macros()) {
      auto *II = Iter.first;
      auto MD = ClangPP.getMacroDefinition(II);
      MD.forAllDefinitions([&](language::Core::MacroInfo *Info) {
        if (Info->isBuiltinMacro())
          return;

        auto Loc = Info->getDefinitionLoc();
        if (Loc.isInvalid() || ClangSM.getFileID(Loc) != FID)
          return;

        ClangNode MacroNode = Info;
        if (filter(MacroNode)) {
          auto Name = Impl.getNameImporter().importMacroName(II, Info);
          if (auto *Imported = Impl.importMacro(Name, MacroNode))
            receiver(Imported);
        }
      });
    }
    // FIXME: Module imports inside that header.
    return false;
  }

  return true; // no info found about that header.
}

void ClangImporter::lookupValue(DeclName name, VisibleDeclConsumer &consumer) {
  Impl.forEachLookupTable([&](CodiraLookupTable &table) -> bool {
    Impl.lookupValue(table, name, consumer);
    return false;
  });
}

ClangNode ClangImporter::getEffectiveClangNode(const Decl *decl) const {
  // Directly...
  if (auto clangNode = decl->getClangNode())
    return clangNode;

  // Or via the nested "Code" enum.
  if (auto *errorWrapper = dyn_cast<StructDecl>(decl)) {
    if (auto *code = Impl.lookupErrorCodeEnum(errorWrapper))
      if (auto clangNode = code->getClangNode())
        return clangNode;
  }

  return ClangNode();
}

void ClangImporter::lookupTypeDecl(
    StringRef rawName, ClangTypeKind kind,
    toolchain::function_ref<void(TypeDecl *)> receiver) {
  language::Core::DeclarationName clangName(
      &Impl.Instance->getASTContext().Idents.get(rawName));

  SmallVector<language::Core::Sema::LookupNameKind, 1> lookupKinds;
  switch (kind) {
  case ClangTypeKind::Typedef:
    lookupKinds.push_back(language::Core::Sema::LookupOrdinaryName);
    break;
  case ClangTypeKind::Tag:
    lookupKinds.push_back(language::Core::Sema::LookupTagName);
    lookupKinds.push_back(language::Core::Sema::LookupNamespaceName);
    break;
  case ClangTypeKind::ObjCProtocol:
    lookupKinds.push_back(language::Core::Sema::LookupObjCProtocolName);
    break;
  }

  // Perform name lookup into the global scope.
  auto &sema = Impl.Instance->getSema();
  bool foundViaClang = false;

  for (auto lookupKind : lookupKinds) {
    language::Core::LookupResult lookupResult(sema, clangName, language::Core::SourceLocation(),
                                     lookupKind);
    if (!Impl.DisableSourceImport &&
        sema.LookupName(lookupResult, /*Scope=*/ sema.TUScope)) {
      for (auto clangDecl : lookupResult) {
        if (!isa<language::Core::TypeDecl>(clangDecl) &&
            !isa<language::Core::NamespaceDecl>(clangDecl) &&
            !isa<language::Core::ObjCContainerDecl>(clangDecl) &&
            !isa<language::Core::ObjCCompatibleAliasDecl>(clangDecl)) {
          continue;
        }
        Decl *imported = Impl.importDecl(clangDecl, Impl.CurrentVersion);

        // Namespaces are imported as extensions for enums.
        if (auto ext = dyn_cast_or_null<ExtensionDecl>(imported)) {
          imported = ext->getExtendedNominal();
        }
        if (auto *importedType = dyn_cast_or_null<TypeDecl>(imported)) {
          foundViaClang = true;
          receiver(importedType);
        }
      }
    }
  }

  // If Clang couldn't find the type, query the DWARFImporterDelegate.
  if (!foundViaClang)
    Impl.lookupTypeDeclDWARF(rawName, kind, receiver);
}

void ClangImporter::lookupRelatedEntity(
    StringRef rawName, ClangTypeKind kind, StringRef relatedEntityKind,
    toolchain::function_ref<void(TypeDecl *)> receiver) {
  using CISTAttr = ClangImporterSynthesizedTypeAttr;
  if (relatedEntityKind ==
        CISTAttr::manglingNameForKind(CISTAttr::Kind::NSErrorWrapper) ||
      relatedEntityKind ==
        CISTAttr::manglingNameForKind(CISTAttr::Kind::NSErrorWrapperAnon)) {
    auto underlyingKind = ClangTypeKind::Tag;
    if (relatedEntityKind ==
          CISTAttr::manglingNameForKind(CISTAttr::Kind::NSErrorWrapperAnon)) {
      underlyingKind = ClangTypeKind::Typedef;
    }
    lookupTypeDecl(rawName, underlyingKind,
                   [this, receiver] (const TypeDecl *foundType) {
      auto *enumDecl =
          dyn_cast_or_null<language::Core::EnumDecl>(foundType->getClangDecl());
      if (!enumDecl)
        return;
      if (!Impl.getEnumInfo(enumDecl).isErrorEnum())
        return;
      auto *enclosingType =
          dyn_cast<NominalTypeDecl>(foundType->getDeclContext());
      if (!enclosingType)
        return;
      receiver(enclosingType);
    });
  }
}

void ClangModuleUnit::lookupVisibleDecls(ImportPath::Access accessPath,
                                         VisibleDeclConsumer &consumer,
                                         NLKind lookupKind) const {
  // FIXME: Ignore submodules, which are empty for now.
  if (clangModule && clangModule->isSubModule())
    return;

  // FIXME: Respect the access path.
  FilteringVisibleDeclConsumer filterConsumer(consumer, this);

  DarwinLegacyFilterDeclConsumer darwinFilterConsumer(filterConsumer,
                                                      getClangASTContext());

  language::VisibleDeclConsumer *actualConsumer = &filterConsumer;
  if (lookupKind == NLKind::UnqualifiedLookup &&
      DarwinLegacyFilterDeclConsumer::needsFiltering(clangModule)) {
    actualConsumer = &darwinFilterConsumer;
  }

  // Find the corresponding lookup table.
  if (auto lookupTable = owner.findLookupTable(clangModule)) {
    // Search it.
    owner.lookupVisibleDecls(*lookupTable, *actualConsumer);
  }
}

namespace {
class VectorDeclPtrConsumer : public language::VisibleDeclConsumer {
public:
  SmallVectorImpl<Decl *> &Results;
  explicit VectorDeclPtrConsumer(SmallVectorImpl<Decl *> &Decls)
    : Results(Decls) {}

  void foundDecl(ValueDecl *VD, DeclVisibilityKind Reason,
                 DynamicLookupInfo) override {
    Results.push_back(VD);
  }
};
} // unnamed namespace

void ClangModuleUnit::getTopLevelDecls(SmallVectorImpl<Decl*> &results) const {
  VectorDeclPtrConsumer consumer(results);
  FilteringDeclaredDeclConsumer filterConsumer(consumer, this);
  DarwinLegacyFilterDeclConsumer darwinFilterConsumer(filterConsumer,
                                                      getClangASTContext());

  const language::Core::Module *topLevelModule =
    clangModule ? clangModule->getTopLevelModule() : nullptr;

  language::VisibleDeclConsumer *actualConsumer = &filterConsumer;
  if (DarwinLegacyFilterDeclConsumer::needsFiltering(topLevelModule))
    actualConsumer = &darwinFilterConsumer;

  // Find the corresponding lookup table.
  if (auto lookupTable = owner.findLookupTable(topLevelModule)) {
    // Search it.
    owner.lookupVisibleDecls(*lookupTable, *actualConsumer);

    // Add the extensions produced by importing categories.
    for (auto category : lookupTable->categories()) {
      if (category->getOwningModule() == clangModule) {
        if (auto extension = cast_or_null<ExtensionDecl>(
          owner.importDecl(category, owner.CurrentVersion,
                          /*UseCanonical*/false))) {
          results.push_back(extension);
        }
      }
    }

    auto findEnclosingExtension = [](Decl *importedDecl) -> ExtensionDecl * {
      for (auto importedDC = importedDecl->getDeclContext();
           !importedDC->isModuleContext();
           importedDC = importedDC->getParent()) {
        if (auto ext = dyn_cast<ExtensionDecl>(importedDC))
          return ext;
      }
      return nullptr;
    };
    // Retrieve all of the globals that will be mapped to members.

    toolchain::SmallPtrSet<ExtensionDecl *, 8> knownExtensions;
    for (auto entry : lookupTable->allGlobalsAsMembers()) {
      auto decl = entry.get<language::Core::NamedDecl *>();
      if (decl->getOwningModule() != clangModule) continue;

      Decl *importedDecl = owner.importDecl(decl, owner.CurrentVersion);
      if (!importedDecl) continue;

      // Find the enclosing extension, if there is one.
      ExtensionDecl *ext = findEnclosingExtension(importedDecl);
      if (ext && knownExtensions.insert(ext).second)
        results.push_back(ext);

      // If this is a compatibility typealias, the canonical type declaration
      // may exist in another extension.
      auto alias = dyn_cast<TypeAliasDecl>(importedDecl);
      if (!alias || !alias->isCompatibilityAlias()) continue;

      auto aliasedTy = alias->getUnderlyingType();
      ext = nullptr;
      importedDecl = nullptr;

      // Note: We can't use getAnyGeneric() here because `aliasedTy`
      // might be typealias.
      if (auto Ty = dyn_cast<TypeAliasType>(aliasedTy.getPointer()))
        importedDecl = Ty->getDecl();
      else if (auto Ty = dyn_cast<AnyGenericType>(aliasedTy.getPointer()))
        importedDecl = Ty->getDecl();
      if (!importedDecl) continue;

      ext = findEnclosingExtension(importedDecl);
      if (ext && knownExtensions.insert(ext).second)
        results.push_back(ext);
    }
  }
}

ImportDecl *language::createImportDecl(ASTContext &Ctx,
                                    DeclContext *DC,
                                    ClangNode ClangN,
                                    ArrayRef<language::Core::Module *> Exported) {
  auto *ImportedMod = ClangN.getClangModule();
  assert(ImportedMod);

  ImportPath::Builder importPath;
  auto *TmpMod = ImportedMod;
  while (TmpMod) {
    // If this is a C++ stdlib module, print its name as `CxxStdlib` instead of
    // `std`. `CxxStdlib` is the only accepted spelling of the C++ stdlib module
    // name in Codira.
    Identifier moduleName = !TmpMod->isSubModule() && TmpMod->Name == "std"
                                ? Ctx.Id_CxxStdlib
                                : Ctx.getIdentifier(TmpMod->Name);
    importPath.push_back(moduleName);
    TmpMod = TmpMod->Parent;
  }
  std::reverse(importPath.begin(), importPath.end());

  bool IsExported = false;
  for (auto *ExportedMod : Exported) {
    if (ImportedMod == ExportedMod) {
      IsExported = true;
      break;
    }
  }

  auto *ID = ImportDecl::create(Ctx, DC, SourceLoc(),
                                ImportKind::Module, SourceLoc(),
                                importPath.get(), ClangN);
  if (IsExported)
    ID->getAttrs().add(new (Ctx) ExportedAttr(/*IsImplicit=*/false));
  return ID;
}

static void getImportDecls(ClangModuleUnit *ClangUnit, const language::Core::Module *M,
                           SmallVectorImpl<Decl *> &Results) {
  assert(M);
  SmallVector<language::Core::Module *, 1> Exported;
  M->getExportedModules(Exported);

  ASTContext &Ctx = ClangUnit->getASTContext();

  for (auto *ImportedMod : M->Imports) {
    auto *ID = createImportDecl(Ctx, ClangUnit, ImportedMod, Exported);
    Results.push_back(ID);
  }
}

void ClangModuleUnit::getDisplayDecls(SmallVectorImpl<Decl*> &results, bool recursive) const {
  if (clangModule)
    getImportDecls(const_cast<ClangModuleUnit *>(this), clangModule, results);
  getTopLevelDecls(results);
}

void ClangModuleUnit::lookupValue(DeclName name, NLKind lookupKind,
                                  OptionSet<ModuleLookupFlags> flags,
                                  SmallVectorImpl<ValueDecl*> &results) const {
  // FIXME: Ignore submodules, which are empty for now.
  if (clangModule && clangModule->isSubModule())
    return;

  VectorDeclConsumer vectorWriter(results);
  FilteringVisibleDeclConsumer filteringConsumer(vectorWriter, this);

  DarwinLegacyFilterDeclConsumer darwinFilterConsumer(filteringConsumer,
                                                      getClangASTContext());

  language::VisibleDeclConsumer *consumer = &filteringConsumer;
  if (lookupKind == NLKind::UnqualifiedLookup &&
      DarwinLegacyFilterDeclConsumer::needsFiltering(clangModule)) {
    consumer = &darwinFilterConsumer;
  }

  // Find the corresponding lookup table.
  if (auto lookupTable = owner.findLookupTable(clangModule)) {
    // Search it.
    owner.lookupValue(*lookupTable, name, *consumer);
  }
}

bool ClangImporter::Implementation::isVisibleClangEntry(
    const language::Core::NamedDecl *clangDecl) {
  // For a declaration, check whether the declaration is hidden.
  language::Core::Sema &clangSema = getClangSema();
  if (clangSema.isVisible(clangDecl)) return true;

  // Is any redeclaration visible?
  for (auto redecl : clangDecl->redecls()) {
    if (clangSema.isVisible(cast<language::Core::NamedDecl>(redecl))) return true;
  }

  return false;
}

bool ClangImporter::Implementation::isVisibleClangEntry(
  CodiraLookupTable::SingleEntry entry) {
  if (auto clangDecl = entry.dyn_cast<language::Core::NamedDecl *>()) {
    return isVisibleClangEntry(clangDecl);
  }

  // If it's a macro from a module, check whether the module has been imported.
  if (auto moduleMacro = entry.dyn_cast<language::Core::ModuleMacro *>()) {
    language::Core::Module *module = moduleMacro->getOwningModule();
    return module->NameVisibility == language::Core::Module::AllVisible;
  }

  return true;
}

TypeDecl *
ClangModuleUnit::lookupNestedType(Identifier name,
                                  const NominalTypeDecl *baseType) const {
  // Special case for error code enums: try looking directly into the struct
  // first. But only if it looks like a synthesized error wrapped struct.
  if (name == getASTContext().Id_Code &&
      !baseType->hasClangNode() &&
      isa<StructDecl>(baseType)) {
    auto *wrapperStruct = cast<StructDecl>(baseType);
    if (auto *codeEnum = owner.lookupErrorCodeEnum(wrapperStruct))
      return codeEnum;

    // Otherwise, fall back and try via lookup table.
  }

  auto lookupTable = owner.findLookupTable(clangModule);
  if (!lookupTable)
    return nullptr;

  auto baseTypeContext = owner.getEffectiveClangContext(baseType);
  if (!baseTypeContext)
    return nullptr;

  // FIXME: This is very similar to what's in Implementation::lookupValue and
  // Implementation::loadAllMembers.
  SmallVector<TypeDecl *, 2> results;
  for (auto entry : lookupTable->lookup(SerializedCodiraName(name.str()),
                                        baseTypeContext)) {
    // If the entry is not visible, skip it.
    if (!owner.isVisibleClangEntry(entry)) continue;

    auto *clangDecl = entry.dyn_cast<language::Core::NamedDecl *>();
    if (!clangDecl)
      continue;

    const auto *clangTypeDecl = clangDecl->getMostRecentDecl();

    bool anyMatching = false;
    TypeDecl *originalDecl = nullptr;
    owner.forEachDistinctName(clangTypeDecl,
                              [&](ImportedName newName,
                                  ImportNameVersion nameVersion) -> bool {
      if (anyMatching)
        return true;
      if (!newName.getDeclName().isSimpleName(name))
        return true;

      auto decl = dyn_cast_or_null<TypeDecl>(
          owner.importDeclReal(clangTypeDecl, nameVersion));
      if (!decl)
        return false;

      if (!originalDecl)
        originalDecl = decl;
      else if (originalDecl == decl)
        return true;

      auto *importedContext = decl->getDeclContext()->getSelfNominalTypeDecl();
      if (importedContext != baseType)
        return true;

      assert(decl->getName() == name &&
             "importFullName behaved differently from importDecl");
      results.push_back(decl);
      anyMatching = true;
      return true;
    });
  }

  if (results.size() != 1) {
    // It's possible that two types were import-as-member'd onto the same base
    // type with the same name. In this case, fall back to regular lookup.
    return nullptr;
  }

  return results.front();
}

void ClangImporter::loadExtensions(NominalTypeDecl *nominal,
                                   unsigned previousGeneration) {
  // Determine the effective Clang context for this Codira nominal type.
  auto effectiveClangContext = Impl.getEffectiveClangContext(nominal);
  if (!effectiveClangContext) return;

  // For an Objective-C class, import all of the visible categories.
  if (auto objcClass = dyn_cast_or_null<language::Core::ObjCInterfaceDecl>(
                         effectiveClangContext.getAsDeclContext())) {
    SmallVector<language::Core::NamedDecl *, 4> DelayedCategories;

    // Simply importing the categories adds them to the list of extensions.
    for (const auto *Cat : objcClass->known_categories()) {
      if (getClangSema().isVisible(Cat)) {
        Impl.importDeclReal(Cat, Impl.CurrentVersion);
      }
    }
  }

  // Dig through each of the Codira lookup tables, creating extensions
  // where needed.
  (void)Impl.forEachLookupTable([&](CodiraLookupTable &table) -> bool {
      // FIXME: If we already looked at this for this generation,
      // skip.

      for (auto entry : table.allGlobalsAsMembersInContext(effectiveClangContext)) {
        // If the entry is not visible, skip it.
        if (!Impl.isVisibleClangEntry(entry)) continue;

        if (auto decl = entry.dyn_cast<language::Core::NamedDecl *>()) {
          // Import the context of this declaration, which has the
          // side effect of creating instantiations.
          (void)Impl.importDeclContextOf(decl, effectiveClangContext);
        } else {
          toolchain_unreachable("Macros cannot be imported as members.");
        }
      }

      return false;
    });
}

void ClangImporter::loadObjCMethods(
       NominalTypeDecl *typeDecl,
       ObjCSelector selector,
       bool isInstanceMethod,
       unsigned previousGeneration,
       toolchain::TinyPtrVector<AbstractFunctionDecl *> &methods) {
  // TODO: We don't currently need to load methods from imported ObjC protocols.
  auto classDecl = dyn_cast<ClassDecl>(typeDecl);
  if (!classDecl)
    return;

  const auto *objcClass =
      dyn_cast_or_null<language::Core::ObjCInterfaceDecl>(classDecl->getClangDecl());
  if (!objcClass)
    return;

  // Collect the set of visible Objective-C methods with this selector.
  language::Core::Selector clangSelector = Impl.exportSelector(selector);

  AbstractFunctionDecl *method = nullptr;
  auto *objcMethod = objcClass->lookupMethod(
      clangSelector, isInstanceMethod,
      /*shallowCategoryLookup=*/false,
      /*followSuper=*/false);

  if (objcMethod) {
    // If we found a property accessor, import the property.
    if (objcMethod->isPropertyAccessor())
      (void)Impl.importDecl(objcMethod->findPropertyDecl(true),
                            Impl.CurrentVersion);

    method = dyn_cast_or_null<AbstractFunctionDecl>(
        Impl.importDecl(objcMethod, Impl.CurrentVersion));
  }

  // If we didn't find anything, we're done.
  if (method == nullptr)
    return;

  // If we did find something, it might be a duplicate of something we found
  // earlier, because we aren't tracking generation counts for Clang modules.
  // Filter out the duplicates.
  // FIXME: We shouldn't need to do this.
  if (!toolchain::is_contained(methods, method))
    methods.push_back(method);
}

void
ClangModuleUnit::lookupClassMember(ImportPath::Access accessPath,
                                   DeclName name,
                                   SmallVectorImpl<ValueDecl*> &results) const {
  // FIXME: Ignore submodules, which are empty for now.
  if (clangModule && clangModule->isSubModule())
    return;

  VectorDeclConsumer consumer(results);

  // Find the corresponding lookup table.
  if (auto lookupTable = owner.findLookupTable(clangModule)) {
    // Search it.
    owner.lookupObjCMembers(*lookupTable, name, consumer);
  }
}

void ClangModuleUnit::lookupClassMembers(ImportPath::Access accessPath,
                                         VisibleDeclConsumer &consumer) const {
  // FIXME: Ignore submodules, which are empty for now.
  if (clangModule && clangModule->isSubModule())
    return;

  // Find the corresponding lookup table.
  if (auto lookupTable = owner.findLookupTable(clangModule)) {
    // Search it.
    owner.lookupAllObjCMembers(*lookupTable, consumer);
  }
}

void ClangModuleUnit::lookupObjCMethods(
       ObjCSelector selector,
       SmallVectorImpl<AbstractFunctionDecl *> &results) const {
  // FIXME: Ignore submodules, which are empty for now.
  if (clangModule && clangModule->isSubModule())
    return;

  // Map the selector into a Clang selector.
  auto clangSelector = owner.exportSelector(selector);
  if (clangSelector.isNull()) return;

  // Collect all of the Objective-C methods with this selector.
  SmallVector<language::Core::ObjCMethodDecl *, 8> objcMethods;
  auto &clangSema = owner.getClangSema();
  auto &clangObjc = clangSema.ObjC();
  clangObjc.CollectMultipleMethodsInGlobalPool(clangSelector,
                                               objcMethods,
                                               /*InstanceFirst=*/true,
                                               /*CheckTheOther=*/false);
  clangObjc.CollectMultipleMethodsInGlobalPool(clangSelector,
                                               objcMethods,
                                               /*InstanceFirst=*/false,
                                               /*CheckTheOther=*/false);

  // Import the methods.
  auto &clangCtx = clangSema.getASTContext();
  for (auto objcMethod : objcMethods) {
    // Verify that this method came from this module.
    auto owningClangModule = getClangTopLevelOwningModule(objcMethod, clangCtx);
    if (owningClangModule != clangModule) continue;

    if (shouldSuppressDeclImport(objcMethod))
      continue;

    // If we found a property accessor, import the property.
    if (objcMethod->isPropertyAccessor())
      (void)owner.importDecl(objcMethod->findPropertyDecl(true),
                             owner.CurrentVersion);
    Decl *imported = owner.importDecl(objcMethod, owner.CurrentVersion);
    if (!imported) continue;

    if (auto fn = dyn_cast<AbstractFunctionDecl>(imported))
      results.push_back(fn);

    // If there is an alternate declaration, also look at it.
    for (auto alternate : owner.getAlternateDecls(imported)) {
      if (auto fn = dyn_cast<AbstractFunctionDecl>(alternate))
        results.push_back(fn);
    }
  }
}

void ClangModuleUnit::lookupAvailabilityDomains(
    Identifier identifier, SmallVectorImpl<AvailabilityDomain> &results) const {
  auto domainName = identifier.str();
  auto &ctx = getASTContext();
  auto &clangASTContext = getClangASTContext();

  auto domainInfo = clangASTContext.getFeatureAvailInfo(domainName);
  if (domainInfo.Kind == language::Core::FeatureAvailKind::None)
    return;

  auto *varDecl = dyn_cast_or_null<language::Core::VarDecl>(domainInfo.Decl);
  if (!varDecl)
    return;

  // The decl that was found may belong to a different Clang module.
  if (varDecl->getOwningModule() != getClangModule())
    return;

  auto *imported = ctx.getClangModuleLoader()->importDeclDirectly(varDecl);
  if (!imported)
    return;

  auto customDomain = AvailabilityDomain::forCustom(imported, ctx);
  ASSERT(customDomain);
  results.push_back(*customDomain);
}

void ClangModuleUnit::collectLinkLibraries(
    ModuleDecl::LinkLibraryCallback callback) const {
  if (!clangModule)
    return;

  // Skip this lib name in favor of export_as name.
  if (clangModule->UseExportAsModuleLinkName)
    return;

  for (auto clangLinkLib : clangModule->LinkLibraries)
    callback(LinkLibrary{clangLinkLib.Library,
                         clangLinkLib.IsFramework ? LibraryKind::Framework
                                                  : LibraryKind::Library,
                         /*static=*/false});
}

StringRef ClangModuleUnit::getFilename() const {
  if (!clangModule) {
    StringRef SinglePCH = owner.getSinglePCHImport();
    if (SinglePCH.empty())
      return "<imports>";
    else
      return SinglePCH;
  }
  if (auto F = clangModule->getASTFile())
    return F->getName();
  return StringRef();
}

StringRef ClangModuleUnit::getLoadedFilename() const {
  if (auto F = clangModule->getASTFile())
    return F->getName();
  return StringRef();
}

language::Core::TargetInfo &ClangImporter::getModuleAvailabilityTarget() const {
  return Impl.Instance->getTarget();
}

language::Core::TargetInfo &ClangImporter::getTargetInfo() const {
  return *Impl.getCodiraTargetInfo();
}

language::Core::ASTContext &ClangImporter::getClangASTContext() const {
  return Impl.getClangASTContext();
}

language::Core::Preprocessor &ClangImporter::getClangPreprocessor() const {
  return Impl.getClangPreprocessor();
}

const language::Core::CompilerInstance &ClangImporter::getClangInstance() const {
  return *Impl.Instance;
}

const language::Core::Module *ClangImporter::getClangOwningModule(ClangNode Node) const {
  return Impl.getClangOwningModule(Node);
}

const language::Core::Module *
ClangImporter::Implementation::getClangOwningModule(ClangNode Node) const {
  return ::getClangOwningModule(Node, getClangASTContext());
}

bool ClangImporter::hasTypedef(const language::Core::Decl *typeDecl) const {
  return Impl.DeclsWithSuperfluousTypedefs.count(typeDecl);
}

language::Core::Sema &ClangImporter::getClangSema() const {
  return Impl.getClangSema();
}

language::Core::CodeGenOptions &ClangImporter::getCodeGenOpts() const {
  return *Impl.getCodiraCodeGenOptions();
}

std::string ClangImporter::getClangModuleHash() const {
  return Impl.Invocation->getModuleHash(Impl.Instance->getDiagnostics());
}

std::vector<std::string>
ClangImporter::getCodiraExplicitModuleDirectCC1Args() const {
  toolchain::SmallVector<const char*> clangArgs;
  clangArgs.reserve(Impl.ClangArgs.size());
  toolchain::for_each(Impl.ClangArgs, [&](const std::string &Arg) {
    clangArgs.push_back(Arg.c_str());
  });

  language::Core::CompilerInvocation instance;
  language::Core::DiagnosticsEngine clangDiags(new language::Core::DiagnosticIDs(),
                                      new language::Core::DiagnosticOptions(),
                                      new language::Core::IgnoringDiagConsumer());
  bool success = language::Core::CompilerInvocation::CreateFromArgs(instance, clangArgs,
                                                           clangDiags);
  (void)success;
  assert(success && "clang options from clangImporter failed to parse");

  if (!Impl.CodiraContext.CASOpts.EnableCaching)
    return instance.getCC1CommandLine();

  // Clear some options that are not needed.
  instance.clearImplicitModuleBuildOptions();

  // CASOpts are forwarded from language arguments.
  instance.getCASOpts() = language::Core::CASOptions();

  // HeaderSearchOptions.
  // Clang search options are only used by scanner and clang importer from main
  // module should not using search paths to find modules.
  auto &HSOpts = instance.getHeaderSearchOpts();
  HSOpts.VFSOverlayFiles.clear();
  HSOpts.UserEntries.clear();
  HSOpts.SystemHeaderPrefixes.clear();

  // FrontendOptions.
  auto &FEOpts = instance.getFrontendOpts();
  FEOpts.IncludeTimestamps = false;
  FEOpts.ModuleMapFiles.clear();

  // IndexStorePath is forwarded from language.
  FEOpts.IndexStorePath.clear();

  // PreprocessorOptions.
  // Cannot clear macros as the main module clang importer doesn't have clang
  // include tree created and it has to be created from command-line. However,
  // include files are no collected into CASFS so they will not be found so
  // clear them to avoid problem.
  auto &PPOpts = instance.getPreprocessorOpts();
  PPOpts.MacroIncludes.clear();
  PPOpts.Includes.clear();

  // Clear benign CodeGenOptions.
  language::Core::tooling::dependencies::resetBenignCodeGenOptions(
      language::Core::frontend::ActionKind::GenerateModule, instance.getLangOpts(),
      instance.getCodeGenOpts());

  // FileSystemOptions.
  auto &FSOpts = instance.getFileSystemOpts();
  FSOpts.WorkingDir.clear();

  if (!Impl.CodiraContext.SearchPathOpts.ScannerPrefixMapper.empty()) {
    // Remap all the paths if requested.
    toolchain::PrefixMapper Mapper;
    language::Core::tooling::dependencies::DepscanPrefixMapping::configurePrefixMapper(
        Impl.CodiraContext.SearchPathOpts.ScannerPrefixMapper, Mapper);
    language::Core::tooling::dependencies::DepscanPrefixMapping::remapInvocationPaths(
        instance, Mapper);
    instance.getFrontendOpts().PathPrefixMappings.clear();
  }

  return instance.getCC1CommandLine();
}

std::optional<Decl *>
ClangImporter::importDeclCached(const language::Core::NamedDecl *ClangDecl) {
  return Impl.importDeclCached(ClangDecl, Impl.CurrentVersion);
}

void ClangImporter::printStatistics() const {
  Impl.Instance->getASTReader()->PrintStats();
}

void ClangImporter::verifyAllModules() {
#ifndef NDEBUG
  if (Impl.VerifiedDeclsCounter == Impl.ImportedDecls.size())
    return;

  // Collect the Decls before verifying them; the act of verifying may cause
  // more decls to be imported and modify the map while we are iterating it.
  size_t verifiedCounter = Impl.ImportedDecls.size();
  SmallVector<Decl *, 8> Decls;
  for (auto &I : Impl.ImportedDecls)
    if (I.first.second == Impl.CurrentVersion)
      if (Decl *D = I.second)
        Decls.push_back(D);

  for (auto D : Decls)
    verify(D);

  Impl.VerifiedDeclsCounter = verifiedCounter;
#endif
}

const language::Core::Type *
ClangImporter::parseClangFunctionType(StringRef typeStr,
                                      SourceLoc loc) const {
  auto &sema = Impl.getClangSema();
  StringRef filename = Impl.CodiraContext.SourceMgr.getDisplayNameForLoc(loc);
  // TODO: Obtain a language::Core::SourceLocation from the language::SourceLoc we have
  auto parsedType = sema.ParseTypeFromStringCallback(typeStr, filename, {});
  if (!parsedType.isUsable())
    return nullptr;
  language::Core::QualType resultType = language::Core::Sema::GetTypeFromParser(parsedType.get());
  auto *typePtr = resultType.getTypePtrOrNull();
  if (typePtr && (typePtr->isFunctionPointerType()
                  || typePtr->isBlockPointerType()))
      return typePtr;
  return nullptr;
}

void ClangImporter::printClangType(const language::Core::Type *type,
                                   toolchain::raw_ostream &os) const {
  auto policy = language::Core::PrintingPolicy(getClangASTContext().getLangOpts());
  language::Core::QualType(type, 0).print(os, policy);
}

//===----------------------------------------------------------------------===//
// ClangModule Implementation
//===----------------------------------------------------------------------===//

static_assert(IsTriviallyDestructible<ClangModuleUnit>::value,
              "ClangModuleUnits are BumpPtrAllocated; the d'tor is not called");

ClangModuleUnit::ClangModuleUnit(ModuleDecl &M,
                                 ClangImporter::Implementation &owner,
                                 const language::Core::Module *clangModule)
  : LoadedFile(FileUnitKind::ClangModule, M), owner(owner),
    clangModule(clangModule) {
  // Capture the file metadata before it goes away.
  if (clangModule)
    ASTSourceDescriptor = {*const_cast<language::Core::Module *>(clangModule)};
}

StringRef ClangModuleUnit::getModuleDefiningPath() const {
  if (!clangModule || clangModule->DefinitionLoc.isInvalid())
    return "";

  auto &clangSourceMgr = owner.getClangASTContext().getSourceManager();
  return clangSourceMgr.getFilename(clangModule->DefinitionLoc);
}

std::optional<language::Core::ASTSourceDescriptor>
ClangModuleUnit::getASTSourceDescriptor() const {
  if (clangModule) {
    assert(ASTSourceDescriptor.getModuleOrNull() == clangModule);
    return ASTSourceDescriptor;
  }
  return std::nullopt;
}

bool ClangModuleUnit::hasClangModule(ModuleDecl *M) {
  for (auto F : M->getFiles()) {
    if (isa<ClangModuleUnit>(F))
      return true;
  }
  return false;
}

bool ClangModuleUnit::isTopLevel() const {
  return !clangModule || !clangModule->isSubModule();
}

bool ClangModuleUnit::isSystemModule() const {
  return clangModule && clangModule->IsSystem;
}

language::Core::ASTContext &ClangModuleUnit::getClangASTContext() const {
  return owner.getClangASTContext();
}

StringRef ClangModuleUnit::getExportedModuleName() const {
  if (clangModule && !clangModule->ExportAsModule.empty())
    return clangModule->ExportAsModule;

  // Return module real name (see FileUnit::getExportedModuleName)
  return getParentModule()->getRealName().str();
}

ModuleDecl *ClangModuleUnit::getOverlayModule() const {
  if (!clangModule)
    return nullptr;

  if (owner.DisableOverlayModules)
    return nullptr;

  if (!isTopLevel()) {
    // FIXME: Is this correct for submodules?
    auto topLevel = clangModule->getTopLevelModule();
    auto wrapper = owner.getWrapperForModule(topLevel);
    return wrapper->getOverlayModule();
  }

  if (!overlayModule.getInt()) {
    // FIXME: Include proper source location.
    ModuleDecl *M = getParentModule();
    ASTContext &Ctx = M->getASTContext();

    ModuleDecl *overlay = nullptr;
    // During compilation of a textual interface with no formal C++ interop mode,
    // i.e. it was built without C++ interop, avoid querying the 'CxxStdlib' overlay
    // for it, since said overlay was not used during compilation of this module.
    if (!importer::isCxxStdModule(clangModule) || Ctx.LangOpts.FormalCxxInteropMode)
      overlay = Ctx.getOverlayModule(this);

    if (overlay) {
      Ctx.addLoadedModule(overlay);
    } else {
      // FIXME: This is the awful legacy of the old implementation of overlay
      // loading laid bare. Because the previous implementation used
      // ASTContext::getModuleByIdentifier, it consulted the clang importer
      // recursively which forced the current module, its dependencies, and
      // the overlays of those dependencies to load and
      // become visible in the current context. All of the callers of
      // ClangModuleUnit::getOverlayModule are relying on this behavior, and
      // untangling them is going to take a heroic amount of effort.
      // Clang module loading should *never* *ever* be allowed to load unrelated
      // Codira modules.
      ImportPath::Module::Builder builder(M->getName());
      (void) owner.loadModule(SourceLoc(), std::move(builder).get());
    }
    // If this Clang module is a part of the C++ stdlib, and we haven't loaded
    // the overlay for it so far, it is a split libc++ module (e.g. std_vector).
    // Load the CxxStdlib overlay explicitly.
    if (!overlay && importer::isCxxStdModule(clangModule) &&
        Ctx.LangOpts.FormalCxxInteropMode) {
      ImportPath::Module::Builder builder(Ctx.Id_CxxStdlib);
      overlay = owner.loadModule(SourceLoc(), std::move(builder).get());
    }
    auto mutableThis = const_cast<ClangModuleUnit *>(this);
    mutableThis->overlayModule.setPointerAndInt(overlay, true);
  }

  return overlayModule.getPointer();
}

void ClangModuleUnit::getImportedModules(
    SmallVectorImpl<ImportedModule> &imports,
    ModuleDecl::ImportFilter filter) const {
  // Bail out if we /only/ want ImplementationOnly imports; Clang modules never
  // have any of these.
  if (filter.containsOnly(ModuleDecl::ImportFilterKind::ImplementationOnly))
    return;

  // [NOTE: Pure-Clang-modules-privately-import-stdlib]:
  // Needed for implicitly synthesized conformances.
  if (filter.contains(ModuleDecl::ImportFilterKind::Default))
    if (auto stdlib = owner.getStdlibModule())
      imports.push_back({ImportPath::Access(), stdlib});

  SmallVector<language::Core::Module *, 8> imported;
  if (!clangModule) {
    // This is the special "imported headers" module.
    if (filter.contains(ModuleDecl::ImportFilterKind::Exported)) {
      imported.append(owner.ImportedHeaderExports.begin(),
                      owner.ImportedHeaderExports.end());
    }

  } else {
    clangModule->getExportedModules(imported);

    if (filter.contains(ModuleDecl::ImportFilterKind::Default)) {
      // Copy in any modules that are imported but not exported.
      toolchain::SmallPtrSet<language::Core::Module *, 8> knownModules(imported.begin(),
                                                         imported.end());
      if (!filter.contains(ModuleDecl::ImportFilterKind::Exported)) {
        // Remove the exported ones now that we're done with them.
        imported.clear();
      }
      toolchain::copy_if(clangModule->Imports, std::back_inserter(imported),
                    [&](language::Core::Module *mod) {
                     return !knownModules.insert(mod).second;
                    });

      // FIXME: The parent module isn't exactly a private import, but it is
      // needed for link dependencies.
      if (clangModule->Parent)
        imported.push_back(clangModule->Parent);
    }
  }

  auto topLevelOverlay = getOverlayModule();
  for (auto importMod : imported) {
    auto wrapper = owner.getWrapperForModule(importMod);

    auto actualMod = wrapper->getOverlayModule();
    if (!actualMod) {
      // HACK: Deal with imports of submodules by importing the top-level module
      // as well.
      auto importTopLevel = importMod->getTopLevelModule();
      if (importTopLevel != importMod) {
        if (!clangModule || importTopLevel != clangModule->getTopLevelModule()){
          auto topLevelWrapper = owner.getWrapperForModule(importTopLevel);
          imports.push_back({ ImportPath::Access(),
                              topLevelWrapper->getParentModule() });
        }
      }
      actualMod = wrapper->getParentModule();
    } else if (actualMod == topLevelOverlay) {
      actualMod = wrapper->getParentModule();
    }

    assert(actualMod && "Missing imported overlay");
    imports.push_back({ImportPath::Access(), actualMod});
  }
}

void ClangModuleUnit::getImportedModulesForLookup(
    SmallVectorImpl<ImportedModule> &imports) const {

  // Reuse our cached list of imports if we have one.
  if (importedModulesForLookup.has_value()) {
    imports.append(importedModulesForLookup->begin(),
                   importedModulesForLookup->end());
    return;
  }

  size_t firstImport = imports.size();

  SmallVector<language::Core::Module *, 8> imported;
  const language::Core::Module *topLevel;
  ModuleDecl *topLevelOverlay = getOverlayModule();
  if (!clangModule) {
    // This is the special "imported headers" module.
    imported.append(owner.ImportedHeaderExports.begin(),
                    owner.ImportedHeaderExports.end());
    topLevel = nullptr;
  } else {
    clangModule->getExportedModules(imported);
    topLevel = clangModule->getTopLevelModule();

    // If this is a C++ module, implicitly import the Cxx module, which contains
    // definitions of Codira protocols that C++ types might conform to, such as
    // CxxSequence.
    if (owner.CodiraContext.LangOpts.EnableCXXInterop &&
        requiresCPlusPlus(clangModule) && clangModule->Name != CXX_SHIM_NAME) {
      auto *cxxModule =
          owner.CodiraContext.getModuleByIdentifier(owner.CodiraContext.Id_Cxx);
      if (cxxModule)
        imports.push_back({ImportPath::Access(), cxxModule});
    }
  }

  if (imported.empty()) {
    importedModulesForLookup = ArrayRef<ImportedModule>();
    return;
  }

  SmallPtrSet<language::Core::Module *, 32> seen{imported.begin(), imported.end()};
  SmallVector<language::Core::Module *, 8> tmpBuf;
  toolchain::SmallSetVector<language::Core::Module *, 8> topLevelImported;

  // Get the transitive set of top-level imports. That is, if a particular
  // import is a top-level import, add it. Otherwise, keep searching.
  while (!imported.empty()) {
    language::Core::Module *next = imported.pop_back_val();

    // HACK: Deal with imports of submodules by importing the top-level module
    // as well, unless it's the top-level module we're currently in.
    language::Core::Module *nextTopLevel = next->getTopLevelModule();
    if (nextTopLevel != topLevel) {
      topLevelImported.insert(nextTopLevel);

      // Don't continue looking through submodules of modules that have
      // overlays. The overlay might shadow things.
      auto wrapper = owner.getWrapperForModule(nextTopLevel);
      if (wrapper->getOverlayModule())
        continue;
    }

    // Only look through the current module if it's not top-level.
    if (nextTopLevel == next)
      continue;

    next->getExportedModules(tmpBuf);
    for (language::Core::Module *nextImported : tmpBuf) {
      if (seen.insert(nextImported).second)
        imported.push_back(nextImported);
    }
    tmpBuf.clear();
  }

  for (auto importMod : topLevelImported) {
    auto wrapper = owner.getWrapperForModule(importMod);

    ModuleDecl *actualMod = nullptr;
    if (owner.CodiraContext.LangOpts.EnableCXXInterop && topLevel &&
        isCxxStdModule(topLevel) && wrapper->clangModule &&
        isCxxStdModule(wrapper->clangModule)) {
      // The CxxStdlib overlay re-exports the clang module std, which in recent
      // libc++ versions re-exports top-level modules for different std headers
      // (std_string, std_vector, etc). The overlay module for each of the std
      // modules is the CxxStdlib module itself. Make sure we return the actual
      // clang modules (std_xyz) as transitive dependencies instead of just
      // CxxStdlib itself.
      actualMod = wrapper->getParentModule();
    } else {
      actualMod = wrapper->getOverlayModule();
      if (!actualMod || actualMod == topLevelOverlay)
        actualMod = wrapper->getParentModule();
    }

    assert(actualMod && "Missing imported overlay");
    imports.push_back({ImportPath::Access(), actualMod});
  }

  // Cache our results for use next time.
  auto importsToCache = toolchain::ArrayRef(imports).slice(firstImport);
  importedModulesForLookup = getASTContext().AllocateCopy(importsToCache);
}

void ClangImporter::getMangledName(raw_ostream &os,
                                   const language::Core::NamedDecl *clangDecl) const {
  if (!Impl.Mangler)
    Impl.Mangler.reset(getClangASTContext().createMangleContext());

  return Impl.getMangledName(Impl.Mangler.get(), clangDecl, os);
}

void ClangImporter::Implementation::getMangledName(
    language::Core::MangleContext *mangler, const language::Core::NamedDecl *clangDecl,
    raw_ostream &os) {
  if (auto ctor = dyn_cast<language::Core::CXXConstructorDecl>(clangDecl)) {
    auto ctorGlobalDecl =
        language::Core::GlobalDecl(ctor, language::Core::CXXCtorType::Ctor_Complete);
    mangler->mangleCXXName(ctorGlobalDecl, os);
  } else {
    mangler->mangleName(clangDecl, os);
  }
}

// ---------------------------------------------------------------------------
// Codira lookup tables
// ---------------------------------------------------------------------------

CodiraLookupTable *ClangImporter::Implementation::findLookupTable(
                    const language::Core::Module *clangModule) {
  // If the Clang module is null, use the bridging header lookup table.
  if (!clangModule)
    return BridgingHeaderLookupTable.get();

  // Submodules share lookup tables with their parents.
  if (clangModule->isSubModule())
    return findLookupTable(clangModule->getTopLevelModule());

  // Look for a Clang module with this name.
  auto known = LookupTables.find(clangModule->Name);
  if (known == LookupTables.end()) return nullptr;

  return known->second.get();
}

CodiraLookupTable *
ClangImporter::Implementation::findLookupTable(const language::Core::Decl *decl) {
  // Contents of a C++ namespace are added to the __ObjC module.
  bool isWithinNamespace = false;
  auto declContext = decl->getDeclContext();
  while (!declContext->isTranslationUnit()) {
    if (declContext->isNamespace()) {
      isWithinNamespace = true;
      break;
    }
    declContext = declContext->getParent();
  }

  language::Core::Module *owningModule = nullptr;
  if (!isWithinNamespace) {
    // Members of class template specializations don't have an owning module.
    if (auto spec = dyn_cast<language::Core::ClassTemplateSpecializationDecl>(decl))
      owningModule = spec->getSpecializedTemplate()->getOwningModule();
    else
      owningModule = decl->getOwningModule();
  }
  return findLookupTable(owningModule);
}

bool ClangImporter::Implementation::forEachLookupTable(
       toolchain::function_ref<bool(CodiraLookupTable &table)> fn) {
  // Visit the bridging header's lookup table.
  if (fn(*BridgingHeaderLookupTable)) return true;

  // Collect and sort the set of module names.
  SmallVector<StringRef, 4> moduleNames;
  for (const auto &entry : LookupTables) {
    moduleNames.push_back(entry.first);
  }
  toolchain::array_pod_sort(moduleNames.begin(), moduleNames.end());

  // Visit the lookup tables.
  for (auto moduleName : moduleNames) {
    if (fn(*LookupTables[moduleName])) return true;
  }

  return false;
}

bool ClangImporter::Implementation::lookupValue(CodiraLookupTable &table,
                                                DeclName name,
                                                VisibleDeclConsumer &consumer) {

  auto &clangCtx = getClangASTContext();
  auto clangTU = clangCtx.getTranslationUnitDecl();
  auto *importer =
      static_cast<ClangImporter *>(CodiraContext.getClangModuleLoader());

  bool declFound = false;

  if (name.isOperator()) {
    for (auto entry : table.lookupMemberOperators(name.getBaseName())) {
      if (isVisibleClangEntry(entry)) {
        if (auto decl = dyn_cast_or_null<ValueDecl>(
                importDeclReal(entry->getMostRecentDecl(), CurrentVersion))) {
          consumer.foundDecl(decl, DeclVisibilityKind::VisibleAtTopLevel);
          declFound = true;
        }
      }
    }

    // If CXXInterop is enabled we need to check the modified operator name as
    // well
    if (CodiraContext.LangOpts.EnableCXXInterop) {
      auto funcBaseName = DeclBaseName(
          getOperatorName(CodiraContext, name.getBaseName().getIdentifier()));
      for (auto entry : table.lookupMemberOperators(funcBaseName)) {
        if (isVisibleClangEntry(entry)) {
          if (auto fn = dyn_cast_or_null<FuncDecl>(
                  importDeclReal(entry->getMostRecentDecl(), CurrentVersion))) {
            if (auto synthesizedOperator =
                    importer->getCXXSynthesizedOperatorFunc(fn)) {
              consumer.foundDecl(synthesizedOperator,
                                 DeclVisibilityKind::VisibleAtTopLevel);
              declFound = true;
            }
          }
        }
      }
    }
  }

  for (auto entry : table.lookup(name.getBaseName(), clangTU)) {
    // If the entry is not visible, skip it.
    if (!isVisibleClangEntry(entry)) continue;

    ValueDecl *decl = nullptr;
    // If it's a Clang declaration, try to import it.
    if (auto clangDecl = entry.dyn_cast<language::Core::NamedDecl *>()) {
      bool isNamespace = isa<language::Core::NamespaceDecl>(clangDecl);
      Decl *realDecl =
          importDeclReal(clangDecl->getMostRecentDecl(), CurrentVersion,
                         /*useCanonicalDecl*/ !isNamespace);

      if (!realDecl)
        continue;
      decl = cast<ValueDecl>(realDecl);
      if (!decl) continue;
    } else if (!name.isSpecial()) {
      // Try to import a macro.
      if (auto modMacro = entry.dyn_cast<language::Core::ModuleMacro *>())
        decl = importMacro(name.getBaseIdentifier(), modMacro);
      else if (auto clangMacro = entry.dyn_cast<language::Core::MacroInfo *>())
        decl = importMacro(name.getBaseIdentifier(), clangMacro);
      else
        toolchain_unreachable("new kind of lookup table entry");
      if (!decl) continue;
    } else {
      continue;
    }

    // If we found a declaration from the standard library, make sure
    // it does not show up in the lookup results for the imported
    // module.
    if (decl->getDeclContext()->isModuleScopeContext() &&
        decl->getModuleContext() == getStdlibModule())
      continue;

    // If the name matched, report this result.
    bool anyMatching = false;

    // Use the base name for operators; they likely won't have parameters.
    auto foundDeclName = decl->getName();
    if (foundDeclName.isOperator())
      foundDeclName = foundDeclName.getBaseName();

    if (foundDeclName.matchesRef(name) &&
        decl->getDeclContext()->isModuleScopeContext()) {
      consumer.foundDecl(decl, DeclVisibilityKind::VisibleAtTopLevel);
      anyMatching = true;
    }

    // If there is an alternate declaration and the name matches,
    // report this result.
    for (auto alternate : getAlternateDecls(decl)) {
      if (alternate->getName().matchesRef(name) &&
          alternate->getDeclContext()->isModuleScopeContext()) {
        consumer.foundDecl(alternate, DeclVisibilityKind::VisibleAtTopLevel);
        anyMatching = true;
      }
    }

    // Visit auxiliary declarations to check for name matches.
    decl->visitAuxiliaryDecls([&](Decl *aux) {
      if (auto auxValue = dyn_cast<ValueDecl>(aux)) {
        if (auxValue->getName().matchesRef(name) &&
            auxValue->getDeclContext()->isModuleScopeContext()) {
          consumer.foundDecl(auxValue, DeclVisibilityKind::VisibleAtTopLevel);
          anyMatching = true;
        }
      }
    });

    // If we have a declaration and nothing matched so far, try the names used
    // in other versions of Codira.
    if (auto clangDecl = entry.dyn_cast<language::Core::NamedDecl *>()) {
      const language::Core::NamedDecl *recentClangDecl =
          clangDecl->getMostRecentDecl();

      CurrentVersion.forEachOtherImportNameVersion(
          [&](ImportNameVersion nameVersion) {
        if (anyMatching)
          return;

        // Check to see if the name and context match what we expect.
        ImportedName newName = importFullName(recentClangDecl, nameVersion);
        if (!newName.getDeclName().matchesRef(name))
          return;

        // If we asked for an async import and didn't find one, skip this.
        // This filters out duplicates.
        if (nameVersion.supportsConcurrency() &&
            !newName.getAsyncInfo())
          return;

        const language::Core::DeclContext *clangDC =
            newName.getEffectiveContext().getAsDeclContext();
        if (!clangDC || !clangDC->isFileContext())
          return;

        // Then try to import the decl under the alternate name.
        auto alternateNamedDecl =
            cast_or_null<ValueDecl>(importDeclReal(recentClangDecl,
                                                   nameVersion));
        if (!alternateNamedDecl || alternateNamedDecl == decl)
          return;
        assert(alternateNamedDecl->getName().matchesRef(name) &&
               "importFullName behaved differently from importDecl");
        if (alternateNamedDecl->getDeclContext()->isModuleScopeContext()) {
          consumer.foundDecl(alternateNamedDecl,
                             DeclVisibilityKind::VisibleAtTopLevel);
          anyMatching = true;
        }
      });
    }
    declFound = declFound || anyMatching;
  }
  return declFound;
}

void ClangImporter::Implementation::lookupVisibleDecls(
       CodiraLookupTable &table,
       VisibleDeclConsumer &consumer) {
  // Retrieve and sort all of the base names in this particular table.
  auto baseNames = table.allBaseNames();
  toolchain::array_pod_sort(baseNames.begin(), baseNames.end());

  // Look for namespace-scope entities with each base name.
  for (auto baseName : baseNames) {
    DeclBaseName name = baseName.toDeclBaseName(CodiraContext);
    if (!lookupValue(table, name, consumer) &&
        CodiraContext.LangOpts.EnableExperimentalEagerClangModuleDiagnostics) {
      diagnoseTopLevelValue(name);
    }
  }
}

void ClangImporter::Implementation::lookupObjCMembers(
       CodiraLookupTable &table,
       DeclName name,
       VisibleDeclConsumer &consumer) {
  for (auto clangDecl : table.lookupObjCMembers(name.getBaseName())) {
    // If the entry is not visible, skip it.
    if (!isVisibleClangEntry(clangDecl)) continue;

    forEachDistinctName(clangDecl,
                        [&](ImportedName importedName,
                            ImportNameVersion nameVersion) -> bool {
      // Import the declaration.
      auto decl =
          cast_or_null<ValueDecl>(importDeclReal(clangDecl, nameVersion));
      if (!decl)
        return false;

      // If the name we found matches, report the declaration.
      // FIXME: If we didn't need to check alternate decls here, we could avoid
      // importing the member at all by checking importedName ahead of time.
      if (decl->getName().matchesRef(name)) {
        consumer.foundDecl(decl, DeclVisibilityKind::DynamicLookup,
                           DynamicLookupInfo::AnyObject);
      }

      // Check for an alternate declaration; if its name matches,
      // report it.
      for (auto alternate : getAlternateDecls(decl)) {
        if (alternate->getName().matchesRef(name)) {
          consumer.foundDecl(alternate, DeclVisibilityKind::DynamicLookup,
                             DynamicLookupInfo::AnyObject);
        }
      }
      return true;
    });
  }
}

void ClangImporter::Implementation::lookupAllObjCMembers(
       CodiraLookupTable &table,
       VisibleDeclConsumer &consumer) {
  // Retrieve and sort all of the base names in this particular table.
  auto baseNames = table.allBaseNames();
  toolchain::array_pod_sort(baseNames.begin(), baseNames.end());

  // Look for Objective-C members with each base name.
  for (auto baseName : baseNames) {
    lookupObjCMembers(table, baseName.toDeclBaseName(CodiraContext), consumer);
  }
}

void ClangImporter::Implementation::diagnoseTopLevelValue(
    const DeclName &name) {
  forEachLookupTable([&](CodiraLookupTable &table) -> bool {
    for (const auto &entry :
         table.lookup(name.getBaseName(),
                      EffectiveClangContext(
                          getClangASTContext().getTranslationUnitDecl()))) {
      diagnoseTargetDirectly(importDiagnosticTargetFromLookupTableEntry(entry));
    }
    return false;
  });
}

void ClangImporter::Implementation::diagnoseMemberValue(
    const DeclName &name, const language::Core::DeclContext *container) {
  forEachLookupTable([&](CodiraLookupTable &table) -> bool {
    for (const auto &entry :
         table.lookup(name.getBaseName(), EffectiveClangContext(container))) {
      if (language::Core::NamedDecl *nd = entry.get<language::Core::NamedDecl *>()) {
        // We are only interested in members of a particular context,
        // skip other contexts.
        if (nd->getDeclContext() != container)
          continue;

        diagnoseTargetDirectly(
            importDiagnosticTargetFromLookupTableEntry(entry));
      }
      // If the entry is not a NamedDecl, it is a form of macro, which cannot be
      // a member value.
    }
    return false;
  });
}

void ClangImporter::Implementation::diagnoseTargetDirectly(
    ImportDiagnosticTarget target) {
  if (const language::Core::Decl *decl = target.dyn_cast<const language::Core::Decl *>()) {
    Walker.TraverseDecl(const_cast<language::Core::Decl *>(decl));
  } else if (const language::Core::MacroInfo *macro =
                 target.dyn_cast<const language::Core::MacroInfo *>()) {
    Walker.VisitMacro(macro);
  }
}

ImportDiagnosticTarget
ClangImporter::Implementation::importDiagnosticTargetFromLookupTableEntry(
    CodiraLookupTable::SingleEntry entry) {
  if (language::Core::NamedDecl *decl = entry.dyn_cast<language::Core::NamedDecl *>()) {
    return decl;
  } else if (const language::Core::MacroInfo *macro =
                 entry.dyn_cast<language::Core::MacroInfo *>()) {
    return macro;
  } else if (const language::Core::ModuleMacro *macro =
                 entry.dyn_cast<language::Core::ModuleMacro *>()) {
    return macro->getMacroInfo();
  }
  toolchain_unreachable("CodiraLookupTable::Single entry must be a NamedDecl, "
                   "MacroInfo or ModuleMacro pointer");
}

static void diagnoseForeignReferenceTypeFixit(ClangImporter::Implementation &Impl,
                                              HeaderLoc loc, Diagnostic diag) {
  auto importedLoc =
    Impl.CodiraContext.getClangModuleLoader()->importSourceLocation(loc.clangLoc);
  Impl.diagnose(loc, diag).fixItInsert(
      importedLoc, "LANGUAGE_SHARED_REFERENCE(<#retain#>, <#release#>) ");
}

bool ClangImporter::Implementation::emitDiagnosticsForTarget(
    ImportDiagnosticTarget target, language::Core::SourceLocation fallbackLoc) {
  for (auto it = ImportDiagnostics[target].rbegin();
       it != ImportDiagnostics[target].rend(); ++it) {
    HeaderLoc loc = HeaderLoc(it->loc.isValid() ? it->loc : fallbackLoc);
    if (it->diag.getID() == diag::record_not_automatically_importable.ID) {
      diagnoseForeignReferenceTypeFixit(*this, loc, it->diag);
    } else {
      diagnose(loc, it->diag);
    }
  }
  return ImportDiagnostics[target].size();
}

static SmallVector<CodiraLookupTable::SingleEntry, 4>
lookupInClassTemplateSpecialization(
    ASTContext &ctx, const language::Core::ClassTemplateSpecializationDecl *clangDecl,
    DeclName name) {
  // TODO: we could make this faster if we can cache class templates in the
  // lookup table as well.
  // Import all the names to figure out which ones we're looking for.
  SmallVector<CodiraLookupTable::SingleEntry, 4> found;
  for (auto member : clangDecl->decls()) {
    auto namedDecl = dyn_cast<language::Core::NamedDecl>(member);
    if (!namedDecl)
      continue;

    auto memberName = ctx.getClangModuleLoader()->importName(namedDecl);
    if (!memberName)
      continue;

    // Use the base names here because *sometimes* our input name won't have
    // any arguments.
    if (name.getBaseName().compare(memberName.getBaseName()) == 0)
      found.push_back(namedDecl);
  }

  return found;
}

static bool isDirectLookupMemberContext(const language::Core::Decl *foundClangDecl,
                                        const language::Core::Decl *memberContext,
                                        const language::Core::Decl *parent) {
  if (memberContext->getCanonicalDecl() == parent->getCanonicalDecl())
    return true;
  if (auto namespaceDecl = dyn_cast<language::Core::NamespaceDecl>(memberContext)) {
    if (namespaceDecl->isInline()) {
      if (auto memberCtxParent =
              dyn_cast<language::Core::Decl>(namespaceDecl->getParent()))
        return isDirectLookupMemberContext(foundClangDecl, memberCtxParent,
                                           parent);
    }
  }
  // Enum constant decl can be found in the parent context of the enum decl.
  if (auto *ED = dyn_cast<language::Core::EnumDecl>(memberContext)) {
    if (isa<language::Core::EnumConstantDecl>(foundClangDecl)) {
      if (auto *firstDecl = dyn_cast<language::Core::Decl>(ED->getDeclContext()))
        return firstDecl->getCanonicalDecl() == parent->getCanonicalDecl();
    }
  }
  return false;
}

SmallVector<CodiraLookupTable::SingleEntry, 4>
ClangDirectLookupRequest::evaluate(Evaluator &evaluator,
                                   ClangDirectLookupDescriptor desc) const {
  auto &ctx = desc.decl->getASTContext();
  auto *clangDecl = desc.clangDecl;
  // Class templates aren't in the lookup table.
  if (auto spec = dyn_cast<language::Core::ClassTemplateSpecializationDecl>(clangDecl))
    return lookupInClassTemplateSpecialization(ctx, spec, desc.name);

  CodiraLookupTable *lookupTable = nullptr;
  if (isa<language::Core::NamespaceDecl>(clangDecl)) {
    // DeclContext of a namespace imported into Codira is the __ObjC module.
    lookupTable = ctx.getClangModuleLoader()->findLookupTable(nullptr);
  } else {
    auto *clangModule =
        getClangOwningModule(clangDecl, clangDecl->getASTContext());
    lookupTable = ctx.getClangModuleLoader()->findLookupTable(clangModule);
  }

  auto foundDecls = lookupTable->lookup(
      SerializedCodiraName(desc.name.getBaseName()), EffectiveClangContext());
  // Make sure that `clangDecl` is the parent of all the members we found.
  SmallVector<CodiraLookupTable::SingleEntry, 4> filteredDecls;
  toolchain::copy_if(foundDecls, std::back_inserter(filteredDecls),
                [clangDecl](CodiraLookupTable::SingleEntry decl) {
                  auto foundClangDecl = decl.dyn_cast<language::Core::NamedDecl *>();
                  if (!foundClangDecl)
                    return false;
                  auto first = foundClangDecl->getDeclContext();
                  auto second = cast<language::Core::DeclContext>(clangDecl);
                  if (auto firstDecl = dyn_cast<language::Core::Decl>(first)) {
                    if (auto secondDecl = dyn_cast<language::Core::Decl>(second))
                      return isDirectLookupMemberContext(foundClangDecl,
                                                         firstDecl, secondDecl);
                    else
                      return false;
                  }
                  return first == second;
                });
  return filteredDecls;
}

namespace {
  /// Collects name lookup results into the given tiny vector, for use in the
  /// various Clang importer lookup routines.
  class CollectLookupResults {
    DeclName name;
    TinyPtrVector<ValueDecl *> &result;

  public:
    CollectLookupResults(DeclName name, TinyPtrVector<ValueDecl *> &result)
      : name(name), result(result) { }

    void add(ValueDecl *imported) {
      result.push_back(imported);

      // Expand any macros introduced by the Clang importer.
      imported->visitAuxiliaryDecls([&](Decl *decl) {
        auto valueDecl = dyn_cast<ValueDecl>(decl);
        if (!valueDecl)
          return;

        // Bail out if the auxiliary decl was not produced by a macro.
        auto module = decl->getDeclContext()->getParentModule();
        auto *sf = module->getSourceFileContainingLocation(decl->getLoc());
        if (!sf || sf->Kind != SourceFileKind::MacroExpansion)
          return;

        // Only produce results that match the requested name.
        if (!valueDecl->getName().matchesRef(name))
          return;

        result.push_back(valueDecl);
      });
    }
  };
}

TinyPtrVector<ValueDecl *> CXXNamespaceMemberLookup::evaluate(
    Evaluator &evaluator, CXXNamespaceMemberLookupDescriptor desc) const {
  EnumDecl *namespaceDecl = desc.namespaceDecl;
  DeclName name = desc.name;
  auto *clangNamespaceDecl =
      cast<language::Core::NamespaceDecl>(namespaceDecl->getClangDecl());
  auto &ctx = namespaceDecl->getASTContext();

  TinyPtrVector<ValueDecl *> result;
  CollectLookupResults collector(name, result);

  toolchain::SmallPtrSet<language::Core::NamedDecl *, 8> importedDecls;
  for (auto redecl : clangNamespaceDecl->redecls()) {
    auto allResults = evaluateOrDefault(
        ctx.evaluator, ClangDirectLookupRequest({namespaceDecl, redecl, name}),
        {});

    for (auto found : allResults) {
      auto clangMember = found.get<language::Core::NamedDecl *>();
      auto it = importedDecls.insert(clangMember);
      // Skip over members already found during lookup in
      // prior redeclarations.
      if (!it.second)
        continue;
      if (auto import =
              ctx.getClangModuleLoader()->importDeclDirectly(clangMember))
        collector.add(cast<ValueDecl>(import));
    }
  }

  return result;
}

static const toolchain::StringMap<std::vector<int>> STLConditionalEscapableParams{
    {"basic_string", {0}},
    {"vector", {0}},
    {"array", {0}},
    {"inplace_vector", {0}},
    {"deque", {0}},
    {"forward_list", {0}},
    {"list", {0}},
    {"set", {0}},
    {"flat_set", {0}},
    {"unordered_set", {0}},
    {"multiset", {0}},
    {"flat_multiset", {0}},
    {"unordered_multiset", {0}},
    {"stack", {0}},
    {"queue", {0}},
    {"priority_queue", {0}},
    {"tuple", {0}},
    {"variant", {0}},
    {"optional", {0}},
    {"pair", {0, 1}},
    {"expected", {0, 1}},
    {"map", {0, 1}},
    {"flat_map", {0, 1}},
    {"unordered_map", {0, 1}},
    {"multimap", {0, 1}},
    {"flat_multimap", {0, 1}},
    {"unordered_multimap", {0, 1}},
};

static std::set<StringRef>
getConditionalEscapableAttrParams(const language::Core::RecordDecl *decl) {
  std::set<StringRef> result;
  if (!decl->hasAttrs())
    return result;
  for (auto attr : decl->getAttrs()) {
    if (auto languageAttr = dyn_cast<language::Core::CodiraAttrAttr>(attr))
      if (languageAttr->getAttribute().starts_with("escapable_if:")) {
        StringRef params = languageAttr->getAttribute().drop_front(
            StringRef("escapable_if:").size());
        auto commaPos = params.find(',');
        StringRef nextParam = params.take_front(commaPos);
        while (!nextParam.empty() && commaPos != StringRef::npos) {
          result.insert(nextParam.trim());
          params = params.drop_front(nextParam.size() + 1);
          commaPos = params.find(',');
          nextParam = params.take_front(commaPos);
        }
      }
  }
  return result;
}

CxxEscapability
ClangTypeEscapability::evaluate(Evaluator &evaluator,
                                EscapabilityLookupDescriptor desc) const {
  bool hadUnknown = false;
  auto evaluateEscapability = [&](const language::Core::Type *type) {
    auto escapability = evaluateOrDefault(
        evaluator,
        ClangTypeEscapability({type, desc.impl, desc.annotationOnly}),
        CxxEscapability::Unknown);
    if (escapability == CxxEscapability::Unknown)
      hadUnknown = true;
    return escapability;
  };

  auto desugared = desc.type->getUnqualifiedDesugaredType();
  if (const auto *recordType = desugared->getAs<language::Core::RecordType>()) {
    auto recordDecl = recordType->getDecl();
    if (hasNonEscapableAttr(recordDecl))
      return CxxEscapability::NonEscapable;
    if (hasEscapableAttr(recordDecl))
      return CxxEscapability::Escapable;
    auto injectedStlAnnotation =
        recordDecl->isInStdNamespace()
            ? STLConditionalEscapableParams.find(recordDecl->getName())
            : STLConditionalEscapableParams.end();
    bool hasInjectedSTLAnnotation =
        injectedStlAnnotation != STLConditionalEscapableParams.end();
    auto conditionalParams = getConditionalEscapableAttrParams(recordDecl);
    if (!conditionalParams.empty() || hasInjectedSTLAnnotation) {
      auto specDecl = cast<language::Core::ClassTemplateSpecializationDecl>(recordDecl);
      SmallVector<std::pair<unsigned, StringRef>, 4> argumentsToCheck;
      HeaderLoc loc{recordDecl->getLocation()};
      while (specDecl) {
        auto templateDecl = specDecl->getSpecializedTemplate();
        if (hasInjectedSTLAnnotation) {
          auto params = templateDecl->getTemplateParameters();
          for (auto idx : injectedStlAnnotation->second)
            argumentsToCheck.push_back(
                std::make_pair(idx, params->getParam(idx)->getName()));
        } else {
          for (auto [idx, param] :
               toolchain::enumerate(*templateDecl->getTemplateParameters())) {
            if (conditionalParams.erase(param->getName()))
              argumentsToCheck.push_back(std::make_pair(idx, param->getName()));
          }
        }
        auto &argList = specDecl->getTemplateArgs();
        for (auto argToCheck : argumentsToCheck) {
          auto arg = argList[argToCheck.first];
          toolchain::SmallVector<language::Core::TemplateArgument, 1> nonPackArgs;
          if (arg.getKind() == language::Core::TemplateArgument::Pack) {
            auto pack = arg.getPackAsArray();
            nonPackArgs.assign(pack.begin(), pack.end());
          } else
            nonPackArgs.push_back(arg);
          for (auto nonPackArg : nonPackArgs) {
            if (nonPackArg.getKind() != language::Core::TemplateArgument::Type &&
                desc.impl) {
              desc.impl->diagnose(loc, diag::type_template_parameter_expected,
                                  argToCheck.second);
              return CxxEscapability::Unknown;
            }

            auto argEscapability = evaluateEscapability(
                nonPackArg.getAsType()->getUnqualifiedDesugaredType());
            if (argEscapability == CxxEscapability::NonEscapable)
              return CxxEscapability::NonEscapable;
          }
        }
        if (hasInjectedSTLAnnotation)
          break;
        language::Core::DeclContext *dc = specDecl;
        specDecl = nullptr;
        while ((dc = dc->getParent())) {
          specDecl = dyn_cast<language::Core::ClassTemplateSpecializationDecl>(dc);
          if (specDecl)
            break;
        }
      }

      if (desc.impl)
        for (auto name : conditionalParams)
          desc.impl->diagnose(loc, diag::unknown_template_parameter, name);

      return hadUnknown ? CxxEscapability::Unknown : CxxEscapability::Escapable;
    }
    if (desc.annotationOnly)
      return CxxEscapability::Unknown;
    auto cxxRecordDecl = dyn_cast<language::Core::CXXRecordDecl>(recordDecl);
    if (!cxxRecordDecl || cxxRecordDecl->isAggregate()) {
      if (cxxRecordDecl) {
        for (auto base : cxxRecordDecl->bases()) {
          auto baseEscapability = evaluateEscapability(
              base.getType()->getUnqualifiedDesugaredType());
          if (baseEscapability == CxxEscapability::NonEscapable)
            return CxxEscapability::NonEscapable;
        }
      }

      for (auto field : recordDecl->fields()) {
        auto fieldEscapability = evaluateEscapability(
            field->getType()->getUnqualifiedDesugaredType());
        if (fieldEscapability == CxxEscapability::NonEscapable)
          return CxxEscapability::NonEscapable;
      }

      return hadUnknown ? CxxEscapability::Unknown : CxxEscapability::Escapable;
    }
  }
  if (desugared->isArrayType()) {
    auto elemTy = cast<language::Core::ArrayType>(desugared)
                      ->getElementType()
                      ->getUnqualifiedDesugaredType();
    return evaluateOrDefault(
        evaluator,
        ClangTypeEscapability({elemTy, desc.impl, desc.annotationOnly}),
        CxxEscapability::Unknown);
  }

  // Base cases
  if (desugared->isAnyPointerType() || desugared->isBlockPointerType() ||
      desugared->isMemberPointerType() || desugared->isReferenceType())
    return desc.annotationOnly ? CxxEscapability::Unknown
                               : CxxEscapability::NonEscapable;
  if (desugared->isScalarType())
    return CxxEscapability::Escapable;
  return CxxEscapability::Unknown;
}

void language::simple_display(toolchain::raw_ostream &out,
                           EscapabilityLookupDescriptor desc) {
  out << "Computing escapability for type '";
  out << language::Core::QualType(desc.type, 0).getAsString();
  out << "'";
}

SourceLoc language::extractNearestSourceLoc(EscapabilityLookupDescriptor) {
  return SourceLoc();
}

// Just create a specialized function decl for "__language_interopStaticCast"
// using the types base and derived.
static
DeclRefExpr *getInteropStaticCastDeclRefExpr(ASTContext &ctx,
                                             const language::Core::Module *owningModule,
                                             Type base, Type derived) {
  if (base->isForeignReferenceType() && derived->isForeignReferenceType()) {
    base = base->wrapInPointer(PTK_UnsafePointer);
    derived = derived->wrapInPointer(PTK_UnsafePointer);
  }

  // Lookup our static cast helper function in the C++ shim module.
  auto wrapperModule = ctx.getLoadedModule(ctx.getIdentifier(CXX_SHIM_NAME));
  assert(wrapperModule &&
         "CxxShim module is required when using members of a base class. "
         "Make sure you `import CxxShim`.");

  SmallVector<ValueDecl *, 1> results;
  ctx.lookupInModule(wrapperModule, "__language_interopStaticCast", results);
  assert(
      results.size() == 1 &&
      "Did you forget to define a __language_interopStaticCast helper function?");
  FuncDecl *staticCastFn = cast<FuncDecl>(results.back());

  // Now we have to force instantiate this. We can't let the type checker do
  // this yet because it can't infer the "To" type.
  auto subst =
      SubstitutionMap::get(staticCastFn->getGenericSignature(), {derived, base},
                           LookUpConformanceInModule());
  auto functionTemplate = const_cast<language::Core::FunctionTemplateDecl *>(
      cast<language::Core::FunctionTemplateDecl>(staticCastFn->getClangDecl()));
  auto spec = ctx.getClangModuleLoader()->instantiateCXXFunctionTemplate(
      ctx, functionTemplate, subst);
  auto specializedStaticCastFn =
      cast<FuncDecl>(ctx.getClangModuleLoader()->importDeclDirectly(spec));

  auto staticCastRefExpr = new (ctx)
      DeclRefExpr(ConcreteDeclRef(specializedStaticCastFn), DeclNameLoc(),
                  /*implicit*/ true);
  staticCastRefExpr->setType(specializedStaticCastFn->getInterfaceType());

  return staticCastRefExpr;
}

// Create the following expressions:
// %0 = Builtin.addressof(&self)
// %1 = Builtin.reinterpretCast<UnsafeMutablePointer<Derived>>(%0)
// %2 = __language_interopStaticCast<UnsafeMutablePointer<Base>?>(%1)
// %3 = %2!
// return %3.pointee
static
MemberRefExpr *getSelfInteropStaticCast(FuncDecl *funcDecl,
                                        NominalTypeDecl *baseStruct,
                                        NominalTypeDecl *derivedStruct) {
  auto &ctx = funcDecl->getASTContext();

  auto mutableSelf = [&ctx](FuncDecl *funcDecl) {
    auto selfDecl = funcDecl->getImplicitSelfDecl();

    auto selfRef =
        new (ctx) DeclRefExpr(selfDecl, DeclNameLoc(), /*implicit*/ true);
    selfRef->setType(LValueType::get(selfDecl->getInterfaceType()));

    return selfRef;
  }(funcDecl);

  auto createCallToBuiltin = [&](Identifier name, ArrayRef<Type> substTypes,
                                 Argument arg) {
    auto builtinFn = cast<FuncDecl>(getBuiltinValueDecl(ctx, name));
    auto substMap =
        SubstitutionMap::get(builtinFn->getGenericSignature(), substTypes,
                             LookUpConformanceInModule());
    ConcreteDeclRef builtinFnRef(builtinFn, substMap);
    auto builtinFnRefExpr =
        new (ctx) DeclRefExpr(builtinFnRef, DeclNameLoc(), /*implicit*/ true);

    auto fnType = builtinFn->getInterfaceType();
    if (auto genericFnType = dyn_cast<GenericFunctionType>(fnType.getPointer()))
      fnType = genericFnType->substGenericArgs(substMap);
    builtinFnRefExpr->setType(fnType);
    auto *argList = ArgumentList::createImplicit(ctx, {arg});
    auto callExpr = CallExpr::create(ctx, builtinFnRefExpr, argList, /*implicit*/ true);
    callExpr->setThrows(nullptr);
    return callExpr;
  };

  auto rawSelfPointer = createCallToBuiltin(
      ctx.getIdentifier("addressof"), {derivedStruct->getSelfInterfaceType()},
      Argument::implicitInOut(ctx, mutableSelf));
  rawSelfPointer->setType(ctx.TheRawPointerType);

  auto derivedPtrType = derivedStruct->getSelfInterfaceType()->wrapInPointer(
      PTK_UnsafeMutablePointer);
  auto selfPointer =
      createCallToBuiltin(ctx.getIdentifier("reinterpretCast"),
                          {ctx.TheRawPointerType, derivedPtrType},
                          Argument::unlabeled(rawSelfPointer));
  selfPointer->setType(derivedPtrType);

  auto staticCastRefExpr = getInteropStaticCastDeclRefExpr(
      ctx, baseStruct->getClangDecl()->getOwningModule(),
      baseStruct->getSelfInterfaceType()->wrapInPointer(
          PTK_UnsafeMutablePointer),
      derivedStruct->getSelfInterfaceType()->wrapInPointer(
          PTK_UnsafeMutablePointer));
  auto *argList = ArgumentList::forImplicitUnlabeled(ctx, {selfPointer});
  auto casted = CallExpr::createImplicit(ctx, staticCastRefExpr, argList);
  // This will be "Optional<UnsafeMutablePointer<Base>>"
  casted->setType(cast<FunctionType>(staticCastRefExpr->getType().getPointer())
                      ->getResult());
  casted->setThrows(nullptr);

  SubstitutionMap pointeeSubst = SubstitutionMap::get(
      ctx.getUnsafeMutablePointerDecl()->getGenericSignature(),
      {baseStruct->getSelfInterfaceType()},
      LookUpConformanceInModule());
  VarDecl *pointeePropertyDecl =
      ctx.getPointerPointeePropertyDecl(PTK_UnsafeMutablePointer);
  auto pointeePropertyRefExpr = new (ctx) MemberRefExpr(
      casted, SourceLoc(),
      ConcreteDeclRef(pointeePropertyDecl, pointeeSubst), DeclNameLoc(),
      /*implicit=*/true);
  pointeePropertyRefExpr->setType(
      LValueType::get(baseStruct->getSelfInterfaceType()));

  return pointeePropertyRefExpr;
}

// Find the base C++ method called by the base function we want to synthesize
// the derived thunk for.
// The base C++ method is either the original C++ method that corresponds
// to the imported base member, or it's the synthesized C++ method thunk
// used in another synthesized derived thunk that acts as a base member here.
const language::Core::CXXMethodDecl *getCalledBaseCxxMethod(FuncDecl *baseMember) {
  if (baseMember->getClangDecl())
    return dyn_cast<language::Core::CXXMethodDecl>(baseMember->getClangDecl());
  // Another synthesized derived thunk is used as a base member here,
  // so extract its synthesized C++ method.
  auto body = baseMember->getBody();
  if (body->getElements().empty())
    return nullptr;
  ReturnStmt *returnStmt = dyn_cast_or_null<ReturnStmt>(
      body->getElements().front().dyn_cast<Stmt *>());
  if (!returnStmt)
    return nullptr;
  Expr *returnExpr = returnStmt->getResult();
  // Look through a potential 'reinterpretCast' that can be used
  // to cast UnsafeMutablePointer to UnsafePointer in the synthesized
  // Codira body for `.pointee`.
  if (auto *ce = dyn_cast<CallExpr>(returnExpr)) {
    if (auto *v = ce->getCalledValue()) {
      if (v->getModuleContext() ==
              baseMember->getASTContext().TheBuiltinModule &&
          v->getBaseName().userFacingName() == "reinterpretCast") {
        returnExpr = ce->getArgs()->get(0).getExpr();
      }
    }
  }
  // A member ref expr for `.pointee` access can be wrapping a call
  // when looking through the synthesized Codira body for `.pointee`
  // accessor.
  if (MemberRefExpr *mre = dyn_cast<MemberRefExpr>(returnExpr))
    returnExpr = mre->getBase();
  auto *callExpr = dyn_cast<CallExpr>(returnExpr);
  if (!callExpr)
    return nullptr;
  auto *cv = callExpr->getCalledValue();
  if (!cv)
    return nullptr;
  if (!cv->getClangDecl())
    return nullptr;
  return dyn_cast<language::Core::CXXMethodDecl>(cv->getClangDecl());
}

// Construct a Codira method that represents the synthesized C++ method
// that invokes the base C++ method.
static FuncDecl *synthesizeBaseFunctionDeclCall(ClangImporter &impl,
                                                ASTContext &ctx,
                                                NominalTypeDecl *derivedStruct,
                                                NominalTypeDecl *baseStruct,
                                                FuncDecl *baseMember) {
  auto *cxxMethod = getCalledBaseCxxMethod(baseMember);
  if (!cxxMethod)
    return nullptr;
  auto *newClangMethod =
      CodiraDeclSynthesizer(&impl).synthesizeCXXForwardingMethod(
          cast<language::Core::CXXRecordDecl>(derivedStruct->getClangDecl()),
          cast<language::Core::CXXRecordDecl>(baseStruct->getClangDecl()), cxxMethod,
          ForwardingMethodKind::Base);
  if (!newClangMethod)
    return nullptr;
  return cast_or_null<FuncDecl>(
      ctx.getClangModuleLoader()->importDeclDirectly(newClangMethod));
}

// Generates the body of a derived method, that invokes the base
// method.
// The method's body takes the following form:
//   return self.__synthesizedBaseCall_fn(args...)
static std::pair<BraceStmt *, bool>
synthesizeBaseClassMethodBody(AbstractFunctionDecl *afd, void *context) {

  ASTContext &ctx = afd->getASTContext();

  auto funcDecl = cast<FuncDecl>(afd);
  auto derivedStruct =
      cast<NominalTypeDecl>(funcDecl->getDeclContext()->getAsDecl());
  auto baseMember = static_cast<FuncDecl *>(context);
  auto baseStruct =
      cast<NominalTypeDecl>(baseMember->getDeclContext()->getAsDecl());

  auto forwardedFunc = synthesizeBaseFunctionDeclCall(
      *static_cast<ClangImporter *>(ctx.getClangModuleLoader()), ctx,
      derivedStruct, baseStruct, baseMember);
  if (!forwardedFunc) {
    ctx.Diags.diagnose(SourceLoc(), diag::failed_base_method_call_synthesis,
                         funcDecl, baseStruct);
    auto body = BraceStmt::create(ctx, SourceLoc(), {}, SourceLoc(),
                                  /*implicit=*/true);
    return {body, /*isTypeChecked=*/true};
  }

  SmallVector<Expr *, 8> forwardingParams;
  for (auto param : *funcDecl->getParameters()) {
    auto paramRefExpr = new (ctx) DeclRefExpr(param, DeclNameLoc(),
                                              /*Implicit=*/true);
    paramRefExpr->setType(param->getTypeInContext());
    forwardingParams.push_back(paramRefExpr);
  }

  Argument selfArg = [&]() {
    auto *selfDecl = funcDecl->getImplicitSelfDecl();
    auto selfExpr = new (ctx) DeclRefExpr(selfDecl, DeclNameLoc(),
                                          /*implicit*/ true);
    if (funcDecl->isMutating()) {
      selfExpr->setType(LValueType::get(selfDecl->getInterfaceType()));
      return Argument::implicitInOut(ctx, selfExpr);
    }
    selfExpr->setType(selfDecl->getTypeInContext());
    return Argument::unlabeled(selfExpr);
  }();

  auto *baseMemberExpr =
      new (ctx) DeclRefExpr(ConcreteDeclRef(forwardedFunc), DeclNameLoc(),
                            /*Implicit=*/true);
  baseMemberExpr->setType(forwardedFunc->getInterfaceType());

  auto baseMemberDotCallExpr =
      DotSyntaxCallExpr::create(ctx, baseMemberExpr, SourceLoc(), selfArg);
  baseMemberDotCallExpr->setType(baseMember->getMethodInterfaceType());
  baseMemberDotCallExpr->setThrows(nullptr);

  auto *argList = ArgumentList::forImplicitUnlabeled(ctx, forwardingParams);
  auto *baseMemberCallExpr = CallExpr::createImplicit(
      ctx, baseMemberDotCallExpr, argList);
  baseMemberCallExpr->setType(baseMember->getResultInterfaceType());
  baseMemberCallExpr->setThrows(nullptr);

  auto *returnStmt = ReturnStmt::createImplicit(ctx, baseMemberCallExpr);

  auto body = BraceStmt::create(ctx, SourceLoc(), {returnStmt}, SourceLoc(),
                                /*implicit=*/true);
  return {body, /*isTypeChecked=*/true};
}

// How should the synthesized C++ method that returns the field of interest
// from the base class should return the value - by value, or by reference.
enum ReferenceReturnTypeBehaviorForBaseAccessorSynthesis {
  ReturnByValue,
  ReturnByReference,
  ReturnByMutableReference
};

// Synthesize a C++ method that returns the field of interest from the base
// class. This lets Clang take care of the cast from the derived class
// to the base class while the field is accessed.
static language::Core::CXXMethodDecl *synthesizeCxxBaseGetterAccessorMethod(
    ClangImporter &impl, const language::Core::CXXRecordDecl *derivedClass,
    const language::Core::CXXRecordDecl *baseClass, const language::Core::FieldDecl *field,
    ValueDecl *retainOperationFn,
    ReferenceReturnTypeBehaviorForBaseAccessorSynthesis behavior) {
  auto &clangCtx = impl.getClangASTContext();
  auto &clangSema = impl.getClangSema();

  // Create a new method in the derived class that calls the base method.
  auto name = field->getDeclName();
  if (name.isIdentifier()) {
    std::string newName;
    toolchain::raw_string_ostream os(newName);
    os << (behavior == ReferenceReturnTypeBehaviorForBaseAccessorSynthesis::
                           ReturnByMutableReference
               ? "__synthesizedBaseSetterAccessor_"
               : "__synthesizedBaseGetterAccessor_")
       << name.getAsIdentifierInfo()->getName();
    name = language::Core::DeclarationName(
        &impl.getClangPreprocessor().getIdentifierTable().get(os.str()));
  }
  auto returnType = field->getType();
  if (returnType->isReferenceType())
    returnType = returnType->getPointeeType();
  auto valueReturnType = returnType;
  if (behavior !=
      ReferenceReturnTypeBehaviorForBaseAccessorSynthesis::ReturnByValue) {
    returnType = clangCtx.getRValueReferenceType(
        behavior == ReferenceReturnTypeBehaviorForBaseAccessorSynthesis::
                        ReturnByReference
            ? returnType.withConst()
            : returnType);
  }
  language::Core::FunctionProtoType::ExtProtoInfo info;
  if (behavior != ReferenceReturnTypeBehaviorForBaseAccessorSynthesis::
                      ReturnByMutableReference)
    info.TypeQuals.addConst();
  info.ExceptionSpec.Type = language::Core::EST_NoThrow;
  auto ftype = clangCtx.getFunctionType(returnType, {}, info);
  auto newMethod = language::Core::CXXMethodDecl::Create(
      clangCtx, const_cast<language::Core::CXXRecordDecl *>(derivedClass),
      field->getSourceRange().getBegin(),
      language::Core::DeclarationNameInfo(name, language::Core::SourceLocation()), ftype,
      clangCtx.getTrivialTypeSourceInfo(ftype), language::Core::SC_None,
      /*UsesFPIntrin=*/false, /*isInline=*/true,
      language::Core::ConstexprSpecKind::Unspecified, field->getSourceRange().getEnd());
  newMethod->setImplicit();
  newMethod->setImplicitlyInline();
  newMethod->setAccess(language::Core::AccessSpecifier::AS_public);
  if (retainOperationFn) {
    // Return an FRT field at +1.
    newMethod->addAttr(language::Core::CFReturnsRetainedAttr::CreateImplicit(clangCtx));
  }

  // Create a new Clang diagnostic pool to capture any diagnostics
  // emitted during the construction of the method.
  language::Core::sema::DelayedDiagnosticPool diagPool{
      clangSema.DelayedDiagnostics.getCurrentPool()};
  auto diagState = clangSema.DelayedDiagnostics.push(diagPool);

  // Returns the expression that accesses the base field from derived type.
  auto createFieldAccess = [&]() -> language::Core::Expr * {
    auto *thisExpr = language::Core::CXXThisExpr::Create(
        clangCtx, language::Core::SourceLocation(), newMethod->getThisType(),
        /*IsImplicit=*/false);
    language::Core::QualType baseClassPtr = clangCtx.getRecordType(baseClass);
    baseClassPtr.addConst();
    baseClassPtr = clangCtx.getPointerType(baseClassPtr);

    language::Core::CastKind Kind;
    language::Core::CXXCastPath Path;
    clangSema.CheckPointerConversion(thisExpr, baseClassPtr, Kind, Path,
                                     /*IgnoreBaseAccess=*/false,
                                     /*Diagnose=*/true);
    auto conv = clangSema.ImpCastExprToType(thisExpr, baseClassPtr, Kind,
                                            language::Core::VK_PRValue, &Path);
    if (!conv.isUsable())
      return nullptr;
    auto memberExpr = clangSema.BuildMemberExpr(
        conv.get(), /*isArrow=*/true, language::Core::SourceLocation(),
        language::Core::NestedNameSpecifierLoc(), language::Core::SourceLocation(),
        const_cast<language::Core::FieldDecl *>(field),
        language::Core::DeclAccessPair::make(const_cast<language::Core::FieldDecl *>(field),
                                    language::Core::AS_public),
        /*HadMultipleCandidates=*/false,
        language::Core::DeclarationNameInfo(field->getDeclName(),
                                   language::Core::SourceLocation()),
        valueReturnType, language::Core::VK_LValue, language::Core::OK_Ordinary);
    auto returnCast = clangSema.ImpCastExprToType(memberExpr, valueReturnType,
                                                  language::Core::CK_LValueToRValue,
                                                  language::Core::VK_PRValue);
    if (!returnCast.isUsable())
      return nullptr;
    return returnCast.get();
  };

  toolchain::SmallVector<language::Core::Stmt *, 2> body;
  if (retainOperationFn) {
    // Check if the returned value needs to be retained. This might occur if the
    // field getter is returning a shared reference type using, as it needs to
    // perform the retain to match the expected @owned convention.
    auto *retainClangFn =
        dyn_cast<language::Core::FunctionDecl>(retainOperationFn->getClangDecl());
    if (!retainClangFn) {
      return nullptr;
    }
    auto *fnRef = new (clangCtx) language::Core::DeclRefExpr(
        clangCtx, const_cast<language::Core::FunctionDecl *>(retainClangFn), false,
        retainClangFn->getType(), language::Core::ExprValueKind::VK_LValue,
        language::Core::SourceLocation());
    auto fieldExpr = createFieldAccess();
    if (!fieldExpr)
      return nullptr;
    auto retainCall = clangSema.BuildResolvedCallExpr(
        fnRef, const_cast<language::Core::FunctionDecl *>(retainClangFn),
        language::Core::SourceLocation(), {fieldExpr}, language::Core::SourceLocation());
    if (!retainCall.isUsable())
      return nullptr;
    body.push_back(retainCall.get());
  }

  // Construct the method's body.
  auto fieldExpr = createFieldAccess();
  if (!fieldExpr)
    return nullptr;
  auto returnStmt = language::Core::ReturnStmt::Create(clangCtx, language::Core::SourceLocation(),
                                              fieldExpr, nullptr);
  body.push_back(returnStmt);

  // Check if there were any Clang errors during the construction
  // of the method body.
  clangSema.DelayedDiagnostics.popWithoutEmitting(diagState);
  if (!diagPool.empty())
    return nullptr;
  newMethod->setBody(body.size() > 1
                         ? language::Core::CompoundStmt::Create(
                               clangCtx, body, language::Core::FPOptionsOverride(),
                               language::Core::SourceLocation(), language::Core::SourceLocation())
                         : body[0]);
  return newMethod;
}

// Generates the body of a derived method, that invokes the base
// field getter or the base subscript.
// The method's body takes the following form:
//   return self.__synthesizedBaseCall_fn(args...)
static std::pair<BraceStmt *, bool>
synthesizeBaseClassFieldGetterOrAddressGetterBody(AbstractFunctionDecl *afd,
                                                  void *context,
                                                  AccessorKind kind) {
  assert(kind == AccessorKind::Get || kind == AccessorKind::Address ||
         kind == AccessorKind::MutableAddress);
  ASTContext &ctx = afd->getASTContext();

  AccessorDecl *getterDecl = cast<AccessorDecl>(afd);
  AbstractStorageDecl *baseClassVar = static_cast<AbstractStorageDecl *>(context);
  NominalTypeDecl *baseStruct =
      cast<NominalTypeDecl>(baseClassVar->getDeclContext()->getAsDecl());
  NominalTypeDecl *derivedStruct =
      cast<NominalTypeDecl>(getterDecl->getDeclContext()->getAsDecl());

  const language::Core::Decl *baseClangDecl;
  if (baseClassVar->getClangDecl())
    baseClangDecl = baseClassVar->getClangDecl();
  else
    baseClangDecl = getCalledBaseCxxMethod(baseClassVar->getAccessor(kind));

  language::Core::CXXMethodDecl *baseGetterCxxMethod = nullptr;
  if (auto *md = dyn_cast_or_null<language::Core::CXXMethodDecl>(baseClangDecl)) {
    // Subscript operator, or `.pointee` wrapper is represented through a
    // generated C++ method call that calls the base operator.
    baseGetterCxxMethod =
        CodiraDeclSynthesizer(
            static_cast<ClangImporter *>(ctx.getClangModuleLoader()))
            .synthesizeCXXForwardingMethod(
                cast<language::Core::CXXRecordDecl>(derivedStruct->getClangDecl()),
                cast<language::Core::CXXRecordDecl>(baseStruct->getClangDecl()), md,
                ForwardingMethodKind::Base,
                getterDecl->getResultInterfaceType()->isForeignReferenceType()
                    ? ReferenceReturnTypeBehaviorForBaseMethodSynthesis::
                          RemoveReferenceIfPointer
                    : (kind != AccessorKind::Get
                           ? ReferenceReturnTypeBehaviorForBaseMethodSynthesis::
                                 KeepReference
                           : ReferenceReturnTypeBehaviorForBaseMethodSynthesis::
                                 RemoveReference),
                /*forceConstQualifier=*/kind != AccessorKind::MutableAddress);
  } else if (auto *fd = dyn_cast_or_null<language::Core::FieldDecl>(baseClangDecl)) {
    ValueDecl *retainOperationFn = nullptr;
    // Check if this field getter is returning a retainable FRT.
    if (getterDecl->getResultInterfaceType()->isForeignReferenceType()) {
      auto retainOperation = evaluateOrDefault(
          ctx.evaluator,
          CustomRefCountingOperation({getterDecl->getResultInterfaceType()
                                          ->lookThroughAllOptionalTypes()
                                          ->getClassOrBoundGenericClass(),
                                      CustomRefCountingOperationKind::retain}),
          {});
      if (retainOperation.kind ==
          CustomRefCountingOperationResult::foundOperation) {
        retainOperationFn = retainOperation.operation;
      }
    }
    // Field getter is represented through a generated
    // C++ method call that returns the value of the base field.
    baseGetterCxxMethod = synthesizeCxxBaseGetterAccessorMethod(
        *static_cast<ClangImporter *>(ctx.getClangModuleLoader()),
        cast<language::Core::CXXRecordDecl>(derivedStruct->getClangDecl()),
        cast<language::Core::CXXRecordDecl>(baseStruct->getClangDecl()), fd,
        retainOperationFn,
        kind == AccessorKind::Get
            ? ReferenceReturnTypeBehaviorForBaseAccessorSynthesis::ReturnByValue
            : (kind == AccessorKind::Address
                   ? ReferenceReturnTypeBehaviorForBaseAccessorSynthesis::
                         ReturnByReference
                   : ReferenceReturnTypeBehaviorForBaseAccessorSynthesis::
                         ReturnByMutableReference));
  }

  if (!baseGetterCxxMethod) {
    ctx.Diags.diagnose(SourceLoc(), diag::failed_base_method_call_synthesis,
                       getterDecl, baseStruct);
    auto body = BraceStmt::create(ctx, SourceLoc(), {}, SourceLoc(),
                                  /*implicit=*/true);
    return {body, true};
  }
  auto *baseGetterMethod = cast<FuncDecl>(
      ctx.getClangModuleLoader()->importDeclDirectly(baseGetterCxxMethod));

  Argument selfArg = [&]() {
    auto selfDecl = getterDecl->getImplicitSelfDecl();
    auto selfExpr = new (ctx) DeclRefExpr(selfDecl, DeclNameLoc(),
                                          /*implicit*/ true);
    if (kind == AccessorKind::MutableAddress) {
      selfExpr->setType(LValueType::get(selfDecl->getInterfaceType()));
      return Argument::implicitInOut(ctx, selfExpr);
    }
    selfExpr->setType(selfDecl->getTypeInContext());
    return Argument::unlabeled(selfExpr);
  }();

  auto *baseMemberExpr =
      new (ctx) DeclRefExpr(ConcreteDeclRef(baseGetterMethod), DeclNameLoc(),
                            /*Implicit=*/true);
  baseMemberExpr->setType(baseGetterMethod->getInterfaceType());

  auto baseMemberDotCallExpr =
      DotSyntaxCallExpr::create(ctx, baseMemberExpr, SourceLoc(), selfArg);
  baseMemberDotCallExpr->setType(baseGetterMethod->getMethodInterfaceType());
  baseMemberDotCallExpr->setThrows(nullptr);

  ArgumentList *argumentList;
  if (isa<SubscriptDecl>(baseClassVar)) {
    auto paramDecl = getterDecl->getParameters()->get(0);
    auto paramRefExpr = new (ctx) DeclRefExpr(paramDecl, DeclNameLoc(),
                                              /*Implicit=*/true);
    paramRefExpr->setType(paramDecl->getTypeInContext());
    argumentList = ArgumentList::forImplicitUnlabeled(ctx, {paramRefExpr});
  } else {
    argumentList = ArgumentList::forImplicitUnlabeled(ctx, {});
  }

  auto *baseMemberCallExpr =
      CallExpr::createImplicit(ctx, baseMemberDotCallExpr, argumentList);
  Type resultType = baseGetterMethod->getResultInterfaceType();
  baseMemberCallExpr->setType(resultType);
  baseMemberCallExpr->setThrows(nullptr);

  Expr *returnExpr = baseMemberCallExpr;
  // Cast an 'address' result from a mutable pointer if needed.
  if (kind == AccessorKind::Address &&
      baseGetterMethod->getResultInterfaceType()->isUnsafeMutablePointer()) {
    auto finalResultType = getterDecl->getResultInterfaceType();
    returnExpr = CodiraDeclSynthesizer::synthesizeReturnReinterpretCast(
        ctx, baseGetterMethod->getResultInterfaceType(), finalResultType,
        returnExpr);
  }

  auto *returnStmt = ReturnStmt::createImplicit(ctx, returnExpr);

  auto body = BraceStmt::create(ctx, SourceLoc(), {returnStmt}, SourceLoc(),
                                /*implicit=*/true);
  return {body, /*isTypeChecked=*/true};
}

static std::pair<BraceStmt *, bool>
synthesizeBaseClassFieldGetterBody(AbstractFunctionDecl *afd, void *context) {
  return synthesizeBaseClassFieldGetterOrAddressGetterBody(afd, context,
                                                           AccessorKind::Get);
}

static std::pair<BraceStmt *, bool>
synthesizeBaseClassFieldAddressGetterBody(AbstractFunctionDecl *afd,
                                          void *context) {
  return synthesizeBaseClassFieldGetterOrAddressGetterBody(
      afd, context, AccessorKind::Address);
}

// For setters we have to pass self as a pointer and then emit an assign:
//   %0 = Builtin.addressof(&self)
//   %1 = Builtin.reinterpretCast<UnsafeMutablePointer<Derived>>(%0)
//   %2 = __language_interopStaticCast<UnsafeMutablePointer<Base>?>(%1)
//   %3 = %2!
//   %4 = %3.pointee
//   assign newValue to %4
static std::pair<BraceStmt *, bool>
synthesizeBaseClassFieldSetterBody(AbstractFunctionDecl *afd, void *context) {
  auto setterDecl = cast<AccessorDecl>(afd);
  AbstractStorageDecl *baseClassVar = static_cast<AbstractStorageDecl *>(context);
  ASTContext &ctx = setterDecl->getASTContext();

  NominalTypeDecl *baseStruct =
      cast<NominalTypeDecl>(baseClassVar->getDeclContext()->getAsDecl());
  NominalTypeDecl *derivedStruct =
      cast<NominalTypeDecl>(setterDecl->getDeclContext()->getAsDecl());

  auto *pointeePropertyRefExpr =
      getSelfInteropStaticCast(setterDecl, baseStruct, derivedStruct);

  Expr *storedRef = nullptr;
  if (auto subscript = dyn_cast<SubscriptDecl>(baseClassVar)) {
    auto paramDecl = setterDecl->getParameters()->get(1);
    auto paramRefExpr = new (ctx) DeclRefExpr(paramDecl,
                                              DeclNameLoc(),
                                              /*Implicit=*/ true);
    paramRefExpr->setType(paramDecl->getTypeInContext());

    auto *argList = ArgumentList::forImplicitUnlabeled(ctx, {paramRefExpr});
    storedRef = SubscriptExpr::create(ctx, pointeePropertyRefExpr, argList, subscript);
    storedRef->setType(LValueType::get(subscript->getElementInterfaceType()));
  } else {
    // If the base class var has a clang decl, that means it's an access into a
    // stored field. Otherwise, we're looking into another base class, so it's a
    // another synthesized accessor.
    AccessSemantics accessKind = baseClassVar->getClangDecl()
                                     ? AccessSemantics::DirectToStorage
                                     : AccessSemantics::DirectToImplementation;

    storedRef =
        new (ctx) MemberRefExpr(pointeePropertyRefExpr, SourceLoc(), baseClassVar,
                                DeclNameLoc(), /*Implicit=*/true, accessKind);
    storedRef->setType(LValueType::get(cast<VarDecl>(baseClassVar)->getTypeInContext()));
  }

  auto newValueParamRefExpr =
      new (ctx) DeclRefExpr(setterDecl->getParameters()->get(0), DeclNameLoc(),
                            /*Implicit=*/true);
  newValueParamRefExpr->setType(setterDecl->getParameters()->get(0)->getTypeInContext());

  auto assignExpr =
      new (ctx) AssignExpr(storedRef, SourceLoc(), newValueParamRefExpr,
                           /*implicit*/ true);
  assignExpr->setType(TupleType::getEmpty(ctx));

  auto body = BraceStmt::create(ctx, SourceLoc(), {assignExpr}, SourceLoc(),
                                /*implicit*/ true);
  return {body, /*isTypeChecked=*/true};
}

static std::pair<BraceStmt *, bool>
synthesizeBaseClassFieldAddressSetterBody(AbstractFunctionDecl *afd,
                                          void *context) {
  return synthesizeBaseClassFieldGetterOrAddressGetterBody(
      afd, context, AccessorKind::MutableAddress);
}

static SmallVector<AccessorDecl *, 2>
makeBaseClassMemberAccessors(DeclContext *declContext,
                             AbstractStorageDecl *computedVar,
                             AbstractStorageDecl *baseClassVar) {
  auto &ctx = declContext->getASTContext();
  auto computedType = computedVar->getInterfaceType();
  auto contextTy = declContext->mapTypeIntoContext(computedType);

  // Use 'address' or 'mutableAddress' accessors for non-copyable
  // types, unless the base accessor returns it by value.
  bool useAddress = contextTy->isNoncopyable() &&
                    (baseClassVar->getReadImpl() == ReadImplKind::Stored ||
                     baseClassVar->getAccessor(AccessorKind::Address));

  ParameterList *bodyParams = nullptr;
  if (auto subscript = dyn_cast<SubscriptDecl>(baseClassVar)) {
    computedType = computedType->getAs<FunctionType>()->getResult();

    auto idxParam = subscript->getIndices()->get(0);
    bodyParams = ParameterList::create(ctx, { idxParam });
  } else {
    bodyParams = ParameterList::createEmpty(ctx);
  }

  auto getterDecl = AccessorDecl::create(
      ctx,
      /*FuncLoc=*/SourceLoc(),
      /*AccessorKeywordLoc=*/SourceLoc(),
      useAddress ? AccessorKind::Address : AccessorKind::Get, computedVar,
      /*Async=*/false, /*AsyncLoc=*/SourceLoc(),
      /*Throws=*/false,
      /*ThrowsLoc=*/SourceLoc(), /*ThrownType=*/TypeLoc(), bodyParams,
      useAddress ? computedType->wrapInPointer(PTK_UnsafePointer)
                 : computedType,
      declContext);
  getterDecl->setIsTransparent(true);
  getterDecl->copyFormalAccessFrom(computedVar);
  getterDecl->setBodySynthesizer(useAddress
                                     ? synthesizeBaseClassFieldAddressGetterBody
                                     : synthesizeBaseClassFieldGetterBody,
                                 baseClassVar);
  if (baseClassVar->getWriteImpl() == WriteImplKind::Immutable)
    return {getterDecl};

  auto newValueParam =
      new (ctx) ParamDecl(SourceLoc(), SourceLoc(), Identifier(), SourceLoc(),
                          ctx.getIdentifier("newValue"), declContext);
  newValueParam->setSpecifier(ParamSpecifier::Default);
  newValueParam->setInterfaceType(computedType);

  SmallVector<ParamDecl *, 2> setterParamDecls;
  if (!useAddress)
    setterParamDecls.push_back(newValueParam);
  if (auto subscript = dyn_cast<SubscriptDecl>(baseClassVar))
    setterParamDecls.push_back(subscript->getIndices()->get(0));
  ParameterList *setterBodyParams =
      ParameterList::create(ctx, setterParamDecls);

  auto setterDecl = AccessorDecl::create(
      ctx,
      /*FuncLoc=*/SourceLoc(),
      /*AccessorKeywordLoc=*/SourceLoc(),
      useAddress ? AccessorKind::MutableAddress : AccessorKind::Set,
      computedVar,
      /*Async=*/false, /*AsyncLoc=*/SourceLoc(),
      /*Throws=*/false,
      /*ThrowsLoc=*/SourceLoc(), /*ThrownType=*/TypeLoc(), setterBodyParams,
      useAddress ? computedType->wrapInPointer(PTK_UnsafeMutablePointer)
                 : TupleType::getEmpty(ctx),
      declContext);
  setterDecl->setIsTransparent(true);
  setterDecl->copyFormalAccessFrom(computedVar);
  setterDecl->setBodySynthesizer(useAddress
                                     ? synthesizeBaseClassFieldAddressSetterBody
                                     : synthesizeBaseClassFieldSetterBody,
                                 baseClassVar);
  setterDecl->setSelfAccessKind(SelfAccessKind::Mutating);

  return {getterDecl, setterDecl};
}

// Clone attributes that have been imported from Clang.
void cloneImportedAttributes(ValueDecl *fromDecl, ValueDecl* toDecl) {
  ASTContext& context = fromDecl->getASTContext();
  DeclAttributes& attrs = toDecl->getAttrs();
  for (auto attr : fromDecl->getAttrs()) {
    switch (attr->getKind()) {
    case DeclAttrKind::Available: {
      attrs.add(cast<AvailableAttr>(attr)->clone(context, true));
      break;
    }
    case DeclAttrKind::Custom: {
      CustomAttr *cAttr = cast<CustomAttr>(attr);
      attrs.add(CustomAttr::create(context, SourceLoc(), cAttr->getTypeExpr(),
                                   cAttr->getInitContext(), cAttr->getArgs(),
                                   true));
      break;
    }
    case DeclAttrKind::DiscardableResult: {
      attrs.add(new (context) DiscardableResultAttr(true));
      break;
    }
    case DeclAttrKind::Effects: {
      attrs.add(cast<EffectsAttr>(attr)->clone(context));
      break;
    }
    case DeclAttrKind::Final: {
      attrs.add(new (context) FinalAttr(true));
      break;
    }
    case DeclAttrKind::Transparent: {
      attrs.add(new (context) TransparentAttr(true));
      break;
    }
    case DeclAttrKind::WarnUnqualifiedAccess: {
      attrs.add(new (context) WarnUnqualifiedAccessAttr(true));
      break;
    }
    default:
      break;
    }
  }
}

static ValueDecl *cloneBaseMemberDecl(ValueDecl *decl, DeclContext *newContext,
                                      ClangInheritanceInfo inheritance) {
  AccessLevel access = inheritance.accessForBaseDecl(decl);
  ASTContext &context = decl->getASTContext();

  if (auto fn = dyn_cast<FuncDecl>(decl)) {
    // TODO: function templates are specialized during type checking so to
    // support these we need to tell Codira to type check the synthesized bodies.
    // TODO: we also currently don't support static functions. That shouldn't be
    // too hard.
    if (fn->isStatic() ||
        isa_and_nonnull<language::Core::FunctionTemplateDecl>(fn->getClangDecl()))
      return nullptr;
    if (auto cxxMethod =
            dyn_cast_or_null<language::Core::CXXMethodDecl>(fn->getClangDecl())) {
      // FIXME: if this function has rvalue this, we won't be able to synthesize
      // the accessor correctly (https://github.com/apple/language/issues/69745).
      if (cxxMethod->getRefQualifier() == language::Core::RefQualifierKind::RQ_RValue)
        return nullptr;
    }

    auto out = FuncDecl::createImplicit(
        context, fn->getStaticSpelling(), fn->getName(),
        fn->getNameLoc(), fn->hasAsync(), fn->hasThrows(),
        fn->getThrownInterfaceType(),
        fn->getGenericParams(), fn->getParameters(),
        fn->getResultInterfaceType(), newContext);
    cloneImportedAttributes(decl, out);
    out->setAccess(access);
    inheritance.setUnavailableIfNecessary(decl, out);
    out->setBodySynthesizer(synthesizeBaseClassMethodBody, fn);
    out->setSelfAccessKind(fn->getSelfAccessKind());
    return out;
  }

  if (auto subscript = dyn_cast<SubscriptDecl>(decl)) {
    auto contextTy =
        newContext->mapTypeIntoContext(subscript->getElementInterfaceType());
    // Subscripts that return non-copyable types are not yet supported.
    // See: https://github.com/apple/language/issues/70047.
    if (contextTy->isNoncopyable())
      return nullptr;
    auto out = SubscriptDecl::create(
        subscript->getASTContext(), subscript->getName(), subscript->getStaticLoc(),
        subscript->getStaticSpelling(), subscript->getSubscriptLoc(),
        subscript->getIndices(), subscript->getNameLoc(), subscript->getElementInterfaceType(),
        newContext, subscript->getGenericParams());
    out->setAccess(access);
    inheritance.setUnavailableIfNecessary(decl, out);
    out->setAccessors(SourceLoc(),
                      makeBaseClassMemberAccessors(newContext, out, subscript),
                      SourceLoc());
    out->setImplInfo(subscript->getImplInfo());
    return out;
  }

  if (auto var = dyn_cast<VarDecl>(decl)) {
    auto oldContext = var->getDeclContext();
    auto oldTypeDecl = oldContext->getSelfNominalTypeDecl();
    // FIXME: this is a workaround for rdar://128013193
    if (oldTypeDecl->getAttrs().hasAttribute<MoveOnlyAttr>() &&
        context.LangOpts.CxxInteropUseOpaquePointerForMoveOnly)
      return nullptr;

    auto rawMemory = allocateMemoryForDecl<VarDecl>(var->getASTContext(),
                                                    sizeof(VarDecl), false);
    auto out =
        new (rawMemory) VarDecl(var->isStatic(), var->getIntroducer(),
                                var->getLoc(), var->getName(), newContext);
    out->setInterfaceType(var->getInterfaceType());
    out->setIsObjC(var->isObjC());
    out->setIsDynamic(var->isDynamic());
    out->setAccess(access);
    inheritance.setUnavailableIfNecessary(decl, out);
    out->getASTContext().evaluator.cacheOutput(HasStorageRequest{out}, false);
    auto accessors = makeBaseClassMemberAccessors(newContext, out, var);
    out->setAccessors(SourceLoc(), accessors, SourceLoc());
    auto isMutable = var->getWriteImpl() == WriteImplKind::Immutable
                         ? StorageIsNotMutable : StorageIsMutable;
    out->setImplInfo(
        accessors[0]->getAccessorKind() == AccessorKind::Address
            ? (accessors.size() > 1
                   ? StorageImplInfo(ReadImplKind::Address,
                                     WriteImplKind::MutableAddress,
                                     ReadWriteImplKind::MutableAddress)
                   : StorageImplInfo(ReadImplKind::Address))
            : StorageImplInfo::getComputed(isMutable));
    out->setIsSetterMutating(true);
    return out;
  }

  if (auto typeAlias = dyn_cast<TypeAliasDecl>(decl)) {
    auto rawMemory = allocateMemoryForDecl<TypeAliasDecl>(
        typeAlias->getASTContext(), sizeof(TypeAliasDecl), false);
    auto out = new (rawMemory)
        TypeAliasDecl(typeAlias->getStartLoc(), typeAlias->getEqualLoc(),
                      typeAlias->getName(), typeAlias->getNameLoc(),
                      typeAlias->getGenericParams(), newContext);
    out->setUnderlyingType(typeAlias->getUnderlyingType());
    out->setAccess(access);
    inheritance.setUnavailableIfNecessary(decl, out);
    return out;
  }

  if (auto typeDecl = dyn_cast<TypeDecl>(decl)) {
    auto rawMemory = allocateMemoryForDecl<TypeAliasDecl>(
        typeDecl->getASTContext(), sizeof(TypeAliasDecl), false);
    auto out = new (rawMemory) TypeAliasDecl(
        typeDecl->getLoc(), typeDecl->getLoc(), typeDecl->getName(),
        typeDecl->getLoc(), nullptr, newContext);
    out->setUnderlyingType(typeDecl->getInterfaceType());
    out->setAccess(access);
    inheritance.setUnavailableIfNecessary(decl, out);
    return out;
  }

  return nullptr;
}

TinyPtrVector<ValueDecl *> ClangRecordMemberLookup::evaluate(
    Evaluator &evaluator, ClangRecordMemberLookupDescriptor desc) const {
  NominalTypeDecl *recordDecl = desc.recordDecl;
  NominalTypeDecl *inheritingDecl = desc.inheritingDecl;
  DeclName name = desc.name;
  ClangInheritanceInfo inheritance = desc.inheritance;

  auto &ctx = recordDecl->getASTContext();

  // Whether to skip non-public members. Feature::ImportNonPublicCxxMembers says
  // to import all non-public members by default; if that is disabled, we only
  // import non-public members annotated with LANGUAGE_PRIVATE_FILEID (since those
  // are the only classes that need non-public members.)
  auto *cxxRecordDecl =
      dyn_cast<language::Core::CXXRecordDecl>(inheritingDecl->getClangDecl());
  auto skipIfNonPublic =
      !ctx.LangOpts.hasFeature(Feature::ImportNonPublicCxxMembers) &&
      cxxRecordDecl && importer::getPrivateFileIDAttrs(cxxRecordDecl).empty();

  auto directResults = evaluateOrDefault(
      ctx.evaluator,
      ClangDirectLookupRequest({recordDecl, recordDecl->getClangDecl(), name}),
      {});

  // The set of declarations we found.
  TinyPtrVector<ValueDecl *> result;
  CollectLookupResults collector(name, result);

  // Find the results that are actually a member of "recordDecl".
  ClangModuleLoader *clangModuleLoader = ctx.getClangModuleLoader();
  for (auto foundEntry : directResults) {
    auto found = foundEntry.get<language::Core::NamedDecl *>();
    if (dyn_cast<language::Core::Decl>(found->getDeclContext()) !=
        recordDecl->getClangDecl())
      continue;

    // We should not import 'found' if the following are all true:
    //
    // -  Feature::ImportNonPublicCxxMembers is not enabled
    // -  'found' is not a member of a LANGUAGE_PRIVATE_FILEID-annotated class
    // -  'found' is a non-public member.
    // -  'found' is not a non-inherited FieldDecl; we must import private
    //    fields because they may affect implicit conformances that iterate
    //    through all of a struct's fields, e.g., Sendable (#76892).
    //
    // Note that we can skip inherited FieldDecls because implicit conformances
    // handle those separately.
    //
    // The first two conditions are captured by skipIfNonPublic. The next two
    // are conveyed by the following:
    auto nonPublic = found->getAccess() == language::Core::AS_private ||
                     found->getAccess() == language::Core::AS_protected;
    auto noninheritedField = !inheritance && isa<language::Core::FieldDecl>(found);
    if (skipIfNonPublic && nonPublic && !noninheritedField)
      continue;

    // Don't import constructors on foreign reference types.
    if (isa<language::Core::CXXConstructorDecl>(found) && isa<ClassDecl>(recordDecl))
      continue;

    auto imported = clangModuleLoader->importDeclDirectly(found);
    if (!imported)
      continue;

    // If this member is found due to inheritance, clone it from the base class
    // by synthesizing getters and setters.
    if (inheritance) {
      imported = clangModuleLoader->importBaseMemberDecl(
          cast<ValueDecl>(imported), inheritingDecl, inheritance);
      if (!imported)
        continue;
    }

    collector.add(cast<ValueDecl>(imported));
  }

  if (inheritance) {
    // For inherited members, add members that are synthesized eagerly, such as
    // subscripts. This is not necessary for non-inherited members because those
    // should already be in the lookup table.
    for (auto member :
         cast<NominalTypeDecl>(recordDecl)->getCurrentMembersWithoutLoading()) {
      auto namedMember = dyn_cast<ValueDecl>(member);
      if (!namedMember || !namedMember->hasName() ||
          namedMember->getName().getBaseName() != name ||
          clangModuleLoader->getOriginalForClonedMember(namedMember))
        continue;

      auto *imported = clangModuleLoader->importBaseMemberDecl(
          namedMember, inheritingDecl, inheritance);
      if (!imported)
        continue;

      collector.add(imported);
    }
  }

  // If this is a C++ record, look through any base classes.
  if (auto cxxRecord =
          dyn_cast<language::Core::CXXRecordDecl>(recordDecl->getClangDecl())) {
    // Capture the arity of already found members in the
    // current record, to avoid adding ambiguous members
    // from base classes.
    const auto getArity =
        ClangImporter::Implementation::getImportedBaseMemberDeclArity;
    toolchain::SmallSet<size_t, 4> foundNameArities;
    for (const auto *valueDecl : result)
      foundNameArities.insert(getArity(valueDecl));

    for (auto base : cxxRecord->bases()) {
      if (skipIfNonPublic && base.getAccessSpecifier() != language::Core::AS_public)
        continue;

      language::Core::QualType baseType = base.getType();
      if (auto spectType = dyn_cast<language::Core::TemplateSpecializationType>(baseType))
        baseType = spectType->desugar();
      if (!isa<language::Core::RecordType>(baseType.getCanonicalType()))
        continue;

      auto *baseRecord = baseType->getAs<language::Core::RecordType>()->getDecl();

      if (isSymbolicCircularBase(cxxRecord, baseRecord))
        // Skip circular bases to avoid unbounded recursion
        continue;

      if (auto import = clangModuleLoader->importDeclDirectly(baseRecord)) {
        // If we are looking up the base class, go no further. We will have
        // already found it during the other lookup.
        if (cast<ValueDecl>(import)->getName() == name)
          continue;

        auto baseInheritance = ClangInheritanceInfo(inheritance, base);

        // Add Clang members that are imported lazily.
        auto baseResults = evaluateOrDefault(
            ctx.evaluator,
            ClangRecordMemberLookup({cast<NominalTypeDecl>(import), name,
                                     inheritingDecl, baseInheritance}),
            {});

        for (auto foundInBase : baseResults) {
          // Do not add duplicate entry with the same arity,
          // as that would cause an ambiguous lookup.
          if (foundNameArities.count(getArity(foundInBase)))
            continue;

          collector.add(foundInBase);
        }
      }
    }
  }

  return result;
}

IterableDeclContext *IterableDeclContext::getImplementationContext() {
  if (auto implDecl = getDecl()->getObjCImplementationDecl())
    if (auto implExt = dyn_cast<ExtensionDecl>(implDecl))
      return implExt;

  return this;
}

namespace {
struct OrderDecls {
  bool operator () (Decl *lhs, Decl *rhs) const {
    if (lhs->getDeclContext()->getModuleScopeContext()
          == rhs->getDeclContext()->getModuleScopeContext()) {
      auto &SM = lhs->getASTContext().SourceMgr;
      return SM.isBeforeInBuffer(lhs->getLoc(), rhs->getLoc());
    }

    auto lhsFile =
        dyn_cast<SourceFile>(lhs->getDeclContext()->getModuleScopeContext());
    auto rhsFile =
        dyn_cast<SourceFile>(rhs->getDeclContext()->getModuleScopeContext());

    if (!lhsFile)
      return false;
    if (!rhsFile)
      return true;

    return lhsFile->getFilename() < rhsFile->getFilename();
  }
};
}

static ObjCInterfaceAndImplementation
constructResult(const toolchain::TinyPtrVector<Decl *> &interfaces,
                toolchain::TinyPtrVector<Decl *> &impls,
                Decl *diagnoseOn, Identifier categoryName) {
  if (interfaces.empty() || impls.empty())
    return ObjCInterfaceAndImplementation();

  if (impls.size() > 1) {
    toolchain::sort(impls, OrderDecls());

    auto &diags = interfaces.front()->getASTContext().Diags;
    for (auto extraImpl : toolchain::ArrayRef<Decl *>(impls).drop_front()) {
      auto attr = extraImpl->getAttrs().getAttribute<ObjCImplementationAttr>();
      attr->setInvalid();

      // @objc @implementations for categories are diagnosed as category
      // conflicts, so we're only concerned with main class bodies and
      // non-category implementations here.
      if (categoryName.empty() || !isa<ExtensionDecl>(impls.front())) {
        diags.diagnose(attr->getLocation(), diag::objc_implementation_two_impls,
                       diagnoseOn)
          .fixItRemove(attr->getRangeWithAt());
        diags.diagnose(impls.front(), diag::previous_objc_implementation);
      }
    }
  }

  return ObjCInterfaceAndImplementation(interfaces, impls.front());
}

static bool isImplValid(ExtensionDecl *ext) {
  auto attr = ext->getAttrs().getAttribute<ObjCImplementationAttr>();

  if (!attr)
    return false;

  // Clients using the stable syntax shouldn't have a category name on the attr.
  // This is diagnosed in AttributeChecker::visitObjCImplementationAttr().
  if (!attr->isEarlyAdopter() && !attr->CategoryName.empty())
    return false;

  return !attr->isCategoryNameInvalid();
}

static ObjCInterfaceAndImplementation
findContextInterfaceAndImplementation(DeclContext *dc) {
  if (!dc)
    return {};

  ClassDecl *classDecl = dc->getSelfClassDecl();
  if (!classDecl || !classDecl->hasClangNode())
    // Only extensions of ObjC classes can have @_objcImplementations.
    return {};

  // We know the class we're trying to work with. Next, the category name.
  Identifier categoryName;

  if (auto ext = dyn_cast<ExtensionDecl>(dc)) {
    assert(ext);
    if (!ext->hasClangNode() && !isImplValid(ext))
      return {};

    categoryName = ext->getObjCCategoryName();
  } else {
    // Must be an imported class. Look for its main implementation.
    assert(isa_and_nonnull<ClassDecl>(dc));
    categoryName = Identifier();
  }

  // Now let's look up the interfaces for this...
  auto interfaceDecls = classDecl->getImportedObjCCategory(categoryName);

  // And the implementations.
  toolchain::TinyPtrVector<Decl *> implDecls;
  for (ExtensionDecl *ext : classDecl->getExtensions()) {
    if (ext->isObjCImplementation()
          && ext->getObjCCategoryName() == categoryName
          && isImplValid(ext))
      implDecls.push_back(ext);
  }

  return constructResult(interfaceDecls, implDecls, classDecl, categoryName);
}

static void lookupRelatedFuncs(AbstractFunctionDecl *fn,
                               SmallVectorImpl<ValueDecl *> &results) {
  DeclName languageName;
  if (auto accessor = dyn_cast<AccessorDecl>(fn))
    languageName = accessor->getStorage()->getName();
  else
    languageName = fn->getName();

  NLOptions options = NL_IgnoreAccessControl | NL_IgnoreMissingImports;
  if (auto ty = fn->getDeclContext()->getSelfNominalTypeDecl()) {
    ty->lookupQualified({ ty }, DeclNameRef(languageName), fn->getLoc(),
                        NL_QualifiedDefault | options, results);
  }
  else {
    auto mod = fn->getDeclContext()->getParentModule();
    mod->lookupQualified(mod, DeclNameRef(languageName), fn->getLoc(),
                         NL_RemoveOverridden | options, results);
  }
}

static ObjCInterfaceAndImplementation
findFunctionInterfaceAndImplementation(AbstractFunctionDecl *fn) {
  if (!fn)
    return {};

  // If this isn't either a clang import or an implementation, there's no point
  // doing any work here.
  if (!fn->hasClangNode() && !fn->isObjCImplementation())
    return {};

  OptionalEnum<AccessorKind> accessorKind;
  if (auto accessor = dyn_cast<AccessorDecl>(fn))
    accessorKind = accessor->getAccessorKind();

  StringRef clangName = fn->getCDeclName();
  if (clangName.empty())
    return {};

  SmallVector<ValueDecl *, 4> results;
  lookupRelatedFuncs(fn, results);

  // Classify the `results` as either the interface or an implementation.
  // (Multiple implementations are invalid but utterable.)
  Decl *interface = nullptr;
  TinyPtrVector<Decl *> impls;

  for (ValueDecl *result : results) {
    AbstractFunctionDecl *resultFunc = nullptr;
    if (accessorKind) {
      if (auto resultStorage = dyn_cast<AbstractStorageDecl>(result))
        resultFunc = resultStorage->getAccessor(*accessorKind);
    }
    else
      resultFunc = dyn_cast<AbstractFunctionDecl>(result);

    if (!resultFunc)
      continue;

    if (resultFunc->getCDeclName() != clangName)
      continue;

    if (resultFunc->hasClangNode()) {
      if (interface) {
        // This clang name is overloaded. That should only happen with C++
        // functions/methods, which aren't currently supported.
        return {};
      }
      interface = result;
    } else if (resultFunc->isObjCImplementation()) {
      impls.push_back(result);
    }
  }

  // If we found enough decls to construct a result, `fn` should be among them
  // somewhere.
  assert(interface == nullptr || impls.empty() ||
         interface == fn || toolchain::is_contained(impls, fn));

  return constructResult({ interface }, impls, interface,
                         /*categoryName=*/Identifier());
}

ObjCInterfaceAndImplementation ObjCInterfaceAndImplementationRequest::
evaluate(Evaluator &evaluator, Decl *decl) const {
  ASSERT(ABIRoleInfo(decl).providesAPI()
            && "@interface request for ABI-only decl?");

  // Types and extensions have direct links to their counterparts through the
  // `@_objcImplementation` attribute. Let's resolve that.
  // (Also directing nulls here, where they'll early-return.)
  if (auto ty = dyn_cast_or_null<NominalTypeDecl>(decl))
    return findContextInterfaceAndImplementation(ty);
  else if (auto ext = dyn_cast<ExtensionDecl>(decl))
    return findContextInterfaceAndImplementation(ext);
  // Abstract functions have to be matched through their @_cdecl attributes.
  else if (auto fn = dyn_cast<AbstractFunctionDecl>(decl))
    return findFunctionInterfaceAndImplementation(fn);

  return {};
}

void language::simple_display(toolchain::raw_ostream &out,
                           const ObjCInterfaceAndImplementation &pair) {
  if (pair.empty()) {
    out << "no clang interface or @_objcImplementation";
    return;
  }

  out << "@implementation ";
  simple_display(out, pair.implementationDecl);
  out << " matches clang interfaces ";
  simple_display(out, pair.interfaceDecls);
}

SourceLoc
language::extractNearestSourceLoc(const ObjCInterfaceAndImplementation &pair) {
  if (pair.implementationDecl)
    return SourceLoc();
  return extractNearestSourceLoc(pair.implementationDecl);
}

toolchain::TinyPtrVector<Decl *> Decl::getAllImplementedObjCDecls() const {
  if (hasClangNode())
    // This *is* the interface, if there is one.
    return {};

  // ABI-only attributes don't have an `@implementation`, so query the API
  // counterpart and map the results back to ABI decls.
  auto abiRole = ABIRoleInfo(this);
  if (!abiRole.providesAPI() && abiRole.getCounterpart()) {
    auto interfaceDecls =
        abiRole.getCounterpart()->getAllImplementedObjCDecls();

    // Map the APIs back to their ABI counterparts (often a no-op)
    for (auto &interfaceDecl : interfaceDecls) {
      interfaceDecl = ABIRoleInfo(interfaceDecl).getCounterpart();
    }

    return interfaceDecls;
  }

  ObjCInterfaceAndImplementationRequest req{const_cast<Decl *>(this)};
  auto result = evaluateOrDefault(getASTContext().evaluator, req, {});
  return result.interfaceDecls;
}

DeclContext *DeclContext::getImplementedObjCContext() const {
  if (auto ED = dyn_cast<ExtensionDecl>(this))
    if (auto impl = dyn_cast_or_null<DeclContext>(ED->getImplementedObjCDecl()))
      return impl;
  return const_cast<DeclContext *>(this);
}

Decl *Decl::getObjCImplementationDecl() const {
  if (!hasClangNode())
    // This *is* the implementation, if it has one.
    return nullptr;

  // ABI-only attributes don't have an `@implementation`, so query the API
  // counterpart and map the results back to ABI decls.
  auto abiRole = ABIRoleInfo(this);
  if (!abiRole.providesAPI() && abiRole.getCounterpart()) {
    auto implDecl = abiRole.getCounterpart()->getObjCImplementationDecl();
    return ABIRoleInfo(implDecl).getCounterpart();
  }

  ObjCInterfaceAndImplementationRequest req{const_cast<Decl *>(this)};
  auto result = evaluateOrDefault(getASTContext().evaluator, req, {});
  return result.implementationDecl;
}

toolchain::TinyPtrVector<Decl *>
ClangCategoryLookupRequest::evaluate(Evaluator &evaluator,
                                     ClangCategoryLookupDescriptor desc) const {
  const ClassDecl *CD = desc.classDecl;
  Identifier categoryName = desc.categoryName;

  auto clangClass =
      dyn_cast_or_null<language::Core::ObjCInterfaceDecl>(CD->getClangDecl());
  if (!clangClass)
    return {};

  auto importCategory = [&](const language::Core::ObjCCategoryDecl *clangCat) -> Decl * {
    return CD->getASTContext().getClangModuleLoader()
                  ->importDeclDirectly(clangCat);
  };

  if (categoryName.empty()) {
    // No category name, so we want the decl for the `@interface` in
    // `clangClass`, as well as any class extensions.
    toolchain::TinyPtrVector<Decl *> results;
    results.push_back(const_cast<ClassDecl *>(CD));

    auto importer =
       static_cast<ClangImporter *>(CD->getASTContext().getClangModuleLoader());
    ClangImporter::Implementation &impl = importer->Impl;

    for (auto clangExt : clangClass->known_extensions()) {
      if (impl.getClangSema().isVisible(clangExt))
        results.push_back(importCategory(clangExt));
    }

    return results;
  }

  auto ident = &clangClass->getASTContext().Idents.get(categoryName.str());
  auto clangCategory = clangClass->FindCategoryDeclaration(ident);
  if (!clangCategory)
    return {};

  return { importCategory(clangCategory) };
}

toolchain::TinyPtrVector<Decl *>
ClassDecl::getImportedObjCCategory(Identifier name) const {
  ClangCategoryLookupDescriptor desc{this, name};
  return evaluateOrDefault(getASTContext().evaluator,
                           ClangCategoryLookupRequest(desc),
                           {});
}

void language::simple_display(toolchain::raw_ostream &out,
                           const ClangCategoryLookupDescriptor &desc) {
  out << "Looking up @interface for ";
  if (!desc.categoryName.empty()) {
    out << "category ";
    simple_display(out, desc.categoryName);
  }
  else {
    out << "main body";
  }
  out << " of ";
  simple_display(out, desc.classDecl);
}

SourceLoc
language::extractNearestSourceLoc(const ClangCategoryLookupDescriptor &desc) {
  return extractNearestSourceLoc(desc.classDecl);
}

TinyPtrVector<ValueDecl *>
ClangImporter::Implementation::loadNamedMembers(
    const IterableDeclContext *IDC, DeclBaseName N, uint64_t extra) {
  auto *D = IDC->getDecl();
  auto *DC = D->getInnermostDeclContext();
  auto *CD = D->getClangDecl();
  auto *CDC = cast_or_null<language::Core::DeclContext>(CD);

  auto *nominal = DC->getSelfNominalTypeDecl();
  auto effectiveClangContext = getEffectiveClangContext(nominal);

  // There are 3 cases:
  //
  //  - The decl is from a bridging header, CMO is Some(nullptr)
  //    which denotes the __ObjC Codira module and its associated
  //    BridgingHeaderLookupTable.
  //
  //  - The decl is from a clang module, CMO is Some(M) for non-null
  //    M and we can use the table for that module.
  //
  //  - The decl is a forward declaration, CMO is None, which should
  //    never be the case if we got here (someone is asking for members).
  //
  // findLookupTable, below, handles the first two cases; we assert on the
  // third.

  std::optional<language::Core::Module *> CMO;
  if (CD)
    CMO = getClangSubmoduleForDecl(CD);
  else {
    // IDC is an extension containing globals imported as members, so it doesn't
    // have a clang node but the submodule pointer has been stashed in `extra`.
    CMO = reinterpret_cast<language::Core::Module *>(static_cast<uintptr_t>(extra));
  }
  assert(CMO && "loadNamedMembers on a forward-declared Decl");

  auto table = findLookupTable(*CMO);
  assert(table && "clang module without lookup table");

  assert(!isa_and_nonnull<language::Core::NamespaceDecl>(CD)
            && "Namespace members should be loaded via a request.");
  assert(!CD || isa<language::Core::ObjCContainerDecl>(CD));

  // Force the members of the entire inheritance hierarchy to be loaded and
  // deserialized before loading the named member of a class. This warms up
  // ClangImporter::Implementation::MembersForNominal, used for computing
  // property overrides.
  //
  // FIXME: If getOverriddenDecl() kicked off a request for imported decls,
  // we could postpone this until overrides are actually requested.
  if (auto *classDecl = dyn_cast<ClassDecl>(D))
    if (auto *superclassDecl = classDecl->getSuperclassDecl())
      (void) const_cast<ClassDecl *>(superclassDecl)->lookupDirect(N);

  // TODO: update this to use the requestified lookup.
  TinyPtrVector<ValueDecl *> Members;

  // Lookup actual, factual clang-side members of the context. No need to do
  // this if we're handling an import-as-member extension.
  if (CD) {
    for (auto entry : table->lookup(SerializedCodiraName(N),
                                    effectiveClangContext)) {
      if (!entry.is<language::Core::NamedDecl *>()) continue;
      auto member = entry.get<language::Core::NamedDecl *>();
      if (!isVisibleClangEntry(member)) continue;

      // Skip Decls from different language::Core::DeclContexts
      if (member->getDeclContext() != CDC) continue;

      SmallVector<Decl*, 4> tmp;
      insertMembersAndAlternates(member, tmp, DC);
      for (auto *TD : tmp) {
        if (auto *V = dyn_cast<ValueDecl>(TD)) {
          // Skip ValueDecls if they import under different names.
          if (V->getBaseName() == N) {
            Members.push_back(V);
          }
        }

        // If the property's accessors have alternate decls, we might have
        // to import those too.
        if (auto *ASD = dyn_cast<AbstractStorageDecl>(TD)) {
          for (auto *AD : ASD->getAllAccessors()) {
            for (auto *D : getAlternateDecls(AD)) {
              if (D->getBaseName() == N)
                Members.push_back(D);
            }
          }
        }
      }
    }
  }

  for (auto entry : table->lookupGlobalsAsMembers(SerializedCodiraName(N),
                                                  effectiveClangContext)) {
    if (!entry.is<language::Core::NamedDecl *>()) continue;
    auto member = entry.get<language::Core::NamedDecl *>();
    if (!isVisibleClangEntry(member)) continue;

    // Skip Decls from different language::Core::DeclContexts. We don't do this for
    // import-as-member extensions because we don't know what decl context to
    // expect; for instance, an enum constant is inside the enum decl, not in
    // the translation unit.
    if (CDC && member->getDeclContext() != CDC) continue;

    SmallVector<Decl*, 4> tmp;
    insertMembersAndAlternates(member, tmp, DC);
    for (auto *TD : tmp) {
      if (auto *V = dyn_cast<ValueDecl>(TD)) {
        // Skip ValueDecls if they import under different names.
        if (V->getBaseName() == N) {
          Members.push_back(V);
        }
      }
    }
  }

  if (CD && N.isConstructor()) {
    if (auto *classDecl = dyn_cast<ClassDecl>(D)) {
      SmallVector<Decl *, 4> ctors;
      importInheritedConstructors(cast<language::Core::ObjCInterfaceDecl>(CD),
                                  classDecl, ctors);
      for (auto ctor : ctors)
        Members.push_back(cast<ValueDecl>(ctor));
    }
  }

  if (CD && !isa<ProtocolDecl>(D)) {
    if (auto *OCD = dyn_cast<language::Core::ObjCContainerDecl>(CD)) {
      SmallVector<Decl *, 1> newMembers;
      importMirroredProtocolMembers(OCD, DC, N, newMembers);
      for (auto member : newMembers)
          Members.push_back(cast<ValueDecl>(member));
    }
  }

  return Members;
}

EffectiveClangContext ClangImporter::Implementation::getEffectiveClangContext(
    const NominalTypeDecl *nominal) {
  // If we have a Clang declaration, look at it to determine the
  // effective Clang context.
  if (auto constClangDecl = nominal->getClangDecl()) {
    auto clangDecl = const_cast<language::Core::Decl *>(constClangDecl);
    if (auto dc = dyn_cast<language::Core::DeclContext>(clangDecl))
      return EffectiveClangContext(dc);
    if (auto typedefName = dyn_cast<language::Core::TypedefNameDecl>(clangDecl))
      return EffectiveClangContext(typedefName);

    return EffectiveClangContext();
  }

  // If it's an @objc entity, go look for it.
  // Note that we're stepping lightly here to avoid computing isObjC()
  // too early.
  if (isa<ClassDecl>(nominal) &&
      (nominal->getAttrs().hasAttribute<ObjCAttr>() ||
       (!nominal->getParentSourceFile() && nominal->isObjC()))) {
    // Map the name. If we can't represent the Codira name in Clang.
    Identifier name = nominal->getName();
    if (auto objcAttr = nominal->getAttrs().getAttribute<ObjCAttr>()) {
      if (auto objcName = objcAttr->getName()) {
        if (objcName->getNumArgs() == 0) {
          // This is an error if not 0, but it should be caught later.
          name = objcName->getSimpleName();
        }
      }
    }
    auto clangName = exportName(name);
    if (!clangName)
      return EffectiveClangContext();

    // Perform name lookup into the global scope.
    auto &sema = Instance->getSema();
    language::Core::LookupResult lookupResult(sema, clangName,
                                     language::Core::SourceLocation(),
                                     language::Core::Sema::LookupOrdinaryName);
    if (sema.LookupName(lookupResult, /*Scope=*/nullptr)) {
      // FIXME: Filter based on access path? C++ access control?
      for (auto clangDecl : lookupResult) {
        if (auto objcClass = dyn_cast<language::Core::ObjCInterfaceDecl>(clangDecl))
          return EffectiveClangContext(objcClass);

        /// FIXME: Other type declarations should also be okay?
      }
    }

    // For source compatibility reasons, fall back to the Codira name.
    //
    // This is how people worked around not being able to import-as-member onto
    // Codira types by their ObjC name before the above code to handle ObjCAttr
    // was added.
    if (name != nominal->getName())
      clangName = exportName(nominal->getName());

    lookupResult.clear();
    lookupResult.setLookupName(clangName);
    // FIXME: This loop is duplicated from above, but doesn't obviously factor
    // out in a nice way.
    if (sema.LookupName(lookupResult, /*Scope=*/nullptr)) {
      // FIXME: Filter based on access path? C++ access control?
      for (auto clangDecl : lookupResult) {
        if (auto objcClass = dyn_cast<language::Core::ObjCInterfaceDecl>(clangDecl))
          return EffectiveClangContext(objcClass);

        /// FIXME: Other type declarations should also be okay?
      }
    }
  }

  return EffectiveClangContext();
}

void ClangImporter::dumpCodiraLookupTables() const {
  Impl.dumpCodiraLookupTables();
}

void ClangImporter::Implementation::dumpCodiraLookupTables() {
  // Sort the module names so we can print in a deterministic order.
  SmallVector<StringRef, 4> moduleNames;
  for (const auto &lookupTable : LookupTables) {
    moduleNames.push_back(lookupTable.first);
  }
  array_pod_sort(moduleNames.begin(), moduleNames.end());

  // Print out the lookup tables for the various modules.
  for (auto moduleName : moduleNames) {
    toolchain::errs() << "<<" << moduleName << " lookup table>>\n";
    auto &lookupTable = LookupTables[moduleName];
    lookupTable->deserializeAll();
    lookupTable->dump(toolchain::errs());
  }

  toolchain::errs() << "<<Bridging header lookup table>>\n";
  BridgingHeaderLookupTable->dump(toolchain::errs());
}

DeclName ClangImporter::
importName(const language::Core::NamedDecl *D,
           language::Core::DeclarationName preferredName) {
  return Impl.importFullName(D, Impl.CurrentVersion, preferredName).
    getDeclName();
}

std::optional<Type>
ClangImporter::importFunctionReturnType(const language::Core::FunctionDecl *clangDecl,
                                        DeclContext *dc) {
  bool isInSystemModule =
      cast<ClangModuleUnit>(dc->getModuleScopeContext())->isSystemModule();
  bool allowNSUIntegerAsInt =
      Impl.shouldAllowNSUIntegerAsInt(isInSystemModule, clangDecl);
  if (auto imported =
          Impl.importFunctionReturnType(dc, clangDecl, allowNSUIntegerAsInt)
              .getType())
    return imported;
  return {};
}

Type ClangImporter::importVarDeclType(
    const language::Core::VarDecl *decl, VarDecl *languageDecl, DeclContext *dc) {
  if (decl->getTemplateInstantiationPattern())
    Impl.getClangSema().InstantiateVariableDefinition(
        decl->getLocation(),
        const_cast<language::Core::VarDecl *>(decl));

  // If the declaration is const, consider it audited.
  // We can assume that loading a const global variable doesn't
  // involve an ownership transfer.
  bool isAudited = decl->getType().isConstQualified();

  auto declType = decl->getType();

  // Special case: NS Notifications
  if (isNSNotificationGlobal(decl))
    if (auto newtypeDecl = findCodiraNewtype(decl, Impl.getClangSema(),
                                            Impl.CurrentVersion))
      declType = Impl.getClangASTContext().getTypedefType(newtypeDecl);

  bool isInSystemModule =
      cast<ClangModuleUnit>(dc->getModuleScopeContext())->isSystemModule();

  // Note that we deliberately don't bridge most globals because we want to
  // preserve pointer identity.
  auto importedType =
      Impl.importType(declType,
                      (isAudited ? ImportTypeKind::AuditedVariable
                                 : ImportTypeKind::Variable),
                      ImportDiagnosticAdder(Impl, decl, decl->getLocation()),
                      isInSystemModule, Bridgeability::None,
                      getImportTypeAttrs(decl));

  if (!importedType)
    return ErrorType::get(Impl.CodiraContext);

  if (importedType.isImplicitlyUnwrapped())
    languageDecl->setImplicitlyUnwrappedOptional(true);

  return importedType.getType();
}

bool ClangImporter::isInOverlayModuleForImportedModule(
                                               const DeclContext *overlayDC,
                                               const DeclContext *importedDC) {
  overlayDC = overlayDC->getModuleScopeContext();
  importedDC = importedDC->getModuleScopeContext();

  auto importedClangModuleUnit = dyn_cast<ClangModuleUnit>(importedDC);
  if (!importedClangModuleUnit || !importedClangModuleUnit->getClangModule())
    return false;

  auto overlayModule = overlayDC->getParentModule();
  if (overlayModule == importedClangModuleUnit->getOverlayModule())
    return true;

  // Is this a private module that's re-exported to the public (overlay) name?
  auto clangModule =
  importedClangModuleUnit->getClangModule()->getTopLevelModule();
  return !clangModule->ExportAsModule.empty() &&
    clangModule->ExportAsModule == overlayModule->getName().str();
}

/// Extract the specified-or-defaulted -module-cache-path that winds up in
/// the clang importer, for reuse as the .codemodule cache path when
/// building a ModuleInterfaceLoader.
std::string
language::getModuleCachePathFromClang(const language::Core::CompilerInstance &Clang) {
  if (!Clang.hasPreprocessor())
    return "";
  std::string SpecificModuleCachePath =
      Clang.getPreprocessor().getHeaderSearchInfo().getModuleCachePath().str();

  // The returned-from-clang module cache path includes a suffix directory
  // that is specific to the clang version and invocation; we want the
  // directory above that.
  return toolchain::sys::path::parent_path(SpecificModuleCachePath).str();
}

language::Core::FunctionDecl *ClangImporter::instantiateCXXFunctionTemplate(
    ASTContext &ctx, language::Core::FunctionTemplateDecl *fn, SubstitutionMap subst) {
  SmallVector<language::Core::TemplateArgument, 4> templateSubst;
  std::unique_ptr<TemplateInstantiationError> error =
      ctx.getClangTemplateArguments(fn->getTemplateParameters(),
                                    subst.getReplacementTypes(), templateSubst);

  auto getFuncName = [&]() -> std::string {
    std::string funcName;
    toolchain::raw_string_ostream funcNameStream(funcName);
    fn->printQualifiedName(funcNameStream);
    return funcName;
  };

  if (error) {
    std::string failedTypesStr;
    toolchain::raw_string_ostream failedTypesStrStream(failedTypesStr);
    toolchain::interleaveComma(error->failedTypes, failedTypesStrStream);

    // TODO: Use the location of the apply here.
    // TODO: This error message should not reference implementation details.
    // See: https://github.com/apple/language/pull/33053#discussion_r477003350
    Impl.diagnose(HeaderLoc(fn->getBeginLoc()),
                  diag::unable_to_convert_generic_language_types, getFuncName(),
                  failedTypesStr);
    return nullptr;
  }

  // Instantiate a specialization of this template using the substitution map.
  auto *templateArgList = language::Core::TemplateArgumentList::CreateCopy(
      fn->getASTContext(), templateSubst);
  auto &sema = getClangInstance().getSema();
  auto *spec = sema.InstantiateFunctionDeclaration(fn, templateArgList,
                                                   language::Core::SourceLocation());
  if (!spec) {
    std::string templateParams;
    toolchain::raw_string_ostream templateParamsStream(templateParams);
    toolchain::interleaveComma(templateArgList->asArray(), templateParamsStream,
                          [&](const language::Core::TemplateArgument &arg) {
                            arg.print(fn->getASTContext().getPrintingPolicy(),
                                      templateParamsStream,
                                      /*IncludeType*/ true);
                          });
    Impl.diagnose(HeaderLoc(fn->getBeginLoc()),
                  diag::unable_to_substitute_cxx_function_template,
                  getFuncName(), templateParams);
    return nullptr;
  }
  sema.InstantiateFunctionDefinition(language::Core::SourceLocation(), spec);
  return spec;
}

StructDecl *
ClangImporter::instantiateCXXClassTemplate(
    language::Core::ClassTemplateDecl *decl,
    ArrayRef<language::Core::TemplateArgument> arguments) {
  void *InsertPos = nullptr;
  auto *ctsd = decl->findSpecialization(arguments, InsertPos);
  if (!ctsd) {
    ctsd = language::Core::ClassTemplateSpecializationDecl::Create(
        decl->getASTContext(), decl->getTemplatedDecl()->getTagKind(),
        decl->getDeclContext(), decl->getTemplatedDecl()->getBeginLoc(),
        decl->getLocation(), decl, arguments, nullptr);
    decl->AddSpecialization(ctsd, InsertPos);
  }

  auto CanonType = decl->getASTContext().getTypeDeclType(ctsd);
  assert(isa<language::Core::RecordType>(CanonType) &&
          "type of non-dependent specialization is not a RecordType");

  return dyn_cast_or_null<StructDecl>(
      Impl.importDecl(ctsd, Impl.CurrentVersion));
}

// On Windows and 32-bit platforms we need to force "Int" to actually be
// re-imported as "Int." This is needed because otherwise, we cannot round-trip
// "Int" and "UInt". For example, on Windows, "Int" will be imported into C++ as
// "long long" and then back into Codira as "Int64" not "Int."
static ValueDecl *rewriteIntegerTypes(SubstitutionMap subst, ValueDecl *oldDecl,
                                      AbstractFunctionDecl *newDecl) {
  auto originalFnSubst = cast<AbstractFunctionDecl>(oldDecl)
                             ->getInterfaceType()
                             ->getAs<GenericFunctionType>()
                             ->substGenericArgs(subst);
  // The constructor type is a function type as follows:
  //   (CType.Type) -> (Generic) -> CType
  // And a method's function type is as follows:
  //   (inout CType) -> (Generic) -> Void
  // In either case, we only want the result of that function type because that
  // is the function type with the generic params that need to be substituted:
  //   (Generic) -> CType
  if (isa<ConstructorDecl>(oldDecl) || oldDecl->isInstanceMember() ||
      oldDecl->isStatic())
    originalFnSubst = cast<FunctionType>(originalFnSubst->getResult().getPointer());

  SmallVector<ParamDecl *, 4> fixedParameters;
  unsigned parameterIndex = 0;
  for (auto *newFnParam : *newDecl->getParameters()) {
    // If the user substituted this param with an (U)Int, use (U)Int.
    auto substParamType =
        originalFnSubst->getParams()[parameterIndex].getParameterType();
    if (substParamType->isEqual(newDecl->getASTContext().getIntType()) ||
        substParamType->isEqual(newDecl->getASTContext().getUIntType())) {
      auto intParam =
          ParamDecl::cloneWithoutType(newDecl->getASTContext(), newFnParam);
      intParam->setInterfaceType(substParamType);
      fixedParameters.push_back(intParam);
    } else {
      fixedParameters.push_back(newFnParam);
    }
    parameterIndex++;
  }

  auto fixedParams =
      ParameterList::create(newDecl->getASTContext(), fixedParameters);
  newDecl->setParameters(fixedParams);

  // Now fix the result type:
  if (originalFnSubst->getResult()->isEqual(
          newDecl->getASTContext().getIntType()) ||
      originalFnSubst->getResult()->isEqual(
          newDecl->getASTContext().getUIntType())) {
    // Constructors don't have a result.
    if (auto fn = dyn_cast<FuncDecl>(newDecl)) {
      // We have to rebuild the whole function.
      auto newFnDecl = FuncDecl::createImported(
          fn->getASTContext(), fn->getNameLoc(),
          fn->getName(), fn->getNameLoc(),
          fn->hasAsync(), fn->hasThrows(),
          fn->getThrownInterfaceType(),
          fixedParams, originalFnSubst->getResult(),
          /*genericParams=*/nullptr, fn->getDeclContext(), newDecl->getClangDecl());
      if (fn->isStatic()) newFnDecl->setStatic();
      if (fn->isImportAsStaticMember()) newFnDecl->setImportAsStaticMember();
      if (fn->getImportAsMemberStatus().isInstance()) {
        newFnDecl->setSelfAccessKind(fn->getSelfAccessKind());
        newFnDecl->setSelfIndex(fn->getSelfIndex());
      }

      return newFnDecl;
    }
  }

  return newDecl;
}

static Argument createSelfArg(FuncDecl *fnDecl) {
  ASTContext &ctx = fnDecl->getASTContext();

  auto selfDecl = fnDecl->getImplicitSelfDecl();
  auto selfRefExpr = new (ctx) DeclRefExpr(selfDecl, DeclNameLoc(),
                                           /*implicit*/ true);

  if (!fnDecl->isMutating()) {
    selfRefExpr->setType(selfDecl->getInterfaceType());
    return Argument::unlabeled(selfRefExpr);
  }
  selfRefExpr->setType(LValueType::get(selfDecl->getInterfaceType()));
  return Argument::implicitInOut(ctx, selfRefExpr);
}

// Synthesize a thunk body for the function created in
// "addThunkForDependentTypes". This will just cast all params and forward them
// along to the specialized function. It will also cast the result before
// returning it.
static std::pair<BraceStmt *, bool>
synthesizeDependentTypeThunkParamForwarding(AbstractFunctionDecl *afd, void *context) {
  ASTContext &ctx = afd->getASTContext();

  auto thunkDecl = cast<FuncDecl>(afd);
  auto specializedFuncDecl = static_cast<FuncDecl *>(context);

  SmallVector<Argument, 8> forwardingParams;
  unsigned paramIndex = 0;
  for (auto param : *thunkDecl->getParameters()) {
    if (isa<MetatypeType>(param->getInterfaceType().getPointer())) {
      paramIndex++;
      continue;
    }
    auto paramTy = param->getTypeInContext();
    auto isInOut = param->isInOut();
    auto specParamTy =
        specializedFuncDecl->getParameters()->get(paramIndex)
          ->getTypeInContext();

    Expr *paramRefExpr = new (ctx) DeclRefExpr(param, DeclNameLoc(),
                                               /*Implicit=*/true);
    paramRefExpr->setType(isInOut ? LValueType::get(paramTy) : paramTy);

    Argument arg = [&]() {
      if (isInOut) {
        assert(specParamTy->isEqual(paramTy));
        return Argument::implicitInOut(ctx, paramRefExpr);
      }
      Expr *argExpr = nullptr;
      if (specParamTy->isEqual(paramTy)) {
        argExpr = paramRefExpr;
      } else {
        argExpr = ForcedCheckedCastExpr::createImplicit(ctx, paramRefExpr,
                                                        specParamTy);
      }
      return Argument::unlabeled(argExpr);
    }();
    forwardingParams.push_back(arg);
    paramIndex++;
  }

  Expr *specializedFuncDeclRef = new (ctx) DeclRefExpr(ConcreteDeclRef(specializedFuncDecl),
                                                       DeclNameLoc(), true);
  specializedFuncDeclRef->setType(specializedFuncDecl->getInterfaceType());

  if (specializedFuncDecl->isInstanceMember()) {
    auto selfArg = createSelfArg(thunkDecl);
    auto *memberCall = DotSyntaxCallExpr::create(ctx, specializedFuncDeclRef,
                                                 SourceLoc(), selfArg);
    memberCall->setThrows(nullptr);
    auto resultType = specializedFuncDecl->getInterfaceType()->getAs<FunctionType>()->getResult();
    specializedFuncDeclRef = memberCall;
    specializedFuncDeclRef->setType(resultType);
  } else if (specializedFuncDecl->isStatic()) {
    auto resultType = specializedFuncDecl->getInterfaceType()->getAs<FunctionType>()->getResult();
    auto selfType = cast<NominalTypeDecl>(thunkDecl->getDeclContext()->getAsDecl())->getDeclaredInterfaceType();
    auto selfTypeExpr = TypeExpr::createImplicit(selfType, ctx);
    auto *memberCall =
        DotSyntaxCallExpr::create(ctx, specializedFuncDeclRef, SourceLoc(),
                                  Argument::unlabeled(selfTypeExpr));
    memberCall->setThrows(nullptr);
    specializedFuncDeclRef = memberCall;
    specializedFuncDeclRef->setType(resultType);
  }

  auto argList = ArgumentList::createImplicit(ctx, forwardingParams);
  auto *specializedFuncCallExpr = CallExpr::createImplicit(ctx, specializedFuncDeclRef, argList);
  specializedFuncCallExpr->setType(specializedFuncDecl->getResultInterfaceType());
  specializedFuncCallExpr->setThrows(nullptr);

  Expr *resultExpr = nullptr;
  if (specializedFuncCallExpr->getType()->isEqual(
        thunkDecl->getResultInterfaceType())) {
    resultExpr = specializedFuncCallExpr;
  } else {
    resultExpr = ForcedCheckedCastExpr::createImplicit(
        ctx, specializedFuncCallExpr, thunkDecl->getResultInterfaceType());
  }

  auto *returnStmt = ReturnStmt::createImplicit(ctx, resultExpr);
  auto body = BraceStmt::create(ctx, SourceLoc(), {returnStmt}, SourceLoc(),
                                /*implicit=*/true);
  return {body, /*isTypeChecked=*/true};
}

// Create a thunk to map functions with dependent types to their specialized
// version. For example, create a thunk with type (Any) -> Any to wrap a
// specialized function template with type (Dependent<T>) -> Dependent<T>.
static ValueDecl *addThunkForDependentTypes(FuncDecl *oldDecl,
                                            FuncDecl *newDecl) {
  bool updatedAnyParams = false;

  SmallVector<ParamDecl *, 4> fixedParameters;
  unsigned parameterIndex = 0;
  for (auto *newFnParam : *newDecl->getParameters()) {
    // If the un-specialized function had a parameter with type "Any" preserve
    // that parameter. Otherwise, use the new function parameter.
    auto oldParamType = oldDecl->getParameters()->get(parameterIndex)->getInterfaceType();
    if (oldParamType->isEqual(newDecl->getASTContext().getAnyExistentialType())) {
      updatedAnyParams = true;
      auto newParam =
          ParamDecl::cloneWithoutType(newDecl->getASTContext(), newFnParam);
      newParam->setInterfaceType(oldParamType);
      fixedParameters.push_back(newParam);
    } else {
      fixedParameters.push_back(newFnParam);
    }
    parameterIndex++;
  }

  // If we don't need this thunk, bail out.
  if (!updatedAnyParams &&
      !oldDecl->getResultInterfaceType()->isEqual(
          oldDecl->getASTContext().getAnyExistentialType()))
    return newDecl;

  auto fixedParams =
      ParameterList::create(newDecl->getASTContext(), fixedParameters);

  Type fixedResultType;
  if (oldDecl->getResultInterfaceType()->isEqual(
          oldDecl->getASTContext().getAnyExistentialType()))
    fixedResultType = oldDecl->getASTContext().getAnyExistentialType();
  else
    fixedResultType = newDecl->getResultInterfaceType();

  // We have to rebuild the whole function.
  auto newFnDecl = FuncDecl::createImplicit(
      newDecl->getASTContext(), newDecl->getStaticSpelling(),
      newDecl->getName(), newDecl->getNameLoc(), newDecl->hasAsync(),
      newDecl->hasThrows(), newDecl->getThrownInterfaceType(),
      /*genericParams=*/nullptr, fixedParams,
      fixedResultType, newDecl->getDeclContext());
  newFnDecl->copyFormalAccessFrom(newDecl);
  newFnDecl->setBodySynthesizer(synthesizeDependentTypeThunkParamForwarding, newDecl);
  newFnDecl->setSelfAccessKind(newDecl->getSelfAccessKind());
  if (newDecl->isStatic()) newFnDecl->setStatic();
  newFnDecl->getAttrs().add(
      new (newDecl->getASTContext()) TransparentAttr(/*IsImplicit=*/true));
  return newFnDecl;
}

// Synthesizes the body of a thunk that takes extra metatype arguments and
// skips over them to forward them along to the FuncDecl contained by context.
// This is used when importing a C++ templated function where the template params
// are not used in the function signature. We supply the type params as explicit
// metatype arguments to aid in typechecking, but they shouldn't be forwarded to
// the corresponding C++ function.
static std::pair<BraceStmt *, bool>
synthesizeForwardingThunkBody(AbstractFunctionDecl *afd, void *context) {
  ASTContext &ctx = afd->getASTContext();

  auto thunkDecl = cast<FuncDecl>(afd);
  auto specializedFuncDecl = static_cast<FuncDecl *>(context);

  SmallVector<Argument, 8> forwardingParams;
  for (auto param : *thunkDecl->getParameters()) {
    if (isa<MetatypeType>(param->getInterfaceType().getPointer())) {
      continue;
    }
    auto paramTy = param->getTypeInContext();
    auto isInOut = param->isInOut();

    Expr *paramRefExpr = new (ctx) DeclRefExpr(param, DeclNameLoc(),
                                               /*Implicit=*/true);
    paramRefExpr->setType(isInOut ? LValueType::get(paramTy) : paramTy);

    auto arg = isInOut ? Argument::implicitInOut(ctx, paramRefExpr)
                       : Argument::unlabeled(paramRefExpr);
    forwardingParams.push_back(arg);
  }

  Expr *specializedFuncDeclRef = new (ctx) DeclRefExpr(ConcreteDeclRef(specializedFuncDecl),
                                                       DeclNameLoc(), true);
  specializedFuncDeclRef->setType(specializedFuncDecl->getInterfaceType());

  if (specializedFuncDecl->isInstanceMember()) {
    auto selfArg = createSelfArg(thunkDecl);
    auto *memberCall = DotSyntaxCallExpr::create(ctx, specializedFuncDeclRef,
                                                 SourceLoc(), selfArg);
    memberCall->setThrows(nullptr);
    auto resultType = specializedFuncDecl->getInterfaceType()->getAs<FunctionType>()->getResult();
    specializedFuncDeclRef = memberCall;
    specializedFuncDeclRef->setType(resultType);
  } else if (specializedFuncDecl->isStatic()) {
    auto resultType = specializedFuncDecl->getInterfaceType()->getAs<FunctionType>()->getResult();
    auto selfType = cast<NominalTypeDecl>(thunkDecl->getDeclContext()->getAsDecl())->getDeclaredInterfaceType();
    auto selfTypeExpr = TypeExpr::createImplicit(selfType, ctx);
    auto *memberCall =
        DotSyntaxCallExpr::create(ctx, specializedFuncDeclRef, SourceLoc(),
                                  Argument::unlabeled(selfTypeExpr));
    memberCall->setThrows(nullptr);
    specializedFuncDeclRef = memberCall;
    specializedFuncDeclRef->setType(resultType);
  }

  auto argList = ArgumentList::createImplicit(ctx, forwardingParams);
  auto *specializedFuncCallExpr = CallExpr::createImplicit(ctx, specializedFuncDeclRef, argList);
  specializedFuncCallExpr->setType(thunkDecl->getResultInterfaceType());
  specializedFuncCallExpr->setThrows(nullptr);

  auto *returnStmt = ReturnStmt::createImplicit(ctx, specializedFuncCallExpr);

  auto body = BraceStmt::create(ctx, SourceLoc(), {returnStmt}, SourceLoc(),
                                /*implicit=*/true);
  return {body, /*isTypeChecked=*/true};
}

static ValueDecl *generateThunkForExtraMetatypes(SubstitutionMap subst,
                                                 FuncDecl *oldDecl,
                                                 FuncDecl *newDecl) {
  // We added additional metatype parameters to aid template
  // specialization, which are no longer now that we've specialized
  // this function. Create a thunk that only forwards the original
  // parameters along to the clang function.
  SmallVector<ParamDecl *, 4> newParams;

  for (auto param : *newDecl->getParameters()) {
    auto *newParamDecl = ParamDecl::clone(newDecl->getASTContext(), param);
    newParams.push_back(newParamDecl);
  }

  auto originalFnSubst = cast<AbstractFunctionDecl>(oldDecl)
                             ->getInterfaceType()
                             ->getAs<GenericFunctionType>()
                             ->substGenericArgs(subst);
  // The constructor type is a function type as follows:
  //   (CType.Type) -> (Generic) -> CType
  // And a method's function type is as follows:
  //   (inout CType) -> (Generic) -> Void
  // In either case, we only want the result of that function type because that
  // is the function type with the generic params that need to be substituted:
  //   (Generic) -> CType
  if (isa<ConstructorDecl>(oldDecl) || oldDecl->isInstanceMember() ||
      oldDecl->isStatic())
    originalFnSubst = cast<FunctionType>(originalFnSubst->getResult().getPointer());

  for (auto paramTy : originalFnSubst->getParams()) {
    if (!paramTy.getPlainType()->is<MetatypeType>())
      continue;

    auto dc = newDecl->getDeclContext();
    auto paramVarDecl =
        new (newDecl->getASTContext()) ParamDecl(
            SourceLoc(), SourceLoc(), Identifier(), SourceLoc(),
            newDecl->getASTContext().getIdentifier("_"), dc);
    paramVarDecl->setInterfaceType(paramTy.getPlainType());
    paramVarDecl->setSpecifier(ParamSpecifier::Default);
    newParams.push_back(paramVarDecl);
  }

  auto *newParamList =
      ParameterList::create(newDecl->getASTContext(), SourceLoc(), newParams, SourceLoc());

  auto thunk = FuncDecl::createImplicit(
      newDecl->getASTContext(), newDecl->getStaticSpelling(), oldDecl->getName(),
      newDecl->getNameLoc(), newDecl->hasAsync(), newDecl->hasThrows(),
      newDecl->getThrownInterfaceType(),
      /*genericParams=*/nullptr, newParamList,
      newDecl->getResultInterfaceType(), newDecl->getDeclContext());
  thunk->copyFormalAccessFrom(newDecl);
  thunk->setBodySynthesizer(synthesizeForwardingThunkBody, newDecl);
  thunk->setSelfAccessKind(newDecl->getSelfAccessKind());
  if (newDecl->isStatic()) thunk->setStatic();
  thunk->getAttrs().add(
      new (newDecl->getASTContext()) TransparentAttr(/*IsImplicit=*/true));

  return thunk;
}

ConcreteDeclRef
ClangImporter::getCXXFunctionTemplateSpecialization(SubstitutionMap subst,
                                                    ValueDecl *decl) {
  PrettyStackTraceDeclAndSubst trace("specializing", subst, decl);

  assert(isa<language::Core::FunctionTemplateDecl>(decl->getClangDecl()) &&
         "This API should only be used with function templates.");

  auto *newFn =
      decl->getASTContext()
          .getClangModuleLoader()
          ->instantiateCXXFunctionTemplate(
              decl->getASTContext(),
              const_cast<language::Core::FunctionTemplateDecl *>(
                  cast<language::Core::FunctionTemplateDecl>(decl->getClangDecl())),
              subst);
  // We failed to specialize this function template. The compiler is going to
  // exit soon. Return something valid in the meantime.
  if (!newFn)
    return ConcreteDeclRef(decl);

  auto [fnIt, inserted] =
      Impl.specializedFunctionTemplates.try_emplace(newFn, nullptr);
  if (!inserted)
    return ConcreteDeclRef(fnIt->second);

  auto newDecl = cast_or_null<ValueDecl>(
      decl->getASTContext().getClangModuleLoader()->importDeclDirectly(
          newFn));

  if (auto fn = dyn_cast<AbstractFunctionDecl>(newDecl)) {
    if (!subst.empty()) {
      newDecl = rewriteIntegerTypes(subst, decl, fn);
    }
  }

  if (auto fn = dyn_cast<FuncDecl>(decl)) {
    newDecl = addThunkForDependentTypes(fn, cast<FuncDecl>(newDecl));
  }

  if (auto fn = dyn_cast<FuncDecl>(decl)) {
    if (newFn->getNumParams() != fn->getParameters()->size()) {
      newDecl = generateThunkForExtraMetatypes(subst, fn,
                                               cast<FuncDecl>(newDecl));
    }
  }

  fnIt->getSecond() = newDecl;
  return ConcreteDeclRef(newDecl);
}

FuncDecl *ClangImporter::getCXXSynthesizedOperatorFunc(FuncDecl *decl) {
  // `decl` is not an operator, it is a regular function which has a
  // name that starts with `__operator`. We were asked for a
  // corresponding synthesized Codira operator, so let's retrieve it.

  // The synthesized Codira operator was added as an alternative decl
  // for `fn`.
  auto alternateDecls = Impl.getAlternateDecls(decl);
  // Did we actually synthesize an operator for `fn`?
  if (alternateDecls.empty())
    return nullptr;
  // If we did, then we should have only synthesized one.
  assert(alternateDecls.size() == 1 &&
         "expected only the synthesized operator as an alternative");

  auto synthesizedOperator = alternateDecls.front();
  assert(synthesizedOperator->isOperator() &&
         "expected the alternative to be a synthesized operator");
  return cast<FuncDecl>(synthesizedOperator);
}

bool ClangImporter::isSynthesizedAndVisibleFromAllModules(
    const language::Core::Decl *decl) {
  return Impl.synthesizedAndAlwaysVisibleDecls.contains(decl);
}

bool ClangImporter::isCXXMethodMutating(const language::Core::CXXMethodDecl *method) {
  if (isa<language::Core::CXXConstructorDecl>(method) || !method->isConst())
    return true;
  if (isAnnotatedWith(method, "mutating"))
    return true;
  if (method->getParent()->hasMutableFields()) {
    if (isAnnotatedWith(method, "nonmutating"))
      return false;
    // FIXME(rdar://91961524): figure out a way to handle mutable fields
    // without breaking classes from the C++ standard library (e.g.
    // `std::string` which has a mutable member in old libstdc++ version used on
    // CentOS 7)
    return false;
  }
  return false;
}

bool ClangImporter::isUnsafeCXXMethod(const FuncDecl *fn) {
  if (!fn->hasClangNode())
    return false;
  auto clangDecl = fn->getClangNode().getAsDecl();
  if (!clangDecl)
    return false;
  auto cxxMethod = dyn_cast<language::Core::CXXMethodDecl>(clangDecl);
  if (!cxxMethod)
    return false;
  if (!fn->hasName())
    return false;
  auto id = fn->getBaseName().userFacingName();
  return id.starts_with("__") && id.ends_with("Unsafe");
}

bool ClangImporter::isAnnotatedWith(const language::Core::CXXMethodDecl *method,
                                    StringRef attr) {
  return method->hasAttrs() &&
         toolchain::any_of(method->getAttrs(), [attr](language::Core::Attr *a) {
           if (auto languageAttr = dyn_cast<language::Core::CodiraAttrAttr>(a)) {
             return languageAttr->getAttribute() == attr;
           }
           return false;
         });
}

FuncDecl *
ClangImporter::getDefaultArgGenerator(const language::Core::ParmVarDecl *param) {
  auto it = Impl.defaultArgGenerators.find(param);
  if (it != Impl.defaultArgGenerators.end())
    return it->second;
  return nullptr;
}

FuncDecl *
ClangImporter::getAvailabilityDomainPredicate(const language::Core::VarDecl *var) {
  auto it = Impl.availabilityDomainPredicates.find(var);
  if (it != Impl.availabilityDomainPredicates.end())
    return it->second;
  return nullptr;
}

CodiraLookupTable *
ClangImporter::findLookupTable(const language::Core::Module *clangModule) {
  return Impl.findLookupTable(clangModule);
}

/// Determine the effective Clang context for the given Codira nominal type.
EffectiveClangContext
ClangImporter::getEffectiveClangContext(const NominalTypeDecl *nominal) {
  return Impl.getEffectiveClangContext(nominal);
}

Decl *ClangImporter::importDeclDirectly(const language::Core::NamedDecl *decl) {
  return Impl.importDecl(decl, Impl.CurrentVersion);
}

ValueDecl *ClangImporter::Implementation::importBaseMemberDecl(
    ValueDecl *decl, DeclContext *newContext,
    ClangInheritanceInfo inheritance) {

  // Make sure we don't clone the decl again for this class, as that would
  // result in multiple definitions of the same symbol.
  std::pair<ValueDecl *, DeclContext *> key = {decl, newContext};
  auto known = clonedBaseMembers.find(key);
  if (known == clonedBaseMembers.end()) {
    ValueDecl *cloned = cloneBaseMemberDecl(decl, newContext, inheritance);
    known = clonedBaseMembers.insert({key, cloned}).first;
    clonedMembers.insert(std::make_pair(cloned, decl));
  }

  return known->second;
}

ValueDecl *ClangImporter::Implementation::getOriginalForClonedMember(
    const ValueDecl *decl) {
  // If this is a cloned decl, we don't want to reclone it
  // Otherwise, we may end up with multiple copies of the same method
  if (!decl->hasClangNode()) {
    // Skip decls with a clang node as those will never be a clone
    auto result = clonedMembers.find(decl);
    if (result != clonedMembers.end())
      return result->getSecond();
  }

  return nullptr;
}

size_t ClangImporter::Implementation::getImportedBaseMemberDeclArity(
    const ValueDecl *valueDecl) {
  if (auto *fn = dyn_cast<FuncDecl>(valueDecl)) {
    if (auto *params = fn->getParameters()) {
      return params->size();
    }
  }
  return 0;
}

ValueDecl *
ClangImporter::importBaseMemberDecl(ValueDecl *decl, DeclContext *newContext,
                                    ClangInheritanceInfo inheritance) {
  return Impl.importBaseMemberDecl(decl, newContext, inheritance);
}

ValueDecl *ClangImporter::getOriginalForClonedMember(const ValueDecl *decl) {
  return Impl.getOriginalForClonedMember(decl);
}

void ClangImporter::diagnoseTopLevelValue(const DeclName &name) {
  Impl.diagnoseTopLevelValue(name);
}

void ClangImporter::diagnoseMemberValue(const DeclName &name,
                                        const Type &baseType) {

  // Return early for any type that namelookup::extractDirectlyReferencedNominalTypes
  // does not know how to handle.
  if (!(baseType->getAnyNominal() ||
        baseType->is<ExistentialType>() ||
        baseType->is<UnboundGenericType>() ||
        baseType->is<ArchetypeType>() ||
        baseType->is<ProtocolCompositionType>() ||
        baseType->is<TupleType>()))
    return;

  SmallVector<NominalTypeDecl *, 4> nominalTypesToLookInto;
  namelookup::extractDirectlyReferencedNominalTypes(baseType,
                                                    nominalTypesToLookInto);
  for (auto containerDecl : nominalTypesToLookInto) {
    const language::Core::Decl *clangContainerDecl = containerDecl->getClangDecl();
    if (isa_and_nonnull<language::Core::DeclContext>(clangContainerDecl)) {
      Impl.diagnoseMemberValue(name,
                               cast<language::Core::DeclContext>(clangContainerDecl));
    }

    if (Impl.ImportForwardDeclarations) {
      const language::Core::Decl *clangContainerDecl = containerDecl->getClangDecl();
      if (const language::Core::ObjCInterfaceDecl *objCInterfaceDecl =
              toolchain::dyn_cast_or_null<language::Core::ObjCInterfaceDecl>(
                  clangContainerDecl); objCInterfaceDecl && !objCInterfaceDecl->hasDefinition()) {
        // Emit a diagnostic about how the base type represents a forward
        // declared ObjC interface and is in all likelihood missing members.
        // We only attach this diagnostic in diagnoseMemberValue rather than
        // in CodiraDeclConverter because it is only relevant when the user
        // tries to access an unavailable member.
        Impl.addImportDiagnostic(
            objCInterfaceDecl,
            Diagnostic(
                diag::
                    placeholder_for_forward_declared_interface_member_access_failure,
                objCInterfaceDecl->getName()),
            objCInterfaceDecl->getSourceRange().getBegin());
        // Emit any diagnostics attached to the source Clang node (ie. forward
        // declaration here note)
        Impl.diagnoseTargetDirectly(clangContainerDecl);
      } else if (const language::Core::ObjCProtocolDecl *objCProtocolDecl =
                     toolchain::dyn_cast_or_null<language::Core::ObjCProtocolDecl>(
                         clangContainerDecl); objCProtocolDecl && !objCProtocolDecl->hasDefinition()) {
        // Same as above but for protocols
        Impl.addImportDiagnostic(
            objCProtocolDecl,
            Diagnostic(
                diag::
                    placeholder_for_forward_declared_protocol_member_access_failure,
                objCProtocolDecl->getName()),
            objCProtocolDecl->getSourceRange().getBegin());
        Impl.diagnoseTargetDirectly(clangContainerDecl);
      }
    }
  }
}

SourceLoc ClangImporter::importSourceLocation(language::Core::SourceLocation loc) {
  auto &bufferImporter = Impl.getBufferImporterForDiagnostics();
  return bufferImporter.resolveSourceLocation(
      getClangASTContext().getSourceManager(), loc);
}

toolchain::Expected<toolchain::cas::ObjectRef>
ClangImporter::createEmbeddedBridgingHeaderCacheKey(
    toolchain::cas::ObjectStore &CAS, toolchain::cas::ObjectRef ChainedPCHIncludeTree) {
  // Create a cache key for looking up embedded bridging header include tree
  // from chained bridging header cache key.
  return CAS.store({ChainedPCHIncludeTree},
                   "ChainedHeaderIncludeTree -> EmbeddedHeaderIncludeTree");
}

static bool hasImportAsRefAttr(const language::Core::RecordDecl *decl) {
  return decl->hasAttrs() && toolchain::any_of(decl->getAttrs(), [](auto *attr) {
           if (auto languageAttr = dyn_cast<language::Core::CodiraAttrAttr>(attr))
             return languageAttr->getAttribute() == "import_reference" ||
                    // TODO: Remove this once libCodira hosttools no longer
                    // requires it.
                    languageAttr->getAttribute() == "import_as_ref";
           return false;
         });
}

static bool hasDiamondInheritanceRefType(const language::Core::CXXRecordDecl *decl) {
  if (!decl->hasDefinition() || decl->isDependentType())
    return false;

  toolchain::DenseSet<const language::Core::CXXRecordDecl *> seenBases;
  bool hasRefDiamond = false;

  decl->forallBases([&](const language::Core::CXXRecordDecl *Base) {
    if (hasImportAsRefAttr(Base) && !seenBases.insert(Base).second &&
        !decl->isVirtuallyDerivedFrom(Base))
      hasRefDiamond = true;
    return true;
  });

  return hasRefDiamond;
}

// Returns the given declaration along with all its parent declarations that are
// reference types.
static toolchain::SmallVector<const language::Core::RecordDecl *, 4>
getRefParentDecls(const language::Core::RecordDecl *decl, ASTContext &ctx,
                  ClangImporter::Implementation *importerImpl) {
  assert(decl && "decl is null inside getRefParentDecls");

  toolchain::SmallVector<const language::Core::RecordDecl *, 4> matchingDecls;

  if (hasImportAsRefAttr(decl))
    matchingDecls.push_back(decl);

  if (const auto *cxxRecordDecl = toolchain::dyn_cast<language::Core::CXXRecordDecl>(decl)) {
    if (!cxxRecordDecl->hasDefinition())
      return matchingDecls;
    if (hasDiamondInheritanceRefType(cxxRecordDecl)) {
      if (importerImpl) {
        if (!importerImpl->DiagnosedCxxRefDecls.count(decl)) {
          HeaderLoc loc(decl->getLocation());
          importerImpl->diagnose(loc, diag::cant_infer_frt_in_cxx_inheritance,
                                 decl);
          importerImpl->DiagnosedCxxRefDecls.insert(decl);
        }
      } else {
        ctx.Diags.diagnose({}, diag::cant_infer_frt_in_cxx_inheritance, decl);
        assert(false && "nullpointer passeed for importerImpl when calling "
                        "getRefParentOrDiag");
      }
      return matchingDecls;
    }
    cxxRecordDecl->forallBases([&](const language::Core::CXXRecordDecl *baseDecl) {
      if (hasImportAsRefAttr(baseDecl))
        matchingDecls.push_back(baseDecl);
      return true;
    });
  }

  return matchingDecls;
}

static toolchain::SmallVector<ValueDecl *, 1>
getValueDeclsForName(const language::Core::Decl *decl, ASTContext &ctx, StringRef name) {
  toolchain::SmallVector<ValueDecl *, 1> results;
  auto *clangMod = decl->getOwningModule();
  if (clangMod && clangMod->isSubModule())
    clangMod = clangMod->getTopLevelModule();
  if (clangMod) {
    auto parentModule =
        ctx.getClangModuleLoader()->getWrapperForModule(clangMod);
    ctx.lookupInModule(parentModule, name, results);
  } else {
    // There is no Clang module for this declaration, so perform lookup from
    // the main module. This will find declarations from the bridging header.
    namelookup::lookupInModule(
        ctx.MainModule, ctx.getIdentifier(name), results,
        NLKind::UnqualifiedLookup, namelookup::ResolutionKind::Overloadable,
        ctx.MainModule, SourceLoc(), NL_UnqualifiedDefault);

    // Filter out any declarations that didn't come from Clang.
    auto newEnd =
        std::remove_if(results.begin(), results.end(),
                       [&](ValueDecl *decl) { return !decl->getClangDecl(); });
    results.erase(newEnd, results.end());
  }
  return results;
}

static const language::Core::RecordDecl *
getRefParentOrDiag(const language::Core::RecordDecl *decl, ASTContext &ctx,
                   ClangImporter::Implementation *importerImpl) {
  auto refParentDecls = getRefParentDecls(decl, ctx, importerImpl);
  if (refParentDecls.empty())
    return nullptr;

  std::set<StringRef> uniqueRetainDecls{}, uniqueReleaseDecls{};
  constexpr StringRef retainPrefix = "retain:";
  constexpr StringRef releasePrefix = "release:";

  for (const auto *refParentDecl : refParentDecls) {
    assert(refParentDecl && "refParentDecl is null inside getRefParentOrDiag");
    for (const auto *attr : refParentDecl->getAttrs()) {
      if (const auto languageAttr = toolchain::dyn_cast<language::Core::CodiraAttrAttr>(attr)) {
        const auto &attribute = languageAttr->getAttribute();
        if (attribute.starts_with(retainPrefix))
          uniqueRetainDecls.insert(attribute.drop_front(retainPrefix.size()));
        else if (attribute.starts_with(releasePrefix))
          uniqueReleaseDecls.insert(attribute.drop_front(releasePrefix.size()));
      }
    }
  }

  // Ensure that exactly one unique retain function and one unique release
  // function are found.
  if (uniqueRetainDecls.size() != 1 || uniqueReleaseDecls.size() != 1) {
    if (importerImpl) {
      if (!importerImpl->DiagnosedCxxRefDecls.count(decl)) {
        HeaderLoc loc(decl->getLocation());
        importerImpl->diagnose(loc, diag::cant_infer_frt_in_cxx_inheritance,
                               decl);
        importerImpl->DiagnosedCxxRefDecls.insert(decl);
      }
    } else {
      ctx.Diags.diagnose({}, diag::cant_infer_frt_in_cxx_inheritance, decl);
      assert(false && "nullpointer passed for importerImpl when calling "
                      "getRefParentOrDiag");
    }
    return nullptr;
  }

  return refParentDecls.front();
}

// Is this a pointer to a foreign reference type.
// TODO: We need to review functions like this to ensure that
// CxxRecordSemantics::evaluate is consistently invoked wherever we need to
// determine whether a C++ type qualifies as a foreign reference type
// rdar://145184659
static bool isForeignReferenceType(const language::Core::QualType type) {
  if (!type->isPointerType())
    return false;

  auto pointeeType =
      dyn_cast<language::Core::RecordType>(type->getPointeeType().getCanonicalType());
  if (pointeeType == nullptr)
    return false;

  return hasImportAsRefAttr(pointeeType->getDecl());
}

static bool hasCodiraAttribute(const language::Core::Decl *decl, StringRef attr) {
  if (decl->hasAttrs() && toolchain::any_of(decl->getAttrs(), [&](auto *A) {
        if (auto languageAttr = dyn_cast<language::Core::CodiraAttrAttr>(A))
          return languageAttr->getAttribute() == attr;
        return false;
      }))
    return true;

  if (auto *P = dyn_cast<language::Core::ParmVarDecl>(decl)) {
    bool found = false;
    findCodiraAttributes(P->getOriginalType(),
                        [&](const language::Core::CodiraAttrAttr *languageAttr) {
                          found |= languageAttr->getAttribute() == attr;
                        });
    return found;
  }

  return false;
}

bool importer::hasOwnedValueAttr(const language::Core::RecordDecl *decl) {
  return hasCodiraAttribute(decl, "import_owned");
}

bool importer::hasUnsafeAPIAttr(const language::Core::Decl *decl) {
  return hasCodiraAttribute(decl, "import_unsafe");
}

bool importer::hasIteratorAPIAttr(const language::Core::Decl *decl) {
  return hasCodiraAttribute(decl, "import_iterator");
}

static bool hasNonCopyableAttr(const language::Core::RecordDecl *decl) {
  return hasCodiraAttribute(decl, "~Copyable");
}

bool importer::hasNonEscapableAttr(const language::Core::RecordDecl *decl) {
  return hasCodiraAttribute(decl, "~Escapable");
}

bool importer::hasEscapableAttr(const language::Core::RecordDecl *decl) {
  return hasCodiraAttribute(decl, "Escapable");
}

/// Recursively checks that there are no pointers in any fields or base classes.
/// Does not check C++ records with specific API annotations.
static bool hasPointerInSubobjects(const language::Core::CXXRecordDecl *decl) {
  language::Core::PrettyStackTraceDecl trace(decl, language::Core::SourceLocation(),
                                    decl->getASTContext().getSourceManager(),
                                    "looking for pointers in subobjects of");

  // Probably a class template that has not yet been specialized:
  if (!decl->getDefinition())
    return false;

  auto checkType = [](language::Core::QualType t) {
    if (t->isPointerType())
      return true;

    if (auto recordType = dyn_cast<language::Core::RecordType>(t.getCanonicalType())) {
      if (auto cxxRecord =
              dyn_cast<language::Core::CXXRecordDecl>(recordType->getDecl())) {
        if (hasImportAsRefAttr(cxxRecord) || hasOwnedValueAttr(cxxRecord) ||
            hasUnsafeAPIAttr(cxxRecord))
          return false;

        if (hasIteratorAPIAttr(cxxRecord) || isIterator(cxxRecord))
          return true;

        if (hasPointerInSubobjects(cxxRecord))
          return true;
      }
    }

    return false;
  };

  for (auto field : decl->fields()) {
    if (checkType(field->getType()))
      return true;
  }

  for (auto base : decl->bases()) {
    if (checkType(base.getType()))
      return true;
  }

  return false;
}

bool importer::isViewType(const language::Core::CXXRecordDecl *decl) {
  return !hasOwnedValueAttr(decl) && hasPointerInSubobjects(decl);
}

static bool copyConstructorIsDefaulted(const language::Core::CXXRecordDecl *decl) {
  auto ctor = toolchain::find_if(decl->ctors(), [](language::Core::CXXConstructorDecl *ctor) {
    return ctor->isCopyConstructor();
  });

  assert(ctor != decl->ctor_end());
  return ctor->isDefaulted();
}

static bool copyAssignOperatorIsDefaulted(const language::Core::CXXRecordDecl *decl) {
  auto copyAssignOp = toolchain::find_if(decl->decls(), [](language::Core::Decl *member) {
    if (auto method = dyn_cast<language::Core::CXXMethodDecl>(member))
      return method->isCopyAssignmentOperator();
    return false;
  });

  assert(copyAssignOp != decl->decls_end());
  return cast<language::Core::CXXMethodDecl>(*copyAssignOp)->isDefaulted();
}

/// Recursively checks that there are no user-provided copy constructors or
/// destructors in any fields or base classes.
/// Does not check C++ records with specific API annotations.
static bool isSufficientlyTrivial(const language::Core::CXXRecordDecl *decl) {
  // Probably a class template that has not yet been specialized:
  if (!decl->getDefinition())
    return true;

  if ((decl->hasUserDeclaredCopyConstructor() &&
       !copyConstructorIsDefaulted(decl)) ||
      (decl->hasUserDeclaredCopyAssignment() &&
       !copyAssignOperatorIsDefaulted(decl)) ||
      (decl->hasUserDeclaredDestructor() && decl->getDestructor() &&
       !decl->getDestructor()->isDefaulted()))
    return false;

  auto checkType = [](language::Core::QualType t) {
    if (auto recordType = dyn_cast<language::Core::RecordType>(t.getCanonicalType())) {
      if (auto cxxRecord =
              dyn_cast<language::Core::CXXRecordDecl>(recordType->getDecl())) {
        if (hasImportAsRefAttr(cxxRecord) || hasOwnedValueAttr(cxxRecord) ||
            hasUnsafeAPIAttr(cxxRecord))
          return true;

        if (!isSufficientlyTrivial(cxxRecord))
          return false;
      }
    }

    return true;
  };

  for (auto field : decl->fields()) {
    if (!checkType(field->getType()))
      return false;
  }

  for (auto base : decl->bases()) {
    if (!checkType(base.getType()))
      return false;
  }

  return true;
}

/// Checks if a record provides the required value type lifetime operations
/// (copy and destroy).
static bool hasCopyTypeOperations(const language::Core::CXXRecordDecl *decl) {
  // Hack for a base type of std::optional from the Microsoft standard library.
  if (decl->isInStdNamespace() && decl->getIdentifier() &&
      decl->getName() == "_Optional_construct_base")
    return true;

  if (decl->hasSimpleCopyConstructor())
    return true;

  // If we have no way of copying the type we can't import the class
  // at all because we cannot express the correct semantics as a language
  // struct.
  return toolchain::any_of(decl->ctors(), [](language::Core::CXXConstructorDecl *ctor) {
    return ctor->isCopyConstructor() && !ctor->isDeleted() &&
           // FIXME: Support default arguments (rdar://142414553)
           ctor->getNumParams() == 1 &&
           ctor->getAccess() == language::Core::AccessSpecifier::AS_public;
  });
}

static bool hasMoveTypeOperations(const language::Core::CXXRecordDecl *decl) {
  // If we have no way of copying the type we can't import the class
  // at all because we cannot express the correct semantics as a language
  // struct.
  if (toolchain::any_of(decl->ctors(), [](language::Core::CXXConstructorDecl *ctor) {
        return ctor->isMoveConstructor() &&
               (ctor->isDeleted() || ctor->getAccess() != language::Core::AS_public);
      }))
    return false;

  return toolchain::any_of(decl->ctors(), [](language::Core::CXXConstructorDecl *ctor) {
    return ctor->isMoveConstructor() &&
           // FIXME: Support default arguments (rdar://142414553)
           ctor->getNumParams() == 1;
  });
}

static bool hasDestroyTypeOperations(const language::Core::CXXRecordDecl *decl) {
  if (auto dtor = decl->getDestructor()) {
    if (dtor->isDeleted() || dtor->getAccess() != language::Core::AS_public) {
      return false;
    }
    return true;
  }
  return false;
}

static bool hasCustomCopyOrMoveConstructor(const language::Core::CXXRecordDecl *decl) {
  return decl->hasUserDeclaredCopyConstructor() ||
         decl->hasUserDeclaredMoveConstructor();
}

static bool
hasConstructorWithUnsupportedDefaultArgs(const language::Core::CXXRecordDecl *decl) {
  return toolchain::any_of(decl->ctors(), [](language::Core::CXXConstructorDecl *ctor) {
    return (ctor->isCopyConstructor() || ctor->isMoveConstructor()) &&
           // FIXME: Support default arguments (rdar://142414553)
           ctor->getNumParams() != 1;
  });
}

static bool isCodiraClassType(const language::Core::CXXRecordDecl *decl) {
  // Codira type must be annotated with external_source_symbol attribute.
  auto essAttr = decl->getAttr<language::Core::ExternalSourceSymbolAttr>();
  if (!essAttr || essAttr->getLanguage() != "Codira" ||
      essAttr->getDefinedIn().empty() || essAttr->getUSR().empty())
    return false;

  // Ensure that the baseclass is language::RefCountedClass.
  auto baseDecl = decl;
  do {
    if (baseDecl->getNumBases() != 1)
      return false;
    auto baseClassSpecifier = *baseDecl->bases_begin();
    auto Ty = baseClassSpecifier.getType();
    auto nextBaseDecl = Ty->getAsCXXRecordDecl();
    if (!nextBaseDecl)
      return false;
    baseDecl = nextBaseDecl;
  } while (baseDecl->getName() != "RefCountedClass");

  return true;
}

CxxRecordSemanticsKind
CxxRecordSemantics::evaluate(Evaluator &evaluator,
                             CxxRecordSemanticsDescriptor desc) const {
  const auto *decl = desc.decl;
  ClangImporter::Implementation *importerImpl = desc.importerImpl;
  if (hasImportAsRefAttr(decl) ||
      getRefParentOrDiag(decl, desc.ctx, importerImpl))
    return CxxRecordSemanticsKind::Reference;

  auto cxxDecl = dyn_cast<language::Core::CXXRecordDecl>(decl);
  if (!cxxDecl) {
    return CxxRecordSemanticsKind::Trivial;
  }

  if (isCodiraClassType(cxxDecl))
    return CxxRecordSemanticsKind::CodiraClassType;

  if (!hasDestroyTypeOperations(cxxDecl) ||
      (!hasCopyTypeOperations(cxxDecl) && !hasMoveTypeOperations(cxxDecl))) {

    if (hasConstructorWithUnsupportedDefaultArgs(cxxDecl))
      return CxxRecordSemanticsKind::UnavailableConstructors;

    return CxxRecordSemanticsKind::MissingLifetimeOperation;
  }

  if (hasNonCopyableAttr(cxxDecl) && hasMoveTypeOperations(cxxDecl)) {
    return CxxRecordSemanticsKind::MoveOnly;
  }

  if (hasOwnedValueAttr(cxxDecl)) {
    return CxxRecordSemanticsKind::Owned;
  }

  if (hasIteratorAPIAttr(cxxDecl) || isIterator(cxxDecl)) {
    return CxxRecordSemanticsKind::Iterator;
  }

  if (hasCopyTypeOperations(cxxDecl)) {
    return CxxRecordSemanticsKind::Owned;
  }

  if (hasMoveTypeOperations(cxxDecl)) {
    return CxxRecordSemanticsKind::MoveOnly;
  }

  if (isSufficientlyTrivial(cxxDecl)) {
    return CxxRecordSemanticsKind::Trivial;
  }

  toolchain_unreachable("Could not classify C++ type.");
}

ValueDecl *
CxxRecordAsCodiraType::evaluate(Evaluator &evaluator,
                               CxxRecordSemanticsDescriptor desc) const {
  auto cxxDecl = dyn_cast<language::Core::CXXRecordDecl>(desc.decl);
  if (!cxxDecl)
    return nullptr;
  if (!isCodiraClassType(cxxDecl))
    return nullptr;

  SmallVector<ValueDecl *, 1> results;
  auto *essaAttr = cxxDecl->getAttr<language::Core::ExternalSourceSymbolAttr>();
  auto *mod = desc.ctx.getModuleByName(essaAttr->getDefinedIn());
  if (!mod) {
    // TODO: warn about missing 'import'.
    return nullptr;
  }
  // FIXME: Support renamed declarations.
  auto languageName = cxxDecl->getName();
  // FIXME: handle nested Codira types once they're supported.
  mod->lookupValue(desc.ctx.getIdentifier(languageName), NLKind::UnqualifiedLookup,
                   results);
  if (results.size() == 1) {
    if (isa<ClassDecl>(results[0]))
      return results[0];
  }
  return nullptr;
}

static bool anySubobjectsSelfContained(const language::Core::CXXRecordDecl *decl) {
  // std::pair and std::tuple might have copy and move constructors, or base
  // classes with copy and move constructors, but they are not self-contained
  // types, e.g. `std::pair<UnsafeType, T>`.
  if (decl->isInStdNamespace() &&
      (decl->getName() == "pair" || decl->getName() == "tuple"))
    return false;

  if (!decl->getDefinition())
    return false;

  if (hasCustomCopyOrMoveConstructor(decl) || hasOwnedValueAttr(decl))
    return true;

  auto checkType = [](language::Core::QualType t) {
    if (auto recordType = dyn_cast<language::Core::RecordType>(t.getCanonicalType())) {
      if (auto cxxRecord =
              dyn_cast<language::Core::CXXRecordDecl>(recordType->getDecl())) {
        return anySubobjectsSelfContained(cxxRecord);
      }
    }

    return false;
  };

  for (auto field : decl->fields()) {
    if (checkType(field->getType()))
      return true;
  }

  for (auto base : decl->bases()) {
    if (checkType(base.getType()))
      return true;
  }

  return false;
}

bool IsSafeUseOfCxxDecl::evaluate(Evaluator &evaluator,
                                  SafeUseOfCxxDeclDescriptor desc) const {
  const language::Core::Decl *decl = desc.decl;

  if (auto method = dyn_cast<language::Core::CXXMethodDecl>(decl)) {
    // The user explicitly asked us to import this method.
    if (hasUnsafeAPIAttr(method))
      return true;

    // If it's a static method, it cannot project anything. It's fine.
    if (method->isOverloadedOperator() || method->isStatic() ||
        isa<language::Core::CXXConstructorDecl>(decl))
      return true;

    if (isForeignReferenceType(method->getReturnType()))
      return true;

    // begin and end methods likely return an interator, so they're unsafe. This
    // is required so that automatic the conformance to RAC works properly.
    if (method->getNameAsString() == "begin" ||
        method->getNameAsString() == "end")
      return false;

    auto parentQualType = method
      ->getParent()->getTypeForDecl()->getCanonicalTypeUnqualified();

    bool parentIsSelfContained =
      !isForeignReferenceType(parentQualType) &&
      anySubobjectsSelfContained(method->getParent());

    // If it returns a pointer or reference from an owned parent, that's a
    // projection (unsafe).
    if (method->getReturnType()->isPointerType() ||
        method->getReturnType()->isReferenceType())
      return !parentIsSelfContained;

    // Check if it's one of the known unsafe methods we currently
    // mark as safe by default.
    if (isUnsafeStdMethod(method))
      return false;

    // Try to figure out the semantics of the return type. If it's a
    // pointer/iterator, it's unsafe.
    if (auto returnType = dyn_cast<language::Core::RecordType>(
            method->getReturnType().getCanonicalType())) {
      if (auto cxxRecordReturnType =
              dyn_cast<language::Core::CXXRecordDecl>(returnType->getDecl())) {
        if (isCodiraClassType(cxxRecordReturnType))
          return true;

        if (hasIteratorAPIAttr(cxxRecordReturnType) ||
            isIterator(cxxRecordReturnType))
          return false;

        // Mark this as safe to help our diganostics down the road.
        if (!cxxRecordReturnType->getDefinition()) {
          return true;
        }

        // A projection of a view type (such as a string_view) from a self
        // contained parent is a proejction (unsafe).
        if (!anySubobjectsSelfContained(cxxRecordReturnType) &&
            isViewType(cxxRecordReturnType)) {
          return !parentIsSelfContained;
        }
      }
    }
  }

  // Otherwise, it's safe.
  return true;
}

void language::simple_display(toolchain::raw_ostream &out,
                           CxxRecordSemanticsDescriptor desc) {
  out << "Matching API semantics of C++ record '"
      << desc.decl->getNameAsString() << "'.\n";
}

SourceLoc language::extractNearestSourceLoc(CxxRecordSemanticsDescriptor desc) {
  return SourceLoc();
}

void language::simple_display(toolchain::raw_ostream &out,
                           SafeUseOfCxxDeclDescriptor desc) {
  out << "Checking if '";
  if (auto namedDecl = dyn_cast<language::Core::NamedDecl>(desc.decl))
    out << namedDecl->getNameAsString();
  else
    out << "<invalid decl>";
  out << "' is safe to use in context.\n";
}

SourceLoc language::extractNearestSourceLoc(SafeUseOfCxxDeclDescriptor desc) {
  return SourceLoc();
}

void language::simple_display(toolchain::raw_ostream &out,
                           CxxDeclExplicitSafetyDescriptor desc) {
  out << "Checking if '";
  if (auto namedDecl = dyn_cast<language::Core::NamedDecl>(desc.decl))
    out << namedDecl->getNameAsString();
  else
    out << "<invalid decl>";
  out << "' is explicitly safe.\n";
}

SourceLoc language::extractNearestSourceLoc(CxxDeclExplicitSafetyDescriptor desc) {
  return SourceLoc();
}

CustomRefCountingOperationResult CustomRefCountingOperation::evaluate(
    Evaluator &evaluator, CustomRefCountingOperationDescriptor desc) const {
  auto languageDecl = desc.decl;
  auto operation = desc.kind;
  auto &ctx = languageDecl->getASTContext();

  std::string operationStr = operation == CustomRefCountingOperationKind::retain
                                 ? "retain:"
                                 : "release:";

  auto decl = cast<language::Core::RecordDecl>(languageDecl->getClangDecl());

  if (!hasImportAsRefAttr(decl)) {
    if (auto parentRefDecl = getRefParentOrDiag(decl, ctx, nullptr))
      decl = parentRefDecl;
  }

  if (!decl->hasAttrs())
    return {CustomRefCountingOperationResult::noAttribute, nullptr, ""};

  toolchain::SmallVector<const language::Core::CodiraAttrAttr *, 1> retainReleaseAttrs;
  for (auto *attr : decl->getAttrs()) {
    if (auto languageAttr = toolchain::dyn_cast<language::Core::CodiraAttrAttr>(attr)) {
      if (languageAttr->getAttribute().starts_with(operationStr)) {
        retainReleaseAttrs.push_back(languageAttr);
      }
    }
  }

  if (retainReleaseAttrs.empty())
    return {CustomRefCountingOperationResult::noAttribute, nullptr, ""};

  if (retainReleaseAttrs.size() > 1)
    return {CustomRefCountingOperationResult::tooManyAttributes, nullptr, ""};

  auto name = retainReleaseAttrs.front()
                  ->getAttribute()
                  .drop_front(StringRef(operationStr).size())
                  .str();

  if (name == "immortal")
    return {CustomRefCountingOperationResult::immortal, nullptr, name};

  toolchain::SmallVector<ValueDecl *, 1> results =
      getValueDeclsForName(languageDecl->getClangDecl(), ctx, name);
  if (results.size() == 1)
    return {CustomRefCountingOperationResult::foundOperation, results.front(),
            name};

  if (results.empty())
    return {CustomRefCountingOperationResult::notFound, nullptr, name};

  return {CustomRefCountingOperationResult::tooManyFound, nullptr, name};
}

/// Check whether the given Clang type involves an unsafe type.
static bool hasUnsafeType(Evaluator &evaluator, language::Core::QualType clangType) {
  // Handle pointers.
  auto pointeeType = clangType->getPointeeType();
  if (!pointeeType.isNull()) {
    // Function pointers are okay.
    if (pointeeType->isFunctionType())
      return false;
    
    // Pointers to record types are okay if they come in as foreign reference
    // types.
    if (auto recordDecl = pointeeType->getAsRecordDecl()) {
      if (hasImportAsRefAttr(recordDecl))
        return false;
    }
    
    // All other pointers are considered unsafe.
    return true;
  }
  
  // Handle records recursively.
  if (auto recordDecl = clangType->getAsTagDecl()) {
    // If we reached this point the types is not imported as a shared reference,
    // so we don't need to check the bases whether they are shared references.
    auto safety = evaluateOrDefault(
        evaluator, ClangDeclExplicitSafety({recordDecl, false}),
        ExplicitSafety::Unspecified);
    switch (safety) {
      case ExplicitSafety::Unsafe:
        return true;
        
      case ExplicitSafety::Safe:
      case ExplicitSafety::Unspecified:
        return false;        
    }
  }
    
  // Everything else is safe.
  return false;
}

ExplicitSafety
ClangDeclExplicitSafety::evaluate(Evaluator &evaluator,
                                  CxxDeclExplicitSafetyDescriptor desc) const {
  // FIXME: Somewhat duplicative with importAsUnsafe.
  // FIXME: Also similar to hasPointerInSubobjects
  // FIXME: should probably also subsume IsSafeUseOfCxxDecl
  
  // Explicitly unsafe.
  auto decl = desc.decl;
  if (hasUnsafeAPIAttr(decl) || hasCodiraAttribute(decl, "unsafe"))
    return ExplicitSafety::Unsafe;
  
  // Explicitly safe.
  if (hasCodiraAttribute(decl, "safe"))
    return ExplicitSafety::Safe;

  // Shared references are considered safe.
  if (desc.isClass)
    return ExplicitSafety::Safe;

  // Enums are always safe.
  if (isa<language::Core::EnumDecl>(decl))
    return ExplicitSafety::Safe;

  // If it's not a record, leave it unspecified.
  auto recordDecl = dyn_cast<language::Core::RecordDecl>(decl);
  if (!recordDecl)
    return ExplicitSafety::Unspecified;

  // Escapable and non-escapable annotations imply that the declaration is
  // safe.
  if (evaluateOrDefault(
          evaluator,
          ClangTypeEscapability({recordDecl->getTypeForDecl(), nullptr}),
          CxxEscapability::Unknown) != CxxEscapability::Unknown)
    return ExplicitSafety::Safe;
  
  // If we don't have a definition, leave it unspecified.
  recordDecl = recordDecl->getDefinition();
  if (!recordDecl)
    return ExplicitSafety::Unspecified;
  
  // If this is a C++ class, check its bases.
  if (auto cxxRecordDecl = dyn_cast<language::Core::CXXRecordDecl>(recordDecl)) {
    for (auto base : cxxRecordDecl->bases()) {
      if (hasUnsafeType(evaluator, base.getType()))
        return ExplicitSafety::Unsafe;
    }
  }
  
  // Check the fields.
  for (auto field : recordDecl->fields()) {
    if (hasUnsafeType(evaluator, field->getType()))
      return ExplicitSafety::Unsafe;
  }
  
  // Okay, call it safe.
  return ExplicitSafety::Safe;
}

bool ClangDeclExplicitSafety::isCached() const {
  return isa<language::Core::RecordDecl>(std::get<0>(getStorage()).decl);
}

const language::Core::TypedefType *ClangImporter::getTypeDefForCXXCFOptionsDefinition(
    const language::Core::Decl *candidateDecl) {

  if (!Impl.CodiraContext.LangOpts.EnableCXXInterop)
    return nullptr;

  auto enumDecl = dyn_cast<language::Core::EnumDecl>(candidateDecl);
  if (!enumDecl)
    return nullptr;

  if (!enumDecl->getDeclName().isEmpty())
    return nullptr;

  const language::Core::ElaboratedType *elaboratedType =
      dyn_cast<language::Core::ElaboratedType>(enumDecl->getIntegerType().getTypePtr());
  if (auto typedefType =
          elaboratedType
              ? dyn_cast<language::Core::TypedefType>(elaboratedType->desugar())
              : dyn_cast<language::Core::TypedefType>(
                    enumDecl->getIntegerType().getTypePtr())) {
    auto enumExtensibilityAttr =
        elaboratedType
            ? enumDecl->getAttr<language::Core::EnumExtensibilityAttr>()
            : typedefType->getDecl()->getAttr<language::Core::EnumExtensibilityAttr>();
    const bool hasFlagEnumAttr =
        elaboratedType ? enumDecl->hasAttr<language::Core::FlagEnumAttr>()
                       : typedefType->getDecl()->hasAttr<language::Core::FlagEnumAttr>();

    if (enumExtensibilityAttr &&
        enumExtensibilityAttr->getExtensibility() ==
            language::Core::EnumExtensibilityAttr::Open &&
        hasFlagEnumAttr) {
      return Impl.isUnavailableInCodira(typedefType->getDecl()) ? typedefType
                                                               : nullptr;
    }
  }

  return nullptr;
}

bool importer::requiresCPlusPlus(const language::Core::Module *module) {
  // The libc++ modulemap doesn't currently declare the requirement.
  if (isCxxStdModule(module))
    return true;

  // Modulemaps often declare the requirement for the top-level module only.
  if (auto parent = module->Parent) {
    if (requiresCPlusPlus(parent))
      return true;
  }

  return toolchain::any_of(module->Requirements, [](language::Core::Module::Requirement req) {
    return req.FeatureName == "cplusplus";
  });
}

bool importer::isCxxStdModule(const language::Core::Module *module) {
  return isCxxStdModule(module->getTopLevelModuleName(),
                        module->getTopLevelModule()->IsSystem);
}

bool importer::isCxxStdModule(StringRef moduleName, bool IsSystem) {
  if (moduleName == "std")
    return true;
  // In recent libc++ versions the module is split into multiple top-level
  // modules (std_vector, std_utility, etc).
  if (IsSystem && moduleName.starts_with("std_")) {
    if (moduleName == "std_errno_h")
      return false;
    return true;
  }
  return false;
}

std::optional<language::Core::QualType>
importer::getCxxReferencePointeeTypeOrNone(const language::Core::Type *type) {
  if (type->isReferenceType())
    return type->getPointeeType();
  return {};
}

bool importer::isCxxConstReferenceType(const language::Core::Type *type) {
  auto pointeeType = getCxxReferencePointeeTypeOrNone(type);
  return pointeeType && pointeeType->isConstQualified();
}

AccessLevel importer::convertClangAccess(language::Core::AccessSpecifier access) {
  switch (access) {
  case language::Core::AS_public:
    // C++ 'public' is actually closer to Codira 'open' than Codira 'public',
    // since C++ 'public' does not prevent users from subclassing a type or
    // overriding a method. However, subclassing and overriding are currently
    // unsupported across the interop boundary, so we conservatively map C++
    // 'public' to Codira 'public' in case there are other C++ subtleties that
    // are being missed at this time (e.g., C++ 'final' vs Codira 'final').
    return AccessLevel::Public;

  case language::Core::AS_protected:
    // Codira does not have a notion of protected fields, so map C++ 'protected'
    // to Codira 'private'.
    return AccessLevel::Private;

  case language::Core::AS_private:
    // N.B. Codira 'private' is more restrictive than C++ 'private' because it
    // also cares about what source file the member is accessed.
    return AccessLevel::Private;

  case language::Core::AS_none:
    // The fictional 'none' specifier is given to top-level C++ declarations,
    // for which C++ lacks the syntax to give an access specifier. (It may also
    // be used in other cases I'm not aware of.) Those declarations are globally
    // visible and thus correspond to Codira 'public' (with the same caveats
    // about Codira 'public' vs 'open'; see above).
    return AccessLevel::Public;
  }
}

AccessLevel
ClangInheritanceInfo::accessForBaseDecl(const ValueDecl *baseDecl) const {
  if (!isInheriting())
    return AccessLevel::Public;

  static_assert(AccessLevel::Private < AccessLevel::Public &&
                "std::min() relies on this ordering");
  auto inherited =
      access ? importer::convertClangAccess(*access) : AccessLevel::Private;
  return std::min(baseDecl->getFormalAccess(), inherited);
}

void ClangInheritanceInfo::setUnavailableIfNecessary(
    const ValueDecl *baseDecl, ValueDecl *clonedDecl) const {
  if (!isInheriting())
    return;

  auto *clangDecl =
      dyn_cast_or_null<language::Core::NamedDecl>(baseDecl->getClangDecl());
  if (!clangDecl)
    return;

  const char *msg = nullptr;

  if (clangDecl->getAccess() == language::Core::AS_private)
    msg = "this base member is not accessible because it is private";
  else if (isNestedPrivate())
    msg = "this base member is not accessible because of private inheritance";

  if (msg)
    clonedDecl->getAttrs().add(AvailableAttr::createUniversallyUnavailable(
        clonedDecl->getASTContext(), msg));
}

SmallVector<std::pair<StringRef, language::Core::SourceLocation>, 1>
importer::getPrivateFileIDAttrs(const language::Core::CXXRecordDecl *decl) {
  toolchain::SmallVector<std::pair<StringRef, language::Core::SourceLocation>, 1> files;
  constexpr auto prefix = StringRef("private_fileid:");

  if (decl->hasAttrs()) {
    for (const auto *attr : decl->getAttrs()) {
      const auto *languageAttr = dyn_cast<language::Core::CodiraAttrAttr>(attr);
      if (languageAttr && languageAttr->getAttribute().starts_with(prefix))
        files.push_back({languageAttr->getAttribute().drop_front(prefix.size()),
                         attr->getLocation()});
    }
  }

  return files;
}

bool importer::declIsCxxOnly(const Decl *decl) {
  if (auto *clangDecl = decl->getClangDecl()) {
    return toolchain::TypeSwitch<const language::Core::Decl *, bool>(clangDecl)
        .template Case<const language::Core::NamespaceAliasDecl>(
            [](auto) { return true; })
        .template Case<const language::Core::NamespaceDecl>([](auto) { return true; })
        // For the issues this filter function was trying to resolve at its
        // time of writing, it suffices to only filter out namespaces. But
        // there are many other kinds of language::Core::Decls that only appear in C++.
        // This is obvious for some decls, e.g., templates, using directives,
        // non-trivial structs, and scoped enums; but it is not obvious for
        // other kinds of decls, e.g., an enum member or some variable.
        //
        // TODO: enumerate those kinds in a more precise and robust way
        .Default([](auto) { return false; });
  }
  return false;
}

bool importer::isClangNamespace(const DeclContext *dc) {
  if (const auto *ed = dc->getSelfEnumDecl())
    return isa_and_nonnull<language::Core::NamespaceDecl>(ed->getClangDecl());

  return false;
}

bool importer::isSymbolicCircularBase(const language::Core::CXXRecordDecl *symbolicClass,
                                      const language::Core::RecordDecl *base) {
  auto *classTemplate = symbolicClass->getDescribedClassTemplate();
  if (!classTemplate)
    return false;

  auto *specializedBase =
      dyn_cast<language::Core::ClassTemplateSpecializationDecl>(base);
  if (!specializedBase)
    return false;

  return classTemplate->getCanonicalDecl() ==
         specializedBase->getSpecializedTemplate()->getCanonicalDecl();
}

std::optional<ResultConvention>
language::importer::getCxxRefConventionWithAttrs(const language::Core::Decl *decl) {
  using RC = ResultConvention;

  if (auto result =
          matchCodiraAttr<RC>(decl, {{"returns_unretained", RC::Unowned},
                                    {"returns_retained", RC::Owned}}))
    return result;

  const language::Core::Type *returnTy = nullptr;
  if (const auto *fn = toolchain::dyn_cast<language::Core::FunctionDecl>(decl))
    returnTy = fn->getReturnType().getTypePtrOrNull();
  else if (const auto *method = toolchain::dyn_cast<language::Core::ObjCMethodDecl>(decl))
    returnTy = method->getReturnType().getTypePtrOrNull();

  if (!returnTy)
    return std::nullopt;

  const language::Core::Type *desugaredReturnTy =
      returnTy->getUnqualifiedDesugaredType();

  if (const auto *ptrType =
          toolchain::dyn_cast<language::Core::PointerType>(desugaredReturnTy)) {
    if (const language::Core::RecordDecl *record =
            ptrType->getPointeeType()->getAsRecordDecl()) {
      return matchCodiraAttrConsideringInheritance<RC>(
          record, {{"returned_as_unretained_by_default", RC::Unowned}});
    }
  }

  return std::nullopt;
}
