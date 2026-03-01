//===- Objcopy.cpp --------------------------------------------------------===//
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

#include "vm/core/ObjCopy/ObjCopy.h"
#include "vm/core/ObjCopy/COFF/COFFConfig.h"
#include "vm/core/ObjCopy/COFF/COFFObjcopy.h"
#include "vm/core/ObjCopy/DXContainer/DXContainerConfig.h"
#include "vm/core/ObjCopy/DXContainer/DXContainerObjcopy.h"
#include "vm/core/ObjCopy/ELF/ELFConfig.h"
#include "vm/core/ObjCopy/ELF/ELFObjcopy.h"
#include "vm/core/ObjCopy/MachO/MachOConfig.h"
#include "vm/core/ObjCopy/MachO/MachOObjcopy.h"
#include "vm/core/ObjCopy/MultiFormatConfig.h"
#include "vm/core/ObjCopy/XCOFF/XCOFFConfig.h"
#include "vm/core/ObjCopy/XCOFF/XCOFFObjcopy.h"
#include "vm/core/ObjCopy/wasm/WasmConfig.h"
#include "vm/core/ObjCopy/wasm/WasmObjcopy.h"
#include "vm/core/Object/COFF.h"
#include "vm/core/Object/DXContainer.h"
#include "vm/core/Object/ELFObjectFile.h"
#include "vm/core/Object/Error.h"
#include "vm/core/Object/MachO.h"
#include "vm/core/Object/MachOUniversal.h"
#include "vm/core/Object/Wasm.h"
#include "vm/core/Object/XCOFFObjectFile.h"

using namespace vm::core;
using namespace vm::core::object;

/// The function executeObjcopyOnBinary does the dispatch based on the format
/// of the input binary (ELF, MachO or COFF).
Error objcopy::executeObjcopyOnBinary(const MultiFormatConfig &Config,
                                      object::Binary &In, raw_ostream &Out) {
  if (auto *ELFBinary = dyn_cast<object::ELFObjectFileBase>(&In)) {
    Expected<const ELFConfig &> ELFConfig = Config.getELFConfig();
    if (!ELFConfig)
      return ELFConfig.takeError();

    return elf::executeObjcopyOnBinary(Config.getCommonConfig(), *ELFConfig,
                                       *ELFBinary, Out);
  }
  if (auto *COFFBinary = dyn_cast<object::COFFObjectFile>(&In)) {
    Expected<const COFFConfig &> COFFConfig = Config.getCOFFConfig();
    if (!COFFConfig)
      return COFFConfig.takeError();

    return coff::executeObjcopyOnBinary(Config.getCommonConfig(), *COFFConfig,
                                        *COFFBinary, Out);
  }
  if (auto *MachOBinary = dyn_cast<object::MachOObjectFile>(&In)) {
    Expected<const MachOConfig &> MachOConfig = Config.getMachOConfig();
    if (!MachOConfig)
      return MachOConfig.takeError();

    return macho::executeObjcopyOnBinary(Config.getCommonConfig(), *MachOConfig,
                                         *MachOBinary, Out);
  }
  if (auto *MachOUniversalBinary =
          dyn_cast<object::MachOUniversalBinary>(&In)) {
    return macho::executeObjcopyOnMachOUniversalBinary(
        Config, *MachOUniversalBinary, Out);
  }
  if (auto *WasmBinary = dyn_cast<object::WasmObjectFile>(&In)) {
    Expected<const WasmConfig &> WasmConfig = Config.getWasmConfig();
    if (!WasmConfig)
      return WasmConfig.takeError();

    return objcopy::wasm::executeObjcopyOnBinary(Config.getCommonConfig(),
                                                 *WasmConfig, *WasmBinary, Out);
  }
  if (auto *XCOFFBinary = dyn_cast<object::XCOFFObjectFile>(&In)) {
    Expected<const XCOFFConfig &> XCOFFConfig = Config.getXCOFFConfig();
    if (!XCOFFConfig)
      return XCOFFConfig.takeError();

    return xcoff::executeObjcopyOnBinary(Config.getCommonConfig(), *XCOFFConfig,
                                         *XCOFFBinary, Out);
  }
  if (auto *DXContainerBinary = dyn_cast<object::DXContainerObjectFile>(&In)) {
    Expected<const DXContainerConfig &> DXContainerConfig =
        Config.getDXContainerConfig();
    if (!DXContainerConfig)
      return DXContainerConfig.takeError();

    return dxbc::executeObjcopyOnBinary(
        Config.getCommonConfig(), *DXContainerConfig, *DXContainerBinary, Out);
  }
  return createStringError(object_error::invalid_file_type,
                           "unsupported object file format");
}
