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

#ifndef PANDA_MEM_GC_G1_G1_PREDICTIONS_H
#define PANDA_MEM_GC_G1_G1_PREDICTIONS_H

#include "libarkbase/utils/sequence.h"

namespace ark::mem {
class G1Predictor {
public:
    explicit G1Predictor(double confidenceFactor) : confidenceFactor_(confidenceFactor) {}

    double Predict(const ark::Sequence &seq) const
    {
        if (seq.IsEmpty()) {
            return 0;
        }
        return seq.GetMean() + confidenceFactor_ * seq.GetStdDev();
    }

private:
    double confidenceFactor_;
};
}  // namespace ark::mem
#endif
