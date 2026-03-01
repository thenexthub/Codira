//===- Target.cpp - MLIR LLVM ROCDL target compilation ----------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// This files defines ROCDL target related functions including registration
// calls for the `#rocdl.target` compilation attribute.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVM/ROCDL/Target.h"

#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/LLVMIR/ROCDLDialect.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Target/LLVM/ROCDL/Utils.h"
#include "mlir/Target/LLVMIR/Export.h"

#include "vm/core/IR/Constants.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCObjectFileInfo.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCParser/MCTargetAsmParser.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/FileUtilities.h"
#include "vm/core/Support/Path.h"
#include "vm/core/Support/Program.h"
#include "vm/core/Support/SourceMgr.h"
#include "vm/core/Support/TargetSelect.h"
#include "vm/core/TargetParser/TargetParser.h"

#include <cstdlib>
#include <optional>

using namespace mlir;
using namespace mlir::ROCDL;

#ifndef __DEFAULT_ROCM_PATH__
#define __DEFAULT_ROCM_PATH__ ""
#endif

namespace {
// Implementation of the `TargetAttrInterface` model.
class ROCDLTargetAttrImpl
    : public gpu::TargetAttrInterface::FallbackModel<ROCDLTargetAttrImpl> {
public:
  std::optional<SmallVector<char, 0>>
  serializeToObject(Attribute attribute, Operation *module,
                    const gpu::TargetOptions &options) const;

  Attribute createObject(Attribute attribute, Operation *module,
                         const SmallVector<char, 0> &object,
                         const gpu::TargetOptions &options) const;
};
} // namespace

// Register the ROCDL dialect, the ROCDL translation and the target interface.
void mlir::ROCDL::registerROCDLTargetInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, ROCDL::ROCDLDialect *dialect) {
    ROCDLTargetAttr::attachInterface<ROCDLTargetAttrImpl>(*ctx);
  });
}

void mlir::ROCDL::registerROCDLTargetInterfaceExternalModels(
    MLIRContext &context) {
  DialectRegistry registry;
  registerROCDLTargetInterfaceExternalModels(registry);
  context.appendDialectRegistry(registry);
}

// Search for the ROCM path.
StringRef mlir::ROCDL::getROCMPath() {
  if (const char *var = std::getenv("ROCM_PATH"))
    return var;
  if (const char *var = std::getenv("ROCM_ROOT"))
    return var;
  if (const char *var = std::getenv("ROCM_HOME"))
    return var;
  return __DEFAULT_ROCM_PATH__;
}

SerializeGPUModuleBase::SerializeGPUModuleBase(
    Operation &module, ROCDLTargetAttr target,
    const gpu::TargetOptions &targetOptions)
    : ModuleToObject(module, target.getTriple(), target.getChip(),
                     target.getFeatures(), target.getO()),
      target(target), toolkitPath(targetOptions.getToolkitPath()),
      librariesToLink(targetOptions.getLibrariesToLink()) {

  // If `targetOptions` has an empty toolkitPath use `getROCMPath`
  if (toolkitPath.empty())
    toolkitPath = getROCMPath();

  // Append the files in the target attribute.
  if (target.getLink())
    librariesToLink.append(target.getLink().begin(), target.getLink().end());
}

void SerializeGPUModuleBase::init() {
  static toolchain::once_flag initializeBackendOnce;
  toolchain::call_once(initializeBackendOnce, []() {
  // If the `AMDGPU` LLVM target was built, initialize it.
#if MLIR_ENABLE_ROCM_CONVERSIONS
    LLVMInitializeAMDGPUTarget();
    LLVMInitializeAMDGPUTargetInfo();
    LLVMInitializeAMDGPUTargetMC();
    LLVMInitializeAMDGPUAsmParser();
    LLVMInitializeAMDGPUAsmPrinter();
#endif
  });
}

ROCDLTargetAttr SerializeGPUModuleBase::getTarget() const { return target; }

StringRef SerializeGPUModuleBase::getToolkitPath() const { return toolkitPath; }

ArrayRef<Attribute> SerializeGPUModuleBase::getLibrariesToLink() const {
  return librariesToLink;
}

LogicalResult SerializeGPUModuleBase::appendStandardLibs(AMDGCNLibraries libs) {
  if (libs == AMDGCNLibraries::None)
    return success();
  StringRef pathRef = getToolkitPath();

  // Get the path for the device libraries
  SmallString<256> path;
  path.insert(path.begin(), pathRef.begin(), pathRef.end());
  toolchain::sys::path::append(path, "amdgcn", "bitcode");
  pathRef = StringRef(path.data(), path.size());

  // Fail if the path is invalid.
  if (!toolchain::sys::fs::is_directory(pathRef)) {
    getOperation().emitError() << "ROCm amdgcn bitcode path: " << pathRef
                               << " does not exist or is not a directory";
    return failure();
  }

  // Helper function for adding a library.
  auto addLib = [&](const Twine &lib) -> bool {
    auto baseSize = path.size();
    toolchain::sys::path::append(path, lib);
    StringRef pathRef(path.data(), path.size());
    if (!toolchain::sys::fs::is_regular_file(pathRef)) {
      getOperation().emitRemark() << "bitcode library path: " << pathRef
                                  << " does not exist or is not a file";
      return true;
    }
    librariesToLink.push_back(StringAttr::get(target.getContext(), pathRef));
    path.truncate(baseSize);
    return false;
  };

  // Add ROCm device libraries. Fail if any of the libraries is not found, ie.
  // if any of the `addLib` failed.
  if ((any(libs & AMDGCNLibraries::Ocml) && addLib("ocml.bc")) ||
      (any(libs & AMDGCNLibraries::Ockl) && addLib("ockl.bc")) ||
      (any(libs & AMDGCNLibraries::Hip) && addLib("hip.bc")) ||
      (any(libs & AMDGCNLibraries::OpenCL) && addLib("opencl.bc")))
    return failure();
  return success();
}

std::optional<SmallVector<std::unique_ptr<toolchain::Module>>>
SerializeGPUModuleBase::loadBitcodeFiles(toolchain::Module &module) {
  // Return if there are no libs to load.
  if (deviceLibs == AMDGCNLibraries::None && librariesToLink.empty())
    return SmallVector<std::unique_ptr<toolchain::Module>>();
  if (failed(appendStandardLibs(deviceLibs)))
    return std::nullopt;
  SmallVector<std::unique_ptr<toolchain::Module>> bcFiles;
  if (failed(loadBitcodeFilesFromList(module.getContext(), librariesToLink,
                                      bcFiles, true)))
    return std::nullopt;
  return std::move(bcFiles);
}

LogicalResult SerializeGPUModuleBase::handleBitcodeFile(toolchain::Module &module) {
  // Some ROCM builds don't strip this like they should
  if (auto *openclVersion = module.getNamedMetadata("opencl.ocl.version"))
    module.eraseNamedMetadata(openclVersion);
  // Stop spamming us with clang version numbers
  if (auto *ident = module.getNamedMetadata("toolchain.ident"))
    module.eraseNamedMetadata(ident);
  // Override the libModules datalayout and target triple with the compiler's
  // data layout should there be a discrepency.
  setDataLayoutAndTriple(module);
  return success();
}

void SerializeGPUModuleBase::handleModulePreLink(toolchain::Module &module) {
  // If all libraries are not set, traverse the module to determine which
  // libraries are required.
  if (deviceLibs != AMDGCNLibraries::All) {
    for (toolchain::Function &f : module.functions()) {
      if (f.hasExternalLinkage() && f.hasName() && !f.hasExactDefinition()) {
        StringRef funcName = f.getName();
        if ("printf" == funcName)
          deviceLibs |= AMDGCNLibraries::OpenCL | AMDGCNLibraries::Ockl |
                        AMDGCNLibraries::Ocml;
        if (funcName.starts_with("__ockl_"))
          deviceLibs |= AMDGCNLibraries::Ockl;
        if (funcName.starts_with("__ocml_"))
          deviceLibs |= AMDGCNLibraries::Ocml;
        if (funcName == "__atomic_work_item_fence")
          deviceLibs |= AMDGCNLibraries::Hip;
      }
    }
  }
  addControlVariables(module, deviceLibs, target.hasWave64(), target.hasDaz(),
                      target.hasFiniteOnly(), target.hasUnsafeMath(),
                      target.hasFastMath(), target.hasCorrectSqrt(),
                      target.getAbi());
}

void SerializeGPUModuleBase::addControlVariables(
    toolchain::Module &module, AMDGCNLibraries libs, bool wave64, bool daz,
    bool finiteOnly, bool unsafeMath, bool fastMath, bool correctSqrt,
    StringRef abiVer) {
  // Helper function for adding control variables.
  auto addControlVariable = [&module](StringRef name, uint32_t value,
                                      uint32_t bitwidth) {
    if (module.getNamedGlobal(name))
      return;
    toolchain::IntegerType *type =
        toolchain::IntegerType::getIntNTy(module.getContext(), bitwidth);
    toolchain::GlobalVariable *controlVariable = new toolchain::GlobalVariable(
        module, /*isConstant=*/type, true,
        toolchain::GlobalValue::LinkageTypes::LinkOnceODRLinkage,
        toolchain::ConstantInt::get(type, value), name, /*before=*/nullptr,
        /*threadLocalMode=*/toolchain::GlobalValue::ThreadLocalMode::NotThreadLocal,
        /*addressSpace=*/4);
    controlVariable->setVisibility(
        toolchain::GlobalValue::VisibilityTypes::ProtectedVisibility);
    controlVariable->setAlignment(toolchain::MaybeAlign(bitwidth / 8));
    controlVariable->setUnnamedAddr(toolchain::GlobalValue::UnnamedAddr::Local);
  };

  // Note that COV6 requires ROCm 6.3+.
  int abi = 600;
  abiVer.getAsInteger(0, abi);
  module.addModuleFlag(toolchain::Module::Error, "amdhsa_code_object_version", abi);
  // Return if no device libraries are required.
  if (libs == AMDGCNLibraries::None)
    return;
  // Add ocml related control variables.
  if (any(libs & AMDGCNLibraries::Ocml)) {
    addControlVariable("__oclc_finite_only_opt", finiteOnly || fastMath, 8);
    addControlVariable("__oclc_daz_opt", daz || fastMath, 8);
    addControlVariable("__oclc_correctly_rounded_sqrt32",
                       correctSqrt && !fastMath, 8);
    addControlVariable("__oclc_unsafe_math_opt", unsafeMath || fastMath, 8);
  }
  // Add ocml or ockl related control variables.
  if (any(libs & (AMDGCNLibraries::Ocml | AMDGCNLibraries::Ockl))) {
    addControlVariable("__oclc_wavefrontsize64", wave64, 8);
    // Get the ISA version.
    toolchain::AMDGPU::IsaVersion isaVersion = toolchain::AMDGPU::getIsaVersion(chip);
    // Add the ISA control variable.
    addControlVariable("__oclc_ISA_version",
                       isaVersion.Minor + 100 * isaVersion.Stepping +
                           1000 * isaVersion.Major,
                       32);
    addControlVariable("__oclc_ABI_version", abi, 32);
  }
}

FailureOr<SmallVector<char, 0>>
mlir::ROCDL::assembleIsa(StringRef isa, StringRef targetTriple, StringRef chip,
                         StringRef features,
                         function_ref<InFlightDiagnostic()> emitError) {
  SmallVector<char, 0> result;
  toolchain::raw_svector_ostream os(result);

  toolchain::Triple triple(toolchain::Triple::normalize(targetTriple));
  std::string error;
  const toolchain::Target *target =
      toolchain::TargetRegistry::lookupTarget(triple, error);
  if (!target)
    return emitError() << "failed to lookup target: " << error;

  toolchain::SourceMgr srcMgr;
  // Copy buffer to ensure it's null terminated.
  srcMgr.AddNewSourceBuffer(toolchain::MemoryBuffer::getMemBufferCopy(isa), SMLoc());

  const toolchain::MCTargetOptions mcOptions;
  std::unique_ptr<toolchain::MCRegisterInfo> mri(target->createMCRegInfo(triple));
  std::unique_ptr<toolchain::MCAsmInfo> mai(
      target->createMCAsmInfo(*mri, triple, mcOptions));
  std::unique_ptr<toolchain::MCSubtargetInfo> sti(
      target->createMCSubtargetInfo(triple, chip, features));

  toolchain::MCContext ctx(triple, mai.get(), mri.get(), sti.get(), &srcMgr,
                      &mcOptions);
  std::unique_ptr<toolchain::MCObjectFileInfo> mofi(target->createMCObjectFileInfo(
      ctx, /*PIC=*/false, /*LargeCodeModel=*/false));
  ctx.setObjectFileInfo(mofi.get());

  SmallString<128> cwd;
  if (!toolchain::sys::fs::current_path(cwd))
    ctx.setCompilationDir(cwd);

  std::unique_ptr<toolchain::MCStreamer> mcStreamer;
  std::unique_ptr<toolchain::MCInstrInfo> mcii(target->createMCInstrInfo());

  toolchain::MCCodeEmitter *ce = target->createMCCodeEmitter(*mcii, ctx);
  toolchain::MCAsmBackend *mab = target->createMCAsmBackend(*sti, *mri, mcOptions);
  mcStreamer.reset(target->createMCObjectStreamer(
      triple, ctx, std::unique_ptr<toolchain::MCAsmBackend>(mab),
      mab->createObjectWriter(os), std::unique_ptr<toolchain::MCCodeEmitter>(ce),
      *sti));

  std::unique_ptr<toolchain::MCAsmParser> parser(
      createMCAsmParser(srcMgr, ctx, *mcStreamer, *mai));
  std::unique_ptr<toolchain::MCTargetAsmParser> tap(
      target->createMCAsmParser(*sti, *parser, *mcii, mcOptions));

  if (!tap)
    return emitError() << "assembler initialization error";

  parser->setTargetParser(*tap);
  parser->Run(false);
  return std::move(result);
}

FailureOr<SmallVector<char, 0>>
mlir::ROCDL::linkObjectCode(ArrayRef<char> objectCode, StringRef toolkitPath,
                            function_ref<InFlightDiagnostic()> emitError) {
  // Save the ISA binary to a temp file.
  int tempIsaBinaryFd = -1;
  SmallString<128> tempIsaBinaryFilename;
  if (toolchain::sys::fs::createTemporaryFile("kernel%%", "o", tempIsaBinaryFd,
                                         tempIsaBinaryFilename))
    return emitError()
           << "failed to create a temporary file for dumping the ISA binary";

  toolchain::FileRemover cleanupIsaBinary(tempIsaBinaryFilename);
  {
    toolchain::raw_fd_ostream tempIsaBinaryOs(tempIsaBinaryFd, true);
    tempIsaBinaryOs << StringRef(objectCode.data(), objectCode.size());
    tempIsaBinaryOs.flush();
  }

  // Create a temp file for HSA code object.
  SmallString<128> tempHsacoFilename;
  if (toolchain::sys::fs::createTemporaryFile("kernel", "hsaco", tempHsacoFilename))
    return emitError()
           << "failed to create a temporary file for the HSA code object";

  toolchain::FileRemover cleanupHsaco(tempHsacoFilename);

  toolchain::SmallString<128> lldPath(toolkitPath);
  toolchain::sys::path::append(lldPath, "toolchain", "bin", "ld.lld");
  int lldResult = toolchain::sys::ExecuteAndWait(
      lldPath,
      {"ld.lld", "-shared", tempIsaBinaryFilename, "-o", tempHsacoFilename});
  if (lldResult != 0)
    return emitError() << "lld invocation failed";

  // Load the HSA code object.
  auto hsacoFile =
      toolchain::MemoryBuffer::getFile(tempHsacoFilename, /*IsText=*/false);
  if (!hsacoFile)
    return emitError()
           << "failed to read the HSA code object from the temp file";

  StringRef buffer = (*hsacoFile)->getBuffer();

  return SmallVector<char, 0>(buffer.begin(), buffer.end());
}

FailureOr<SmallVector<char, 0>>
SerializeGPUModuleBase::compileToBinary(StringRef serializedISA) {
  auto errCallback = [&]() { return getOperation().emitError(); };
  // Assemble the ISA.
  FailureOr<SmallVector<char, 0>> isaBinary = ROCDL::assembleIsa(
      serializedISA, this->triple, this->chip, this->features, errCallback);

  if (failed(isaBinary))
    return failure();

  // Link the object code.
  FailureOr<SmallVector<char, 0>> linkedCode =
      ROCDL::linkObjectCode(*isaBinary, toolkitPath, errCallback);
  if (failed(linkedCode))
    return failure();

  return linkedCode;
}

FailureOr<SmallVector<char, 0>> SerializeGPUModuleBase::moduleToObjectImpl(
    const gpu::TargetOptions &targetOptions, toolchain::Module &llvmModule) {
  // Return LLVM IR if the compilation target is offload.
#define DEBUG_TYPE "serialize-to-toolchain"
  LLVM_DEBUG({
    toolchain::dbgs() << "LLVM IR for module: "
                 << cast<gpu::GPUModuleOp>(getOperation()).getNameAttr() << "\n"
                 << llvmModule << "\n";
  });
#undef DEBUG_TYPE
  if (targetOptions.getCompilationTarget() == gpu::CompilationTarget::Offload)
    return SerializeGPUModuleBase::moduleToObject(llvmModule);

  FailureOr<toolchain::TargetMachine *> targetMachine = getOrCreateTargetMachine();
  if (failed(targetMachine))
    return getOperation().emitError()
           << "target Machine unavailable for triple " << triple
           << ", can't compile with LLVM";

  // Translate the Module to ISA.
  FailureOr<SmallString<0>> serializedISA =
      translateModuleToISA(llvmModule, **targetMachine,
                           [&]() { return getOperation().emitError(); });
  if (failed(serializedISA))
    return getOperation().emitError() << "failed translating the module to ISA";

#define DEBUG_TYPE "serialize-to-isa"
  LLVM_DEBUG({
    toolchain::dbgs() << "ISA for module: "
                 << cast<gpu::GPUModuleOp>(getOperation()).getNameAttr() << "\n"
                 << *serializedISA << "\n";
  });
#undef DEBUG_TYPE
  // Return ISA assembly code if the compilation target is assembly.
  if (targetOptions.getCompilationTarget() == gpu::CompilationTarget::Assembly)
    return SmallVector<char, 0>(serializedISA->begin(), serializedISA->end());

  // Compiling to binary requires a valid ROCm path, fail if it's not found.
  if (getToolkitPath().empty())
    return getOperation().emitError()
           << "invalid ROCm path, please set a valid path";

  // Compile to binary.
  return compileToBinary(*serializedISA);
}

#if MLIR_ENABLE_ROCM_CONVERSIONS
namespace {
class AMDGPUSerializer : public SerializeGPUModuleBase {
public:
  AMDGPUSerializer(Operation &module, ROCDLTargetAttr target,
                   const gpu::TargetOptions &targetOptions);

  FailureOr<SmallVector<char, 0>>
  moduleToObject(toolchain::Module &llvmModule) override;

private:
  // Target options.
  gpu::TargetOptions targetOptions;
};
} // namespace

AMDGPUSerializer::AMDGPUSerializer(Operation &module, ROCDLTargetAttr target,
                                   const gpu::TargetOptions &targetOptions)
    : SerializeGPUModuleBase(module, target, targetOptions),
      targetOptions(targetOptions) {}

FailureOr<SmallVector<char, 0>>
AMDGPUSerializer::moduleToObject(toolchain::Module &llvmModule) {
  return moduleToObjectImpl(targetOptions, llvmModule);
}
#endif // MLIR_ENABLE_ROCM_CONVERSIONS

std::optional<SmallVector<char, 0>> ROCDLTargetAttrImpl::serializeToObject(
    Attribute attribute, Operation *module,
    const gpu::TargetOptions &options) const {
  assert(module && "The module must be non null.");
  if (!module)
    return std::nullopt;
  if (!mlir::isa<gpu::GPUModuleOp>(module)) {
    module->emitError("module must be a GPU module");
    return std::nullopt;
  }
#if MLIR_ENABLE_ROCM_CONVERSIONS
  AMDGPUSerializer serializer(*module, cast<ROCDLTargetAttr>(attribute),
                              options);
  serializer.init();
  return serializer.run();
#else
  module->emitError("the `AMDGPU` target was not built. Please enable it when "
                    "building LLVM");
  return std::nullopt;
#endif // MLIR_ENABLE_ROCM_CONVERSIONS
}

Attribute
ROCDLTargetAttrImpl::createObject(Attribute attribute, Operation *module,
                                  const SmallVector<char, 0> &object,
                                  const gpu::TargetOptions &options) const {
  gpu::CompilationTarget format = options.getCompilationTarget();
  // If format is `fatbin` transform it to binary as `fatbin` is not yet
  // supported.
  gpu::KernelTableAttr kernels;
  if (format > gpu::CompilationTarget::Binary) {
    format = gpu::CompilationTarget::Binary;
    kernels = ROCDL::getKernelMetadata(module, object);
  }
  DictionaryAttr properties{};
  Builder builder(attribute.getContext());
  StringAttr objectStr =
      builder.getStringAttr(StringRef(object.data(), object.size()));
  return builder.getAttr<gpu::ObjectAttr>(attribute, format, objectStr,
                                          properties, kernels);
}
