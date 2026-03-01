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
#include "overload.impl.hpp"
#include <numeric>
#include "overload.Color.proj.1.hpp"
#include "overload.Mystruct.proj.1.hpp"
#include "overload.OverloadInterface.proj.2.hpp"
#include "taihe/array.hpp"
#include "taihe/map.hpp"
#include "taihe/string.hpp"

using namespace taihe;

namespace {
class OverloadInterface {
public:
    int8_t OverloadFuncI8(int8_t a, int8_t b)
    {
        std::cout << "OverloadFuncI8: a = " << (int)(a) << ", b = " << (int)(b) << std::endl;
        return a;
    }

    int16_t OverloadFuncI16(int16_t a, int16_t b)
    {
        std::cout << "OverloadFuncI16: a = " << a << ", b = " << b << std::endl;
        return a;
    }

    int32_t OverloadFuncI32(int32_t a, int32_t b)
    {
        std::cout << "OverloadFuncI32: a = " << a << ", b = " << b << std::endl;
        return a;
    }

    float OverloadFuncF32(float a, float b)
    {
        std::cout << "OverloadFuncF32: a = " << a << ", b = " << b << std::endl;
        return a;
    }

    double OverloadFuncF64(double a, double b)
    {
        std::cout << "OverloadFuncF64: a = " << a << ", b = " << b << std::endl;
        return a;
    }

    bool OverloadFuncBool(bool a, bool b)
    {
        std::cout << "OverloadFuncBool: a = " << a << ", b = " << b << std::endl;
        return a;
    }

    string OverloadFuncString(string_view a, string_view b)
    {
        std::cout << "OverloadFuncString: a = " << a << ", b = " << b << std::endl;
        return string(a);
    }

    int8_t OverloadFuncI8I16(int8_t a, int16_t b)
    {
        std::cout << "OverloadFuncI8I16: a = " << (int)(a) << ", b = " << b << std::endl;
        return a;
    }

    int8_t OverloadFuncI8F32(int8_t a, float b)
    {
        std::cout << "OverloadFuncI8F32: a = " << (int)(a) << ", b = " << b << std::endl;
        return a;
    }

    int8_t OverloadFuncI8String(int8_t a, string_view b)
    {
        std::cout << "OverloadFuncI8String: a = " << (int)(a) << ", b = " << b << std::endl;
        return a;
    }

    int32_t OverloadFuncEnum(::overload::Color const &p0)
    {
        std::cout << "OverloadFuncEnum: color = " << p0 << std::endl;
        return static_cast<int32_t>(p0);
    }

    string OverloadFuncMystruct(::overload::Mystruct const &p0)
    {
        std::cout << "OverloadFuncMystruct: testNum = " << p0.testNum << ", testStr = " << p0.testStr << std::endl;
        return p0.testStr;
    }

    void OverloadFunc5param1(int8_t p0, int16_t p1, int32_t p2, float p3, double p4)
    {
        std::cout << "OverloadFunc5param1: p0 = " << (int)p0 << ", p1 = " << p1 << ", p2 = " << p2 << ", p3 = " << p3
                  << ", p4 = " << p4 << std::endl;
    }

    bool OverloadFunc5paramPrimitive2(bool p0, string_view p1, int8_t p2, int16_t p3, int32_t p4)
    {
        std::cout << "OverloadFunc5paramPrimitive2: p0 = " << p0 << ", p1 = " << p1 << ", p2 = " << (int)p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return true;
    }

    float OverloadFunc5paramPrimitive3(float p0, double p1, bool p2, string_view p3, int8_t p4)
    {
        std::cout << "OverloadFunc5paramPrimitive3: p0 = " << p0 << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << (int)p4 << std::endl;
        return p0;
    }

    string OverloadFunc5paramPrimitive4(string_view p0, int16_t p1, int32_t p2, float p3, bool p4)
    {
        std::cout << "OverloadFunc5paramPrimitive4: p0 = " << p0 << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return string(p0);
    }

    string OverloadFunc5paramPrimitive5(string_view p0, string_view p1, string_view p2, bool p3, bool p4)
    {
        std::cout << "OverloadFunc5paramPrimitive5: p0 = " << p0 << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return string(p0);
    }

    int16_t OverloadFunc5paramPrimitive6(int16_t p0, int32_t p1, float p2, double p3, bool p4)
    {
        std::cout << "OverloadFunc5paramPrimitive6: p0 = " << p0 << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return p0;
    }

    string OverloadFunc5paramPrimitive7(string_view p0, int8_t p1, int16_t p2, float p3, bool p4)
    {
        std::cout << "OverloadFunc5paramPrimitive7: p0 = " << p0 << ", p1 = " << (int)p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return string(p0);
    }

    bool OverloadFunc5paramPrimitive8(bool p0, int8_t p1, int32_t p2, double p3, string_view p4)
    {
        std::cout << "OverloadFunc5paramPrimitive8: p0 = " << p0 << ", p1 = " << (int)p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return true;
    }

    double OverloadFunc5paramPrimitive9(double p0, bool p1, string_view p2, int16_t p3, int32_t p4)
    {
        std::cout << "OverloadFunc5paramPrimitive9: p0 = " << p0 << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return p0;
    }

    int8_t OverloadFunc5paramPrimitive10(int8_t p0, float p1, bool p2, string_view p3, int32_t p4)
    {
        std::cout << "OverloadFunc5paramPrimitive10: p0 = " << (int)p0 << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return p0;
    }

    // These functions are testcases for overload, which use many input parameters
    // NOLINTBEGIN(readability-function-size)
    void OverloadFunc10param(int8_t p0, int16_t p1, int32_t p2, float p3, double p4, bool p5, string_view p6,
                             array_view<int8_t> p7, array_view<int16_t> p8, array_view<int32_t> p9)
    {
        std::cout << "OverloadFunc10param: p0 = " << static_cast<int>(p0) << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << ", p5 = " << p5 << ", p6 = " << p6 << ", p7 = [";

        for (size_t i = 0; i < p7.size(); ++i) {
            std::cout << static_cast<int>(p7.data()[i]);
            if (i < p7.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p8 = [";
        for (size_t i = 0; i < p8.size(); ++i) {
            std::cout << p8.data()[i];
            if (i < p8.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p9 = [";
        for (size_t i = 0; i < p9.size(); ++i) {
            std::cout << p9.data()[i];
            if (i < p9.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "]" << std::endl;
    }

    void OverloadFunc10param1(int8_t p0, int16_t p1, int32_t p2, float p3, double p4, bool p5, string_view p6,
                              array_view<int8_t> p7, ::overload::Mystruct const &p8, ::overload::Color const &p9)
    {
        std::cout << "OverloadFunc10param1: p0 = " << static_cast<int>(p0) << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << ", p5 = " << p5 << ", p6 = " << p6 << ", p7 = [";

        for (size_t i = 0; i < p7.size(); ++i) {
            std::cout << static_cast<int>(p7.data()[i]);
            if (i < p7.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p8 = {testNum = " << p8.testNum << ", testStr = " << p8.testStr << "}"
                  << ", p9 = " << p9 << std::endl;
    }

    void OverloadFunc10param2(int8_t p0, ::overload::Mystruct const &p1, ::overload::Color const &p2,
                              array_view<float> p3, array_view<double> p4, array_view<bool> p5, array_view<string> p6,
                              array_view<int8_t> p7, array_view<int16_t> p8, array_view<int32_t> p9)
    {
        std::cout << "OverloadFunc10param2: p0 = " << static_cast<int>(p0) << ", p1 = {testNum = " << p1.testNum
                  << ", testStr = " << p1.testStr << "}"
                  << ", p2 = " << p2 << ", p3 = [";

        for (size_t i = 0; i < p3.size(); ++i) {
            std::cout << p3.data()[i];
            if (i < p3.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p4 = [";
        for (size_t i = 0; i < p4.size(); ++i) {
            std::cout << p4.data()[i];
            if (i < p4.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p5 = [";
        for (size_t i = 0; i < p5.size(); ++i) {
            std::cout << p5.data()[i];
            if (i < p5.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p6 = [";
        for (size_t i = 0; i < p6.size(); ++i) {
            std::cout << p6.data()[i];
            if (i < p6.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p7 = [";
        for (size_t i = 0; i < p7.size(); ++i) {
            std::cout << static_cast<int>(p7.data()[i]);
            if (i < p7.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p8 = [";
        for (size_t i = 0; i < p8.size(); ++i) {
            std::cout << p8.data()[i];
            if (i < p8.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p9 = [";
        for (size_t i = 0; i < p9.size(); ++i) {
            std::cout << p9.data()[i];
            if (i < p9.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "]" << std::endl;
    }

    void OverloadFunc10param3(int8_t p0, int16_t p1, int32_t p2, float p3, double p4, bool p5, string_view p6,
                              array_view<uint8_t> p7, ::overload::Mystruct const &p8, ::overload::Color const &p9)
    {
        std::cout << "OverloadFunc10param3: p0 = " << static_cast<int>(p0) << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << ", p5 = " << p5 << ", p6 = " << p6 << ", p7 = [";

        for (size_t i = 0; i < p7.size(); ++i) {
            std::cout << static_cast<int>(p7.data()[i]);
            if (i < p7.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p8 = {testNum = " << p8.testNum << ", testStr = " << p8.testStr << "}"
                  << ", p9 = " << p9 << std::endl;
    }

    void OverloadFunc10param4(int8_t p0, int16_t p1, int32_t p2, float p3, double p4, bool p5, string_view p6,
                              array_view<int8_t> p7, array_view<uint8_t> p8, ::overload::Color const &p9)
    {
        std::cout << "OverloadFunc10param4: p0 = " << static_cast<int>(p0) << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << ", p5 = " << p5 << ", p6 = " << p6 << ", p7 = [";

        for (size_t i = 0; i < p7.size(); ++i) {
            std::cout << static_cast<int>(p7.data()[i]);
            if (i < p7.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p8 = [";
        for (size_t i = 0; i < p8.size(); ++i) {
            std::cout << static_cast<int>(p8.data()[i]);
            if (i < p8.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p9 = " << p9 << std::endl;
    }

    // NOLINTEND(readability-function-size)

    int32_t OverloadFuncPoint(array_view<int32_t> a)
    {
        std::cout << "OverloadFuncPoint: a = [";
        for (size_t i = 0; i < a.size(); ++i) {
            std::cout << a.data()[i];
            if (i < a.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
        return a.data()[0];
    }

    uint8_t OverloadFuncArrayBuffer(array_view<uint8_t> a)
    {
        std::cout << "OverloadFuncArrayBuffer: a = [";
        for (size_t i = 0; i < a.size(); ++i) {
            std::cout << static_cast<int>(a.data()[i]);
            if (i < a.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
        return std::accumulate(a.begin(), a.end(), 0);
    }

    void OverloadFuncEnumRecord(::overload::Color const &p1, map_view<string, int16_t> p2)
    {
        // Function body
    }

    void OverloadFuncArrayRecord(array_view<int32_t> p1, map_view<string, int16_t> p2)
    {
        // Function body
    }
};

::overload::OverloadInterface get_interface()
{
    return make_holder<OverloadInterface, ::overload::OverloadInterface>();
}

}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_get_interface(get_interface);
// NOLINTEND