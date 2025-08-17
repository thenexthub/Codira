//===--- SYCLKernelInfo.h --- Information about SYCL kernels --------------===//
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
/// \file
/// This file declares types used to describe SYCL kernels.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_SYCLKERNELINFO_H
#define LANGUAGE_CORE_AST_SYCLKERNELINFO_H

#include "language/Core/AST/CanonicalType.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/Type.h"

namespace language::Core {

class SYCLKernelInfo {
public:
  SYCLKernelInfo(CanQualType KernelNameType,
                 const FunctionDecl *KernelEntryPointDecl,
                 const std::string &KernelName)
      : KernelNameType(KernelNameType),
        KernelEntryPointDecl(KernelEntryPointDecl), KernelName(KernelName) {}

  CanQualType getKernelNameType() const { return KernelNameType; }

  const FunctionDecl *getKernelEntryPointDecl() const {
    return KernelEntryPointDecl;
  }

  const std::string &GetKernelName() const { return KernelName; }

private:
  CanQualType KernelNameType;
  const FunctionDecl *KernelEntryPointDecl;
  std::string KernelName;
};

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_SYCLKERNELINFO_H
