//===-- RemoteJITUtils.h - Utilities for remote-JITing ----------*- C++ -*-===//
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
// Utilities for ExecutorProcessControl-based remote JITing with Orc and
// JITLink.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_INTERPRETER_REMOTEJITUTILS_H
#define LANGUAGE_CORE_INTERPRETER_REMOTEJITUTILS_H

#include "toolchain/ADT/StringRef.h"
#include "toolchain/ExecutionEngine/Orc/Core.h"
#include "toolchain/ExecutionEngine/Orc/Layer.h"
#include "toolchain/ExecutionEngine/Orc/SimpleRemoteEPC.h"
#include "toolchain/Support/Error.h"

#include <cstdint>
#include <memory>
#include <string>

toolchain::Expected<std::unique_ptr<toolchain::orc::SimpleRemoteEPC>>
launchExecutor(toolchain::StringRef ExecutablePath, bool UseSharedMemory,
               toolchain::StringRef SlabAllocateSizeString);

/// Create a JITLinkExecutor that connects to the given network address
/// through a TCP socket. A valid NetworkAddress provides hostname and port,
/// e.g. localhost:20000.
toolchain::Expected<std::unique_ptr<toolchain::orc::SimpleRemoteEPC>>
connectTCPSocket(toolchain::StringRef NetworkAddress, bool UseSharedMemory,
                 toolchain::StringRef SlabAllocateSizeString);

#endif // LANGUAGE_CORE_INTERPRETER_REMOTEJITUTILS_H
