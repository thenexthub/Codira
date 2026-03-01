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

#include "libarkfile/file_writer.h"
#include "zip_archive.h"

#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

namespace ark::panda_file::test {

static std::vector<uint8_t> Str2Bytes(std::string str)
{
    std::vector<uint8_t> vec;
    vec.assign(str.begin(), str.end());
    return vec;
}

TEST(FileWriter, MemoryWriteByteWithCheck)
{
    const size_t chksumOffset = 0;
    std::vector<uint8_t> tdata = Str2Bytes("TestPanda");
    uint32_t expected = adler32(1, tdata.data(), tdata.size());
    {
        MemoryWriter memWriter;
        ASSERT_TRUE(memWriter.Write<uint32_t>(0));
        memWriter.CountChecksum(true);
        for (auto item : tdata) {
            ASSERT_TRUE(memWriter.WriteByte(item));
        }
        ASSERT_TRUE(memWriter.WriteChecksum(chksumOffset));
        std::vector<uint8_t> rdata = memWriter.GetData();
        auto pRealChecksum = reinterpret_cast<uint32_t *>(rdata.data());
        ASSERT_EQ(expected, *pRealChecksum);
    }
    {
        MemoryWriter memWriter;
        ASSERT_TRUE(memWriter.Write<uint32_t>(0));
        memWriter.CountChecksum(false);
        for (auto item : tdata) {
            ASSERT_TRUE(memWriter.WriteByte(item));
        }
        ASSERT_TRUE(memWriter.WriteChecksum(chksumOffset));
        std::vector<uint8_t> rdata = memWriter.GetData();
        auto pRealChecksum = reinterpret_cast<uint32_t *>(rdata.data());
        ASSERT_NE(expected, *pRealChecksum);
    }
}

TEST(FileWriter, MemoryWriteBytesWithCheck)
{
    const size_t chksumOffset = 0;
    std::vector<uint8_t> tdata = Str2Bytes("TestPanda");
    uint32_t expected = adler32(1, tdata.data(), tdata.size());
    {
        MemoryWriter memWriter;
        memWriter.CountChecksum(false);
        ASSERT_TRUE(memWriter.Write<uint32_t>(0));
        ASSERT_TRUE(memWriter.WriteBytes(tdata));
        ASSERT_TRUE(memWriter.WriteChecksum(chksumOffset));
        auto pRealChecksum = reinterpret_cast<const uint32_t *>(memWriter.GetData().data());
        ASSERT_NE(expected, *pRealChecksum);
    }
    {
        MemoryWriter memWriter;
        memWriter.Write<uint32_t>(0);
        memWriter.CountChecksum(true);
        ASSERT_TRUE(memWriter.WriteBytes(tdata));
        ASSERT_TRUE(memWriter.WriteChecksum(chksumOffset));
        auto pRealChecksum = reinterpret_cast<const uint32_t *>(memWriter.GetData().data());
        ASSERT_EQ(expected, *pRealChecksum);
    }
}

TEST(FileWriter, MemoryBufferWriteBytesWithCheck)
{
    std::vector<uint8_t> tdata = Str2Bytes("TestBufferWriter");
    uint32_t expected = adler32(1, tdata.data(), tdata.size());
    const size_t chksumOffset = 0;
    const auto bufSize = 64;
    std::vector<uint8_t> memBuf(bufSize);
    {
        MemoryBufferWriter writer(memBuf.data(), memBuf.size());
        ASSERT_TRUE(writer.Write<uint32_t>(0));
        writer.CountChecksum(true);
        ASSERT_TRUE(writer.WriteBytes(tdata));
        ASSERT_TRUE(writer.WriteChecksum(chksumOffset));
        auto pRealChecksum = reinterpret_cast<const uint32_t *>(memBuf.data());
        ASSERT_EQ(expected, *pRealChecksum);
    }
    {
        MemoryBufferWriter writer(memBuf.data(), memBuf.size());
        ASSERT_TRUE(writer.Write<uint32_t>(0));
        writer.CountChecksum(false);
        ASSERT_TRUE(writer.WriteBytes(tdata));
        ASSERT_TRUE(writer.WriteChecksum(chksumOffset));
        auto pRealChecksum = reinterpret_cast<const uint32_t *>(memBuf.data());
        ASSERT_NE(expected, *pRealChecksum);
    }
}

TEST(FileWriter, MemoryBufferWriteWithCheck)
{
    std::vector<uint8_t> tdata = Str2Bytes("@#Test-Buffer-Writer!");
    uint32_t expected = adler32(1, tdata.data(), tdata.size());
    const size_t chksumOffset = 0;
    const auto bufSize = 64;
    std::vector<uint8_t> memBuf(bufSize);
    {
        MemoryBufferWriter writer(memBuf.data(), memBuf.size());
        ASSERT_TRUE(writer.Write<uint32_t>(0));
        writer.CountChecksum(true);
        for (auto item : tdata) {
            ASSERT_TRUE(writer.WriteByte(item));
        }
        ASSERT_TRUE(writer.WriteChecksum(chksumOffset));
        auto pRealChecksum = reinterpret_cast<const uint32_t *>(memBuf.data());
        ASSERT_EQ(expected, *pRealChecksum);
    }
    {
        MemoryBufferWriter writer(memBuf.data(), memBuf.size());
        ASSERT_TRUE(writer.Write<uint32_t>(0));
        writer.CountChecksum(false);
        for (size_t i = 0; i < tdata.size(); ++i) {
            if (i == tdata.size() / 2) {
                writer.CountChecksum(true);
            }
            ASSERT_TRUE(writer.WriteByte(tdata[i]));
        }
        writer.WriteChecksum(chksumOffset);
        auto pRealChecksum = reinterpret_cast<const uint32_t *>(memBuf.data());
        ASSERT_NE(expected, *pRealChecksum);
    }
}

}  // namespace ark::panda_file::test
