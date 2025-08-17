//===----- hlsl.h - HLSL definitions --------------------------------------===//
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

#ifndef _HLSL_H_
#define _HLSL_H_

#if defined(__clang__)
// Don't warn about any of the DXC compatibility warnings in the clang-only
// headers since these will never be used with DXC anyways.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Whlsl-dxc-compatability"
#endif

// Basic types, type traits and type-independent templates.
#include "hlsl/hlsl_basic_types.h"
#include "hlsl/hlsl_detail.h"

// HLSL standard library function declarations/definitions.
#include "hlsl/hlsl_alias_intrinsics.h"
#if __HLSL_VERSION <= __HLSL_202x
#include "hlsl/hlsl_compat_overloads.h"
#endif
#include "hlsl/hlsl_intrinsics.h"

#ifdef __spirv__
#include "hlsl/hlsl_spirv.h"
#endif // __spirv__

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif //_HLSL_H_
