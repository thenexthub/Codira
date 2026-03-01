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
#include "bigint_test.impl.hpp"
#include <iostream>
#include "stdexcept"
#include "taihe/array.hpp"

using namespace taihe;

namespace {

bool BigInt01(double a, array_view<int64_t> b)
{
    return true;
}

double BigInt02(double a)
{
    return a;
}

array<int64_t> BigInt03(array_view<int64_t> a)
{
    return a;
}

void BigInt04(array_view<int64_t> a)
{
    for (int i = 0; i < a.size(); i++) {
        std::cout << a[i] << std::endl;
    }
}

array<int64_t> BigInt05(double a, array_view<int64_t> b)
{
    return b;
}

double BigInt06(double a, array_view<int64_t> b)
{
    return a;
}
}  // namespace

TH_EXPORT_CPP_API_BigInt01(BigInt01);
TH_EXPORT_CPP_API_BigInt02(BigInt02);
TH_EXPORT_CPP_API_BigInt03(BigInt03);
TH_EXPORT_CPP_API_BigInt04(BigInt04);
TH_EXPORT_CPP_API_BigInt05(BigInt05);
TH_EXPORT_CPP_API_BigInt06(BigInt06);
// NOLINTEND