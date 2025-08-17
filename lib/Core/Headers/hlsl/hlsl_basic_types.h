//===----- hlsl_basic_types.h - HLSL definitions for basic types ----------===//
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

#ifndef _HLSL_HLSL_BASIC_TYPES_H_
#define _HLSL_HLSL_BASIC_TYPES_H_

namespace hlsl {
// built-in scalar data types:

/// \typedef template<typename Ty, int Size> using vector = Ty
/// __attribute__((ext_vector_type(Size)))
///
/// \tparam Ty The base type of the vector may be any builtin integral or
/// floating point type.
/// \tparam Size The size of the vector may be any value between 1 and 4.

#ifdef __HLSL_ENABLE_16_BIT
// 16-bit integer.
typedef unsigned short uint16_t;
typedef short int16_t;

// 16-bit floating point.
typedef half float16_t;
#endif

// 32-bit integer.
typedef int int32_t;

// unsigned 32-bit integer.
typedef unsigned int uint;
typedef unsigned int uint32_t;

// 32-bit floating point.
typedef float float32_t;

// 64-bit integer.
typedef unsigned long uint64_t;
typedef long int64_t;

// 64-bit floating point
typedef double float64_t;

// built-in vector data types:

#ifdef __HLSL_ENABLE_16_BIT
typedef vector<int16_t, 1> int16_t1;
typedef vector<int16_t, 2> int16_t2;
typedef vector<int16_t, 3> int16_t3;
typedef vector<int16_t, 4> int16_t4;
typedef vector<uint16_t, 1> uint16_t1;
typedef vector<uint16_t, 2> uint16_t2;
typedef vector<uint16_t, 3> uint16_t3;
typedef vector<uint16_t, 4> uint16_t4;
#endif
typedef vector<bool, 1> bool1;
typedef vector<bool, 2> bool2;
typedef vector<bool, 3> bool3;
typedef vector<bool, 4> bool4;
typedef vector<int, 1> int1;
typedef vector<int, 2> int2;
typedef vector<int, 3> int3;
typedef vector<int, 4> int4;
typedef vector<uint, 1> uint1;
typedef vector<uint, 2> uint2;
typedef vector<uint, 3> uint3;
typedef vector<uint, 4> uint4;
typedef vector<int32_t, 1> int32_t1;
typedef vector<int32_t, 2> int32_t2;
typedef vector<int32_t, 3> int32_t3;
typedef vector<int32_t, 4> int32_t4;
typedef vector<uint32_t, 1> uint32_t1;
typedef vector<uint32_t, 2> uint32_t2;
typedef vector<uint32_t, 3> uint32_t3;
typedef vector<uint32_t, 4> uint32_t4;
typedef vector<int64_t, 1> int64_t1;
typedef vector<int64_t, 2> int64_t2;
typedef vector<int64_t, 3> int64_t3;
typedef vector<int64_t, 4> int64_t4;
typedef vector<uint64_t, 1> uint64_t1;
typedef vector<uint64_t, 2> uint64_t2;
typedef vector<uint64_t, 3> uint64_t3;
typedef vector<uint64_t, 4> uint64_t4;

typedef vector<half, 1> half1;
typedef vector<half, 2> half2;
typedef vector<half, 3> half3;
typedef vector<half, 4> half4;
typedef vector<float, 1> float1;
typedef vector<float, 2> float2;
typedef vector<float, 3> float3;
typedef vector<float, 4> float4;
typedef vector<double, 1> double1;
typedef vector<double, 2> double2;
typedef vector<double, 3> double3;
typedef vector<double, 4> double4;

#ifdef __HLSL_ENABLE_16_BIT
typedef vector<float16_t, 1> float16_t1;
typedef vector<float16_t, 2> float16_t2;
typedef vector<float16_t, 3> float16_t3;
typedef vector<float16_t, 4> float16_t4;
#endif

typedef vector<float32_t, 1> float32_t1;
typedef vector<float32_t, 2> float32_t2;
typedef vector<float32_t, 3> float32_t3;
typedef vector<float32_t, 4> float32_t4;
typedef vector<float64_t, 1> float64_t1;
typedef vector<float64_t, 2> float64_t2;
typedef vector<float64_t, 3> float64_t3;
typedef vector<float64_t, 4> float64_t4;

} // namespace hlsl

#endif //_HLSL_HLSL_BASIC_TYPES_H_
