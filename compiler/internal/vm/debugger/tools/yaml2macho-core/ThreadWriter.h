//===----------------------------------------------------------------------===//
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
/// \file
/// Functions to emit LC_THREAD bytes to the corefile's Mach-O load commands,
/// specifying the threads, the register sets ("flavors") within those threads,
/// and all of the registers within those register sets.
//===----------------------------------------------------------------------===//

#ifndef YAML2MACHOCOREFILE_THREADWRITER_H
#define YAML2MACHOCOREFILE_THREADWRITER_H

#include "CoreSpec.h"

#include <vector>

void add_lc_threads(CoreSpec &spec,
                    std::vector<std::vector<uint8_t>> &load_commands);

#endif
