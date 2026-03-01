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
//

#include "libarkfile/external/file_ext.h"
#include "libarkfile/external/panda_file_external.h"
#include "libarkfile/file-inl.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "libarkfile/debug_helpers.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/os/native_stack.h"
#include <map>

namespace ark::panda_file::ext {

struct MethodSymEntry {
    ark::panda_file::File::EntityId id;
    uint32_t length {0};
    std::string name;
};

}  // namespace ark::panda_file::ext

#ifdef __cplusplus
extern "C" {
#endif

struct PandaFileExt {
public:
    explicit PandaFileExt(std::unique_ptr<const ark::panda_file::File> &&pandaFile) : pandaFile_(std::move(pandaFile))
    {
    }

    size_t GetExtFileLineNumber(ark::panda_file::MethodDataAccessor mda, uint32_t bcOffset)
    {
        return ark::panda_file::debug_helpers::GetLineNumber(mda, bcOffset, pandaFile_.get());
    }

    ark::panda_file::ext::MethodSymEntry *QueryMethodSymByOffset(uint64_t offset)
    {
        auto it = methodSymbols_.upper_bound(offset);
        if (it != methodSymbols_.end() && offset >= it->second.id.GetOffset()) {
            return &it->second;
        }

        // Enmuate all methods and put them to local cache.
        ark::panda_file::ext::MethodSymEntry *found = nullptr;

        auto callBack = [this, offset, &found](ark::panda_file::MethodDataAccessor &mda) -> void {
            if (mda.GetCodeId().has_value()) {
                ark::panda_file::CodeDataAccessor ca {*pandaFile_, mda.GetCodeId().value()};
                ark::panda_file::ext::MethodSymEntry entry;
                entry.id = mda.GetCodeId().value();
                entry.length = ca.GetCodeSize();
                entry.name = std::string(ark::utf::Mutf8AsCString(pandaFile_->GetStringData(mda.GetNameId()).data));

                auto ret = methodSymbols_.emplace(offset, entry);
                if (mda.GetCodeId().value().GetOffset() <= offset &&
                    offset < mda.GetCodeId().value().GetOffset() + ca.GetCodeSize()) {
                    found = &ret.first->second;
                }
            }
        };

        for (uint32_t id : pandaFile_->GetClasses()) {
            if (pandaFile_->IsExternal(ark::panda_file::File::EntityId(id))) {
                continue;
            }

            ark::panda_file::ClassDataAccessor cda {*pandaFile_, ark::panda_file::File::EntityId(id)};
            cda.EnumerateMethods(callBack);
        }
        return found;
    }

    ark::panda_file::ext::MethodSymEntry *EnumerateAllMethods(uint32_t id, uint64_t offset,
                                                              ark::panda_file::ext::MethodSymEntry *found)
    {
        ark::panda_file::ClassDataAccessor cda {*pandaFile_, ark::panda_file::File::EntityId(id)};
        cda.EnumerateMethods([this, &cda, &offset, &found](ark::panda_file::MethodDataAccessor &mda) -> void {
            if (mda.GetCodeId().has_value()) {
                ark::panda_file::CodeDataAccessor ca {*pandaFile_, mda.GetCodeId().value()};
                ark::panda_file::ext::MethodSymEntry entry;
                entry.id = mda.GetCodeId().value();
                entry.length = ca.GetCodeSize();
                entry.name = std::string(ark::utf::Mutf8AsCString(pandaFile_->GetStringData(mda.GetNameId()).data));

                auto ret = methodSymbols_.emplace(offset, entry);
                if (mda.GetCodeId().value().GetOffset() <= offset &&
                    offset < mda.GetCodeId().value().GetOffset() + ca.GetCodeSize()) {
                    size_t lineNumber = GetExtFileLineNumber(mda, offset - mda.GetCodeId().value().GetOffset());
                    found = &ret.first->second;
                    ark::panda_file::File::EntityId idNew(lineNumber);
                    auto nameId = cda.GetDescriptor();
                    found->id = idNew;
                    found->name = ark::os::native_stack::ChangeJaveStackFormat(reinterpret_cast<const char *>(nameId)) +
                                  "." + found->name;
                }
            }
        });

        return found;
    }

    ark::panda_file::ext::MethodSymEntry *QueryMethodSymAndLineByOffset(uint64_t offset)
    {
        auto it = methodSymbols_.upper_bound(offset);
        if (it != methodSymbols_.end() && offset >= it->second.id.GetOffset()) {
            return &it->second;
        }

        // Enmuate all methods and put them to local cache.
        ark::panda_file::ext::MethodSymEntry *found = nullptr;
        for (uint32_t id : pandaFile_->GetClasses()) {
            if (pandaFile_->IsExternal(ark::panda_file::File::EntityId(id))) {
                continue;
            }

            found = EnumerateAllMethods(id, offset, found);
        }
        return found;
    }

    std::vector<struct MethodSymInfoExt> QueryAllMethodSyms()
    {
        // Enmuate all methods and put them to local cache.
        std::vector<ark::panda_file::ext::MethodSymEntry> res;
        for (uint32_t id : pandaFile_->GetClasses()) {
            if (pandaFile_->IsExternal(ark::panda_file::File::EntityId(id))) {
                continue;
            }

            EnumerateMethods(id, res);
        }

        std::vector<struct MethodSymInfoExt> methodInfo;
        methodInfo.reserve(res.size());
        for (auto const &me : res) {
            struct MethodSymInfoExt msi {};
            msi.offset = me.id.GetOffset();
            msi.length = me.length;
            msi.name = me.name;
            methodInfo.push_back(msi);
        }
        return methodInfo;
    }

private:
    inline void EnumerateMethods(uint32_t id, std::vector<ark::panda_file::ext::MethodSymEntry> &res)
    {
        ark::panda_file::ClassDataAccessor cda {*pandaFile_, ark::panda_file::File::EntityId(id)};
        cda.EnumerateMethods([&](ark::panda_file::MethodDataAccessor &mda) -> void {
            if (mda.GetCodeId().has_value()) {
                ark::panda_file::CodeDataAccessor ca {*pandaFile_, mda.GetCodeId().value()};
                res.push_back({mda.GetCodeId().value(), ca.GetCodeSize(), GetMethodSymName(mda)});
            }
        });
    }

    inline std::string GetMethodSymName(ark::panda_file::MethodDataAccessor &mda)
    {
        std::stringstream ss;
        std::string_view cname(ark::utf::Mutf8AsCString(pandaFile_->GetStringData(mda.GetClassId()).data));
        if (!cname.empty()) {
            ss << cname.substr(0, cname.size() - 1);
        }
        ss << "." << ark::utf::Mutf8AsCString(pandaFile_->GetStringData(mda.GetNameId()).data);

        return ss.str();
    }

    std::map<uint64_t, ark::panda_file::ext::MethodSymEntry> methodSymbols_;
    std::unique_ptr<const ark::panda_file::File> pandaFile_;
};

bool OpenPandafileFromMemoryExt(void *addr, const uint64_t *size, [[maybe_unused]] const std::string &fileName,
                                PandaFileExt **pandaFileExt)
{
    if (pandaFileExt == nullptr) {
        return false;
    }

    ark::os::mem::ConstBytePtr ptr(
        reinterpret_cast<std::byte *>(addr), *size,
        []([[maybe_unused]] std::byte *paramBuffer, [[maybe_unused]] size_t paramSize) noexcept {});
    auto pf = ark::panda_file::File::OpenFromMemory(std::move(ptr));
    if (pf == nullptr) {
        return false;
    }

    auto pfExt = std::make_unique<PandaFileExt>(std::move(pf));
    *pandaFileExt = pfExt.release();
    return true;
}

bool OpenPandafileFromFdExt([[maybe_unused]] int fd, [[maybe_unused]] uint64_t offset, const std::string &fileName,
                            PandaFileExt **pandaFileExt)
{
    auto pf = ark::panda_file::OpenPandaFile(fileName);
    if (pf == nullptr) {
        return false;
    }

    auto pfExt = std::make_unique<PandaFileExt>(std::move(pf));
    *pandaFileExt = pfExt.release();
    return true;
}

bool QueryMethodSymByOffsetExt(struct PandaFileExt *pf, uint64_t offset, struct MethodSymInfoExt *methodInfo)
{
    auto entry = pf->QueryMethodSymByOffset(offset);
    if ((entry != nullptr) && (methodInfo != nullptr)) {
        methodInfo->offset = entry->id.GetOffset();
        methodInfo->length = entry->length;
        methodInfo->name = entry->name;
        return true;
    }
    return false;
}

bool QueryMethodSymAndLineByOffsetExt(struct PandaFileExt *pf, uint64_t offset, struct MethodSymInfoExt *methodInfo)
{
    auto entry = pf->QueryMethodSymAndLineByOffset(offset);
    if ((entry != nullptr) && (methodInfo != nullptr)) {
        methodInfo->offset = entry->id.GetOffset();
        methodInfo->length = entry->length;
        methodInfo->name = entry->name;
        return true;
    }
    return false;
}

void QueryAllMethodSymsExt(PandaFileExt *pf, MethodSymInfoExtCallBack callback, void *userData)
{
    auto methodInfos = pf->QueryAllMethodSyms();
    for (auto mi : methodInfos) {
        callback(&mi, userData);
    }
}

#ifdef __cplusplus
}  // extern "C"
#endif
