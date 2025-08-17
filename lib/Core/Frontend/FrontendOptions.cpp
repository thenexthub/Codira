//===- FrontendOptions.cpp ------------------------------------------------===//
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

#include "language/Core/Frontend/FrontendOptions.h"
#include "language/Core/Basic/LangStandard.h"
#include "toolchain/ADT/StringSwitch.h"

using namespace language::Core;

InputKind FrontendOptions::getInputKindForExtension(StringRef Extension) {
  return toolchain::StringSwitch<InputKind>(Extension)
      .Cases("ast", "pcm", InputKind(Language::Unknown, InputKind::Precompiled))
      .Case("c", Language::C)
      .Cases("S", "s", Language::Asm)
      .Case("i", InputKind(Language::C).getPreprocessed())
      .Case("ii", InputKind(Language::CXX).getPreprocessed())
      .Case("cui", InputKind(Language::CUDA).getPreprocessed())
      .Case("m", Language::ObjC)
      .Case("mi", InputKind(Language::ObjC).getPreprocessed())
      .Cases("mm", "M", Language::ObjCXX)
      .Case("mii", InputKind(Language::ObjCXX).getPreprocessed())
      .Cases("C", "cc", "cp", Language::CXX)
      .Cases("cpp", "CPP", "c++", "cxx", "hpp", "hxx", Language::CXX)
      .Case("cppm", Language::CXX)
      .Cases("iim", "iih", InputKind(Language::CXX).getPreprocessed())
      .Case("cl", Language::OpenCL)
      .Case("clcpp", Language::OpenCLCXX)
      .Cases("cu", "cuh", Language::CUDA)
      .Case("hip", Language::HIP)
      .Cases("ll", "bc", Language::LLVM_IR)
      .Case("hlsl", Language::HLSL)
      .Case("cir", Language::CIR)
      .Default(Language::Unknown);
}
