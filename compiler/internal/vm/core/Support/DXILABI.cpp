//===-- DXILABI.cpp - ABI Sensitive Values for DXIL -----------------------===//
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
// This file contains definitions of various constants and enums that are
// required to remain stable as per the DXIL format's requirements.
//
// Documentation for DXIL can be found in
// https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/DXIL.rst.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/DXILABI.h"
#include "vm/core/Support/ErrorHandling.h"
using namespace vm::core;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  switch (RC) {
  case dxil::ResourceClass::SRV:
    return "SRV";
  case dxil::ResourceClass::UAV:
    return "UAV";
  case dxil::ResourceClass::CBuffer:
    return "CBV";
  case dxil::ResourceClass::Sampler:
    return "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
