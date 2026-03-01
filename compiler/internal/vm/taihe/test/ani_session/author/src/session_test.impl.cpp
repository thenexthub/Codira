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
#include "session_test.impl.hpp"

#include <cstdint>
#include <iostream>

#include "session_test.IfaceD.proj.2.hpp"
#include "session_test.IfaceE.proj.2.hpp"
#include "session_test.session_type.proj.1.hpp"
#include "stdexcept"
#include "taihe/callback.hpp"
#include "taihe/runtime.hpp"
#include "taihe/string.hpp"

using namespace taihe;

namespace {
class IfaceA {
public:
    void func_a()
    {
        throw std::runtime_error("Function IfaceA::func_a Not implemented");
    }
};

class IfaceB {
public:
    void func_b()
    {
        throw std::runtime_error("Function IfaceB::func_b Not implemented");
    }

    void func_a()
    {
        throw std::runtime_error("Function IfaceB::func_a Not implemented");
    }
};

class IfaceC {
public:
    void func_c()
    {
        throw std::runtime_error("Function IfaceC::func_c Not implemented");
    }

    void func_a()
    {
        throw std::runtime_error("Function IfaceC::func_a Not implemented");
    }
};

class IfaceD {
    string name_ {"IfaceD"};

public:
    string func_d()
    {
        return "d";
    }

    string func_b()
    {
        return "b";
    }

    string func_a()
    {
        return "a";
    }

    string func_c()
    {
        return "c";
    }

    string getName()
    {
        return name_;
    }

    void setName(string_view name)
    {
        name_ = name;
    }

    void onSet(callback_view<void()> a)
    {
        a();
        std::cout << "IfaceD::onSet" << std::endl;
    }

    void offSet(callback_view<void()> a)
    {
        a();
        std::cout << "IfaceD::offSet" << std::endl;
    }
};

::session_test::IfaceD getIfaceD()
{
    return make_holder<IfaceD, ::session_test::IfaceD>();
}

class IfaceE {
public:
    string func_e()
    {
        return "ee";
    }

    string func_b()
    {
        return "bb";
    }

    string func_a()
    {
        return "aa";
    }

    string func_c()
    {
        return "cc";
    }
};

::session_test::IfaceE getIfaceE()
{
    return make_holder<IfaceE, ::session_test::IfaceE>();
}

class Session {
public:
    void beginConfig()
    {
        throw std::runtime_error("Function Session::beginConfig Not implemented");
    }
};

class PhotoSession {
public:
    bool canPreconfig()
    {
        return true;
    }

    void beginConfig()
    {
        std::cout << "PhotoSession" << std::endl;
    }

    string func_a()
    {
        std::cout << "func_a in PhotoSession" << std::endl;
        return "psa";
    }

    string func_c()
    {
        std::cout << "func_c in PhotoSession" << std::endl;
        return "psc";
    }
};

class VideoSession {
public:
    bool canPreconfig()
    {
        return true;
    }

    void beginConfig()
    {
        std::cout << "VideoSession" << std::endl;
    }

    string func_a()
    {
        std::cout << "func_a in VideoSession" << std::endl;
        return "vsa";
    }

    string func_c()
    {
        std::cout << "func_c in VideoSession" << std::endl;
        return "vsc";
    }
};

::session_test::session_type getSession(int32_t ty)
{
    if (ty == 1) {
        return ::session_test::session_type::make_ps(make_holder<PhotoSession, ::session_test::PhotoSession>());
    } else {
        return ::session_test::session_type::make_vs(make_holder<VideoSession, ::session_test::VideoSession>());
    }
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_getIfaceD(getIfaceD);
TH_EXPORT_CPP_API_getIfaceE(getIfaceE);
TH_EXPORT_CPP_API_getSession(getSession);
// NOLINTEND