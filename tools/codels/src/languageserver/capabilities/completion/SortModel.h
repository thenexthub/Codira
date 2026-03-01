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

#ifndef LSPSERVER_SORTMODEL_H
#define LSPSERVER_SORTMODEL_H

#include <algorithm>
#include <cstdint>
#include <string>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>
#include "CompletionImpl.h"

namespace ark {
class SortModel {
public:
    static constexpr double BASE_SCORE = 1.0;
    static constexpr double LENGTH_WEIGHT = 0.3;
    static constexpr double PREFIX_POSITION_WEIGHT = 0.4;
    static constexpr double CONTINUITY_WEIGHT = 0.3;

    explicit SortModel(double edw = 0.4, double spw = 0.3, double stw = 0.1, double ufw = 0.2)
        : editDistanceWeight(edw), scopePathWeight(spw), symbolTypeWeight(stw), usageFrequencyWeight(ufw)
    {
    }

    void UpdateUsageFrequency(const std::string &item) { usageFrequency[item]++; }

    double CalculateScore(const CodeCompletion &item, const std::string &prefix, uint8_t cursorDepth);

private:
    double editDistanceWeight;
    double scopePathWeight;
    double symbolTypeWeight;
    double usageFrequencyWeight;

    std::unordered_map<std::string, int> usageFrequency;

    static double CalculateMatchScore(std::string_view completion, std::string_view prefix);

    static double CalculateSymbolTypeScore(SortType type);

    static std::pair<double, double> CalculatePositionAndContinuityScores(std::string_view completion,
        std::string_view prefix);
};
} // namespace ark

#endif // LSPSERVER_SORTMODEL_H
