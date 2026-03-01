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

#include "abckit_wrapper/file_view.h"

#include "name_cache/name_cache_parser.h"
#include "name_cache/name_cache_keeper.h"
#include "name_mapping/name_mapping_manager.h"
#include "obfuscator/element_preprocessor.h"
#include "obfuscator/member_preprocessor.h"

using namespace testing::ext;

class ObfuscatorPreprocessorTest : public ::testing::Test {
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    void Init(const std::string &abcPath, const std::string &nameCachePath)
    {
        ASSERT_EQ(fileView_.Init(abcPath), AbckitWrapperErrorCode::ERR_SUCCESS);

        ark::guard::NameCacheParser parser(nameCachePath);
        parser.Parse();

        ark::guard::NameCacheKeeper keeper(fileView_);
        keeper.Process(parser.GetNameCache());
    }

    abckit_wrapper::FileView fileView_;
};


/**
 * @tc.name: obfuscator_preprocess_test_001
 * @tc.desc: test member obfuscator preprocessor
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorPreprocessorTest, obfuscator_preprocess_test_001, TestSize.Level0)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator_preprocess/preprocess_test_001.abc";
    std::string applyNameCache = GUARD_UNIT_TEST_DIR "ut/obfuscator_preprocess/preprocess_test_001.json";

    this->Init(abcFilePath, applyNameCache);

    ark::guard::NameMappingManager nameMappingManager;
    ark::guard::MemberPreProcessor preProcessor(nameMappingManager);
    ASSERT_TRUE(preProcessor.Process(this->fileView_));

    // Interface1 field1, ClassA field1, B1 fieldB1, B2 fieldB2 --> descriptor: ""
    auto nameMapping = nameMappingManager.GetNameMapping("");
    ASSERT_TRUE(nameMapping != nullptr);
    auto obfName = nameMapping->GetName("xxx");
    ASSERT_NE(obfName, "a");
    ASSERT_NE(obfName, "b");
    ASSERT_NE(obfName, "c");
    ASSERT_EQ(obfName, "d");

    // Interface1 foo1, ClassA foo1 --> descriptor: "()"
    nameMapping = nameMappingManager.GetNameMapping("()");
    ASSERT_TRUE(nameMapping != nullptr);
    obfName = nameMapping->GetName("xxx");
    ASSERT_NE(obfName, "a");
    ASSERT_EQ(obfName, "b");
}

/**
 * @tc.name: obfuscator_preprocess_test_002
 * @tc.desc: test element obfuscator preprocessor
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorPreprocessorTest, obfuscator_preprocess_test_002, TestSize.Level0)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator_preprocess/preprocess_test_002.abc";
    std::string applyNameCache = GUARD_UNIT_TEST_DIR "ut/obfuscator_preprocess/preprocess_test_002.json";

    this->Init(abcFilePath, applyNameCache);

    ark::guard::NameMappingManager nameMappingManager;
    ark::guard::ElementPreProcessor preProcessor(nameMappingManager);
    ASSERT_TRUE(preProcessor.Process(this->fileView_));

    // field1, ns1, cl1, anno1 --> descriptor: "preprocess_test_002"
    auto nameMapping = nameMappingManager.GetNameMapping("preprocess_test_002");
    ASSERT_TRUE(nameMapping != nullptr);
    auto obfName = nameMapping->GetName("xxx");
    ASSERT_NE(obfName, "a");
    ASSERT_NE(obfName, "b");
    ASSERT_NE(obfName, "c");
    ASSERT_NE(obfName, "d");
    ASSERT_EQ(obfName, "e");

    // foo1 --> descriptor: "preprocess_test_002()"
    nameMapping = nameMappingManager.GetNameMapping("preprocess_test_002()");
    ASSERT_TRUE(nameMapping != nullptr);
    obfName = nameMapping->GetName("xxx");
    ASSERT_NE(obfName, "a"); // main --> a
    ASSERT_EQ(obfName, "b");
}
