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
#ifndef PANDA_RUNTIME_PROFILE_DUMP_INFO_H_
#define PANDA_RUNTIME_PROFILE_DUMP_INFO_H_

#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "libarkbase/utils/logger.h"
#include "libarkfile/file.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/mem/panda_string.h"

// NB! suppose that, panda file can always provide such profile file format data!
// NB! we use serializer saving way which means the profile file is just binary file!
// profile header
//      magic
//      version
//      checksum?(ommit)
//      #lines
// Line1:
//      profileline header
//          file location
//          #method
//          #class
//          checksum
//      methods index/id(#method)
//      class index/id(#class)
// LineN:
//      ...

namespace ark {

/*
 * Any newly added information, we have to change the following info naturally, especially
 * ExtractedResolvedClasses
 */
struct ExtractedMethod {
    ExtractedMethod(const panda_file::File *file, panda_file::File::EntityId pfFileId)
        : pandaFile(file), fileId(pfFileId)
    {
    }
    const panda_file::File *pandaFile;  // NOLINT(misc-non-private-member-variables-in-classes)
    panda_file::File::EntityId fileId;  // NOLINT(misc-non-private-member-variables-in-classes)
};

struct ExtractedResolvedClasses {
public:
    // NOLINTNEXTLINE(modernize-pass-by-value)
    ExtractedResolvedClasses(const PandaString &location, uint32_t checksum)
        : panda_file_location_(location), panda_file_checksum_(checksum)
    {
    }

    int Compare(const ExtractedResolvedClasses &other) const
    {
        if (panda_file_checksum_ != other.panda_file_checksum_) {
            return static_cast<int>(panda_file_checksum_ - other.panda_file_checksum_);
        }
        return panda_file_location_.compare(other.panda_file_location_);
    }

    template <class InputIt>
    void AddClasses(InputIt begin, InputIt end) const
    {
        classes_.insert(begin, end);
    }

    void AddClass(uint32_t classindex) const
    {
        classes_.insert(classindex);
    }

    // NOLINTNEXTLINE(readability-const-return-type)
    const PandaString GetPandaFileLocation() const
    {
        return panda_file_location_;
    }

    uint32_t GetPandaFileChecksum() const
    {
        return panda_file_checksum_;
    }

    const PandaUnorderedSet<uint32_t> &GetClasses() const
    {
        return classes_;
    }

private:
    const PandaString panda_file_location_;  // NOLINT(readability-identifier-naming)
    const uint32_t panda_file_checksum_;     // NOLINT(readability-identifier-naming)
    // Array of resolved class def indexes. we leave this as extension
    mutable PandaUnorderedSet<uint32_t> classes_;
};

// we define this for PandaSet find() function
inline bool operator<(const ExtractedResolvedClasses &a, const ExtractedResolvedClasses &b)
{
    return a.Compare(b) < 0;
}

class ProfileDumpInfo {
public:
    // Content of profile header
    static const uint8_t kProfileMagic[];    // NOLINT(modernize-avoid-c-arrays, readability-identifier-naming)
    static const uint8_t kProfileVersion[];  // NOLINT(modernize-avoid-c-arrays, readability-identifier-naming)

    /*
     * Saves the profile data to the given file descriptor.
     */
    bool Save(int fd);

    /*
     * Loads profile information from the given file descriptor.
     */
    bool Load(int fd);

    /*
     * Merge the data from another ProfileDumpInfo into the current object.
     */
    bool MergeWith(const ProfileDumpInfo &other);

    /*
     * Add the given methods and classes to the current profile object
     */
    bool AddMethodsAndClasses(const PandaVector<ExtractedMethod> &methods,
                              const PandaSet<ExtractedResolvedClasses> &resolvedClasses);

    /*
     * Loads and merges profile information from the given file into the current cache
     * object and tries to save it back to disk.
     *
     * If `force` is true then the save will be forced regardless of bad data or mismatched version.
     */
    bool MergeAndSave(const PandaString &filename, ssize_t *bytesWritten, bool force);

    /*
     * Returns the number of methods that were profiled.
     */
    uint64_t GetNumberOfMethods() const;

    /*
     * Returns the number of resolved classes that were profiled.
     */
    uint64_t GetNumberOfResolvedClasses() const;

    /*
     * Returns true if the method reference is present in the profiling info.
     */
    bool ContainsMethod(const ExtractedMethod &methodRef) const;

    /*
     * Returns true if the class is present in the profiling info.
     */
    bool ContainsClass(const panda_file::File &pandafile, uint32_t classDefIdx) const;

private:
    enum ProfileLoadSatus {
        PROFILE_LOAD_IO_ERROR,
        PROFILE_LOAD_VERSION_MISMATCH,
        PROFILE_LOAD_BAD_DATA,
        PROFILE_LOAD_EMPTYFILE,
        PROFILE_LOAD_SUCCESS
    };

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    struct ProfileLineHeader {
        PandaString pandaFileLocation;
        uint32_t methodSetSize;
        uint32_t classSetSize;
        uint32_t checksum;
    };

    // A helper structure to make sure we don't read past our buffers in the loops.
    struct SerializerBuffer {
    public:
        explicit SerializerBuffer(size_t size)
        {
            // NOLINTNEXTLINE(modernize-avoid-c-arrays)
            storage_ = MakePandaUnique<uint8_t[]>(size);
            ptrCurrent_ = storage_.get();
            ptrEnd_ = ptrCurrent_ + size;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic
        }

        ProfileLoadSatus FillFromFd(int fd, const PandaString &source, PandaString *error);

        template <typename T>
        T ReadUintAndAdvance();

        bool CompareAndAdvance(const uint8_t *data, size_t dataSize);

        uint8_t *Get()
        {
            return storage_.get();
        }

    private:
        PandaUniquePtr<uint8_t[]> storage_;  // NOLINT(modernize-avoid-c-arrays)
        uint8_t *ptrCurrent_;
        uint8_t *ptrEnd_;
    };

    struct MethodWrapper {
        explicit MethodWrapper(uint32_t index) : methodId(index) {}
        uint32_t methodId;  // NOLINT(misc-non-private-member-variables-in-classes)

        bool operator==(const MethodWrapper &other) const
        {
            return methodId == other.methodId;
        }

        bool operator<(const MethodWrapper &other) const
        {
            return methodId < other.methodId;
        }
    };

    struct ClassWrapper {
        explicit ClassWrapper(uint32_t index) : classId(index) {}
        uint32_t classId;  // NOLINT(misc-non-private-member-variables-in-classes)

        bool operator==(const ClassWrapper &other) const
        {
            return classId == other.classId;
        }

        bool operator<(const ClassWrapper &other) const
        {
            return classId < other.classId;
        }
    };

    struct ProfileLineData {
        explicit ProfileLineData(uint32_t fileChecksum) : checksum(fileChecksum) {}
        uint32_t checksum;                         // NOLINT(misc-non-private-member-variables-in-classes)
        PandaSet<MethodWrapper> methodWrapperSet;  // NOLINT(misc-non-private-member-variables-in-classes)
        PandaSet<ClassWrapper> classWrapperSet;    // NOLINT(misc-non-private-member-variables-in-classes)

        bool operator==(const ProfileLineData &other) const
        {
            return checksum == other.checksum && methodWrapperSet == other.methodWrapperSet &&
                   classWrapperSet == other.classWrapperSet;
        }

        // NOLINTNEXTLINE(readability-identifier-naming)
        bool empty() const
        {
            return methodWrapperSet.empty() && classWrapperSet.empty();
        }
    };

    ProfileLoadSatus LoadInternal(int fd, PandaString *error);
    ProfileLoadSatus ReadProfileHeader(int fd, uint32_t *numberOfLines, PandaString *error);
    ProfileLoadSatus ReadProfileLineHeader(int fd, ProfileLineHeader *lineHeader, PandaString *error);
    ProfileLoadSatus ReadProfileLine(int fd, const ProfileLineHeader &lineHeader, PandaString *error);
    // NOLINTNEXTLINE(google-runtime-references)
    bool ProcessLine(SerializerBuffer &lineBuffer, uint32_t methodSetSize, uint32_t classSetSize, uint32_t checksum,
                     const PandaString &pandaFileLocation);

    bool AddMethodWrapper(const PandaString &pandaFileLocation, uint32_t checksum, const MethodWrapper &methodToAdd);
    bool AddClassWrapper(const PandaString &pandaFileLocation, uint32_t checksum, const ClassWrapper &classToAdd);
    bool AddResolvedClasses(const ExtractedResolvedClasses &classes);
    ProfileLineData *GetOrAddProfileLineData(const PandaString &pandaFileLocation, uint32_t checksum);
    bool Save(const PandaString &filename, ssize_t *bytesWritten, int fd);

    PandaMap<const PandaString, ProfileLineData> dumpInfo_;
};

}  // namespace ark

#endif
