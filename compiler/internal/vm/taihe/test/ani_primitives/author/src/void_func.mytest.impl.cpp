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
#include "void_func.mytest.impl.hpp"

#include "stdexcept"
#include "taihe/optional.hpp"
#include "taihe/string.hpp"
#include "void_func.mytest.BarTest.proj.1.hpp"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {
void Myfunc1()
{
    std::cout << "Myfunc1 is true " << std::endl;
}

void Myfunc2(int8_t option1, int16_t option2)
{
    std::cout << "Myfunc2 is option1  " << (int)option1 << std::endl;
    std::cout << "Myfunc2 is option2  " << option2 << std::endl;
}

void Myfunc3(int32_t option1, bool option2)
{
    if (option2) {
        std::cout << "Myfunc3 is option1  " << option1 << std::endl;
        std::cout << "Myfunc3 is option2  " << option2 << std::endl;
    } else {
        std::cout << "Myfunc3 is option1  " << option1 << std::endl;
        std::cout << "Myfunc3 is option2  " << option2 << std::endl;
    }
}

void Myfunc4(int32_t option1, int64_t option2)
{
    std::cout << "Myfunc4 is option1  " << option1 << std::endl;
    std::cout << "Myfunc4 is option2  " << option2 << std::endl;
}

void Myfunc5(int32_t option1, string_view option2)
{
    std::cout << "Myfunc5 is option1  " << option1 << std::endl;
    std::cout << "Myfunc5 is option2  " << option2 << std::endl;
}

void Myfunc6(int64_t option1, bool option2)
{
    if (option2) {
        std::cout << "Myfunc6 is option1  " << option1 << std::endl;
        std::cout << "Myfunc6 is option2  " << option2 << std::endl;
    } else {
        std::cout << "Myfunc6 is option1  " << option1 << std::endl;
        std::cout << "Myfunc6 is option2  " << option2 << std::endl;
    }
}

void Myfunc7(int64_t option1, float option2)
{
    std::cout << "Myfunc7 is option1  " << option1 << std::endl;
    std::cout << "Myfunc7 is option2  " << option2 << std::endl;
}

void Myfunc8(int64_t option1, double option2)
{
    std::cout << "Myfunc8 is option1  " << option1 << std::endl;
    std::cout << "Myfunc8 is option2  " << option2 << std::endl;
}

void Myfunc9(float option1, bool option2)
{
    if (option2) {
        std::cout << "Myfunc9 is option1  " << option1 << std::endl;
        std::cout << "Myfunc9 is option2  " << option2 << std::endl;
    } else {
        std::cout << "Myfunc9 is option1  " << option1 << std::endl;
        std::cout << "Myfunc9 is option2  " << option2 << std::endl;
    }
}

void Myfunc10(float option1, string_view option2)
{
    std::cout << "Myfunc10 is option1  " << option1 << std::endl;
    std::cout << "Myfunc10 is option2  " << option2 << std::endl;
}

void Myfunc11(double option1, string_view option2)
{
    std::cout << "Myfunc11 is option1  " << option1 << std::endl;
    std::cout << "Myfunc11 is option2  " << option2 << std::endl;
}

void Myfunc12(optional_view<int32_t> option1, optional_view<int64_t> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}

void Myfunc13(optional_view<float> option1, optional_view<double> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}

void Myfunc14(optional_view<string> option1, optional_view<bool> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}

void Myfunc15(optional_view<int16_t> option1, optional_view<int64_t> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}

void Myfunc16(optional_view<int16_t> option1, ::void_func::mytest::BarTest const &option2)
{
    switch (option2.get_key()) {
        case ::void_func::mytest::BarTest::key_t::A:
            std::cout << static_cast<int32_t>(option2.get_key()) << " A: " << std::endl;
            break;
        case ::void_func::mytest::BarTest::key_t::B:
            std::cout << static_cast<int32_t>(option2.get_key()) << " B: " << std::endl;
            break;
        case ::void_func::mytest::BarTest::key_t::C:
            std::cout << static_cast<int32_t>(option2.get_key()) << " C: " << std::endl;
            break;
    }
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_Myfunc1(Myfunc1);
TH_EXPORT_CPP_API_Myfunc2(Myfunc2);
TH_EXPORT_CPP_API_Myfunc3(Myfunc3);
TH_EXPORT_CPP_API_Myfunc4(Myfunc4);
TH_EXPORT_CPP_API_Myfunc5(Myfunc5);
TH_EXPORT_CPP_API_Myfunc6(Myfunc6);
TH_EXPORT_CPP_API_Myfunc7(Myfunc7);
TH_EXPORT_CPP_API_Myfunc8(Myfunc8);
TH_EXPORT_CPP_API_Myfunc9(Myfunc9);
TH_EXPORT_CPP_API_Myfunc10(Myfunc10);
TH_EXPORT_CPP_API_Myfunc11(Myfunc11);
TH_EXPORT_CPP_API_Myfunc12(Myfunc12);
TH_EXPORT_CPP_API_Myfunc13(Myfunc13);
TH_EXPORT_CPP_API_Myfunc14(Myfunc14);
TH_EXPORT_CPP_API_Myfunc15(Myfunc15);
TH_EXPORT_CPP_API_Myfunc16(Myfunc16);
// NOLINTEND