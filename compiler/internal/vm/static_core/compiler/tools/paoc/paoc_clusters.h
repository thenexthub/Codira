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

#ifndef COMPILER_TOOLS_PAOC_PAOC_CLUSTERS_H
#define COMPILER_TOOLS_PAOC_PAOC_CLUSTERS_H

#include <unordered_map>
#include <string_view>
#include <string>

#include "libarkbase/utils/json_parser.h"
#include "libarkbase/utils/pandargs.h"
#include "libarkbase/utils/logger.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_PAOC_CLUSTERS(level) LOG(level, COMPILER) << "PAOC_CLUSTERS: "

namespace ark {

/**
 * Implements `--paoc-clusters` option.
 * Stores clusters themselves  and "function:cluster" relations
 */
class PaocClusters {
    using JsonObjPointer = JsonObject::JsonObjPointer;
    using StringT = JsonObject::StringT;
    using ArrayT = JsonObject::ArrayT;
    using NumT = JsonObject::NumT;
    using Key = JsonObject::Key;

public:
    static constexpr std::string_view CLUSTERS_MAP_OBJ_NAME {"clusters_map"};  // Contains `method-clusters` relations
    static constexpr std::string_view CLUSTERS_OBJ_NAME {"compiler_options"};  // Contains `cluster-options` relations
    class OptionsCluster;

    const std::vector<OptionsCluster *> *Find(const std::string &method)
    {
        auto iter = specialOptions_.find(method);
        if (iter == specialOptions_.end()) {
            return nullptr;
        }
        LOG_PAOC_CLUSTERS(INFO) << "Found clusters for method `" << method << "`";
        return &iter->second;
    }

    void Init(const JsonObject &obj, PandArgParser *paParser)
    {
        ASSERT(paParser != nullptr);
        InitClusters(obj, paParser);
        InitClustersMap(obj);
    }

    void InitClusters(const JsonObject &mainObj, PandArgParser *paParser)
    {
        if (mainObj.GetValue<JsonObjPointer>(Key(CLUSTERS_OBJ_NAME)) == nullptr) {
            LOG_PAOC_CLUSTERS(FATAL) << "Can't find `" << CLUSTERS_OBJ_NAME << "` object";
        }
        const auto clustersJson = mainObj.GetValue<JsonObjPointer>(Key(CLUSTERS_OBJ_NAME))->get();

        // Fill clusters_ in order of presence in JSON-obj:
        size_t nClusters = clustersJson->GetSize();
        for (size_t idx = 0; idx < nClusters; idx++) {
            ASSERT(clustersJson->GetValue<JsonObjPointer>(idx) != nullptr);
            const auto clusterJson = clustersJson->GetValue<JsonObjPointer>(idx)->get();
            if (clusterJson == nullptr) {
                LOG_PAOC_CLUSTERS(FATAL) << "Can't find a cluster (idx = " << idx << ")";
            }
            clusters_.emplace_back(clustersJson->GetKeyByIndex(idx));

            // Fill current cluster:
            const auto &optionsJson = clusterJson->GetUnorderedMap();
            // Add option-value pair:
            for (const auto &p : optionsJson) {
                const auto &optionName = p.first;
                const auto *optionValue = clusterJson->GetValueSourceString(optionName);
                if (optionValue == nullptr || optionValue->empty()) {
                    LOG_PAOC_CLUSTERS(FATAL) << "Can't find option's value (cluster `"
                                             << clustersJson->GetKeyByIndex(idx) << "`, option `" << optionName << "`)";
                }

                auto *option = paParser->GetPandArg(optionName);
                if (option == nullptr) {
                    LOG_PAOC_CLUSTERS(FATAL) << "Unknown option: `" << optionName << "`";
                }
                ASSERT(optionValue != nullptr);
                auto value = OptionsCluster::ParseOptionValue(*optionValue, option, paParser);
                clusters_.back().GetVector().emplace_back(option, std::move(value));
            }
        }
    }

    void InitClustersMap(const JsonObject &mainObj)
    {
        if (mainObj.GetValue<JsonObjPointer>(Key(CLUSTERS_MAP_OBJ_NAME)) == nullptr) {
            LOG_PAOC_CLUSTERS(FATAL) << "Can't find `" << CLUSTERS_MAP_OBJ_NAME << "` object";
        }
        const auto clustersMapJson = mainObj.GetValue<JsonObjPointer>(Key(CLUSTERS_MAP_OBJ_NAME))->get();

        // Fill special_options_:
        for (const auto &p : clustersMapJson->GetUnorderedMap()) {
            const auto &methodName = p.first;
            const auto &clustersArray = p.second;
            const auto *curClustersJson = clustersArray.Get<ArrayT>();

            if (curClustersJson == nullptr) {
                LOG_PAOC_CLUSTERS(FATAL) << "Can't get clusters array for method `" << methodName << "`";
            }
            auto &curClusters = specialOptions_.try_emplace(methodName).first->second;
            ASSERT(curClustersJson != nullptr);
            for (const auto &idx : *curClustersJson) {
                // Cluster may be referenced by integer number (cluster's order) or string (cluster's name):
                const auto *numIdx = idx.Get<NumT>();
                const auto *strIdx = idx.Get<StringT>();
                size_t clusterIdx = 0;
                if (numIdx != nullptr) {
                    clusterIdx = static_cast<size_t>(*numIdx);
                } else if (strIdx != nullptr) {
                    const auto clustersJson = mainObj.GetValue<JsonObjPointer>(Key(CLUSTERS_OBJ_NAME))->get();
                    clusterIdx = static_cast<size_t>(clustersJson->GetIndexByKey(*strIdx));
                    ASSERT(clusterIdx != static_cast<size_t>(-1));
                } else {
                    LOG_PAOC_CLUSTERS(FATAL) << "Incorrect reference to a cluster for `" << methodName << "`";
                    UNREACHABLE();
                }
                if (clusterIdx >= clusters_.size()) {
                    LOG_PAOC_CLUSTERS(FATAL) << "Cluster's index out of range for `" << methodName << "`";
                }
                curClusters.push_back(&clusters_[clusterIdx]);
                ASSERT(curClusters.back() != nullptr);
            }
        }
    }

    /**
     * Implements a cluster and contains info related to it.
     * (i.e. vector of "option:alternative_value" pairs)
     */
    class OptionsCluster {
    public:
        // Variant of possible PandArg types:
        using ValueVariant = std::variant<std::string, int, double, bool, arg_list_t, uint32_t, uint64_t>;

        // NOLINTNEXTLINE(modernize-pass-by-value)
        explicit OptionsCluster(const std::string &clusterName) : clusterName_(clusterName) {}

        void Apply()
        {
            for (auto &pairOptionValue : cluster_) {
                SwapValue(pairOptionValue.first, &pairOptionValue.second);
            }
        }
        void Restore()
        {
            for (auto &pairOptionValue : cluster_) {
                SwapValue(pairOptionValue.first, &pairOptionValue.second);
            }
        }

        static ValueVariant ParseOptionValue(const std::string_view &valueString, PandArgBase *option,
                                             PandArgParser *paParser)
        {
            ASSERT(option != nullptr);
            switch (option->GetType()) {
                case PandArgType::STRING:
                    return ParseOptionValue<std::string>(valueString, option, paParser);

                case PandArgType::INTEGER:
                    return ParseOptionValue<int>(valueString, option, paParser);

                case PandArgType::DOUBLE:
                    return ParseOptionValue<double>(valueString, option, paParser);

                case PandArgType::BOOL:
                    return ParseOptionValue<bool>(valueString, option, paParser);

                case PandArgType::LIST:
                    return ParseOptionValue<arg_list_t>(valueString, option, paParser);

                case PandArgType::UINT32:
                    return ParseOptionValue<uint32_t>(valueString, option, paParser);

                case PandArgType::UINT64:
                    return ParseOptionValue<uint64_t>(valueString, option, paParser);

                case PandArgType::NOTYPE:
                default:
                    UNREACHABLE();
            }
        }

        std::vector<std::pair<PandArgBase *const, ValueVariant>> &GetVector()
        {
            return cluster_;
        }

    private:
        template <typename T>
        static ValueVariant ParseOptionValue(const std::string_view &valueString, PandArgBase *optionBase,
                                             PandArgParser *paParser)
        {
            ASSERT(optionBase != nullptr);
            auto option = static_cast<PandArg<T> *>(optionBase);
            auto optionCopy = *option;
            paParser->ParseSingleArg(&optionCopy, valueString);
            return optionCopy.GetValue();
        }

        void SwapValue(PandArgBase *option, ValueVariant *value)
        {
            PandArgType typeId {static_cast<uint8_t>(value->index())};
            switch (typeId) {
                case PandArgType::STRING:
                    SwapValue<std::string>(option, value);
                    return;
                case PandArgType::INTEGER:
                    SwapValue<int>(option, value);
                    return;
                case PandArgType::DOUBLE:
                    SwapValue<double>(option, value);
                    return;
                case PandArgType::BOOL:
                    SwapValue<bool>(option, value);
                    return;
                case PandArgType::LIST:
                    SwapValue<arg_list_t>(option, value);
                    return;
                case PandArgType::UINT32:
                    SwapValue<uint32_t>(option, value);
                    return;
                case PandArgType::UINT64:
                    SwapValue<uint64_t>(option, value);
                    return;
                case PandArgType::NOTYPE:
                default:
                    UNREACHABLE();
            }
        }

        template <typename T>
        void SwapValue(PandArgBase *optionBase, ValueVariant *value)
        {
            auto *option = static_cast<PandArg<T> *>(optionBase);
            T temp(option->GetValue());
            ASSERT(std::holds_alternative<T>(*value));
            option->template SetValue<false>(*std::get_if<T>(value));
            *value = std::move(temp);
        }

    private:
        std::string clusterName_;
        std::vector<std::pair<PandArgBase *const, ValueVariant>> cluster_;
    };

    /**
     * Searches for a cluster for the specified method in @param clusters_info and applies it to the compiler.
     * On destruction, restores previously applied options.
     */
    class ScopedApplySpecialOptions {
    public:
        NO_COPY_SEMANTIC(ScopedApplySpecialOptions);
        NO_MOVE_SEMANTIC(ScopedApplySpecialOptions);

        explicit ScopedApplySpecialOptions(const std::string &method, PaocClusters *clustersInfo)
        {
            // Find clusters for required method:
            currentClusters_ = clustersInfo->Find(method);
            if (currentClusters_ == nullptr) {
                return;
            }
            // Apply clusters:
            for (auto cluster : *currentClusters_) {
                cluster->Apply();
            }
        }
        ~ScopedApplySpecialOptions()
        {
            if (currentClusters_ != nullptr) {
                for (auto cluster : *currentClusters_) {
                    cluster->Restore();
                }
            }
        }

    private:
        const std::vector<OptionsCluster *> *currentClusters_ {nullptr};
    };

private:
    std::vector<OptionsCluster> clusters_;
    std::unordered_map<std::string, std::vector<OptionsCluster *>> specialOptions_;
};

#undef LOG_PAOC_CLUSTERS

}  // namespace ark

#endif  // COMPILER_TOOLS_PAOC_CLUSTERS_H
