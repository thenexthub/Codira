//===-- language/Compability/Runtime/allocator-registry-consts.h -------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_RUNTIME_ALLOCATOR_REGISTRY_CONSTS_H_
#define LANGUAGE_COMPABILITY_RUNTIME_ALLOCATOR_REGISTRY_CONSTS_H_

static constexpr unsigned kDefaultAllocator = 0;

// Allocator used for CUF
static constexpr unsigned kPinnedAllocatorPos = 1;
static constexpr unsigned kDeviceAllocatorPos = 2;
static constexpr unsigned kManagedAllocatorPos = 3;
static constexpr unsigned kUnifiedAllocatorPos = 4;

#endif /* FORTRAN_RUNTIME_ALLOCATOR_REGISTRY_CONSTS_H_ */
