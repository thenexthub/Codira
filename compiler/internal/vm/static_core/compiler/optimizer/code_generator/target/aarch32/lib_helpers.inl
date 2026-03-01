/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Gcc lib-helpers:
#ifdef __arm__

// defined in libgcc_s.so

// NOLINTBEGIN(readability-identifier-naming)
#define AeabiUldivmod __aeabi_uldivmod
#define AeabiLdivmod __aeabi_ldivmod
#define AeabiUidivmod __aeabi_uidivmod
#define AeabiIdivmod __aeabi_idivmod
#define AeabiL2f __aeabi_l2f
#define AeabiUl2f __aeabi_ul2f
#define AeabiL2d __aeabi_l2d
#define AeabiUl2d __aeabi_ul2d
#define AeabiF2lz __aeabi_f2lz
#define AeabiF2ulz __aeabi_f2ulz
#define AeabiD2lz __aeabi_d2lz
#define AeabiD2ulz __aeabi_d2ulz
// NOLINTEND(readability-identifier-naming)

extern "C" uint64_t AeabiUldivmod(uint64_t numerator, uint64_t denominator);
extern "C" int64_t AeabiLdivmod(int64_t numerator, int64_t denominator);

extern "C" uint32_t AeabiUidivmod(uint32_t numerator, uint32_t denominator);
extern "C" int32_t AeabiIdivmod(int32_t numerator, int32_t denominator);

extern "C" float AeabiL2f(int64_t data);
extern "C" float AeabiUl2f(uint64_t data);
extern "C" double AeabiL2d(int64_t data);
extern "C" double AeabiUl2d(uint64_t data);

extern "C" int64_t AeabiF2lz(float data);
extern "C" uint64_t AeabiF2ulz(float data);
extern "C" int64_t AeabiD2lz(double data);
extern "C" uint64_t AeabiD2ulz(double data);

#define AEABIuldivmod __aeabi_uldivmod  // __aeabi_uldivmod returns two consecutive uint64_t
#define AEABIldivmod __aeabi_ldivmod    // __aeabi_uldivmod returns two consecutive int64_t

auto AEABIuidivmod(uint32_t numerator, uint32_t denominator)
{
    return AeabiUidivmod(numerator, denominator);
}
auto AEABIidivmod(int32_t numerator, int32_t denominator)
{
    return AeabiIdivmod(numerator, denominator);
}

float AEABIl2f(int64_t data)
{
    return AeabiL2f(data);
}

float AEABIul2f(uint64_t data)
{
    return AeabiUl2f(data);
}

double AEABIl2d(int64_t data)
{
    return AeabiL2d(data);
}

double AEABIul2d(uint64_t data)
{
    return AeabiUl2d(data);
}

int64_t AEABIf2lz(float data)
{
    return AeabiF2lz(data);
}

uint64_t AEABIf2ulz(float data)
{
    return AeabiF2ulz(data);
}

int64_t AEABId2lz(double data)
{
    return AeabiD2lz(data);
}

uint64_t AEABId2ulz(double data)
{
    return AeabiD2ulz(data);
}

#else

struct DivLUResult {
    uint64_t quotient;
    uint64_t remainder;
};

struct DivLSResult {
    int64_t quotient;
    int64_t remainder;
};

auto AEABIuldivmod(uint64_t numerator, uint64_t denominator)
{
    ASSERT(denominator != 0);
    DivLUResult res {0, 0};
    res.quotient = numerator / denominator;
    res.remainder = numerator % denominator;
    return res;
}
auto AEABIldivmod(int64_t numerator, int64_t denominator)
{
    ASSERT(denominator != 0);
    DivLSResult res {0, 0};
    res.quotient = numerator / denominator;
    res.remainder = numerator % denominator;
    return res;
}

struct DivUResult {
    uint32_t quotient;
    uint32_t remainder;
};

struct DivSResult {
    int32_t quotient;
    int32_t remainder;
};

auto AEABIuidivmod(uint32_t numerator, uint32_t denominator)
{
    ASSERT(denominator != 0);
    DivUResult res {0, 0};
    res.quotient = numerator / denominator;
    res.remainder = numerator % denominator;
    return res;
}
DivSResult AEABIidivmod(int32_t numerator, int32_t denominator)
{
    ASSERT(denominator != 0);
    DivSResult res {0, 0};
    res.quotient = numerator / denominator;
    res.remainder = numerator % denominator;
    return res;
}

float AEABIl2f(int64_t data)
{
    return static_cast<float>(data);
}

float AEABIul2f(uint64_t data)
{
    return static_cast<float>(data);
}

double AEABIl2d(int64_t data)
{
    return static_cast<double>(data);
}

double AEABIul2d(uint64_t data)
{
    return static_cast<double>(data);
}

int64_t AEABIf2lz(float data)
{
    return static_cast<int64_t>(data);
}

uint64_t AEABIf2ulz(float data)
{
    return static_cast<uint64_t>(data);
}

int64_t AEABId2lz(double data)
{
    return static_cast<int64_t>(data);
}

uint64_t AEABId2ulz(double data)
{
    return static_cast<uint64_t>(data);
}
#endif

// defined in libm.so, fmodf function from math.h
extern "C" float fmodf(float a, float b);  // NOLINT(misc-definitions-in-headers,readability-identifier-naming)
// defined in libm.so, fmod function from math.h
extern "C" double fmod(double a, double b);  // NOLINT(misc-definitions-in-headers,readability-identifier-naming)
