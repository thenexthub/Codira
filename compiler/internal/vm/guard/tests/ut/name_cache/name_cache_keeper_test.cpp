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
#include "obfuscator/obfuscator_data_manager.h"

using namespace testing::ext;

namespace {
template <typename T>
void ValidateObfName(abckit_wrapper::FileView &fileView, const std::string &name, const std::string &expectedObfName)
{
    const auto object = fileView.Get<T>(name);
    ASSERT_TRUE(object.has_value());

    ark::guard::ObfuscatorDataManager manager(object.value());
    auto obfuscateData = manager.GetData();
    ASSERT_TRUE(obfuscateData != nullptr);
    auto obfName = obfuscateData->GetObfName();
    ASSERT_TRUE(!obfName.empty()) << "name cache keeper set failed, name:" << name;
    ASSERT_EQ(obfName, expectedObfName) << "not expected name, expect:" << expectedObfName
                                        << ", but actual is same. name:" << obfName;

    std::string oriName;
    if constexpr (std::is_same_v<T, abckit_wrapper::Namespace> || std::is_same_v<T, abckit_wrapper::Class>) {
        oriName = object.value()->GetName();
    } else if constexpr (std::is_same_v<T, abckit_wrapper::Method> || std::is_same_v<T, abckit_wrapper::Field>) {
        oriName = object.value()->GetRawName();
    }

    ASSERT_NE(oriName, obfName) << "expect different, but actual is same. name:" << obfName;
}
}  // namespace

/**
 * @tc.name: name_cache_keeper_test_001
 * @tc.desc: test whether the name cache keeper is working properly.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(NameCacheKeeperTest, name_cache_keeper_test_001, TestSize.Level0)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/name_cache/name_cache_keeper_test_001.abc";
    string applyNameCache = GUARD_UNIT_TEST_DIR "ut/name_cache/name_cache_keeper_test_001.json";
    ark::guard::NameCacheParser parser(applyNameCache);
    parser.Parse();

    abckit_wrapper::FileView fileView;
    ASSERT_EQ(fileView.Init(abcFilePath), AbckitWrapperErrorCode::ERR_SUCCESS);

    ark::guard::NameCacheKeeper keeper(fileView);
    keeper.Process(parser.GetNameCache());

    ValidateObfName<abckit_wrapper::Field>(fileView, "name_cache_keeper_test_001.field1", "a");
    ValidateObfName<abckit_wrapper::Method>(fileView, "name_cache_keeper_test_001.foo1:void;", "a");
    ValidateObfName<abckit_wrapper::Namespace>(fileView, "name_cache_keeper_test_001.ns1", "c");
    ValidateObfName<abckit_wrapper::Field>(fileView, "name_cache_keeper_test_001.ns1.nsField1", "a");
    ValidateObfName<abckit_wrapper::Method>(fileView, "name_cache_keeper_test_001.ns1.nsFoo1:void;", "a");
    ValidateObfName<abckit_wrapper::Class>(fileView, "name_cache_keeper_test_001.cl1", "b");
    ValidateObfName<abckit_wrapper::Field>(fileView, "name_cache_keeper_test_001.cl1.clField1", "a");
    ValidateObfName<abckit_wrapper::Method>(
        fileView, "name_cache_keeper_test_001.cl1.cl1Foo1:name_cache_keeper_test_001.cl1;void;", "a");
    ValidateObfName<abckit_wrapper::AnnotationInterface>(fileView, "name_cache_keeper_test_001.anno1", "d");
}

