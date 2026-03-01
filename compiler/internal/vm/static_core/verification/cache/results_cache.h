/**
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

#ifndef PANDA_VERIFIER_CACHE_RESULTS_CACHE_H_
#define PANDA_VERIFIER_CACHE_RESULTS_CACHE_H_

#include <string>
#include <cstdint>

namespace ark::verifier {
class VerificationResultCache {
public:
    enum class Status { OK, FAILED, UNKNOWN };
    static void Initialize(const std::string &filename);
    static void Destroy(bool updateFile = true);
    static void CacheResult(uint64_t methodId, bool result);
    static Status Check(uint64_t methodId);
    static bool Enabled();

private:
    struct Impl;
    static Impl *impl_;
};
}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_CACHE_RESULTS_CACHE_H_
