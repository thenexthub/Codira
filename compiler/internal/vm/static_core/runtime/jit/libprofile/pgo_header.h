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

#ifndef PGO_HEADER_H
#define PGO_HEADER_H

#include <array>
#include <cstdint>

namespace ark::pgo {

constexpr uint32_t PGO_HEADER_MAGIC_SIZE = 4;
constexpr uint32_t PGO_HEADER_VERSION_SIZE = 4;

struct PgoHeader {
    alignas(alignof(uint32_t)) std::array<char, PGO_HEADER_MAGIC_SIZE> magic;
    alignas(alignof(uint32_t)) std::array<char, PGO_HEADER_VERSION_SIZE> version;
    uint32_t versionProfileType;
    uint32_t savedProfileType;
    uint32_t headerSize;
    uint32_t withCha;
};

static_assert((sizeof(PgoHeader) % sizeof(uint32_t)) == 0);
static_assert(alignof(PgoHeader) == alignof(uint32_t));

struct PandaFilesSectionHeader {
    uint32_t numberOfFiles;
    uint32_t sectionSize;
};

struct PandaFileInfoHeader {
    uint32_t type;
    uint32_t fileNameLen;
};

struct SectionsInfoSectionHeader {
    uint32_t sectionNumber;
    uint32_t sectionSize;
};

struct SectionInfo {
    uint32_t checkSum;
    uint32_t zippedSize;
    uint32_t unzippedSize;
    uint32_t sectionType;
    int32_t pandaFileIdx;
};

struct MethodsHeader {
    uint32_t numberOfMethods;
    int32_t pandaFileIdx;
};

struct MethodDataHeader {
    uint32_t methodIdx;
    uint32_t classIdx;
    uint32_t savedType;
    uint32_t chunkSize;
};

struct AotProfileDataHeader {
    uint32_t profType;
    uint32_t numberOfRecords;
    uint32_t chunkSize;
};

}  // namespace ark::pgo

#endif  // PGO_HEADER_H
