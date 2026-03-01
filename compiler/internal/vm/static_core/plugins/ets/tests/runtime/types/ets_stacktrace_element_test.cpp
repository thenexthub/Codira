/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "ets_mirror_class_test_base.h"

#include "types/ets_class.h"
#include "types/ets_stacktrace_element.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsStackTraceElementTest : public EtsMirrorClassTestBase {
public:
    static std::vector<MirrorFieldInfo> GetMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsStackTraceElement, className_, "className"),
                                             MIRROR_FIELD_INFO(EtsStackTraceElement, methodName_, "methodName"),
                                             MIRROR_FIELD_INFO(EtsStackTraceElement, sourceFileName_, "sourceFileName"),
                                             MIRROR_FIELD_INFO(EtsStackTraceElement, lineNumber_, "lineNumber"),
                                             MIRROR_FIELD_INFO(EtsStackTraceElement, colNumber_, "colNumber")};
    }
};

TEST_F(EtsStackTraceElementTest, MemoryLayout)
{
    auto *klass = GetPlatformTypes()->coreStackTraceElement;
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}
}  // namespace ark::ets::test
