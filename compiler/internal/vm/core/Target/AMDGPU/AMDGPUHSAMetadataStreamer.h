//===--- AMDGPUHSAMetadataStreamer.h ----------------------------*- C++ -*-===//
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
/// \file
/// AMDGPU HSA Metadata Streamer.
///
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_AMDGPUHSAMETADATASTREAMER_H
#define LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_AMDGPUHSAMETADATASTREAMER_H

#include "Utils/AMDGPUDelayedMCExpr.h"
#include "vm/core/BinaryFormat/MsgPackDocument.h"
#include "vm/core/Support/AMDGPUMetadata.h"
#include "vm/core/Support/Alignment.h"
#include "vm/core/Support/Compiler.h"

namespace vm::core {

class AMDGPUTargetMachine;
class AMDGPUTargetStreamer;
class Argument;
class DataLayout;
class Function;
class MachineFunction;
class MDNode;
class Module;
struct SIProgramInfo;
class Type;

namespace AMDGPU {

namespace IsaInfo {
class AMDGPUTargetID;
}

namespace HSAMD {

class MetadataStreamer {
public:
  virtual ~MetadataStreamer() = default;

  virtual bool emitTo(AMDGPUTargetStreamer &TargetStreamer) = 0;

  virtual void begin(const Module &Mod,
                     const IsaInfo::AMDGPUTargetID &TargetID) = 0;

  virtual void end() = 0;

  virtual void emitKernel(const MachineFunction &MF,
                          const SIProgramInfo &ProgramInfo) = 0;

protected:
  virtual void emitVersion() = 0;
  virtual void emitHiddenKernelArgs(const MachineFunction &MF, unsigned &Offset,
                                    msgpack::ArrayDocNode Args) = 0;
  virtual void emitKernelAttrs(const AMDGPUTargetMachine &TM,
                               const MachineFunction &MF,
                               msgpack::MapDocNode Kern) = 0;
};

class LLVM_EXTERNAL_VISIBILITY MetadataStreamerMsgPackV4
    : public MetadataStreamer {
protected:
  std::unique_ptr<DelayedMCExprs> DelayedExprs =
      std::make_unique<DelayedMCExprs>();

  std::unique_ptr<msgpack::Document> HSAMetadataDoc =
      std::make_unique<msgpack::Document>();

  void dump(StringRef HSAMetadataString) const;

  void verify(StringRef HSAMetadataString) const;

  std::optional<StringRef> getAccessQualifier(StringRef AccQual) const;

  std::optional<StringRef>
  getAddressSpaceQualifier(unsigned AddressSpace) const;

  StringRef getValueKind(Type *Ty, StringRef TypeQual,
                         StringRef BaseTypeName) const;

  std::string getTypeName(Type *Ty, bool Signed) const;

  msgpack::ArrayDocNode getWorkGroupDimensions(MDNode *Node) const;

  msgpack::MapDocNode getHSAKernelProps(const MachineFunction &MF,
                                        const SIProgramInfo &ProgramInfo,
                                        unsigned CodeObjectVersion) const;

  void emitVersion() override;

  void emitTargetID(const IsaInfo::AMDGPUTargetID &TargetID);

  void emitPrintf(const Module &Mod);

  void emitKernelLanguage(const Function &Func, msgpack::MapDocNode Kern);

  void emitKernelAttrs(const AMDGPUTargetMachine &TM, const MachineFunction &MF,
                       msgpack::MapDocNode Kern) override;

  void emitKernelArgs(const MachineFunction &MF, msgpack::MapDocNode Kern);

  void emitKernelArg(const Argument &Arg, unsigned &Offset,
                     msgpack::ArrayDocNode Args);

  void emitKernelArg(const DataLayout &DL, Type *Ty, Align Alignment,
                     StringRef ValueKind, unsigned &Offset,
                     msgpack::ArrayDocNode Args,
                     MaybeAlign PointeeAlign = std::nullopt,
                     StringRef Name = "", StringRef TypeName = "",
                     StringRef BaseTypeName = "", StringRef ActAccQual = "",
                     StringRef AccQual = "", StringRef TypeQual = "");

  void emitHiddenKernelArgs(const MachineFunction &MF, unsigned &Offset,
                            msgpack::ArrayDocNode Args) override;

  msgpack::DocNode &getRootMetadata(StringRef Key) {
    return HSAMetadataDoc->getRoot().getMap(/*Convert=*/true)[Key];
  }

  msgpack::DocNode &getHSAMetadataRoot() {
    return HSAMetadataDoc->getRoot();
  }

public:
  MetadataStreamerMsgPackV4() = default;
  ~MetadataStreamerMsgPackV4() override = default;

  bool emitTo(AMDGPUTargetStreamer &TargetStreamer) override;

  void begin(const Module &Mod,
             const IsaInfo::AMDGPUTargetID &TargetID) override;

  void end() override;

  void emitKernel(const MachineFunction &MF,
                  const SIProgramInfo &ProgramInfo) override;
};

class MetadataStreamerMsgPackV5 : public MetadataStreamerMsgPackV4 {
protected:
  void emitVersion() override;
  void emitHiddenKernelArgs(const MachineFunction &MF, unsigned &Offset,
                            msgpack::ArrayDocNode Args) override;
  void emitKernelAttrs(const AMDGPUTargetMachine &TM, const MachineFunction &MF,
                       msgpack::MapDocNode Kern) override;

public:
  MetadataStreamerMsgPackV5() = default;
  ~MetadataStreamerMsgPackV5() override = default;
};

class MetadataStreamerMsgPackV6 final : public MetadataStreamerMsgPackV5 {
protected:
  void emitVersion() override;

public:
  MetadataStreamerMsgPackV6() = default;
  ~MetadataStreamerMsgPackV6() override = default;

  void emitKernelAttrs(const AMDGPUTargetMachine &TM, const MachineFunction &MF,
                       msgpack::MapDocNode Kern) override;
};

} // end namespace HSAMD
} // end namespace AMDGPU
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_AMDGPUHSAMETADATASTREAMER_H
