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

#include "runtime/profilesaver/profile_dump_info.h"

#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <cerrno>
#include <climits>
#include <cstring>

#include "libarkbase/os/failure_retry.h"
#include "libarkbase/trace/trace.h"

#ifndef PATH_MAX
constexpr uint16_t PATH_MAX = 1024;
#endif

namespace ark {
static constexpr size_t K_BITS_PER_BYTE = 8;

static constexpr size_t K_LINE_HEADER_SIZE = 3 * sizeof(uint32_t) + sizeof(uint16_t);
static constexpr size_t K_METHOD_BYTES = 4;
static constexpr size_t K_CLASS_BYTES = 4;

const uint8_t ProfileDumpInfo::kProfileMagic[] = {'p', 'r', 'o', 'f', '\0'};  // NOLINT
const uint8_t ProfileDumpInfo::kProfileVersion[] = {'0', '1', '\0'};          // NOLINT

static constexpr uint16_t K_MAX_FILE_KEY_LENGTH = PATH_MAX;  // NOLINT

static bool WriteBuffer(int fd, const uint8_t *buffer, size_t byteCount)
{  // NOLINT
    while (byteCount > 0) {
        int bytesWritten = write(fd, buffer, byteCount);  // real place to write
        if (bytesWritten == -1) {
            return false;
        }
        byteCount -= static_cast<size_t>(bytesWritten);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        buffer += static_cast<size_t>(bytesWritten);
    }
    return true;
}

static void AddStringToBuffer(PandaVector<uint8_t> *buffer, const PandaString &value)
{  // NOLINT
    buffer->insert(buffer->end(), value.begin(), value.end());
}

template <typename T>
static void AddUintToBuffer(PandaVector<uint8_t> *buffer, T value)
{
    for (size_t i = 0; i < sizeof(T); i++) {
        buffer->push_back((value >> (i * K_BITS_PER_BYTE)) & 0xff);  // NOLINT
    }
}

/*
 * Tests for EOF by trying to read 1 byte from the descriptor.
 * Returns:
 *   0 if the descriptor is at the EOF,
 *  -1 if there was an IO error
 *   1 if the descriptor has more content to read
 */
// NOLINTNEXTLINE(readability-identifier-naming)
static int testEOF(int fd)
{
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    uint8_t buffer[1];
    return read(fd, buffer, 1);
}

ssize_t GetFileSizeBytes(const PandaString &filename)
{
    struct stat statBuf {};
    int rc = stat(filename.c_str(), &statBuf);
    return (rc == 0) ? statBuf.st_size : -1;
}

ProfileDumpInfo::ProfileLoadSatus ProfileDumpInfo::SerializerBuffer::FillFromFd(int fd, const PandaString &source,
                                                                                PandaString *error)
{
    size_t byteCount = ptrEnd_ - ptrCurrent_;
    uint8_t *buffer = ptrCurrent_;
    while (byteCount > 0) {
        int bytesRead = read(fd, buffer, byteCount);
        if (bytesRead == 0) {  // NOLINT
            *error += "Profile EOF reached prematurely for " + source;
            return PROFILE_LOAD_BAD_DATA;
        } else if (bytesRead < 0) {  // NOLINT
            *error += "Profile IO error for " + source + ConvertToString(os::Error(errno).ToString());
            return PROFILE_LOAD_IO_ERROR;
        }
        byteCount -= static_cast<size_t>(bytesRead);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        buffer += static_cast<size_t>(bytesRead);
    }
    return PROFILE_LOAD_SUCCESS;
}

template <typename T>
T ProfileDumpInfo::SerializerBuffer::ReadUintAndAdvance()
{
    static_assert(std::is_unsigned<T>::value, "Type is not unsigned");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ASSERT(ptrCurrent_ + sizeof(T) <= ptrEnd_);
    T value = 0;
    for (size_t i = 0; i < sizeof(T); i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        value += ptrCurrent_[i] << (i * K_BITS_PER_BYTE);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ptrCurrent_ += sizeof(T);
    return value;
}

bool ProfileDumpInfo::SerializerBuffer::CompareAndAdvance(const uint8_t *data, size_t dataSize)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (ptrCurrent_ + dataSize > ptrEnd_) {
        return false;
    }
    if (memcmp(ptrCurrent_, data, dataSize) == 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ptrCurrent_ += dataSize;
        return true;
    }
    return false;
}

bool ProfileDumpInfo::MergeWith(const ProfileDumpInfo &other)
{
    for (const auto &otherIt : other.dumpInfo_) {
        auto infoIt = dumpInfo_.find(otherIt.first);
        if ((infoIt != dumpInfo_.end()) && (infoIt->second.checksum != otherIt.second.checksum)) {
            LOG(INFO, RUNTIME) << "info_it->second.checksum" << infoIt->second.checksum;
            LOG(INFO, RUNTIME) << "other_it->second.checksum" << otherIt.second.checksum;
            LOG(INFO, RUNTIME) << "Checksum mismatch" << otherIt.first;
            return false;
        }
    }
    LOG(INFO, RUNTIME) << "All checksums match";

    for (const auto &otherIt : other.dumpInfo_) {
        const PandaString &otherProfileLocation = otherIt.first;
        const ProfileLineData &otherProfileData = otherIt.second;
        auto infoIt = dumpInfo_.find(otherProfileLocation);
        if (infoIt == dumpInfo_.end()) {
            auto ret =
                dumpInfo_.insert(std::make_pair(otherProfileLocation, ProfileLineData(otherProfileData.checksum)));
            ASSERT(ret.second);
            infoIt = ret.first;
        }
        infoIt->second.methodWrapperSet.insert(otherProfileData.methodWrapperSet.begin(),
                                               otherProfileData.methodWrapperSet.end());
        infoIt->second.classWrapperSet.insert(otherProfileData.classWrapperSet.begin(),
                                              otherProfileData.classWrapperSet.end());
    }
    return true;
}

bool ProfileDumpInfo::AddMethodsAndClasses(const PandaVector<ExtractedMethod> &methods,
                                           const PandaSet<ExtractedResolvedClasses> &resolvedClasses)
{
    for (const ExtractedMethod &method : methods) {
        if (!AddMethodWrapper(ConvertToString(method.pandaFile->GetFilename()), method.pandaFile->GetHeader()->checksum,
                              MethodWrapper(method.fileId.GetOffset()))) {
            return false;
        }
    }

    for (const ExtractedResolvedClasses &classResolved : resolvedClasses) {
        if (!AddResolvedClasses(classResolved)) {
            return false;
        }
    }
    return true;
}

uint64_t ProfileDumpInfo::GetNumberOfMethods() const
{
    uint64_t total = 0;
    for (const auto &it : dumpInfo_) {
        total += it.second.methodWrapperSet.size();
    }
    return total;
}

uint64_t ProfileDumpInfo::GetNumberOfResolvedClasses() const
{
    uint64_t total = 0;
    for (const auto &it : dumpInfo_) {
        total += it.second.classWrapperSet.size();
    }
    return total;
}

bool ProfileDumpInfo::ContainsMethod(const ExtractedMethod &methodRef) const
{
    auto infoIt = dumpInfo_.find(ConvertToString(methodRef.pandaFile->GetFilename()));
    if (infoIt != dumpInfo_.end()) {
        if (methodRef.pandaFile->GetHeader()->checksum != infoIt->second.checksum) {
            return false;
        }
        const PandaSet<MethodWrapper> &methods = infoIt->second.methodWrapperSet;
        return methods.find(MethodWrapper(methodRef.fileId.GetOffset())) != methods.end();
    }
    return false;
}

bool ProfileDumpInfo::ContainsClass(const panda_file::File &pandafile, uint32_t classDefIdx) const
{
    auto infoIt = dumpInfo_.find(ConvertToString(pandafile.GetFilename()));
    if (infoIt != dumpInfo_.end()) {
        if (pandafile.GetHeader()->checksum != infoIt->second.checksum) {
            return false;
        }
        const PandaSet<ClassWrapper> &classes = infoIt->second.classWrapperSet;
        return classes.find(ClassWrapper(classDefIdx)) != classes.end();
    }
    return false;
}

bool ProfileDumpInfo::AddMethodWrapper(const PandaString &pandaFileLocation, uint32_t checksum,
                                       const ProfileDumpInfo::MethodWrapper &methodToAdd)
{
    ProfileLineData *const data = GetOrAddProfileLineData(pandaFileLocation, checksum);
    if (data == nullptr) {
        return false;
    }
    data->methodWrapperSet.insert(methodToAdd);
    return true;
}

bool ProfileDumpInfo::AddClassWrapper(const PandaString &pandaFileLocation, uint32_t checksum,
                                      const ProfileDumpInfo::ClassWrapper &classToAdd)
{
    ProfileLineData *const data = GetOrAddProfileLineData(pandaFileLocation, checksum);
    if (data == nullptr) {
        return false;
    }
    data->classWrapperSet.insert(classToAdd);
    return true;
}

bool ProfileDumpInfo::AddResolvedClasses(const ExtractedResolvedClasses &classes)
{                                                                            // NOLINT(readability-identifier-naming)
    const PandaString panda_file_location = classes.GetPandaFileLocation();  // NOLINT(readability-identifier-naming)
    const uint32_t checksum = classes.GetPandaFileChecksum();                // NOLINT(readability-identifier-naming)
    ProfileLineData *const data = GetOrAddProfileLineData(panda_file_location, checksum);
    if (data == nullptr) {
        return false;
    }
    for (auto const &i : classes.GetClasses()) {
        data->classWrapperSet.insert(ClassWrapper(i));
    }
    return true;
}

ProfileDumpInfo::ProfileLineData *ProfileDumpInfo::GetOrAddProfileLineData(const PandaString &pandaFileLocation,
                                                                           uint32_t checksum)
{
    auto infoIt = dumpInfo_.find(pandaFileLocation);
    if (infoIt == dumpInfo_.end()) {
        auto ret = dumpInfo_.insert(std::make_pair(pandaFileLocation, ProfileLineData(checksum)));
        ASSERT(ret.second);
        infoIt = ret.first;
    }
    if (infoIt->second.checksum != checksum) {
        LOG(INFO, RUNTIME) << "Checksum mismatch" << pandaFileLocation;
        return nullptr;
    }
    return &(infoIt->second);
}

bool ProfileDumpInfo::Save(int fd)
{
    ASSERT(fd >= 0);
    trace::ScopedTrace scopedTrace(__PRETTY_FUNCTION__);

    static constexpr size_t K_MAX_BUFFER_SIZE = 8 * 1024;
    PandaVector<uint8_t> buffer;  // each element 1 byte

    WriteBuffer(fd, kProfileMagic, sizeof(kProfileMagic));
    WriteBuffer(fd, kProfileVersion, sizeof(kProfileVersion));
    AddUintToBuffer(&buffer, static_cast<uint32_t>(dumpInfo_.size()));

    for (const auto &it : dumpInfo_) {
        if (buffer.size() > K_MAX_BUFFER_SIZE) {
            if (!WriteBuffer(fd, buffer.data(), buffer.size())) {
                return false;
            }
            buffer.clear();
        }
        const PandaString &fileLocation = it.first;
        const ProfileLineData &fileData = it.second;

        if (fileLocation.size() >= K_MAX_FILE_KEY_LENGTH) {
            LOG(INFO, RUNTIME) << "PandaFileKey exceeds allocated limit";
            return false;
        }

        size_t requiredCapacity = buffer.size() + K_LINE_HEADER_SIZE + fileLocation.size() +
                                  K_METHOD_BYTES * fileData.methodWrapperSet.size() +
                                  K_CLASS_BYTES * fileData.classWrapperSet.size();
        buffer.reserve(requiredCapacity);

        ASSERT(fileLocation.size() <= std::numeric_limits<uint16_t>::max());
        ASSERT(fileData.methodWrapperSet.size() <= std::numeric_limits<uint32_t>::max());
        ASSERT(fileData.classWrapperSet.size() <= std::numeric_limits<uint32_t>::max());

        AddUintToBuffer(&buffer, static_cast<uint16_t>(fileLocation.size()));
        AddUintToBuffer(&buffer, static_cast<uint32_t>(fileData.methodWrapperSet.size()));
        AddUintToBuffer(&buffer, static_cast<uint32_t>(fileData.classWrapperSet.size()));
        AddUintToBuffer(&buffer, fileData.checksum);
        AddStringToBuffer(&buffer, fileLocation);

        if (UNLIKELY(fileData.empty())) {
            LOG(INFO, RUNTIME) << "EMPTY FILE DATA, WERIED!";
        }

        for (auto methodIt : fileData.methodWrapperSet) {
            AddUintToBuffer(&buffer, methodIt.methodId);
        }
        for (auto classIt : fileData.classWrapperSet) {
            AddUintToBuffer(&buffer, classIt.classId);
        }
        ASSERT(requiredCapacity == buffer.size());
    }
    return WriteBuffer(fd, buffer.data(), buffer.size());
}

bool ProfileDumpInfo::Load(int fd)
{
    trace::ScopedTrace scopedTrace(__PRETTY_FUNCTION__);
    PandaString error;
    ProfileLoadSatus status = LoadInternal(fd, &error);
    if (status == PROFILE_LOAD_SUCCESS) {
        return true;
    }
    LOG(INFO, RUNTIME) << "Error when reading profile " << error;
    return false;
}

ProfileDumpInfo::ProfileLoadSatus ProfileDumpInfo::LoadInternal(int fd, PandaString *error)
{
    ASSERT(fd >= 0);
    trace::ScopedTrace scopedTrace(__PRETTY_FUNCTION__);

    struct stat statBuffer {};
    if (fstat(fd, &statBuffer) != 0) {
        return PROFILE_LOAD_IO_ERROR;
    }

    if (statBuffer.st_size == 0) {
        LOG(INFO, RUNTIME) << "empty file";
        return PROFILE_LOAD_EMPTYFILE;
    }

    uint32_t numberOfLines;
    ProfileLoadSatus status = ReadProfileHeader(fd, &numberOfLines, error);
    if (status != PROFILE_LOAD_SUCCESS) {
        return status;
    }
    LOG(INFO, RUNTIME) << "number of profile items = " << numberOfLines;

    while (numberOfLines > 0) {
        ProfileLineHeader lineHeader;
        status = ReadProfileLineHeader(fd, &lineHeader, error);
        if (status != PROFILE_LOAD_SUCCESS) {
            return status;
        }

        status = ReadProfileLine(fd, lineHeader, error);
        if (status != PROFILE_LOAD_SUCCESS) {
            return status;
        }
        numberOfLines--;
    }

    int result = testEOF(fd);
    if (result == 0) {
        return PROFILE_LOAD_SUCCESS;
    }

    if (result < 0) {
        return PROFILE_LOAD_IO_ERROR;
    }

    *error = "Unexpected content in the profile file";
    return PROFILE_LOAD_BAD_DATA;
}

ProfileDumpInfo::ProfileLoadSatus ProfileDumpInfo::ReadProfileHeader(int fd, uint32_t *numberOfLines,
                                                                     PandaString *error)
{
    const size_t kMagicVersionSize = sizeof(kProfileMagic) + sizeof(kProfileVersion) + sizeof(uint32_t);

    SerializerBuffer safeBuffer(kMagicVersionSize);

    ProfileLoadSatus status = safeBuffer.FillFromFd(fd, "ReadProfileHeader", error);
    if (status != PROFILE_LOAD_SUCCESS) {
        return status;
    }

    if (!safeBuffer.CompareAndAdvance(kProfileMagic, sizeof(kProfileMagic))) {
        *error = "Profile missing magic";
        return PROFILE_LOAD_VERSION_MISMATCH;
    }
    if (!safeBuffer.CompareAndAdvance(kProfileVersion, sizeof(kProfileVersion))) {
        *error = "Profile version mismatch";
        return PROFILE_LOAD_VERSION_MISMATCH;
    }

    *numberOfLines = safeBuffer.ReadUintAndAdvance<uint32_t>();
    return PROFILE_LOAD_SUCCESS;
}

ProfileDumpInfo::ProfileLoadSatus ProfileDumpInfo::ReadProfileLineHeader(int fd, ProfileLineHeader *lineHeader,
                                                                         PandaString *error)
{
    SerializerBuffer headerBuffer(K_LINE_HEADER_SIZE);
    ProfileLoadSatus status = headerBuffer.FillFromFd(fd, "ReadProfileLineHeader", error);
    if (status != PROFILE_LOAD_SUCCESS) {
        return status;
    }

    auto pandaLocationSize = headerBuffer.ReadUintAndAdvance<uint16_t>();  // max chars in location, 4096 = 2 ^ 12
    lineHeader->methodSetSize = headerBuffer.ReadUintAndAdvance<uint32_t>();
    lineHeader->classSetSize = headerBuffer.ReadUintAndAdvance<uint32_t>();
    lineHeader->checksum = headerBuffer.ReadUintAndAdvance<uint32_t>();

    if (pandaLocationSize == 0 || pandaLocationSize > K_MAX_FILE_KEY_LENGTH) {
        *error = "PandaFileKey has an invalid size: " + std::to_string(pandaLocationSize);
        return PROFILE_LOAD_BAD_DATA;
    }

    SerializerBuffer locationBuffer(pandaLocationSize);
    // Read the binary data: location string
    status = locationBuffer.FillFromFd(fd, "ReadProfileLineHeader", error);
    if (status != PROFILE_LOAD_SUCCESS) {
        return status;
    }
    lineHeader->pandaFileLocation.assign(reinterpret_cast<char *>(locationBuffer.Get()), pandaLocationSize);
    return PROFILE_LOAD_SUCCESS;
}

ProfileDumpInfo::ProfileLoadSatus ProfileDumpInfo::ReadProfileLine(int fd, const ProfileLineHeader &lineHeader,
                                                                   PandaString *error)
{
    static constexpr uint32_t K_MAX_NUMBER_OF_ENTRIES_TO_READ = 8000;  // ~8 kb
    uint32_t methodsLeftToRead = lineHeader.methodSetSize;
    uint32_t classesLeftToRead = lineHeader.classSetSize;

    while ((methodsLeftToRead > 0) || (classesLeftToRead > 0)) {
        uint32_t methodsToRead = std::min(K_MAX_NUMBER_OF_ENTRIES_TO_READ, methodsLeftToRead);
        uint32_t maxClassesToRead = K_MAX_NUMBER_OF_ENTRIES_TO_READ - methodsToRead;  // >=0
        uint32_t classesToRead = std::min(maxClassesToRead, classesLeftToRead);

        size_t lineSize = K_METHOD_BYTES * methodsToRead + K_CLASS_BYTES * classesToRead;
        SerializerBuffer lineBuffer(lineSize);

        ProfileLoadSatus status = lineBuffer.FillFromFd(fd, "ReadProfileLine", error);
        if (status != PROFILE_LOAD_SUCCESS) {
            return status;
        }
        if (!ProcessLine(lineBuffer, methodsToRead, classesToRead, lineHeader.checksum, lineHeader.pandaFileLocation)) {
            *error = "Error when reading profile file line";
            return PROFILE_LOAD_BAD_DATA;
        }

        methodsLeftToRead -= methodsToRead;
        classesLeftToRead -= classesToRead;
    }
    return PROFILE_LOAD_SUCCESS;
}

// NOLINTNEXTLINE(google-runtime-references)
bool ProfileDumpInfo::ProcessLine(SerializerBuffer &lineBuffer, uint32_t methodSetSize, uint32_t classSetSize,
                                  uint32_t checksum, const PandaString &pandaFileLocation)
{
    for (uint32_t i = 0; i < methodSetSize; i++) {
        // NB! Read the method info from buffer...
        auto methodIdx = lineBuffer.ReadUintAndAdvance<uint32_t>();
        if (!AddMethodWrapper(pandaFileLocation, checksum, MethodWrapper(methodIdx))) {
            return false;
        }
    }

    for (uint32_t i = 0; i < classSetSize; i++) {
        auto classDefIdx = lineBuffer.ReadUintAndAdvance<uint32_t>();
        if (!AddClassWrapper(pandaFileLocation, checksum, ClassWrapper(classDefIdx))) {
            return false;
        }
    }
    return true;
}

bool ProfileDumpInfo::Save(const PandaString &filename, ssize_t *bytesWritten, int fd)
{
    bool result = Save(fd);
    if (result) {
        if (bytesWritten != nullptr) {
            LOG(INFO, RUNTIME) << "      Profile Saver Bingo! and bytes written = " << bytesWritten;
            int64_t writedBytes = GetFileSizeBytes(filename);
            *bytesWritten = writedBytes < 0 ? 0 : static_cast<int64_t>(writedBytes);
        }
    } else {
        LOG(ERROR, RUNTIME) << "Failed to save profile info to " << filename;
    }
    return result;
}

bool ProfileDumpInfo::MergeAndSave(const PandaString &filename, ssize_t *bytesWritten, bool force)
{
    // NB! we using READWRITE mode to leave the creation job to framework layer.
    ark::os::unix::file::File myfile = ark::os::file::Open(filename, ark::os::file::Mode::READWRITE);
    if (!myfile.IsValid()) {
        LOG(ERROR, RUNTIME) << "Cannot open the profile file" << filename;
        return false;
    }
    ark::os::file::FileHolder fholder(myfile);
    int fd = myfile.GetFd();

    LOG(INFO, RUNTIME) << "  Step3.2: starting merging ***";
    PandaString error;
    ProfileDumpInfo fileDumpInfo;
    ProfileLoadSatus status = fileDumpInfo.LoadInternal(fd, &error);
    if (status == PROFILE_LOAD_SUCCESS || status == PROFILE_LOAD_EMPTYFILE) {
        bool isMergeWith = MergeWith(fileDumpInfo);
        if (isMergeWith && dumpInfo_ == fileDumpInfo.dumpInfo_) {
            if (bytesWritten != nullptr) {
                *bytesWritten = 0;
            }
            LOG(INFO, RUNTIME) << "  No Saving as no change byte_written = 0";
            if (status != PROFILE_LOAD_EMPTYFILE) {
                return true;
            }
        } else if (!isMergeWith) {
            LOG(INFO, RUNTIME) << "  No Saving as Could not merge previous profile data from file " << filename;
            if (!force) {
                return false;
            }
        }
    } else if (force && ((status == PROFILE_LOAD_VERSION_MISMATCH) || (status == PROFILE_LOAD_BAD_DATA))) {
        LOG(INFO, RUNTIME) << "  Clearing bad or mismatch version profile data from file " << filename << ": " << error;
    } else {
        LOG(INFO, RUNTIME) << "  No Saving as Could not load profile data from file " << filename << ": " << error;
        return false;
    }

    LOG(INFO, RUNTIME) << "  Step3.3: starting Saving ***";
    LOG(INFO, RUNTIME) << "      clear file data firstly";
    if (!myfile.ClearData()) {
        LOG(INFO, RUNTIME) << "Could not clear profile file: " << filename;
        return false;
    }

    return Save(filename, bytesWritten, fd);
}

}  // namespace ark
