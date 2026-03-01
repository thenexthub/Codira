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

#ifndef PANDA_LIBPANDABASE_UTILS_RANGE_H_
#define PANDA_LIBPANDABASE_UTILS_RANGE_H_
namespace ark {

template <class It>
class Range {
public:
    Range(It begin, It end) : begin_(begin), end_(end) {}

    // NOLINTNEXTLINE(readability-identifier-naming)
    It begin() const
    {
        return begin_;
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    It end() const
    {
        return end_;
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    size_t size() const
    {
        return std::distance(begin_, end_);
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    bool empty() const
    {
        return size() == 0;
    }

private:
    It begin_;
    It end_;
};

}  // namespace ark

#endif
