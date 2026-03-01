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
#include "finalization_test.impl.hpp"
#include "finalization_test.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

#include <iostream>
#include <taihe/vector.hpp>

using namespace taihe;
using namespace finalization_test;

namespace {
// To be implemented.

class FooImpl {
    vector<callback<void()>> callbacks;

public:
    FooImpl()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    void addCallback(callback_view<void()> callback)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        callbacks.emplace_back(callback);
    }

    ~FooImpl()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        for (callback_view<void()> callback : callbacks) {
            callback();
        }
    }
};

Foo makeFoo()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<FooImpl, Foo>();
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_makeFoo(makeFoo);
// NOLINTEND