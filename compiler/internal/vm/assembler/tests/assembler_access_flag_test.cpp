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

#include <gtest/gtest.h>

#include "class_data_accessor-inl.h"

using namespace testing::ext;

namespace panda::pandasm {
class AssemblyEmitterTest : public testing::Test {
};

static const std::string SLOT_NUMBER = "L_ESSlotNumberAnnotation;";
static const std::string CONCURRENT_MODULE_REQUESTS = "L_ESConcurrentModuleRequestsAnnotation;";
static const std::string EXPECTED_PROPERTY = "L_ESExpectedPropertyCountAnnotation;";

/**
 * @tc.name: assembly_access_flag_test_001
 * @tc.desc: Verify class access flag.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(AssemblyEmitterTest, assembly_access_flag_test_001, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_class_access_flags.abc";
    auto file_to_verify = panda_file::File::Open(file_name);
    std::unique_ptr<const panda_file::File> file_;
    file_.swap(file_to_verify);

    const auto class_idx = file_->GetClasses();
    for (size_t i = 0; i < class_idx.size(); i++) {
        uint32_t class_id = class_idx[i];
        ASSERT(class_id < file_->GetHeader()->file_size);
        const panda_file::File::EntityId record_id {class_id};
        panda_file::ClassDataAccessor class_accessor {*file_, record_id};
        auto class_name = class_accessor.GetName();
        auto access_flag = class_accessor.GetAccessFlags();
        auto name = std::string(utf::Mutf8AsCString(class_name.data));
        if (name == SLOT_NUMBER || name == CONCURRENT_MODULE_REQUESTS || name == EXPECTED_PROPERTY) {
            EXPECT_EQ(access_flag, ACC_PUBLIC | ACC_ANNOTATION);
        } else {
            EXPECT_EQ(access_flag, ACC_PUBLIC);
        }
    }
}
}
