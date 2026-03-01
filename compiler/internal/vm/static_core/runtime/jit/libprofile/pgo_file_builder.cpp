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

#include "pgo_file_builder.h"
#include "compiler/aot/aot_manager.h"
#include "include/mem/panda_string.h"
#include "zlib.h"
#include "libarkbase/utils/expected.h"
#include "libarkbase/utils/logger.h"

#include <iostream>
#include <iomanip>

namespace ark::pgo {
// NOLINTNEXTLINE(readability-magic-numbers)
static_assert(sizeof(PgoHeader) == 24, "PgoHeader is 24 bytes");
// NOLINTNEXTLINE(readability-magic-numbers)
static_assert(sizeof(PandaFilesSectionHeader) == 8, "PandaFiles section header is 8 bytes");
// NOLINTNEXTLINE(readability-magic-numbers)
static_assert(sizeof(PandaFileInfoHeader) == 8,
              "Header of each item of PandaFileInfo in PandaFiles section is 8 bytes");
// NOLINTNEXTLINE(readability-magic-numbers)
static_assert(sizeof(SectionsInfoSectionHeader) == 8, "Header of SectionsInfo section is 8 bytes");
// NOLINTNEXTLINE(readability-magic-numbers)
static_assert(sizeof(SectionInfo) == 20, "Each item of section info in SectionsInfo section is 20 bytes");
// NOLINTNEXTLINE(readability-magic-numbers)
static_assert(sizeof(MethodsHeader) == 8, "Header of Methods section is 8 bytes");
// NOLINTNEXTLINE(readability-magic-numbers)
static_assert(sizeof(MethodDataHeader) == 16, "Header of one method data sub-section is 16 bytes");
// NOLINTNEXTLINE(readability-magic-numbers)
static_assert(sizeof(AotProfileDataHeader) == 12, "Header of one profile data sub-section is 12 bytes");

uint32_t AotPgoFile::WritePandaFilesSection(std::ofstream &fd, PandaMap<int32_t, std::string_view> &pandaFileMap)
{
    PandaFilesSectionHeader sectionHeader = {static_cast<uint32_t>(pandaFileMap.size()),
                                             static_cast<uint32_t>(sizeof(PandaFilesSectionHeader))};

    for (auto &fileInfo : pandaFileMap) {
        sectionHeader.sectionSize += sizeof(PandaFileInfoHeader) + fileInfo.second.size() + 1;
    }
    AotPgoFile::Buffer buffer(sectionHeader.sectionSize);

    uint32_t currPos = 0;
    if (buffer.CopyToBuffer(&sectionHeader, sizeof(sectionHeader), currPos) == 0) {
        return 0;
    }
    currPos += sizeof(sectionHeader);

    for (auto &fileInfo : pandaFileMap) {
        PandaFileInfoHeader fileInfoHeader = {FileType::BOOT, static_cast<uint32_t>(fileInfo.second.size() + 1)};
        if (buffer.CopyToBuffer(&fileInfoHeader, sizeof(fileInfoHeader), currPos) == 0) {
            return 0;
        }
        currPos += sizeof(fileInfoHeader);
        if (buffer.CopyToBuffer(fileInfo.second.data(), fileInfoHeader.fileNameLen, currPos) == 0) {
            return 0;
        }
        currPos += fileInfoHeader.fileNameLen;
    }

    buffer.WriteBuffer(fd);
    return sectionHeader.sectionSize;
}

uint32_t AotPgoFile::WriteSectionInfosSection(std::ofstream &fd, PandaVector<SectionInfo> &sectionInfos)
{
    SectionsInfoSectionHeader header = {static_cast<uint32_t>(sectionInfos.size()),
                                        static_cast<uint32_t>(sizeof(SectionsInfoSectionHeader))};
    header.sectionSize += sectionInfos.size() * sizeof(SectionInfo);

    AotPgoFile::Buffer buffer(header.sectionSize);

    auto currPos = 0;
    if (buffer.CopyToBuffer(&header, sizeof(SectionsInfoSectionHeader), currPos) == 0) {
        return 0;
    }
    currPos += sizeof(SectionsInfoSectionHeader);

    for (auto &secInfo : sectionInfos) {
        if (buffer.CopyToBuffer(&secInfo, sizeof(SectionInfo), currPos) == 0) {
            return 0;
        }
        currPos += sizeof(SectionInfo);
    }

    buffer.WriteBuffer(fd);
    return header.sectionSize;
}

uint32_t AotPgoFile::WriteAllMethodsSections(std::ofstream &fd, FileToMethodsMap &methods)
{
    uint32_t totalSize = 0;
    for (auto &section : methods) {
        uint32_t checkSum = adler32(0L, Z_NULL, 0);
        auto sectionSize = WriteMethodsSection(fd, section.first, &checkSum, section.second);
        if (sectionSize == 0) {
            continue;
        }

        SectionInfo sectionInfo = {checkSum, 0, sectionSize, SectionType::METHODS, section.first};
        sectionInfos_.push_back(sectionInfo);

        totalSize += sectionSize;
    }
    return totalSize;
}

uint32_t AotPgoFile::GetMaxMethodSectionSize(AotProfilingData::MethodsMap &methods)
{
    uint32_t size = 0;
    for (auto &[methodIdx, methodProfData] : methods) {
        bool empty = true;
        auto inlineCaches = methodProfData.GetInlineCaches();
        auto branches = methodProfData.GetBranchData();
        auto throws = methodProfData.GetThrowData();
        if (!inlineCaches.empty()) {
            size += sizeof(AotProfileDataHeader) + inlineCaches.SizeBytes();
            empty = false;
        }
        if (!branches.empty()) {
            size += sizeof(AotProfileDataHeader) + branches.SizeBytes();
            empty = false;
        }
        if (!throws.empty()) {
            size += sizeof(AotProfileDataHeader) + throws.SizeBytes();
            empty = false;
        }
        if (!empty) {
            size += sizeof(MethodDataHeader);
        }
    }
    if (size > 0) {
        size += sizeof(MethodsHeader);
    }
    return size;
}

uint32_t AotPgoFile::GetMethodSectionProf(AotProfilingData::MethodsMap &methods)
{
    uint32_t savedType = 0;
    for (const auto &[methodIdx, methodProfData] : methods) {
        auto inlineCaches = methodProfData.GetInlineCaches();
        auto branches = methodProfData.GetBranchData();
        auto throws = methodProfData.GetThrowData();
        if (!inlineCaches.empty()) {
            savedType |= ProfileType::VCALL;
        }
        if (!branches.empty()) {
            savedType |= ProfileType::BRANCH;
        }
        if (!throws.empty()) {
            savedType |= ProfileType::THROW;
        }
    }
    return savedType;
}

uint32_t AotPgoFile::WriteMethodsSection(std::ofstream &fd, int32_t pandaFileIdx, uint32_t *checkSum,
                                         AotProfilingData::MethodsMap &methods)
{
    MethodsHeader methodsSectionHeader = {static_cast<uint32_t>(methods.size()), pandaFileIdx};

    uint32_t sectionSize = GetMaxMethodSectionSize(methods);
    Buffer buffer(sectionSize);

    uint32_t writtenBytes = 0;
    uint32_t currPos = 0;
    currPos += sizeof(MethodsHeader);

    for (auto &[methodIdx, methodProfData] : methods) {
        writtenBytes += WriteMethodSubSection(currPos, &buffer, methodIdx, methodProfData);
    }

    if (writtenBytes == 0) {
        return 0;
    }

    if (buffer.CopyToBuffer(&methodsSectionHeader, sizeof(MethodsHeader), 0) == 0) {
        return 0;
    }

    buffer.WriteBuffer(fd);
    writtenBytes += sizeof(MethodsHeader);

    *checkSum = adler32(*checkSum, buffer.GetBuffer().data(), writtenBytes);

    return writtenBytes;
}

uint32_t AotPgoFile::WriteMethodSubSection(uint32_t &currPos, Buffer *buffer, uint32_t methodIdx,
                                           AotProfilingData::AotMethodProfilingData &methodProfData)
{
    MethodDataHeader methodHeader = {methodIdx, methodProfData.GetClassIdx(), ProfileType::NO,
                                     sizeof(MethodDataHeader)};

    auto currMethodHeaderPos = currPos;
    currPos += sizeof(MethodDataHeader);
    uint32_t writtenBytes = 0;

    auto inlineCaches = methodProfData.GetInlineCaches();
    if (!inlineCaches.empty()) {
        methodHeader.savedType |= ProfileType::VCALL;
        auto icSize = WriteInlineCachesToStream(currPos, buffer, inlineCaches);
        methodHeader.chunkSize += icSize;
        writtenBytes += icSize;
        currPos += icSize;
    }

    auto branches = methodProfData.GetBranchData();
    if (!branches.empty()) {
        methodHeader.savedType |= ProfileType::BRANCH;
        auto brSize = WriteBranchDataToStream(currPos, buffer, branches);
        methodHeader.chunkSize += brSize;
        writtenBytes += brSize;
        currPos += brSize;
    }

    auto throws = methodProfData.GetThrowData();
    if (!throws.empty()) {
        methodHeader.savedType |= ProfileType::THROW;
        auto thSize = WriteThrowDataToStream(currPos, buffer, throws);
        methodHeader.chunkSize += thSize;
        writtenBytes += thSize;
        currPos += thSize;
    }

    if (methodHeader.chunkSize > sizeof(MethodDataHeader)) {
        if (buffer->CopyToBuffer(&methodHeader, sizeof(MethodDataHeader), currMethodHeaderPos) == 0) {
            return 0;
        }
        writtenBytes += sizeof(MethodDataHeader);
    } else {
        currPos -= sizeof(MethodDataHeader);
    }
    return writtenBytes;
}

uint32_t AotPgoFile::WriteInlineCachesToStream(uint32_t streamBegin, Buffer *buffer,
                                               Span<AotProfilingData::AotCallSiteInlineCache> inlineCaches)
{
    AotProfileDataHeader icHeader = {ProfileType::VCALL, static_cast<uint32_t>(inlineCaches.size()),
                                     static_cast<uint32_t>(sizeof(AotProfileDataHeader) + inlineCaches.SizeBytes())};
    uint32_t writtenBytes = 0;
    auto currPos = streamBegin;

    if (buffer->CopyToBuffer(&icHeader, sizeof(AotProfileDataHeader), currPos) == 0) {
        return 0;
    }
    currPos += sizeof(AotProfileDataHeader);
    writtenBytes += sizeof(AotProfileDataHeader);

    if (buffer->CopyToBuffer(inlineCaches.data(), inlineCaches.SizeBytes(), currPos) == 0) {
        return 0;
    }
    writtenBytes += inlineCaches.SizeBytes();
    return writtenBytes;
}

uint32_t AotPgoFile::WriteBranchDataToStream(uint32_t streamBegin, Buffer *buffer,
                                             Span<AotProfilingData::AotBranchData> branches)
{
    AotProfileDataHeader brHeader = {ProfileType::BRANCH, static_cast<uint32_t>(branches.size()),
                                     static_cast<uint32_t>(sizeof(AotProfileDataHeader) + branches.SizeBytes())};
    uint32_t writtenBytes = 0;
    auto currPos = streamBegin;

    if (buffer->CopyToBuffer(&brHeader, sizeof(AotProfileDataHeader), currPos) == 0) {
        return 0;
    }
    currPos += sizeof(AotProfileDataHeader);
    writtenBytes += sizeof(AotProfileDataHeader);

    if (buffer->CopyToBuffer(branches.data(), branches.SizeBytes(), currPos) == 0) {
        return 0;
    }
    writtenBytes += branches.SizeBytes();
    return writtenBytes;
}

uint32_t AotPgoFile::WriteThrowDataToStream(uint32_t streamBegin, Buffer *buffer,
                                            Span<AotProfilingData::AotThrowData> throws)
{
    AotProfileDataHeader thHeader = {ProfileType::THROW, static_cast<uint32_t>(throws.size()),
                                     static_cast<uint32_t>(sizeof(AotProfileDataHeader) + throws.SizeBytes())};
    uint32_t writtenBytes = 0;
    auto currPos = streamBegin;

    if (buffer->CopyToBuffer(&thHeader, sizeof(AotProfileDataHeader), currPos) == 0) {
        return 0;
    }
    currPos += sizeof(AotProfileDataHeader);
    writtenBytes += sizeof(AotProfileDataHeader);

    if (buffer->CopyToBuffer(throws.data(), throws.SizeBytes(), currPos) == 0) {
        return 0;
    }
    writtenBytes += throws.SizeBytes();
    return writtenBytes;
}

uint32_t AotPgoFile::GetSectionNumbers(FileToMethodsMap &methods)
{
    uint32_t count = 0;
    for (auto &method : methods) {
        if (GetMaxMethodSectionSize(method.second) > 0) {
            count++;
        }
    }
    return count;
}

uint32_t AotPgoFile::GetSavedTypes(FileToMethodsMap &allMethodsMap)
{
    uint32_t savedType = 0;
    for (auto &methodMap : allMethodsMap) {
        savedType |= GetMethodSectionProf(methodMap.second);
        if (savedType == PROFILE_TYPE) {
            break;
        }
    }
    return savedType;
}

// CC-OFFNXT(G.FUN.01-CPP) Decreasing the number of arguments will decrease the clarity of the code.
uint32_t AotPgoFile::WriteFileHeader(std::ofstream &fd, const std::array<char, MAGIC_SIZE> &magic,
                                     const std::array<char, VERSION_SIZE> &version, uint32_t versionPType,
                                     uint32_t savedPType, const PandaString &classCtxStr)
{
    uint32_t cha = classCtxStr.size() + 1;
    uint32_t headerSize = sizeof(PgoHeader) + cha;
    PgoHeader header = {magic, version, versionPType, savedPType, headerSize, cha};
    Buffer buffer(headerSize);

    if (buffer.CopyToBuffer(&header, sizeof(PgoHeader), 0) == 0) {
        return 0;
    }

    if (buffer.CopyToBuffer(classCtxStr.c_str(), cha, sizeof(PgoHeader)) == 0) {
        return 0;
    }

    buffer.WriteBuffer(fd);
    return header.headerSize;
}

// CC-OFFNXT(G.PRE.06) code generation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckAndAddBytes(fd, filePath, checkBytes, writtenBytes)                \
    {                                                                           \
        if ((checkBytes) == 0) {                                                \
            (fd).close();                                                       \
            if (remove((filePath).data()) == -1) {                              \
                LOG(ERROR, RUNTIME) << "Failed to remove file: " << (filePath); \
            }                                                                   \
            /* CC-OFFNXT(G.PRE.05) function gen */                              \
            return 0;                                                           \
        }                                                                       \
        (writtenBytes) += (checkBytes);                                         \
    }

uint32_t AotPgoFile::Save(const PandaString &fileName, AotProfilingData *profObject, const PandaString &classCtxStr)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    umask(S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    std::ofstream fd(fileName.data(), std::ios::binary | std::ios::out);
    if (!fd.is_open()) {
        return 0;
    }
    uint32_t writtenBytes = 0;

    auto savedProf = GetSavedTypes(profObject->GetAllMethods());
    auto headerBytes = WriteFileHeader(fd, MAGIC, VERSION, PROFILE_TYPE, savedProf, classCtxStr);
    CheckAndAddBytes(fd, fileName, headerBytes, writtenBytes);

    auto pandaFilesBytes = WritePandaFilesSection(fd, profObject->GetPandaFileMapReverse());
    CheckAndAddBytes(fd, fileName, pandaFilesBytes, writtenBytes);

    uint32_t offset = writtenBytes;
    auto sectionNum = GetSectionNumbers(profObject->GetAllMethods());
    auto sectionInfosSize = GetSectionInfosSectionSize(sectionNum);

    fd.seekp(sectionInfosSize, std::ios::cur);

    auto methodsBytes = WriteAllMethodsSections(fd, profObject->GetAllMethods());
    CheckAndAddBytes(fd, fileName, methodsBytes, writtenBytes);

    fd.seekp(offset, std::ios::beg);
    auto sectionInfoBytes = WriteSectionInfosSection(fd, sectionInfos_);
    CheckAndAddBytes(fd, fileName, sectionInfoBytes, writtenBytes);

    return writtenBytes;
}
#undef CheckAndAddBytes

std::ifstream &AotPgoFile::ReadBytes(std::ifstream &inputFile, char *dst, size_t n, uint32_t *checkSum)
{
    if (dst != nullptr) {
        inputFile.read(dst, n);
        if (checkSum != nullptr) {
            *checkSum = adler32(*checkSum, reinterpret_cast<unsigned char *>(dst), n);
        }
    } else {
        char ch;
        for (size_t i = 0; i < n; i++) {
            if (!ReadBytes(inputFile, &ch, 1, checkSum)) {
                break;
            }
        }
    }
    return inputFile;
}

using UnexpectedS = Unexpected<PandaString>;

Expected<PandaVector<PandaString>, PandaString> AotPgoFile::ReadPandaFilesSection(std::ifstream &inputFile)
{
    PandaFilesSectionHeader sectionHeader {};
    if (!Read(inputFile, &sectionHeader)) {
        return UnexpectedS("Cannot read panda files section header");
    }

    PandaVector<PandaString> pandaFiles(sectionHeader.numberOfFiles);
    for (auto &pandaFile : pandaFiles) {
        PandaFileInfoHeader fileInfoHeader {};
        if (!Read(inputFile, &fileInfoHeader)) {
            return UnexpectedS("Cannot read panda file info header");
        }

        ASSERT(fileInfoHeader.fileNameLen != 0);
        pandaFile.reserve(fileInfoHeader.fileNameLen - 1);
        ASSERT(pandaFile.data() != nullptr);
        pandaFile.resize(fileInfoHeader.fileNameLen - 1, '\0');

        if (!Read(inputFile, pandaFile.data(), fileInfoHeader.fileNameLen - 1) ||
            // fileNameLen is written with the 0-terminator
            !ReadBytes(inputFile, nullptr, 1)) {
            return UnexpectedS("Cannot read panda file info name");
        }
    }

    return pandaFiles;
}

Expected<uint32_t, PandaString> AotPgoFile::ReadMethodsSection(std::ifstream &inputFile, AotProfilingData &data)
{
    using namespace std::string_literals;
    uint32_t checkSum = adler32(0L, Z_NULL, 0);
    MethodsHeader methodsSectionHeader {};
    if (!Read(inputFile, &methodsSectionHeader, 1, &checkSum)) {
        return UnexpectedS("Couldn't read profile methods section header");
    }

    AotProfilingData::MethodsMap methodsMap;

    for (uint32_t i = 0; i < methodsSectionHeader.numberOfMethods; i++) {
        auto methodProfDataOrError = ReadMethodSubSection(inputFile, checkSum);
        if (!methodProfDataOrError) {
            return Unexpected(methodProfDataOrError.Error());
        }
        data.AddMethod(methodsSectionHeader.pandaFileIdx, methodProfDataOrError->GetMethodIdx(),
                       std::move(*methodProfDataOrError));
    }

    return checkSum;
}

Expected<AotProfilingData::AotMethodProfilingData, PandaString> AotPgoFile::ReadMethodSubSection(
    std::ifstream &inputFile, uint32_t &checkSum)
{
    using namespace std::string_literals;
    MethodDataHeader methodHeader {};
    if (!Read(inputFile, &methodHeader, 1, &checkSum)) {
        return UnexpectedS("Couldn't read profile method data header");
    }
    PandaVector<AotProfilingData::AotCallSiteInlineCache> inlineCaches;
    PandaVector<AotProfilingData::AotBranchData> branchData;
    PandaVector<AotProfilingData::AotThrowData> throwData;

    size_t readBytes = sizeof(MethodDataHeader);
    while (readBytes < methodHeader.chunkSize) {
        AotProfileDataHeader header {};
        if (!Read(inputFile, &header, 1, &checkSum)) {
            return UnexpectedS("Couldn't read profile data header");
        }

        Expected<size_t, PandaString> errorOrBytesRead;
        if ((methodHeader.savedType & ProfileType::VCALL) != 0 && header.profType == ProfileType::VCALL) {
            errorOrBytesRead = ReadProfileData(inputFile, header, inlineCaches, checkSum);
        } else if ((methodHeader.savedType & ProfileType::BRANCH) != 0 && header.profType == ProfileType::BRANCH) {
            errorOrBytesRead = ReadProfileData(inputFile, header, branchData, checkSum);
        } else if ((methodHeader.savedType & ProfileType::THROW) != 0 && header.profType == ProfileType::THROW) {
            errorOrBytesRead = ReadProfileData(inputFile, header, throwData, checkSum);
        } else {
            errorOrBytesRead = 0;
        }

        if (!errorOrBytesRead) {
            return Unexpected(errorOrBytesRead.Error());
        }

        if (sizeof(AotProfileDataHeader) + *errorOrBytesRead < header.chunkSize) {
            ReadBytes(inputFile, nullptr, header.chunkSize - sizeof(AotProfileDataHeader) - *errorOrBytesRead,
                      &checkSum);
        }

        readBytes += header.chunkSize;
    }

    if (readBytes != methodHeader.chunkSize) {
        return UnexpectedS("Inconsistent method data size in profile");
    }

    return AotProfilingData::AotMethodProfilingData(methodHeader.methodIdx, methodHeader.classIdx,
                                                    std::move(inlineCaches), std::move(branchData),
                                                    std::move(throwData));
}

template <typename T>
Expected<size_t, PandaString> AotPgoFile::ReadProfileData(std::ifstream &inputFile, const AotProfileDataHeader &header,
                                                          PandaVector<T> &data, uint32_t &checkSum)
{
    using namespace std::string_literals;
    size_t oldSize = data.size();
    data.resize(oldSize + header.numberOfRecords);
    size_t bytesToRead = sizeof(AotProfilingData::AotCallSiteInlineCache) * header.numberOfRecords;
    if (!Read(inputFile, &data[oldSize], header.numberOfRecords, &checkSum)) {
        return UnexpectedS("Couldn't read method profile data");
    }
    return bytesToRead;
}

template <typename Range>
static PandaStringStream &PrintBytesToSS(PandaStringStream &ss, const Range &rng)
{
    ss << std::setfill('0');
    for (auto &b : rng) {
        ss << std::hex << std::setw(2U) << static_cast<int>(b);
    }
    return ss;
}

Expected<std::pair<PgoHeader, PandaString>, PandaString> AotPgoFile::ReadFileHeader(std::ifstream &inputFile)
{
    using namespace std::string_literals;
    PgoHeader header {};
    if (!Read(inputFile, &header)) {
        return UnexpectedS("Couldn't read profile header");
    }
    if (header.magic != MAGIC) {
        PandaStringStream ss("Wrong profile header magic: expected ");
        PrintBytesToSS(ss, MAGIC) << ", got ";
        PrintBytesToSS(ss, header.magic);
        return Unexpected(std::move(ss).str());
    }
    if (header.version != VERSION) {
        PandaStringStream ss("Unsupported profile version: expected");
        PrintBytesToSS(ss, VERSION) << ", got ";
        PrintBytesToSS(ss, header.version);
        return Unexpected(std::move(ss).str());
    }
    if ((header.versionProfileType & ~PROFILE_TYPE) != 0) {
        // CC-OFFNXT(G.NAM.03-CPP) project code style
        constexpr size_t MASK_SIZE = sizeof(PROFILE_TYPE) * CHAR_BIT;
        PandaStringStream ss("Unsupported profile type: ");
        ss << std::bitset<MASK_SIZE>(header.versionProfileType) << ", expected submask of "
           << std::bitset<MASK_SIZE>(PROFILE_TYPE);
        return Unexpected(std::move(ss).str());
    }
    // Class context string is written with the 0-terminator
    auto classCtxStrSize = header.withCha;
    PandaString classCtxStr(classCtxStrSize - 1, '\0');
    if (!Read(inputFile, classCtxStr.data(), classCtxStrSize - 1) || !ReadBytes(inputFile, nullptr, 1)) {
        return UnexpectedS("Cannot read profile class context");
    }

    return std::make_pair(header, classCtxStr);
}

Expected<PandaVector<SectionInfo>, PandaString> AotPgoFile::ReadSectionInfos(std::ifstream &inputFile)
{
    using namespace std::string_literals;
    SectionsInfoSectionHeader header {};
    if (!Read(inputFile, &header)) {
        return UnexpectedS("Couldn't read profile section information header");
    }

    PandaVector<SectionInfo> sectionInfos(header.sectionNumber);
    if (!Read(inputFile, sectionInfos.data(), sectionInfos.size())) {
        return UnexpectedS("Couldn't read profile section information");
    }

    return sectionInfos;
}

Expected<size_t, PandaString> AotPgoFile::ReadAllMethodsSections(std::ifstream &inputFile,
                                                                 PandaVector<SectionInfo> &sectionInfos,
                                                                 AotProfilingData &data)
{
    using namespace std::string_literals;
    size_t nSections = 0;
    for (auto &info : sectionInfos) {
        if (info.sectionType != SectionType::METHODS) {
            inputFile.seekg(info.zippedSize != 0 ? info.zippedSize : info.unzippedSize, std::ios_base::cur);
            continue;
        }
        if (info.zippedSize != 0) {
            return UnexpectedS("Zipped profile method sections are unsupported");
        }
        auto checkSumOrError = ReadMethodsSection(inputFile, data);
        if (!checkSumOrError) {
            return Unexpected(checkSumOrError.Error());
        }
        if (info.checkSum != *checkSumOrError) {
            return UnexpectedS("Check sum mismatch for method profile data");
        }
        nSections++;
    }

    return nSections;
}

Expected<AotProfilingData, PandaString> AotPgoFile::Load(const PandaString &fileName, PandaString &classCtxStr,
                                                         PandaVector<PandaString> &pandaFiles)
{
    using namespace std::string_literals;
    AotProfilingData data;

    std::ifstream inputFile(fileName.c_str(), std::ios_base::in | std::ios_base::binary);
    if (!inputFile.is_open()) {
        return UnexpectedS("Cannot open file");
    }

    {
        auto headerOrError = ReadFileHeader(inputFile);
        if (!headerOrError) {
            return Unexpected(headerOrError.Error());
        }
        classCtxStr = std::move(headerOrError->second);
    }

    {
        auto pandaFilesOrError = ReadPandaFilesSection(inputFile);
        if (!pandaFilesOrError) {
            return Unexpected(pandaFilesOrError.Error());
        }
        pandaFiles = std::move(*pandaFilesOrError);
    }
    data.AddPandaFiles(pandaFiles);

    auto sectionInfosOrError = ReadSectionInfos(inputFile);
    if (!sectionInfosOrError) {
        return Unexpected(sectionInfosOrError.Error());
    }

    auto nOrError = ReadAllMethodsSections(inputFile, *sectionInfosOrError, data);
    if (!nOrError) {
        return Unexpected(nOrError.Error());
    }

    return data;
}
}  // namespace ark::pgo
