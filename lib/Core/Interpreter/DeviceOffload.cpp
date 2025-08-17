//===---------- DeviceOffload.cpp - Device Offloading------------*- C++ -*-===//
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
// This file implements offloading to CUDA devices.
//
//===----------------------------------------------------------------------===//

#include "DeviceOffload.h"

#include "language/Core/Basic/TargetOptions.h"
#include "language/Core/CodeGen/ModuleBuilder.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Interpreter/PartialTranslationUnit.h"

#include "toolchain/IR/LegacyPassManager.h"
#include "toolchain/IR/Module.h"
#include "toolchain/MC/TargetRegistry.h"
#include "toolchain/Target/TargetMachine.h"

namespace language::Core {

IncrementalCUDADeviceParser::IncrementalCUDADeviceParser(
    CompilerInstance &DeviceInstance, CompilerInstance &HostInstance,
    toolchain::IntrusiveRefCntPtr<toolchain::vfs::InMemoryFileSystem> FS,
    toolchain::Error &Err, const std::list<PartialTranslationUnit> &PTUs)
    : IncrementalParser(DeviceInstance, Err), PTUs(PTUs), VFS(FS),
      CodeGenOpts(HostInstance.getCodeGenOpts()),
      TargetOpts(DeviceInstance.getTargetOpts()) {
  if (Err)
    return;
  StringRef Arch = TargetOpts.CPU;
  if (!Arch.starts_with("sm_") || Arch.substr(3).getAsInteger(10, SMVersion)) {
    Err = toolchain::joinErrors(std::move(Err), toolchain::make_error<toolchain::StringError>(
                                               "Invalid CUDA architecture",
                                               toolchain::inconvertibleErrorCode()));
    return;
  }
}

toolchain::Expected<toolchain::StringRef> IncrementalCUDADeviceParser::GeneratePTX() {
  auto &PTU = PTUs.back();
  std::string Error;

  const toolchain::Target *Target = toolchain::TargetRegistry::lookupTarget(
      PTU.TheModule->getTargetTriple(), Error);
  if (!Target)
    return toolchain::make_error<toolchain::StringError>(std::move(Error),
                                               std::error_code());
  toolchain::TargetOptions TO = toolchain::TargetOptions();
  toolchain::TargetMachine *TargetMachine = Target->createTargetMachine(
      PTU.TheModule->getTargetTriple(), TargetOpts.CPU, "", TO,
      toolchain::Reloc::Model::PIC_);
  PTU.TheModule->setDataLayout(TargetMachine->createDataLayout());

  PTXCode.clear();
  toolchain::raw_svector_ostream dest(PTXCode);

  toolchain::legacy::PassManager PM;
  if (TargetMachine->addPassesToEmitFile(PM, dest, nullptr,
                                         toolchain::CodeGenFileType::AssemblyFile)) {
    return toolchain::make_error<toolchain::StringError>(
        "NVPTX backend cannot produce PTX code.",
        toolchain::inconvertibleErrorCode());
  }

  if (!PM.run(*PTU.TheModule))
    return toolchain::make_error<toolchain::StringError>("Failed to emit PTX code.",
                                               toolchain::inconvertibleErrorCode());

  PTXCode += '\0';
  while (PTXCode.size() % 8)
    PTXCode += '\0';
  return PTXCode.str();
}

toolchain::Error IncrementalCUDADeviceParser::GenerateFatbinary() {
  enum FatBinFlags {
    AddressSize64 = 0x01,
    HasDebugInfo = 0x02,
    ProducerCuda = 0x04,
    HostLinux = 0x10,
    HostMac = 0x20,
    HostWindows = 0x40
  };

  struct FatBinInnerHeader {
    uint16_t Kind;             // 0x00
    uint16_t unknown02;        // 0x02
    uint32_t HeaderSize;       // 0x04
    uint32_t DataSize;         // 0x08
    uint32_t unknown0c;        // 0x0c
    uint32_t CompressedSize;   // 0x10
    uint32_t SubHeaderSize;    // 0x14
    uint16_t VersionMinor;     // 0x18
    uint16_t VersionMajor;     // 0x1a
    uint32_t CudaArch;         // 0x1c
    uint32_t unknown20;        // 0x20
    uint32_t unknown24;        // 0x24
    uint32_t Flags;            // 0x28
    uint32_t unknown2c;        // 0x2c
    uint32_t unknown30;        // 0x30
    uint32_t unknown34;        // 0x34
    uint32_t UncompressedSize; // 0x38
    uint32_t unknown3c;        // 0x3c
    uint32_t unknown40;        // 0x40
    uint32_t unknown44;        // 0x44
    FatBinInnerHeader(uint32_t DataSize, uint32_t CudaArch, uint32_t Flags)
        : Kind(1 /*PTX*/), unknown02(0x0101), HeaderSize(sizeof(*this)),
          DataSize(DataSize), unknown0c(0), CompressedSize(0),
          SubHeaderSize(HeaderSize - 8), VersionMinor(2), VersionMajor(4),
          CudaArch(CudaArch), unknown20(0), unknown24(0), Flags(Flags),
          unknown2c(0), unknown30(0), unknown34(0), UncompressedSize(0),
          unknown3c(0), unknown40(0), unknown44(0) {}
  };

  struct FatBinHeader {
    uint32_t Magic;      // 0x00
    uint16_t Version;    // 0x04
    uint16_t HeaderSize; // 0x06
    uint32_t DataSize;   // 0x08
    uint32_t unknown0c;  // 0x0c
  public:
    FatBinHeader(uint32_t DataSize)
        : Magic(0xba55ed50), Version(1), HeaderSize(sizeof(*this)),
          DataSize(DataSize), unknown0c(0) {}
  };

  FatBinHeader OuterHeader(sizeof(FatBinInnerHeader) + PTXCode.size());
  FatbinContent.append((char *)&OuterHeader,
                       ((char *)&OuterHeader) + OuterHeader.HeaderSize);

  FatBinInnerHeader InnerHeader(PTXCode.size(), SMVersion,
                                FatBinFlags::AddressSize64 |
                                    FatBinFlags::HostLinux);
  FatbinContent.append((char *)&InnerHeader,
                       ((char *)&InnerHeader) + InnerHeader.HeaderSize);

  FatbinContent.append(PTXCode.begin(), PTXCode.end());

  const PartialTranslationUnit &PTU = PTUs.back();

  std::string FatbinFileName = "/" + PTU.TheModule->getName().str() + ".fatbin";

  VFS->addFile(FatbinFileName, 0,
               toolchain::MemoryBuffer::getMemBuffer(
                   toolchain::StringRef(FatbinContent.data(), FatbinContent.size()),
                   "", false));

  CodeGenOpts.CudaGpuBinaryFileName = std::move(FatbinFileName);

  FatbinContent.clear();

  return toolchain::Error::success();
}

IncrementalCUDADeviceParser::~IncrementalCUDADeviceParser() {}

} // namespace language::Core
