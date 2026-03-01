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

#ifndef PANDA_RUNTIME_HISTOGRAM_INL_H_
#define PANDA_RUNTIME_HISTOGRAM_INL_H_

#include "histogram.h"
#include "mem/panda_containers.h"
#include "mem/panda_string.h"

namespace ark {

template <class Value>
template <class ForwardIterator>
SimpleHistogram<Value>::SimpleHistogram(ForwardIterator start, ForwardIterator finish, helpers::ValueType typeOfValue)
    : typeOfValue_(typeOfValue)
{
    for (auto it = start; it != finish; ++it) {
        AddValue(*it);
    }
}

template <class Value>
PandaString SimpleHistogram<Value>::GetGeneralStatistic() const
{
    PandaStringStream statistic;
    statistic << "Sum: " << helpers::ValueConverter(sum_, typeOfValue_) << " ";
    statistic << "Avg: " << helpers::ValueConverter(GetAvg(), typeOfValue_) << " ";
    statistic << "Max: " << helpers::ValueConverter(max_, typeOfValue_);
    return statistic.str();
}

template <class Value>
void SimpleHistogram<Value>::AddValue(const Value &element, size_t number)
{
    sum_ += element * Value(number);
    sumOfSquares_ += element * element * Value(number);
    if (count_ == 0) {
        min_ = element;
        max_ = element;
    } else {
        min_ = std::min(min_, element);
        max_ = std::max(max_, element);
    }
    count_ += number;
}

template <class Value>
template <class ForwardIterator>
Histogram<Value>::Histogram(ForwardIterator start, ForwardIterator finish, helpers::ValueType typeOfValue)
    : SimpleHistogram<Value>(typeOfValue)
{
    for (auto it = start; it != finish; ++it) {
        AddValue(*it);
    }
}

template <class Value>
PandaString Histogram<Value>::GetTopDump(size_t countTop) const
{
    PandaStringStream statistic;
    bool first = true;
    for (auto it : frequency_) {
        if (countTop-- == 0) {
            break;
        }
        if (!first) {
            statistic << ",";
        }
        statistic << it.first << ":" << it.second;
        first = false;
    }
    return statistic.str();
}

template <class Value>
void Histogram<Value>::AddValue(const Value &element, size_t number)
{
    frequency_[element] += number;
    SimpleHistogram<Value>::AddValue(element, number);
}

}  // namespace ark

#endif  // PANDA_RUNTIME_HISTOGRAM_INL_H_
