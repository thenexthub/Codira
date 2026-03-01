/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "os/library_loader.h"

namespace panda::test {

HWTEST(LibraryLoaderTest, LoadTest, testing::ext::TestSize.Level0)
{
    remove("./test_library_loader.txt");
    auto res = os::library_loader::Load("./test_library_loader.txt");
    ASSERT_FALSE(res.HasValue());
    ASSERT_EQ(res.Error().ToString(),
              "./test_library_loader.txt: cannot open shared object file: No such file or directory");
}

}  // namespace panda::test
