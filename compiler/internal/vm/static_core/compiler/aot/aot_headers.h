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

#ifndef COMPILER_AOT_AOT_HEADERS_H
#define COMPILER_AOT_AOT_HEADERS_H

#include <array>
#include <cstddef>
#include <cstdint>

namespace ark::compiler {

constexpr size_t AOT_HEADER_MAGIC_SIZE = 4;
constexpr size_t AOT_HEADER_VERSION_SIZE = 4;

struct AotHeader {
    alignas(alignof(uint32_t)) std::array<char, AOT_HEADER_MAGIC_SIZE> magic;
    alignas(alignof(uint32_t)) std::array<char, AOT_HEADER_VERSION_SIZE> version;
    uint32_t checksum;
    uint32_t environmentChecksum;
    uint32_t arch;
    uint32_t gcType;
    uint32_t filesCount;
    uint32_t filesOffset;
    uint32_t classHashTablesOffset;
    uint32_t classesOffset;
    uint32_t methodsOffset;
    uint32_t bitmapOffset;
    uint32_t strtabOffset;
    uint32_t fileNameStr;
    uint32_t cmdlineStr;
    uint32_t bootAot;
    uint32_t withCha;
    uint32_t classCtxStr;
};

static_assert((sizeof(AotHeader) % sizeof(uint32_t)) == 0);
static_assert(alignof(AotHeader) == alignof(uint32_t));

struct PandaFileHeader {
    uint32_t classHashTableSize;
    uint32_t classHashTableOffset;
    uint32_t classesCount;
    uint32_t classesOffset;
    uint32_t methodsCount;
    uint32_t methodsOffset;
    uint32_t fileChecksum;
    uint32_t fileOffset;
    uint32_t fileNameStr;
};

struct ClassHeader {
    uint32_t classId;
    uint32_t pabOffset;
    uint32_t methodsCount;
    uint32_t methodsOffset;
    // Offset to the methods bitmap (aligned as uint32_t)
    uint32_t methodsBitmapOffset;
    // Size of bitmap in bits
    uint32_t methodsBitmapSize;
};

struct MethodHeader {
    uint32_t methodId;
    uint32_t codeOffset;
    uint32_t codeSize;
};

}  // namespace ark::compiler

#endif  // COMPILER_AOT_AOT_HEADERS_H
