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

#ifndef DPROF_CONVERTER_FEATURES_HOTNESS_COUNTERS_H
#define DPROF_CONVERTER_FEATURES_HOTNESS_COUNTERS_H

#include "libarkbase/macros.h"
#include "features_manager.h"
#include "dprof/storage.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/serializer/serializer.h"

#include <list>

namespace ark::dprof {
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
inline const char HCOUNTERS_FEATURE_NAME[] = "hotness_counters.v1";

class HCountersFunctor : public FeaturesManager::Functor {
    struct HCountersInfo {
        struct MethodInfo {
            std::string name;
            uint32_t value;
        };
        std::string appName;
        uint64_t hash;
        uint32_t pid;
        std::list<MethodInfo> methodsList;
    };

public:
    explicit HCountersFunctor(std::ostream &out) : out_(out) {}
    ~HCountersFunctor() = default;

    bool operator()(const AppData &appData, const std::vector<uint8_t> &data) override
    {
        std::unordered_map<std::string, uint32_t> methodInfoMap;
        if (!serializer::BufferToType(data.data(), data.size(), methodInfoMap)) {
            LOG(ERROR, DPROF) << "Cannot deserialize methodInfoMap";
            return false;
        }

        std::list<HCountersInfo::MethodInfo> methodsList;
        for (auto &it : methodInfoMap) {
            methodsList.emplace_back(HCountersInfo::MethodInfo {it.first, it.second});
        }

        hcountersInfoList_.emplace_back(
            HCountersInfo {appData.GetName(), appData.GetHash(), appData.GetPid(), std::move(methodsList)});

        return true;
    }

    bool ShowInfo(const std::string &format)
    {
        if (hcountersInfoList_.empty()) {
            return false;
        }

        if (format == "text") {
            ShowText();
        } else if (format == "json") {
            ShowJson();
        } else {
            LOG(ERROR, DPROF) << "Unknown format: " << format << std::endl;
            return false;
        }
        return true;
    }

private:
    void ShowText()
    {
        out_ << "Feature: " << HCOUNTERS_FEATURE_NAME << std::endl;
        for (auto &hcountersInfo : hcountersInfoList_) {
            out_ << "  app: name=" << hcountersInfo.appName << " pid=" << hcountersInfo.pid
                 << " hash=" << hcountersInfo.hash << std::endl;

            for (auto &methodInfo : hcountersInfo.methodsList) {
                out_ << "    " << methodInfo.name << ":" << methodInfo.value << std::endl;
            }
        }
    }

    void ShowJson()
    {
        out_ << "{" << std::endl;
        out_ << "  \"" << HCOUNTERS_FEATURE_NAME << "\": [" << std::endl;
        for (auto &hcountersInfo : hcountersInfoList_) {
            out_ << "    {" << std::endl;
            out_ << R"(      "app_name": ")" << hcountersInfo.appName << "\"," << std::endl;
            out_ << R"(      "pid": ")" << hcountersInfo.pid << "\"," << std::endl;
            out_ << R"(      "hash": ")" << hcountersInfo.hash << "\"," << std::endl;
            out_ << R"(      "counters": [)" << std::endl;
            for (auto &methodInfo : hcountersInfo.methodsList) {
                out_ << "        {" << std::endl;
                out_ << R"(          "name": ")" << methodInfo.name << "\"," << std::endl;
                out_ << R"(          "value": ")" << methodInfo.value << "\"" << std::endl;
                out_ << "        }";
                if (&methodInfo != &hcountersInfo.methodsList.back()) {
                    out_ << ",";
                }
                out_ << std::endl;
            }
            out_ << "      ]" << std::endl;
            out_ << "    }";
            if (&hcountersInfo != &hcountersInfoList_.back()) {
                out_ << ",";
            }
            out_ << std::endl;
        }
        out_ << "  ]" << std::endl;
        out_ << "}" << std::endl;
    }

    std::list<HCountersInfo> hcountersInfoList_;
    std::ostream &out_;

    NO_COPY_SEMANTIC(HCountersFunctor);
    NO_MOVE_SEMANTIC(HCountersFunctor);
};
}  // namespace ark::dprof

#endif  // DPROF_CONVERTER_FEATURES_HOTNESS_COUNTERS_H
