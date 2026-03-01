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

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libpandabase/os/file.h"

namespace panda::test {

HWTEST(UnixFileTest, OpenAndGetFlagsTest, testing::ext::TestSize.Level0)
{
    remove("./test_openfile.txt");
    os::file::File file1 = os::file::Open("./test_openfile.txt", os::file::Mode::READONLY);
    ASSERT_FALSE(file1.IsValid());
    remove("./test_openfile.txt");
    os::file::File file2 = os::file::Open("./test_openfile.txt", os::file::Mode::READWRITE);
    ASSERT_FALSE(file2.IsValid());
    remove("./test_openfile.txt");
    os::file::File file3 = os::file::Open("./test_openfile.txt", os::file::Mode::WRITEONLY);
    ASSERT_TRUE(file3.IsValid());
    remove("./test_openfile.txt");
    os::file::File file4 = os::file::Open("./test_openfile.txt", os::file::Mode::READWRITECREATE);
    ASSERT_TRUE(file4.IsValid());
}

}  // namespace panda::test