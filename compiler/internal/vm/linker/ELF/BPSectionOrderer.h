//===- BPSectionOrderer.h -------------------------------------------------===//
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
///
/// This file uses Balanced Partitioning to order sections to improve startup
/// time and compressed size.
///
//===----------------------------------------------------------------------===//

#ifndef LLD_ELF_BPSECTION_ORDERER_H
#define LLD_ELF_BPSECTION_ORDERER_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"

namespace lld::elf {
struct Ctx;
class InputSectionBase;

/// Run Balanced Partitioning to find the optimal function and data order to
/// improve startup time and compressed size.
///
/// It is important that -ffunction-sections and -fdata-sections compiler flags
/// are used to ensure functions and data are in their own sections and thus
/// can be reordered.
llvm::DenseMap<const InputSectionBase *, int>
runBalancedPartitioning(Ctx &ctx, llvm::StringRef profilePath,
                        bool forFunctionCompression, bool forDataCompression,
                        bool compressionSortStartupFunctions, bool verbose);

} // namespace lld::elf

#endif
