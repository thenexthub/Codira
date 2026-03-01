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

#include <string>

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libpandabase/os/native_stack.h"

namespace panda::test {

HWTEST(NativeStackTest, ReadAndWriteFileTest, testing::ext::TestSize.Level0)
{
    remove("./test_native_stack.txt");

    FILE* fp = fopen("./test_native_stack.txt", "a");
    ASSERT_NE(fp, nullptr);

    char buff[1024] = "abcdefg";
    ASSERT_TRUE(os::native_stack::WriterOsFile(reinterpret_cast<void *>(buff), static_cast<size_t>(strlen(buff)),
                                               fileno(fp)));

    std::string result = "";
    ASSERT_TRUE(os::native_stack::ReadOsFile("./test_native_stack.txt", &result));

    ASSERT_EQ(result, "abcdefg");

    fclose(fp);
}

}  // namespace panda::test
