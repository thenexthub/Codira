/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */
#include "base_test.impl.hpp"

#include "stdexcept"
#include "taihe/runtime.hpp"
// Please delete this include when you implement
using namespace taihe;

namespace {

int8_t AddI8(int8_t a, int8_t b)
{
    return a + b;
}

int16_t SubI16(int16_t a, int16_t b)
{
    return a - b;
}

int32_t MulI32(int32_t a, int32_t b)
{
    return a * b;
}

int64_t DivI64(int64_t a, int64_t b)
{
    if (b == 0) {
        taihe::set_error("some error happen");
        return -1;
    }
    return a / b;
}

float AddF32(float a, float b)
{
    return a + b;
}

float SubF32(float a, float b)
{
    return a - b;
}

double AddF64(float a, double b)
{
    return a + b;
}

double SubF64(float a, double b)
{
    return a - b;
}

double MulF64(float a, float b)
{
    return a * b;
}

bool Check(bool a, bool b)
{
    if (a && b) {
        return true;
    }
    return false;
}

string Concatx(string_view a, string_view b)
{
    return a + b;
}

string Splitx(string_view a, int32_t n)
{
    int32_t size = a.size();
    if (n >= size) {
        n = size;
    } else if (n < 0) {
        n = 0;
    }
    return a.substr(0, n);
}

int32_t ToI32(string_view a)
{
    return std::atoi(a.c_str());
}

string FromI32(int32_t a)
{
    return taihe::to_string(a);
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_AddI8(AddI8);
TH_EXPORT_CPP_API_SubI16(SubI16);
TH_EXPORT_CPP_API_MulI32(MulI32);
TH_EXPORT_CPP_API_DivI64(DivI64);
TH_EXPORT_CPP_API_AddF32(AddF32);
TH_EXPORT_CPP_API_SubF32(SubF32);
TH_EXPORT_CPP_API_AddF64(AddF64);
TH_EXPORT_CPP_API_SubF64(SubF64);
TH_EXPORT_CPP_API_MulF64(MulF64);
TH_EXPORT_CPP_API_Check(Check);
TH_EXPORT_CPP_API_Concatx(Concatx);
TH_EXPORT_CPP_API_Splitx(Splitx);
TH_EXPORT_CPP_API_ToI32(ToI32);
TH_EXPORT_CPP_API_FromI32(FromI32);
// NOLINTEND