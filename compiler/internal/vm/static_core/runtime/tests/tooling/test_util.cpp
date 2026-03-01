/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "test_util.h"

#include "include/method.h"
#include "include/tooling/pt_location.h"

namespace ark::tooling::test {

// NOLINTBEGIN(fuchsia-statically-constructed-objects)
TestMap TestUtil::testMap_;
os::memory::Mutex TestUtil::eventMutex_;
os::memory::ConditionVariable TestUtil::eventCv_;
DebugEvent TestUtil::lastEvent_ = DebugEvent::UNINITIALIZED;
bool TestUtil::initialized_ = false;
os::memory::Mutex TestUtil::suspendMutex_;
os::memory::ConditionVariable TestUtil::suspendCv_;
bool TestUtil::suspended_;
PtThread TestUtil::lastEventThread_ = PtThread(nullptr);
PtLocation TestUtil::lastEventLocation_("", EntityId(0), 0);
TestExtractorFactory *TestUtil::extractorFactory_;
// NOLINTEND(fuchsia-statically-constructed-objects)

int32_t TestUtil::GetValueRegister(Method *method, const char *varName, uint32_t offset)
{
    auto methodId = method->GetFileId();
    auto pf = method->GetPandaFile();
    PtLocation location(pf->GetFilename().c_str(), methodId, offset);
    auto extractor = extractorFactory_->MakeTestExtractor(pf);

    auto variables = extractor->GetLocalVariableInfo(location.GetMethodId(), location.GetBytecodeOffset());
    for (const auto &var : variables) {
        if (var.name == varName) {
            return var.regNumber;
        }
    }

    auto params = extractor->GetParameterInfo(location.GetMethodId());
    auto paramReg = method->GetNumVregs();

    for (const auto &param : params) {
        if (param.name == varName) {
            return paramReg;
        }
        paramReg++;
    }

    return -2;  // NOTE(maksenov): Replace with invalid register constant;
}

#ifndef PANDA_TARGET_MOBILE
std::ostream &operator<<(std::ostream &out, std::nullptr_t)
{
    return out << "nullptr";
}
#endif  // PANDA_TARGET_MOBILE

ApiTest::ApiTest()
{
    scenario = []() {
        ASSERT_EXITED();
        return true;
    };
}

void SetExtractorFactoryForTest(TestExtractorFactory *factory)
{
    TestUtil::SetExtractorFactory(factory);
}
}  // namespace ark::tooling::test
