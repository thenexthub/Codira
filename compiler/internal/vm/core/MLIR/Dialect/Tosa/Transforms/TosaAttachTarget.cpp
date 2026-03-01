//===- TosaAttachTarget.cpp
//------------------------------------------------===//
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
// Attach target information to a TOSA module.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Tosa/IR/TargetEnv.h"
#include "mlir/Dialect/Tosa/Transforms/Passes.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
namespace tosa {

#define GEN_PASS_DEF_TOSAATTACHTARGET
#include "mlir/Dialect/Tosa/Transforms/Passes.h.inc"

namespace {

class TosaAttachTarget
    : public tosa::impl::TosaAttachTargetBase<TosaAttachTarget> {
  using Base::Base;

public:
  void runOnOperation() override {
    toolchain::SmallVector<Profile, 2> selectedProfiles;
    if (!profiles.empty()) {
      for (const std::string &prof : profiles) {
        std::optional<Profile> profSymbol = symbolizeProfile(prof);
        if (!profSymbol) {
          toolchain::SmallVector<Profile> allProfiles = ProfileAttr::getAllValues();
          toolchain::errs() << buildUnkownParameterErrorMessage(allProfiles,
                                                           "profile", prof);
          return signalPassFailure();
        }
        selectedProfiles.push_back(profSymbol.value());
      }
    }

    toolchain::SmallVector<Extension, 10> selectedExtensions;
    if (!extensions.empty()) {
      for (const std::string &ext : extensions) {
        std::optional<Extension> extSymbol = symbolizeExtension(ext);
        if (!extSymbol) {
          toolchain::SmallVector<Extension> allExtensions =
              ExtensionAttr::getAllValues();
          toolchain::errs() << buildUnkownParameterErrorMessage(allExtensions,
                                                           "extension", ext);
          return signalPassFailure();
        }
        selectedExtensions.push_back(extSymbol.value());
      }
    }

    ModuleOp mod = getOperation();
    MLIRContext *ctx = &getContext();
    const auto targetEnvAttr = TargetEnvAttr::get(
        ctx, specificationVersion, level, selectedProfiles, selectedExtensions);
    mod->setAttr(TargetEnvAttr::name, targetEnvAttr);
  }

private:
  template <typename T>
  std::string buildUnkownParameterErrorMessage(toolchain::SmallVector<T> &enumValues,
                                               std::string enumName,
                                               std::string unknownArgument) {
    std::string message;
    toolchain::raw_string_ostream os(message);
    os << "Unknown TOSA " << enumName << " name passed in '" << unknownArgument
       << "', supported " << enumName << "s are: ";
    toolchain::interleaveComma(enumValues, os);
    os << "\n";
    return message;
  }
};

} // namespace

} // namespace tosa
} // namespace mlir
