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
#include "mate.foo.impl.hpp"
#include "mate.foo.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace mate::foo;

namespace {
// To be implemented.

void testMate(::mate::MateType const &mate)
{
    TH_THROW(std::runtime_error, "testMate not implemented");
}

void testNova(::nova::weak::NovaType nova)
{
    TH_THROW(std::runtime_error, "testNova not implemented");
}

void testPura(::pura::PuraType pura)
{
    TH_THROW(std::runtime_error, "testPura not implemented");
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_testMate(testMate);
TH_EXPORT_CPP_API_testNova(testNova);
TH_EXPORT_CPP_API_testPura(testPura);
// NOLINTEND