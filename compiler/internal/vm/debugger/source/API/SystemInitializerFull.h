//===-- SystemInitializerFull.h ---------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_API_SYSTEMINITIALIZERFULL_H
#define LLDB_SOURCE_API_SYSTEMINITIALIZERFULL_H

#include "lldb/Initialization/SystemInitializerCommon.h"

namespace lldb_private {
/// Initializes lldb.
///
/// This class is responsible for initializing all of lldb system
/// services needed to use the full LLDB application.  This class is
/// not intended to be used externally, but is instead used
/// internally by SBDebugger to initialize the system.
class SystemInitializerFull : public SystemInitializerCommon {
public:
  SystemInitializerFull();
  ~SystemInitializerFull() override;

  llvm::Error Initialize() override;
  void Terminate() override;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_API_SYSTEMINITIALIZERFULL_H
