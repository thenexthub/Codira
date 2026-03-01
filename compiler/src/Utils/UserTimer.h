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

#ifndef CODIRA_USERTIMER_H
#define CODIRA_USERTIMER_H

#include <chrono>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include "UserBase.h"

namespace Codira {
class UserTimer : public UserBase {
public:
    UserTimer() = default;
    ~UserTimer() override
    {
        OutputResult();
    }
    static UserTimer& Instance()
    {
        static UserTimer single{};
        return single;
    }

    void Start(const std::string& title, const std::string& subtitle, const std::string& desc);
    void Stop(const std::string& title, const std::string& subtitle, const std::string& desc);

private:
    using ResultDataType = std::unordered_map<std::string, std::vector<std::pair<std::string, long int>>>;
    struct Info {
        std::string title;
        std::string subtitle;
        std::string desc;
        std::chrono::system_clock::time_point start;
        std::chrono::system_clock::time_point end;
        std::chrono::duration<double, std::milli> costMs{};
        bool isDone = false;
        explicit Info() = default;
        explicit Info(std::string title, std::string subtitle, std::string desc)
            : title(std::move(title)), subtitle(std::move(subtitle)), desc(std::move(desc))
        {
#if defined(__APPLE__) || defined(__MINGW64__)
            this->start = std::chrono::system_clock::now();
#else
            this->start = std::chrono::high_resolution_clock::now();
#endif
        }
    };
    std::pair<ResultDataType, std::vector<std::string>> GetDataAndOrder() const;
    std::string GetJson() const override;
    std::string GetSuffix() const final
    {
        return ".time.prof";
    }
    std::list<Info> infoList;
};
} // namespace Codira

#endif // CODIRA_USERTIMER_H
