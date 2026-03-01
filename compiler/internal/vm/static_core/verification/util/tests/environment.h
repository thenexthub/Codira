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

#ifndef PANDA_VERIFIER_TESTS_ENV_HPP
#define PANDA_VERIFIER_TESTS_ENV_HPP

#include <unordered_map>
#include <string>
#include <variant>
#include <optional>

namespace ark::verifier::test {
using OptionValue = std::variant<std::string, int, bool>;

class EnvOptions {
public:
    std::optional<OptionValue> operator[](const std::string &name) const;
    template <typename T>
    T Get(const std::string &name, T deflt = T {}) const
    {
        auto val = (*this)[name];
        if (!val) {
            return deflt;
        }
        if (auto pntr = std::get_if<T>(&*val)) {
            return *pntr;
        }
        return deflt;
    }
    // NOLINTNEXTLINE(google-explicit-constructor)
    EnvOptions(const char *);
    EnvOptions(const EnvOptions &) = default;
    EnvOptions(EnvOptions &&) = default;
    EnvOptions &operator=(const EnvOptions &) = default;
    EnvOptions &operator=(EnvOptions &&) = default;
    ~EnvOptions() = default;

private:
    std::unordered_map<std::string, OptionValue> options_;
};
}  // namespace ark::verifier::test

#endif  // !PANDA_VERIFIER_TESTS_ENV_HPP
