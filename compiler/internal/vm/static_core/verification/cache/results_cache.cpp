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

#include "verification/cache/results_cache.h"
#include "verification/util/synchronized.h"

#include "runtime/include/runtime.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/mem/panda_containers.h"

#include "libarkbase/os/file.h"
#include "libarkbase/utils/logger.h"

namespace ark::verifier {

struct VerificationResultCache::Impl {
    std::string filename;
    Synchronized<PandaUnorderedSet<uint64_t>> verifiedOk;
    Synchronized<PandaUnorderedSet<uint64_t>> verifiedFail;

    template <typename It>
    Impl(std::string fileName, It dataStart, It dataEnd)
        : filename {std::move(fileName)}, verifiedOk {dataStart, dataEnd}
    {
    }
};

VerificationResultCache::Impl *VerificationResultCache::impl_ {nullptr};

bool VerificationResultCache::Enabled()
{
    return impl_ != nullptr;
}

void VerificationResultCache::Initialize(const std::string &filename)
{
    if (Enabled()) {
        return;
    }
    using ark::os::file::Mode;
    using ark::os::file::Open;
    using Data = PandaVector<uint64_t>;

    auto file = Open(filename, Mode::READONLY);
    if (!file.IsValid()) {
        file = Open(filename, Mode::READWRITECREATE);
    }
    if (!file.IsValid()) {
        LOG(INFO, VERIFIER) << "Cannot open verification cache file '" << filename << "'";
        return;
    }

    auto size = file.GetFileSize();
    if (!size.HasValue()) {
        LOG(INFO, VERIFIER) << "Cannot get verification cache file size";
        file.Close();
        return;
    }

    Data data;

    auto elements = *size / sizeof(Data::value_type);

    if (elements > 0) {
        data.resize(elements, 0);
        if (!file.ReadAll(data.data(), *size)) {
            LOG(INFO, VERIFIER) << "Cannot read verification cache data";
            file.Close();
            return;
        }
    }

    file.Close();

    impl_ = new (mem::AllocatorAdapter<Impl>().allocate(1)) Impl {filename, data.cbegin(), data.cend()};
    ASSERT(Enabled());
}

void VerificationResultCache::Destroy(bool updateFile)
{
    if (!Enabled()) {
        return;
    }
    if (updateFile) {
        PandaVector<uint64_t> data;
        impl_->verifiedOk.Apply([&data](const auto &set) {
            data.reserve(set.size());
            data.insert(data.begin(), set.cbegin(), set.cend());
        });
        using ark::os::file::Mode;
        using ark::os::file::Open;
        do {
            auto file = Open(impl_->filename, Mode::READWRITECREATE);
            if (!file.IsValid()) {
                LOG(INFO, VERIFIER) << "Cannot open verification cache file '" << impl_->filename << "'";
                break;
            }
            if (!file.ClearData()) {
                LOG(INFO, VERIFIER) << "Cannot clear verification cache file '" << impl_->filename << "'";
                file.Close();
                break;
            }
            if (!file.WriteAll(data.data(), data.size() * sizeof(uint64_t))) {
                LOG(INFO, VERIFIER) << "Cannot write to verification cache file '" << impl_->filename << "'";
                file.ClearData();
                file.Close();
                break;
            }
            file.Close();
        } while (false);
    }
    impl_->~Impl();
    mem::AllocatorAdapter<Impl>().deallocate(impl_, 1);
    impl_ = nullptr;
}

void VerificationResultCache::CacheResult(uint64_t methodId, bool result)
{
    if (Enabled()) {
        if (result) {
            impl_->verifiedOk->insert(methodId);
        } else {
            impl_->verifiedFail->insert(methodId);
        }
    }
}

VerificationResultCache::Status VerificationResultCache::Check(uint64_t methodId)
{
    if (Enabled()) {
        if (impl_->verifiedOk->count(methodId) > 0) {
            return Status::OK;
        }
        if (impl_->verifiedFail->count(methodId) > 0) {
            return Status::FAILED;
        }
    }
    return Status::UNKNOWN;
}

}  // namespace ark::verifier
