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

#ifndef PGO_FILE_BUILDER_H
#define PGO_FILE_BUILDER_H

#include "pgo_header.h"
#include "aot_profiling_data.h"
#include "libarkbase/utils/span.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark::pgo {

class AotPgoFile {
public:
    class Buffer {
    public:
        Buffer() : buffer_(0), bufferSize_(0), currSize_(0) {}

        explicit Buffer(uint32_t size)
            : buffer_(size > 1_MB ? LOG(FATAL, RUNTIME) << "Allocated of huge memory is not allowed" : size),
              bufferSize_(size),
              currSize_(0)
        {
        }

        Span<uint8_t> GetBuffer()
        {
            Span<uint8_t> buffer(buffer_.data(), bufferSize_);
            return buffer;
        }

        template <typename T>
        uint32_t CopyToBuffer(const T *from, size_t size, size_t beginIndex)
        {
            const char *data = reinterpret_cast<const char *>(from);
            if (beginIndex >= bufferSize_) {
                return 0;
            }
            auto maxSize {bufferSize_ - beginIndex};
            errno_t res = memcpy_s(&buffer_[beginIndex], maxSize, data, size);
            if (res != 0) {
                return 0;
            }
            currSize_ += size;
            return size;
        }

        void WriteBuffer(std::ofstream &fd)
        {
            fd.write(reinterpret_cast<const char *>(buffer_.data()), currSize_);

            if (!fd) {
                return;
            }
        }

    private:
        PandaVector<uint8_t> buffer_;
        uint32_t bufferSize_;
        uint32_t currSize_;
    };

    uint32_t Save(const PandaString &fileName, AotProfilingData *profObject, const PandaString &classCtxStr);

    static Expected<AotProfilingData, PandaString> Load(const PandaString &fileName, PandaString &classCtxStr,
                                                        PandaVector<PandaString> &pandaFiles);

private:
    static constexpr std::array MAGIC = {'.', 'a', 'p', '\0'};    // CC-OFF(G.NAM.03-CPP) project code style
    static constexpr std::array VERSION = {'0', '0', '1', '\0'};  // CC-OFF(G.NAM.03-CPP) project code style
    static constexpr uint32_t MAGIC_SIZE = 4;                     // CC-OFF(G.NAM.03-CPP) project code style
    static constexpr uint32_t VERSION_SIZE = 4;                   // CC-OFF(G.NAM.03-CPP) project code style

    enum ProfileType : uint32_t { NO = 0x00, VCALL = 0x01, BRANCH = 0x02, THROW = 0x04, ALL = 0xFFFFFFFF };
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr uint32_t PROFILE_TYPE = ProfileType::VCALL | ProfileType::BRANCH | ProfileType::THROW;

    enum FileType : uint32_t { PFILE = 0, BOOT = 1, APP = 2 };

    enum SectionType : uint32_t {
        UNKNOW_SECTION = 0,
        FILE_HEADER = 1,
        PANDA_FILES = 2,
        SECTIONS_INFO = 3,
        METHODS = 4
    };

    // CC-OFFNXT(G.FUN.01-CPP) Decreasing the number of arguments will decrease the clarity of the code.
    static uint32_t WriteFileHeader(std::ofstream &fd, const std::array<char, MAGIC_SIZE> &magic,
                                    const std::array<char, VERSION_SIZE> &version, uint32_t versionPType,
                                    uint32_t savedPType, const PandaString &classCtxStr);

    static uint32_t WriteSectionInfosSection(std::ofstream &fd, PandaVector<SectionInfo> &sectionInfos);

    static uint32_t GetSectionInfosSectionSize(uint32_t sectionNumer)
    {
        return sectionNumer * sizeof(SectionInfo) + sizeof(SectionsInfoSectionHeader);
    }

    static uint32_t GetMaxMethodSectionSize(AotProfilingData::MethodsMap &methods);
    static uint32_t GetMethodSectionProf(AotProfilingData::MethodsMap &methods);
    static uint32_t WriteInlineCachesToStream(uint32_t streamBegin, Buffer *buffer,
                                              Span<AotProfilingData::AotCallSiteInlineCache> inlineCaches);
    static uint32_t WriteBranchDataToStream(uint32_t streamBegin, Buffer *buffer,
                                            Span<AotProfilingData::AotBranchData> branches);
    static uint32_t WriteThrowDataToStream(uint32_t streamBegin, Buffer *buffer,
                                           Span<AotProfilingData::AotThrowData> throws);
    uint32_t WriteMethodsSection(std::ofstream &fd, int32_t pandaFileIdx, uint32_t *checkSum,
                                 AotProfilingData::MethodsMap &methods);
    uint32_t WriteMethodSubSection(uint32_t &currPos, Buffer *buffer, uint32_t methodIdx,
                                   AotProfilingData::AotMethodProfilingData &methodProfData);

    using FileToMethodsMap = PandaMap<PandaFileIdxType, AotProfilingData::MethodsMap>;
    uint32_t WriteAllMethodsSections(std::ofstream &fd, FileToMethodsMap &methods);
    uint32_t WritePandaFilesSection(std::ofstream &fd, PandaMap<int32_t, std::string_view> &pandaFileMap);
    uint32_t GetSectionNumbers(FileToMethodsMap &methods);
    uint32_t GetSavedTypes(FileToMethodsMap &allMethodsMap);

    PandaVector<SectionInfo> sectionInfos_;

    template <typename T>
    static std::ifstream &Read(std::ifstream &inputFile, T *dst, size_t n = 1, uint32_t *checkSum = nullptr)
    {
        return ReadBytes(inputFile, reinterpret_cast<char *>(dst), sizeof(T) * n, checkSum);
    }

    static std::ifstream &ReadBytes(std::ifstream &inputFile, char *dst, size_t n, uint32_t *checkSum = nullptr);

    static Expected<PandaVector<PandaString>, PandaString> ReadPandaFilesSection(std::ifstream &inputFile);

    static Expected<PandaVector<SectionInfo>, PandaString> ReadSectionInfos(std::ifstream &inputFile);

    template <typename T>
    static Expected<size_t, PandaString> ReadProfileData(std::ifstream &inputFile, const AotProfileDataHeader &header,
                                                         PandaVector<T> &data, uint32_t &checkSum);

    static Expected<uint32_t, PandaString> ReadMethodsSection(std::ifstream &inputFile, AotProfilingData &data);
    static Expected<AotProfilingData::AotMethodProfilingData, PandaString> ReadMethodSubSection(
        std::ifstream &inputFile, uint32_t &checkSum);

    static Expected<std::pair<PgoHeader, PandaString>, PandaString> ReadFileHeader(std::ifstream &inputFile);

    static Expected<size_t, PandaString> ReadAllMethodsSections(std::ifstream &inputFile,
                                                                PandaVector<SectionInfo> &sectionInfos,
                                                                AotProfilingData &data);
};

}  // namespace ark::pgo

#endif  // PGO_FILE_BUILDER_H
