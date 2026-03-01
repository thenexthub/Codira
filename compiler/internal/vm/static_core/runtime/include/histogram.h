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

#ifndef PANDA_RUNTIME_HISTOGRAM_H_
#define PANDA_RUNTIME_HISTOGRAM_H_

#include "libarkbase/utils/type_converter.h"
#include "libarkbase/macros.h"
#include "mem/panda_containers.h"
#include "mem/panda_string.h"

namespace ark {

/**
 * @brief Class for providing distribution statistics
 * Minimum, maximum, count, average, sum, dispersion
 */
template <class Value>
class SimpleHistogram {
public:
    explicit SimpleHistogram(helpers::ValueType typeOfValue = helpers::ValueType::VALUE_TYPE_OBJECT)
        : typeOfValue_(typeOfValue)
    {
    }

    /**
     *  @brief Add all element to statistics at the half-interval from @param start to @param finish
     *  @param start begin of values inclusive
     *  @param finish end of values ​​not inclusive
     */
    template <class ForwardIterator>
    SimpleHistogram(ForwardIterator start, ForwardIterator finish,
                    helpers::ValueType typeOfValue = helpers::ValueType::VALUE_TYPE_OBJECT);

    /**
     *  @brief Output the General statistics of Histogram
     *  @return PandaString with Sum, Avg, Max
     */
    PandaString GetGeneralStatistic() const;

    /**
     *  @brief Add @param element to statistics @param number of times
     *  @param element
     *  @param count
     */
    void AddValue(const Value &element, size_t number = 1);

    size_t GetCount() const
    {
        return count_;
    }

    Value GetSum() const
    {
        return sum_;
    }

    Value GetMin() const
    {
        return min_;
    }

    Value GetMax() const
    {
        return max_;
    }

    double GetAvg() const
    {
        if (count_ > 0U) {
            return sum_ / static_cast<double>(count_);
        }
        return 0;
    }

    double GetDispersion() const
    {
        return sumOfSquares_ / static_cast<double>(count_) - GetAvg() * GetAvg();
    }

    ~SimpleHistogram() = default;

    DEFAULT_COPY_SEMANTIC(SimpleHistogram);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(SimpleHistogram);

private:
    size_t count_ = 0;
    Value sum_ {};
    Value sumOfSquares_ {};
    Value min_ {};
    Value max_ {};
    helpers::ValueType typeOfValue_;
};

/**
 * @brief Class for providing distribution statistics
 * Minimum, maximum, count, average, sum, dispersion
 */
template <class Value>
class Histogram : public SimpleHistogram<Value> {
public:
    explicit Histogram(helpers::ValueType typeOfValue = helpers::ValueType::VALUE_TYPE_OBJECT)
        : SimpleHistogram<Value>(typeOfValue)
    {
    }

    /**
     *  @brief Add all element to statistics at the half-interval from @param start to @param finish
     *  @param start begin of values inclusive
     *  @param finish end of values ​​not inclusive
     */
    template <class ForwardIterator>
    Histogram(ForwardIterator start, ForwardIterator finish,
              helpers::ValueType typeOfValue = helpers::ValueType::VALUE_TYPE_OBJECT);

    /**
     *  @brief Output the first @param count_top of the lowest values by key
     *  with the number of their count
     *  @param count_top Number of first values to output
     *  @return PandaString with in format: "[key:count[,]]*"
     */
    PandaString GetTopDump(size_t countTop = DEFAULT_TOP_SIZE) const;

    /**
     *  @brief Add @param element to statistics @param number of times
     *  @param element
     *  @param count
     */
    void AddValue(const Value &element, size_t number = 1);

    size_t GetCountDifferent() const
    {
        return frequency_.size();
    }

private:
    PandaMap<Value, uint32_t> frequency_;

    static constexpr size_t DEFAULT_TOP_SIZE = 10;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_HISTOGRAM_H_
