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

#include "libarkbase/utils/utils.h"
#include "include/histogram-inl.h"
#include "include/runtime.h"
#include <gtest/gtest.h>

namespace ark::test {

class HistogramTest : public testing::Test {
public:
    HistogramTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~HistogramTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(HistogramTest);
    NO_MOVE_SEMANTIC(HistogramTest);

    template <class Value>
    void CompareTwoHistogram(const Histogram<Value> &lhs, const Histogram<Value> &rhs)
    {
        ASSERT_EQ(lhs.GetSum(), rhs.GetSum());
        ASSERT_EQ(lhs.GetMin(), rhs.GetMin());
        ASSERT_EQ(lhs.GetMax(), rhs.GetMax());
        ASSERT_EQ(lhs.GetAvg(), rhs.GetAvg());
        ASSERT_EQ(lhs.GetCount(), rhs.GetCount());
    }

    // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
    struct IntWrapper {
        IntWrapper() = default;
        ~IntWrapper() = default;
        explicit IntWrapper(int newElement) : element_(newElement) {}
        IntWrapper(const IntWrapper &newWrapper) = default;
        IntWrapper &operator=(const IntWrapper &newWrapper) = default;
        bool operator<(const IntWrapper &otherWrapper) const
        {
            return element_ < otherWrapper.element_;
        }
        bool operator==(const IntWrapper &otherWrapper) const
        {
            return element_ == otherWrapper.element_;
        }
        double operator/(double divider) const
        {
            return element_ / divider;
        }
        IntWrapper operator+(const IntWrapper &otherWrapper) const
        {
            return IntWrapper(element_ + otherWrapper.element_);
        }
        void operator+=(const IntWrapper &otherWrapper)
        {
            element_ += otherWrapper.element_;
        }
        IntWrapper operator*(const IntWrapper &otherWrapper) const
        {
            return IntWrapper(element_ * otherWrapper.element_);
        }

        std::ostream &operator<<(std::ostream &os)
        {
            return os << element_;
        }

    private:
        int element_ {};
    };

private:
    ark::MTManagedThread *thread_;
};

// NOLINTBEGIN(readability-magic-numbers)

TEST_F(HistogramTest, SimpleIntTest)
{
    std::vector<int> simpleVector = {1_I, 1515_I, -12_I, 130_I, -1_I, 124_I, 0_I};
    Histogram<int> hist;
    for (auto element : simpleVector) {
        hist.AddValue(element);
    }
    CompareTwoHistogram(hist, Histogram<int>(simpleVector.begin(), simpleVector.end()));
    ASSERT_EQ(hist.GetSum(), 1757_I);
    ASSERT_EQ(hist.GetMin(), -12_I);
    ASSERT_EQ(hist.GetMax(), 1515_I);
    ASSERT_EQ(hist.GetAvg(), 251_I);
    ASSERT_EQ(hist.GetDispersion(), 269520_I);
    ASSERT_EQ(hist.GetCount(), 7_I);
}

TEST_F(HistogramTest, IntWrapperTest)
{
    Histogram<IntWrapper> hist;
    std::vector<int> simpleVector = {1_I, 1515_I, -12_I, 129_I, 0_I, 124_I, 0_I};
    for (auto element : simpleVector) {
        hist.AddValue(IntWrapper(element));
    }
    ASSERT_EQ(hist.GetSum(), IntWrapper(1757_I));
    ASSERT_EQ(hist.GetMin(), IntWrapper(-12_I));
    ASSERT_EQ(hist.GetMax(), IntWrapper(1515_I));
    ASSERT_EQ(hist.GetAvg(), 251_I);
    ASSERT_EQ(hist.GetCount(), 7_I);
}

TEST_F(HistogramTest, CompareTwoDifferentTest)
{
    std::vector<int> simpleVectorFirst = {1_I, 1515_I, -12_I, 129_I, 0_I, 124_I, 0_I};
    std::vector<int> simpleVectorSecond = {1_I, 1515_I, -12_I, 130_I, 3_I, 120_I, 0_I};
    Histogram<int> histFirst(simpleVectorFirst.begin(), simpleVectorFirst.end());
    Histogram<int> histSecond(simpleVectorSecond.begin(), simpleVectorSecond.end());
    CompareTwoHistogram(histFirst, histSecond);
}

TEST_F(HistogramTest, CompareDifferentTypeTest)
{
    std::unordered_set<int> simpleSetFirst = {1_I, 1515_I, -12_I, 130_I, -1_I, 124_I, 0_I};
    PandaSet<int> pandaSetFirst = {1_I, 1515_I, -12_I, 129_I, 2_I, 122_I, 0_I};

    std::vector<int> simpleVectorSecond = {1_I, 1515_I, -12_I, 129_I, 0_I, 124_I, 0_I};
    PandaVector<int> pandaVectorFirst = {5_I, 1515_I, -12_I, 128_I, -3_I, 124_I, 0_I};

    Histogram<int> histFirst(simpleSetFirst.begin(), simpleSetFirst.end());
    Histogram<int> histSecond(pandaSetFirst.begin(), pandaSetFirst.end());
    Histogram<int> histThird(simpleVectorSecond.begin(), simpleVectorSecond.end());
    Histogram<int> histFourth(pandaVectorFirst.begin(), pandaVectorFirst.end());

    CompareTwoHistogram(histFirst, histSecond);
    CompareTwoHistogram(histFirst, histThird);
    CompareTwoHistogram(histFirst, histFourth);
    CompareTwoHistogram(histSecond, histThird);
    CompareTwoHistogram(histSecond, histFourth);
    CompareTwoHistogram(histThird, histFourth);
}

TEST_F(HistogramTest, CheckGetTopDumpTest)
{
    std::vector<int> simpleVector = {1_I, 1_I, 0_I, 12_I, 0_I, 1_I, 12_I};
    Histogram<int> hist(simpleVector.begin(), simpleVector.end());
    ASSERT_EQ(hist.GetTopDump(), "0:2,1:3,12:2");
    ASSERT_EQ(hist.GetTopDump(2U), "0:2,1:3");
    ASSERT_EQ(hist.GetTopDump(1U), "0:2");
    ASSERT_EQ(hist.GetTopDump(0U), "");
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::test
