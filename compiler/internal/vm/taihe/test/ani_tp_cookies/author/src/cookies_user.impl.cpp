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
#include "cookies_user.impl.hpp"
#include <iostream>
#include "cookies_user.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

namespace {
class CallbackAImpl {
public:
    CallbackAImpl() {}

    void operator()(bool arg)
    {
        std::cout << "CallbackA" << std::endl;
    }
};

void RunNativeBusiness(::cookies::weak::AntUserCookiesProvider cookieprovider)
{
    ::cookies::AntUserCookie cookie1 {"example1.com", "2099-01-01T23:59:59Z", "/", true, "sessionid=abc123"};
    ::cookies::AntUserCookie cookie2 {"example2.com", "2099-01-01T23:59:59Z", "/", true, "sessionid=cba321"};
    ::taihe::array cookies {cookie1, cookie2};
    ::taihe::callback<void(bool)> cb = ::taihe::make_holder<CallbackAImpl, ::taihe::callback<void(bool)>>();
    cookieprovider->setCookiesAsync(cookies, cb);
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_RunNativeBusiness(RunNativeBusiness);
// NOLINTEND