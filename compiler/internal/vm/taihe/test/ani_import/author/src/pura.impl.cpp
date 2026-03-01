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
#include "pura.impl.hpp"
#include "pura.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace pura;

namespace {
// To be implemented.

void testBar(::mate::bar::BarType const &bar)
{
    TH_THROW(std::runtime_error, "testBar not implemented");
}

void testNova(::nova::weak::NovaType nove)
{
    TH_THROW(std::runtime_error, "testNova not implemented");
}

void testMyStruct(::test::inner::MyStruct const &s)
{
    TH_THROW(std::runtime_error, "testMyStruct not implemented");
}

void testPuraClass(::pura::PuraClass const &c)
{
    TH_THROW(std::runtime_error, "testPuraClass not implemented");
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_testBar(testBar);
TH_EXPORT_CPP_API_testNova(testNova);
TH_EXPORT_CPP_API_testMyStruct(testMyStruct);
TH_EXPORT_CPP_API_testPuraClass(testPuraClass);
// NOLINTEND