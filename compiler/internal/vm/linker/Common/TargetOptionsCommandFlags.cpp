//===-- TargetOptionsCommandFlags.cpp ---------------------------*- C++ -*-===//
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

#include "lld/Common/TargetOptionsCommandFlags.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Triple.h"
#include <optional>

llvm::TargetOptions lld::initTargetOptionsFromCodeGenFlags() {
  return llvm::codegen::InitTargetOptionsFromCodeGenFlags(llvm::Triple());
}

std::optional<llvm::Reloc::Model> lld::getRelocModelFromCMModel() {
  return llvm::codegen::getExplicitRelocModel();
}

std::optional<llvm::CodeModel::Model> lld::getCodeModelFromCMModel() {
  return llvm::codegen::getExplicitCodeModel();
}

std::string lld::getCPUStr() { return llvm::codegen::getCPUStr(); }

std::vector<std::string> lld::getMAttrs() { return llvm::codegen::getMAttrs(); }
