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

#ifndef PANDA_PLUGINS_ETS_VERIFY_ANI_GTEST_H
#define PANDA_PLUGINS_ETS_VERIFY_ANI_GTEST_H

#include <gmock/gmock.h>

#include "ani_gtest.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_vm_options.h"

namespace ark::ets::ani::verify::testing {

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
// CC-OFFNXT(G.PRE.02,G.PRE.06) solid logic
#define ASSERT_ERROR_ANI_ARGS_MSG(methodName, testLines)                                                     \
    do {                                                                                                     \
        std::vector<std::string> lineList;                                                                   \
        std::string abortMsg = GetAndClearAbortMessage();                                                    \
        std::stringstream ss(abortMsg);                                                                      \
        std::string line;                                                                                    \
        while (getline(ss, line, '\n')) {                                                                    \
            lineList.push_back(line);                                                                        \
        }                                                                                                    \
        ASSERT_GT(lineList.size(), 2 + testLines.size()) << abortMsg; /* CC-OFF(G.PRE.02) solid logic */     \
        ASSERT_STREQ(lineList[0].c_str(), "DETECT AN ERROR WHEN USING ANI:") << abortMsg;                    \
        ASSERT_STREQ(lineList[1].c_str(), ("  ANI method: " + std::string(methodName)).c_str()) << abortMsg; \
        size_t i = 2;                                                                                        \
        for (auto &t : testLines) { /* CC-OFF(G.PRE.02) solid logic */                                       \
            auto &name = t.name;                                                                             \
            ASSERT_FALSE(name.empty()) << abortMsg;                                                          \
            ASSERT_THAT(lineList[i].c_str(), ::testing::HasSubstr((" " + name + ":").c_str())) << abortMsg;  \
            auto &type = t.type;                                                                             \
            ASSERT_FALSE(type.empty()) << abortMsg;                                                          \
            ASSERT_THAT(lineList[i].c_str(), ::testing::HasSubstr(("| " + type + " ").c_str())) << abortMsg; \
            auto &error = t.error;                                                                           \
            if (!error.has_value()) {                                                                        \
                ASSERT_THAT(lineList[i].c_str(), ::testing::HasSubstr(" | VALID")) << abortMsg;              \
            } else {                                                                                         \
                std::string invalidStr = " | INVALID: ";                                                     \
                ASSERT_THAT(lineList[i].c_str(), ::testing::HasSubstr(invalidStr.c_str())) << abortMsg;      \
                auto errStartPos = lineList[i].find(invalidStr.c_str());                                     \
                auto errStr = lineList[i].substr(errStartPos + invalidStr.size());                           \
                ASSERT_STREQ(error->c_str(), errStr.c_str()) << abortMsg;                                    \
            }                                                                                                \
            ++i;                                                                                             \
        }                                                                                                    \
        ASSERT_THAT(lineList[i].c_str(), ::testing::HasSubstr("  Called from:")) << abortMsg;                \
    } while (false)

// CC-OFFNXT(G.PRE.02,G.PRE.06) solid logic
#define ASSERT_ERROR_ANI_MSG(methodName, msg)                                                                \
    do {                                                                                                     \
        std::vector<std::string> lineList;                                                                   \
        std::string abortMsg = GetAndClearAbortMessage();                                                    \
        std::stringstream ss(abortMsg);                                                                      \
        std::string line;                                                                                    \
        while (getline(ss, line, '\n')) {                                                                    \
            lineList.push_back(line);                                                                        \
        }                                                                                                    \
        ASSERT_GT(lineList.size(), 3) << abortMsg;                                                           \
        ASSERT_STREQ(lineList[0].c_str(), "DETECT AN ERROR WHEN USING ANI:") << abortMsg;                    \
        ASSERT_STREQ(lineList[1].c_str(), ("  ANI method: " + std::string(methodName)).c_str()) << abortMsg; \
        ASSERT_THAT(lineList[2].c_str(), ::testing::HasSubstr(std::string_view(msg).data())) << abortMsg;    \
    } while (false)

// CC-OFFNXT(G.PRE.02-CPP) macro definition
#define ASSERT_NO_ABORT_MESSAGE()                         \
    do {                                                  \
        std::string abortMsg = GetAndClearAbortMessage(); \
        ASSERT_TRUE(abortMsg.empty()) << abortMsg;        \
    } while (false)
// NOLINTEND(cppcoreguidelines-macro-usage)

class VerifyAniTest : public ani::testing::AniTest {
public:
    void SetUp() override
    {
        AniTest::SetUp();

        const EtsVmOptions *vmOps = GetEtsVmOptions(Runtime::GetOptions());
        ASSERT_NE(vmOps, nullptr);
        ASSERT_EQ(vmOps->IsVerifyANI(), true);

        ASSERT_NE(PandaEtsVM::GetCurrent()->GetANIVerifier(), nullptr);
        PandaEtsVM::GetCurrent()->GetANIVerifier()->SetAbortHook(AbortHook, this);
    }

    void TearDown() override
    {
        PandaEtsVM::GetCurrent()->GetANIVerifier()->SetAbortHook(nullptr, nullptr);
        AniTest::TearDown();
        ASSERT_NO_ABORT_MESSAGE();
        ss_.str("");  // clean ss_
    }

    std::vector<ani_option> GetExtraAniOptions() override
    {
        return {{"--verify:ani", nullptr}};
    }

    struct TestLineInfo {
        std::string name;
        std::string type;
        std::optional<std::string> error = {};
    };

    std::string GetAndClearAbortMessage()
    {
        std::string msg = ss_.str();
        ss_.str("");  // clean ss_
        return msg;
    }

    void ThrowError()
    {
        ASSERT_NO_ABORT_MESSAGE();
        AniTest::ThrowError();
        ASSERT_NO_ABORT_MESSAGE();
    }

private:
    static void AbortHook(void *data, const std::string_view message)
    {
        auto self = reinterpret_cast<VerifyAniTest *>(data);
        self->ss_ << message;
    }

    std::stringstream ss_;
};

}  // namespace ark::ets::ani::verify::testing

#endif  // PANDA_PLUGINS_ETS_VERIFY_ANI_GTEST_H
