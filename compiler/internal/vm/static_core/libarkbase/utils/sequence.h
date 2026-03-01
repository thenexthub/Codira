/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef LIBPANDABASE_UTILS_SEQUENCE_H
#define LIBPANDABASE_UTILS_SEQUENCE_H

#include <cmath>
#include "libarkbase/macros.h"

namespace ark {
class Sequence {
public:
    void Add(double val)
    {
        ASSERT(std::isfinite(val));
        if (empty_) {
            mean_ = val;
            variance_ = 0.0;
            empty_ = false;
        } else {
            // Formula from "Incremental calculation of weighted mean and variance" by Tony Finch
            // diff := x - mean
            // incr := alpha * diff
            // mean := mean + incr
            // variance := (1 - alpha) * (variance + diff * incr)
            // PDF available at https://fanf2.user.srcf.net/hermes/doc/antiforgery/stats.pdf
            double diff = val - mean_;
            double incr = DEFAULT_INCREMENTAL_FACTOR * diff;
            mean_ += incr;
            variance_ = (1.0 - DEFAULT_INCREMENTAL_FACTOR) * (variance_ + diff * incr);
        }
    }

    double GetMean() const
    {
        return mean_;
    }

    double GetVariance() const
    {
        return variance_;
    }

    double GetStdDev() const
    {
        return sqrt(GetVariance());
    }

    bool IsEmpty() const
    {
        return empty_;
    }

private:
    bool empty_ {true};
    double mean_ {0.0};
    double variance_ {0.0};
    static constexpr double DEFAULT_INCREMENTAL_FACTOR = 0.3;
};
}  // namespace ark
#endif
