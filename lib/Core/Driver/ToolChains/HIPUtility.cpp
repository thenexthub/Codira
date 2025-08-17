//===--- HIPUtility.cpp - Common HIP Tool Chain Utilities -------*- C++ -*-===//
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

#include "HIPUtility.h"
#include "language/Core/Driver/CommonArgs.h"
#include "language/Core/Driver/Compilation.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Object/Archive.h"
#include "toolchain/Object/ObjectFile.h"
#include "toolchain/Support/MD5.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/TargetParser/Triple.h"
#include <deque>
#include <set>

using namespace language::Core;
using namespace language::Core::driver;
using namespace language::Core::driver::tools;
using namespace toolchain::opt;
using toolchain::dyn_cast;

#if defined(_WIN32) || defined(_WIN64)
#define NULL_FILE "nul"
#else
#define NULL_FILE "/dev/null"
#endif

namespace {
const unsigned HIPCodeObjectAlign = 4096;
} // namespace

// Constructs a triple string for clang offload bundler.
static std::string normalizeForBundler(const toolchain::Triple &T,
                                       bool HasTargetID) {
  return HasTargetID ? (T.getArchName() + "-" + T.getVendorName() + "-" +
                        T.getOSName() + "-" + T.getEnvironmentName())
                           .str()
                     : T.normalize(toolchain::Triple::CanonicalForm::FOUR_IDENT);
}

// Collect undefined __hip_fatbin* and __hip_gpubin_handle* symbols from all
// input object or archive files.
class HIPUndefinedFatBinSymbols {
public:
  HIPUndefinedFatBinSymbols(const Compilation &C,
                            const toolchain::opt::ArgList &Args_)
      : C(C), Args(Args_),
        DiagID(C.getDriver().getDiags().getCustomDiagID(
            DiagnosticsEngine::Error,
            "Error collecting HIP undefined fatbin symbols: %0")),
        Quiet(C.getArgs().hasArg(options::OPT__HASH_HASH_HASH)),
        Verbose(C.getArgs().hasArg(options::OPT_v)) {
    populateSymbols();
    processStaticLibraries();
    if (Verbose) {
      for (const auto &Name : FatBinSymbols)
        toolchain::errs() << "Found undefined HIP fatbin symbol: " << Name << "\n";
      for (const auto &Name : GPUBinHandleSymbols)
        toolchain::errs() << "Found undefined HIP gpubin handle symbol: " << Name
                     << "\n";
    }
  }

  const std::set<std::string> &getFatBinSymbols() const {
    return FatBinSymbols;
  }

  const std::set<std::string> &getGPUBinHandleSymbols() const {
    return GPUBinHandleSymbols;
  }

  // Collect symbols from static libraries specified by -l options.
  void processStaticLibraries() {
    toolchain::SmallVector<toolchain::StringRef, 16> LibNames;
    toolchain::SmallVector<toolchain::StringRef, 16> LibPaths;
    toolchain::SmallVector<toolchain::StringRef, 16> ExactLibNames;
    toolchain::Triple Triple(C.getDriver().getTargetTriple());
    bool IsMSVC = Triple.isWindowsMSVCEnvironment();
    toolchain::StringRef Ext = IsMSVC ? ".lib" : ".a";

    for (const auto *Arg : Args.filtered(options::OPT_l)) {
      toolchain::StringRef Value = Arg->getValue();
      if (Value.starts_with(":"))
        ExactLibNames.push_back(Value.drop_front());
      else
        LibNames.push_back(Value);
    }
    for (const auto *Arg : Args.filtered(options::OPT_L)) {
      auto Path = Arg->getValue();
      LibPaths.push_back(Path);
      if (Verbose)
        toolchain::errs() << "HIP fatbin symbol search uses library path:  " << Path
                     << "\n";
    }

    auto ProcessLib = [&](toolchain::StringRef LibName, bool IsExact) {
      toolchain::SmallString<256> FullLibName(
          IsExact  ? Twine(LibName).str()
          : IsMSVC ? (Twine(LibName) + Ext).str()
                   : (Twine("lib") + LibName + Ext).str());

      bool Found = false;
      for (const auto Path : LibPaths) {
        toolchain::SmallString<256> FullPath = Path;
        toolchain::sys::path::append(FullPath, FullLibName);

        if (toolchain::sys::fs::exists(FullPath)) {
          if (Verbose)
            toolchain::errs() << "HIP fatbin symbol search found library: "
                         << FullPath << "\n";
          auto BufferOrErr = toolchain::MemoryBuffer::getFile(FullPath);
          if (!BufferOrErr) {
            errorHandler(toolchain::errorCodeToError(BufferOrErr.getError()));
            continue;
          }
          processInput(BufferOrErr.get()->getMemBufferRef());
          Found = true;
          break;
        }
      }
      if (!Found && Verbose)
        toolchain::errs() << "HIP fatbin symbol search could not find library: "
                     << FullLibName << "\n";
    };

    for (const auto LibName : ExactLibNames)
      ProcessLib(LibName, true);

    for (const auto LibName : LibNames)
      ProcessLib(LibName, false);
  }

private:
  const Compilation &C;
  const toolchain::opt::ArgList &Args;
  unsigned DiagID;
  bool Quiet;
  bool Verbose;
  std::set<std::string> FatBinSymbols;
  std::set<std::string> GPUBinHandleSymbols;
  std::set<std::string, std::less<>> DefinedFatBinSymbols;
  std::set<std::string, std::less<>> DefinedGPUBinHandleSymbols;
  const std::string FatBinPrefix = "__hip_fatbin";
  const std::string GPUBinHandlePrefix = "__hip_gpubin_handle";

  void populateSymbols() {
    std::deque<const Action *> WorkList;
    std::set<const Action *> Visited;

    for (const auto &Action : C.getActions())
      WorkList.push_back(Action);

    while (!WorkList.empty()) {
      const Action *CurrentAction = WorkList.front();
      WorkList.pop_front();

      if (!CurrentAction || !Visited.insert(CurrentAction).second)
        continue;

      if (const auto *IA = dyn_cast<InputAction>(CurrentAction)) {
        std::string ID = IA->getId().str();
        if (!ID.empty()) {
          ID = toolchain::utohexstr(toolchain::MD5Hash(ID), /*LowerCase=*/true);
          FatBinSymbols.insert((FatBinPrefix + Twine('_') + ID).str());
          GPUBinHandleSymbols.insert(
              (GPUBinHandlePrefix + Twine('_') + ID).str());
          continue;
        }
        if (IA->getInputArg().getNumValues() == 0)
          continue;
        const char *Filename = IA->getInputArg().getValue();
        if (!Filename)
          continue;
        auto BufferOrErr = toolchain::MemoryBuffer::getFile(Filename);
        // Input action could be options to linker, therefore, ignore it
        // if cannot read it. If it turns out to be a file that cannot be read,
        // the error will be caught by the linker.
        if (!BufferOrErr)
          continue;

        processInput(BufferOrErr.get()->getMemBufferRef());
      } else
        toolchain::append_range(WorkList, CurrentAction->getInputs());
    }
  }

  void processInput(const toolchain::MemoryBufferRef &Buffer) {
    // Try processing as object file first.
    auto ObjFileOrErr = toolchain::object::ObjectFile::createObjectFile(Buffer);
    if (ObjFileOrErr) {
      processSymbols(**ObjFileOrErr);
      return;
    }

    // Then try processing as archive files.
    toolchain::consumeError(ObjFileOrErr.takeError());
    auto ArchiveOrErr = toolchain::object::Archive::create(Buffer);
    if (ArchiveOrErr) {
      toolchain::Error Err = toolchain::Error::success();
      toolchain::object::Archive &Archive = *ArchiveOrErr.get();
      for (auto &Child : Archive.children(Err)) {
        auto ChildBufOrErr = Child.getMemoryBufferRef();
        if (ChildBufOrErr)
          processInput(*ChildBufOrErr);
        else
          errorHandler(ChildBufOrErr.takeError());
      }

      if (Err)
        errorHandler(std::move(Err));
      return;
    }

    // Ignore other files.
    toolchain::consumeError(ArchiveOrErr.takeError());
  }

  void processSymbols(const toolchain::object::ObjectFile &Obj) {
    for (const auto &Symbol : Obj.symbols()) {
      auto FlagOrErr = Symbol.getFlags();
      if (!FlagOrErr) {
        errorHandler(FlagOrErr.takeError());
        continue;
      }

      auto NameOrErr = Symbol.getName();
      if (!NameOrErr) {
        errorHandler(NameOrErr.takeError());
        continue;
      }
      toolchain::StringRef Name = *NameOrErr;

      bool isUndefined =
          FlagOrErr.get() & toolchain::object::SymbolRef::SF_Undefined;
      bool isFatBinSymbol = Name.starts_with(FatBinPrefix);
      bool isGPUBinHandleSymbol = Name.starts_with(GPUBinHandlePrefix);

      // Handling for defined symbols
      if (!isUndefined) {
        if (isFatBinSymbol) {
          DefinedFatBinSymbols.insert(Name.str());
          FatBinSymbols.erase(Name.str());
        } else if (isGPUBinHandleSymbol) {
          DefinedGPUBinHandleSymbols.insert(Name.str());
          GPUBinHandleSymbols.erase(Name.str());
        }
        continue;
      }

      // Add undefined symbols if they are not in the defined sets
      if (isFatBinSymbol &&
          DefinedFatBinSymbols.find(Name) == DefinedFatBinSymbols.end())
        FatBinSymbols.insert(Name.str());
      else if (isGPUBinHandleSymbol && DefinedGPUBinHandleSymbols.find(Name) ==
                                           DefinedGPUBinHandleSymbols.end())
        GPUBinHandleSymbols.insert(Name.str());
    }
  }

  void errorHandler(toolchain::Error Err) {
    if (Quiet)
      return;
    C.getDriver().Diag(DiagID) << toolchain::toString(std::move(Err));
  }
};

// Construct a clang-offload-bundler command to bundle code objects for
// different devices into a HIP fat binary.
void HIP::constructHIPFatbinCommand(Compilation &C, const JobAction &JA,
                                    toolchain::StringRef OutputFileName,
                                    const InputInfoList &Inputs,
                                    const toolchain::opt::ArgList &Args,
                                    const Tool &T) {
  // Construct clang-offload-bundler command to bundle object files for
  // for different GPU archs.
  ArgStringList BundlerArgs;
  BundlerArgs.push_back(Args.MakeArgString("-type=o"));
  BundlerArgs.push_back(
      Args.MakeArgString("-bundle-align=" + Twine(HIPCodeObjectAlign)));

  // ToDo: Remove the dummy host binary entry which is required by
  // clang-offload-bundler.
  std::string BundlerTargetArg = "-targets=host-x86_64-unknown-linux-gnu";
  // AMDGCN:
  // For code object version 2 and 3, the offload kind in bundle ID is 'hip'
  // for backward compatibility. For code object version 4 and greater, the
  // offload kind in bundle ID is 'hipv4'.
  std::string OffloadKind = "hip";
  auto &TT = T.getToolChain().getTriple();
  if (TT.isAMDGCN() && getAMDGPUCodeObjectVersion(C.getDriver(), Args) >= 4)
    OffloadKind = OffloadKind + "v4";
  for (const auto &II : Inputs) {
    const auto *A = II.getAction();
    auto ArchStr = toolchain::StringRef(A->getOffloadingArch());
    BundlerTargetArg += ',' + OffloadKind + '-';
    if (ArchStr == "amdgcnspirv")
      BundlerTargetArg +=
          normalizeForBundler(toolchain::Triple("spirv64-amd-amdhsa"), true);
    else
      BundlerTargetArg += normalizeForBundler(TT, !ArchStr.empty());
    if (!ArchStr.empty())
      BundlerTargetArg += '-' + ArchStr.str();
  }
  BundlerArgs.push_back(Args.MakeArgString(BundlerTargetArg));

  // Use a NULL file as input for the dummy host binary entry
  std::string BundlerInputArg = "-input=" NULL_FILE;
  BundlerArgs.push_back(Args.MakeArgString(BundlerInputArg));
  for (const auto &II : Inputs) {
    BundlerInputArg = std::string("-input=") + II.getFilename();
    BundlerArgs.push_back(Args.MakeArgString(BundlerInputArg));
  }

  std::string Output = std::string(OutputFileName);
  auto *BundlerOutputArg =
      Args.MakeArgString(std::string("-output=").append(Output));
  BundlerArgs.push_back(BundlerOutputArg);

  addOffloadCompressArgs(Args, BundlerArgs);

  const char *Bundler = Args.MakeArgString(
      T.getToolChain().GetProgramPath("clang-offload-bundler"));
  C.addCommand(std::make_unique<Command>(
      JA, T, ResponseFileSupport::None(), Bundler, BundlerArgs, Inputs,
      InputInfo(&JA, Args.MakeArgString(Output))));
}

/// Add Generated HIP Object File which has device images embedded into the
/// host to the argument list for linking. Using MC directives, embed the
/// device code and also define symbols required by the code generation so that
/// the image can be retrieved at runtime.
void HIP::constructGenerateObjFileFromHIPFatBinary(
    Compilation &C, const InputInfo &Output, const InputInfoList &Inputs,
    const ArgList &Args, const JobAction &JA, const Tool &T) {
  const Driver &D = C.getDriver();
  std::string Name = std::string(toolchain::sys::path::stem(Output.getFilename()));

  // Create Temp Object File Generator,
  // Offload Bundled file and Bundled Object file.
  // Keep them if save-temps is enabled.
  const char *ObjinFile;
  const char *BundleFile;
  if (D.isSaveTempsEnabled()) {
    ObjinFile = C.getArgs().MakeArgString(Name + ".mcin");
    BundleFile = C.getArgs().MakeArgString(Name + ".hipfb");
  } else {
    auto TmpNameMcin = D.GetTemporaryPath(Name, "mcin");
    ObjinFile = C.addTempFile(C.getArgs().MakeArgString(TmpNameMcin));
    auto TmpNameFb = D.GetTemporaryPath(Name, "hipfb");
    BundleFile = C.addTempFile(C.getArgs().MakeArgString(TmpNameFb));
  }
  HIP::constructHIPFatbinCommand(C, JA, BundleFile, Inputs, Args, T);

  // Create a buffer to write the contents of the temp obj generator.
  std::string ObjBuffer;
  toolchain::raw_string_ostream ObjStream(ObjBuffer);

  auto HostTriple =
      C.getSingleOffloadToolChain<Action::OFK_Host>()->getTriple();

  HIPUndefinedFatBinSymbols Symbols(C, Args);

  std::string PrimaryHipFatbinSymbol;
  std::string PrimaryGpuBinHandleSymbol;
  bool FoundPrimaryHipFatbinSymbol = false;
  bool FoundPrimaryGpuBinHandleSymbol = false;

  std::vector<std::string> AliasHipFatbinSymbols;
  std::vector<std::string> AliasGpuBinHandleSymbols;

  // Iterate through symbols to find the primary ones and collect others for
  // aliasing
  for (const auto &Symbol : Symbols.getFatBinSymbols()) {
    if (!FoundPrimaryHipFatbinSymbol) {
      PrimaryHipFatbinSymbol = Symbol;
      FoundPrimaryHipFatbinSymbol = true;
    } else
      AliasHipFatbinSymbols.push_back(Symbol);
  }

  for (const auto &Symbol : Symbols.getGPUBinHandleSymbols()) {
    if (!FoundPrimaryGpuBinHandleSymbol) {
      PrimaryGpuBinHandleSymbol = Symbol;
      FoundPrimaryGpuBinHandleSymbol = true;
    } else
      AliasGpuBinHandleSymbols.push_back(Symbol);
  }

  // Add MC directives to embed target binaries. We ensure that each
  // section and image is 16-byte aligned. This is not mandatory, but
  // increases the likelihood of data to be aligned with a cache block
  // in several main host machines.
  ObjStream << "#       HIP Object Generator\n";
  ObjStream << "# *** Automatically generated by Clang ***\n";
  if (FoundPrimaryGpuBinHandleSymbol) {
    // Define the first gpubin handle symbol
    if (HostTriple.isWindowsMSVCEnvironment())
      ObjStream << "  .section .hip_gpubin_handle,\"dw\"\n";
    else {
      ObjStream << "  .protected " << PrimaryGpuBinHandleSymbol << "\n";
      ObjStream << "  .type " << PrimaryGpuBinHandleSymbol << ",@object\n";
      ObjStream << "  .section .hip_gpubin_handle,\"aw\"\n";
    }
    ObjStream << "  .globl " << PrimaryGpuBinHandleSymbol << "\n";
    ObjStream << "  .p2align 3\n"; // Align 8
    ObjStream << PrimaryGpuBinHandleSymbol << ":\n";
    ObjStream << "  .zero 8\n"; // Size 8

    // Generate alias directives for other gpubin handle symbols
    for (const auto &AliasSymbol : AliasGpuBinHandleSymbols) {
      ObjStream << "  .globl " << AliasSymbol << "\n";
      ObjStream << "  .set " << AliasSymbol << "," << PrimaryGpuBinHandleSymbol
                << "\n";
    }
  }
  if (FoundPrimaryHipFatbinSymbol) {
    // Define the first fatbin symbol
    if (HostTriple.isWindowsMSVCEnvironment())
      ObjStream << "  .section .hip_fatbin,\"dw\"\n";
    else {
      ObjStream << "  .protected " << PrimaryHipFatbinSymbol << "\n";
      ObjStream << "  .type " << PrimaryHipFatbinSymbol << ",@object\n";
      ObjStream << "  .section .hip_fatbin,\"a\",@progbits\n";
    }
    ObjStream << "  .globl " << PrimaryHipFatbinSymbol << "\n";
    ObjStream << "  .p2align " << toolchain::Log2(toolchain::Align(HIPCodeObjectAlign))
              << "\n";
    // Generate alias directives for other fatbin symbols
    for (const auto &AliasSymbol : AliasHipFatbinSymbols) {
      ObjStream << "  .globl " << AliasSymbol << "\n";
      ObjStream << "  .set " << AliasSymbol << "," << PrimaryHipFatbinSymbol
                << "\n";
    }
    ObjStream << PrimaryHipFatbinSymbol << ":\n";
    ObjStream << "  .incbin ";
    toolchain::sys::printArg(ObjStream, BundleFile, /*Quote=*/true);
    ObjStream << "\n";
  }
  if (HostTriple.isOSLinux() && HostTriple.isOSBinFormatELF())
    ObjStream << "  .section .note.GNU-stack, \"\", @progbits\n";

  // Dump the contents of the temp object file gen if the user requested that.
  // We support this option to enable testing of behavior with -###.
  if (C.getArgs().hasArg(options::OPT_fhip_dump_offload_linker_script))
    toolchain::errs() << ObjBuffer;

  // Open script file and write the contents.
  std::error_code EC;
  toolchain::raw_fd_ostream Objf(ObjinFile, EC, toolchain::sys::fs::OF_None);

  if (EC) {
    D.Diag(language::Core::diag::err_unable_to_make_temp) << EC.message();
    return;
  }

  Objf << ObjBuffer;

  ArgStringList ClangArgs{"-target", Args.MakeArgString(HostTriple.normalize()),
                       "-o",      Output.getFilename(),
                       "-x",      "assembler",
                       ObjinFile, "-c"};
  C.addCommand(std::make_unique<Command>(JA, T, ResponseFileSupport::None(),
                                         D.getClangProgramPath(), ClangArgs,
                                         Inputs, Output, D.getPrependArg()));
}
