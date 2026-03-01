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

#ifndef ANI_GTEST_TUPLE_OPS_H
#define ANI_GTEST_TUPLE_OPS_H

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class AniGTestTupleOps : public AniTest {
protected:
    ani_tuple_value GetTupleWithCheck(std::string_view moduleName, std::string_view tupleGetterName)
    {
        ani_tuple_value tuple;
        GetTupleWithCheckImpl(moduleName, tupleGetterName, &tuple);
        return tuple;
    }

private:
    void GetTupleWithCheckImpl(std::string_view moduleName, std::string_view tupleGetterName, ani_tuple_value *tuple)
    {
        auto ref = CallEtsFunction<ani_ref>(moduleName.data(), tupleGetterName.data());
        ani_boolean isUndefined = ANI_TRUE;
        ASSERT_EQ(env_->Reference_IsUndefined(ref, &isUndefined), ANI_OK);
        ASSERT_EQ(isUndefined, ANI_FALSE);
        *tuple = static_cast<ani_tuple_value>(ref);
    }

public:
    static constexpr int32_t LOOP_COUNT = 3;
};
}  // namespace ark::ets::ani::testing

#endif  // ANI_GTEST_TUPLE_OPS_H