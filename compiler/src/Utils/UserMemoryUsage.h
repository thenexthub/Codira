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

#ifndef CODIRA_USERMEMORYUSAGE_H
#define CODIRA_USERMEMORYUSAGE_H

#include <string>
#include <unordered_map>
#include <vector>

#include "UserBase.h"

namespace Codira {
class UserMemoryUsage : public UserBase {
public:
    UserMemoryUsage() = default;
    ~UserMemoryUsage() override
    {
        OutputResult();
    }
    static UserMemoryUsage& Instance()
    {
        static UserMemoryUsage single{};
        return single;
    }

    void Start(const std::string& title, const std::string& subtitle, const std::string& desc);
    void Stop(const std::string& title, const std::string& subtitle, const std::string& desc);

private:
    std::string GetJson() const override;
    std::string GetSuffix() const final
    {
        return ".mem.prof";
    }
    /**
     * @brief get current process(codec)'s memory usage at callsite
     *
     * @return float
     */
    static float Sampling();

    struct Info {
        std::string title;
        std::string subtitle;
        std::string desc;
        float start{0.};
        float end{0.};
        explicit Info(std::string title, std::string subtitle, std::string desc, float start)
            : title(std::move(title)), subtitle(std::move(subtitle)), desc(std::move(desc)), start(start)
        {
        }
    };

    std::vector<std::string> titleOrder;
    std::unordered_map<std::string, std::vector<Info>> titleInfoMap;
};
} // namespace Codira

#endif // CODIRA_USERMEMORYUSAGE_H
