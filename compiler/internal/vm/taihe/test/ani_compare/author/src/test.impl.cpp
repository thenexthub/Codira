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
#include <vector>

#include "taihe/array.hpp"
#include "taihe/runtime.hpp"
#include "taihe/string.hpp"

#include "test.impl.hpp"
#include "test.proj.hpp"

namespace {
// To be implemented.

class CallbackManagerImpl {
    std::vector<::taihe::callback<taihe::string()>> callbacks_;

public:
    CallbackManagerImpl() {}

    bool addCallback(::taihe::callback_view<taihe::string()> new_cb)
    {
        for (auto const &old_cb : callbacks_) {
            if (old_cb == new_cb) {
                std::cerr << "Callback already exists." << std::endl;
                return false;
            }
        }
        callbacks_.emplace_back(new_cb);
        return true;
    }

    bool removeCallback(::taihe::callback_view<taihe::string()> cb)
    {
        for (auto it = callbacks_.begin(); it != callbacks_.end(); ++it) {
            if (*it == cb) {
                callbacks_.erase(it);
                return true;
            }
        }
        std::cerr << "Callback not found." << std::endl;
        return false;
    }

    taihe::array<taihe::string> invokeCallbacks()
    {
        std::vector<taihe::string> results;
        for (auto const &cb : callbacks_) {
            results.push_back(cb());
        }
        return taihe::array<taihe::string>(taihe::copy_data, results.data(), results.size());
    }
};

::test::CallbackManager getCallbackManager()
{
    return taihe::make_holder<CallbackManagerImpl, ::test::CallbackManager>();
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_getCallbackManager(getCallbackManager);
// NOLINTEND