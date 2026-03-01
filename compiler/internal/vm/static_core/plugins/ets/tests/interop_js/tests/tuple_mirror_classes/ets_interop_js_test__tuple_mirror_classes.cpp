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
#include <cstddef>
#include "ets_interop_js_gtest.h"
#include "ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_tuple.h"
#include "runtime/include/thread_scopes.h"

namespace ark::ets::interop::js::testing {

namespace descriptors = panda_file_items::class_descriptors;

class EtsInteropTupleMirroredClassTest : public EtsInteropTest {};

struct MemberInfo {
    size_t offset;
    const char *name;
};

static void CheckOffsetOfFields(const char *className, const std::vector<MemberInfo> &membersList)
{
    ScopedManagedCodeThread scoped(ManagedThread::GetCurrent());

    EtsClassLinker *etsClassLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    EtsClass *klass = etsClassLinker->GetClass(className);
    ASSERT_NE(klass, nullptr);

    auto fields = klass->GetFields();
    ASSERT_EQ(fields.size(), membersList.size());

    for (size_t i = 0; i < fields.size(); i++) {
        ASSERT_NE(fields[i], nullptr);
        EXPECT_STREQ(fields[i]->GetNameString()->GetMutf8().data(), membersList[i].name);
    }
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple1)
{
    CheckOffsetOfFields(descriptors::TUPLE1.data(), []() -> std::vector<MemberInfo> {
        // just for get the offset
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple1MirroredClass = EtsTuple<1>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple1MirroredClass->GetElementOffset(0), "$0"},
        };
    }());
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple2)
{
    CheckOffsetOfFields(descriptors::TUPLE2.data(), []() -> std::vector<MemberInfo> {
        // just for get the offset
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple2MirroredClass = EtsTuple<2>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple2MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple2MirroredClass->GetElementOffset(1), "$1"},
        };
    }());
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple3)
{
    CheckOffsetOfFields(descriptors::TUPLE3.data(), []() -> std::vector<MemberInfo> {
        // just for get the offset
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple3MirroredClass = EtsTuple<3>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple3MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple3MirroredClass->GetElementOffset(1), "$1"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple3MirroredClass->GetElementOffset(2), "$2"},
        };
    }());
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple4)
{
    CheckOffsetOfFields(descriptors::TUPLE4.data(), []() -> std::vector<MemberInfo> {
        // just for get the offset
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple4MirroredClass = EtsTuple<4>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple4MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple4MirroredClass->GetElementOffset(1), "$1"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple4MirroredClass->GetElementOffset(2), "$2"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple4MirroredClass->GetElementOffset(3), "$3"},
        };
    }());
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple5)
{
    CheckOffsetOfFields(descriptors::TUPLE5.data(), []() -> std::vector<MemberInfo> {
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple5MirroredClass = EtsTuple<5>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple5MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple5MirroredClass->GetElementOffset(1), "$1"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple5MirroredClass->GetElementOffset(2), "$2"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple5MirroredClass->GetElementOffset(3), "$3"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple5MirroredClass->GetElementOffset(4), "$4"},
        };
    }());
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple6)
{
    CheckOffsetOfFields(descriptors::TUPLE6.data(), []() -> std::vector<MemberInfo> {
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple6MirroredClass = EtsTuple<6>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple6MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple6MirroredClass->GetElementOffset(1), "$1"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple6MirroredClass->GetElementOffset(2), "$2"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple6MirroredClass->GetElementOffset(3), "$3"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple6MirroredClass->GetElementOffset(4), "$4"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple6MirroredClass->GetElementOffset(5), "$5"},
        };
    }());
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple7)
{
    CheckOffsetOfFields(descriptors::TUPLE7.data(), []() -> std::vector<MemberInfo> {
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple7MirroredClass = EtsTuple<7>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple7MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple7MirroredClass->GetElementOffset(1), "$1"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple7MirroredClass->GetElementOffset(2), "$2"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple7MirroredClass->GetElementOffset(3), "$3"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple7MirroredClass->GetElementOffset(4), "$4"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple7MirroredClass->GetElementOffset(5), "$5"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple7MirroredClass->GetElementOffset(6), "$6"},
        };
    }());
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple8)
{
    CheckOffsetOfFields(descriptors::TUPLE8.data(), []() -> std::vector<MemberInfo> {
        // just for get the offset
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple8MirroredClass = EtsTuple<8>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple8MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple8MirroredClass->GetElementOffset(1), "$1"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple8MirroredClass->GetElementOffset(2), "$2"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple8MirroredClass->GetElementOffset(3), "$3"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple8MirroredClass->GetElementOffset(4), "$4"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple8MirroredClass->GetElementOffset(5), "$5"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple8MirroredClass->GetElementOffset(6), "$6"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple8MirroredClass->GetElementOffset(7), "$7"},
        };
    }());
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple9)
{
    CheckOffsetOfFields(descriptors::TUPLE9.data(), []() -> std::vector<MemberInfo> {
        // just for get the offset
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple9MirroredClass = EtsTuple<9>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple9MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple9MirroredClass->GetElementOffset(1), "$1"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple9MirroredClass->GetElementOffset(2), "$2"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple9MirroredClass->GetElementOffset(3), "$3"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple9MirroredClass->GetElementOffset(4), "$4"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple9MirroredClass->GetElementOffset(5), "$5"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple9MirroredClass->GetElementOffset(6), "$6"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple9MirroredClass->GetElementOffset(7), "$7"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple9MirroredClass->GetElementOffset(8), "$8"},
        };
    }());
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple10)
{
    CheckOffsetOfFields(descriptors::TUPLE10.data(), []() -> std::vector<MemberInfo> {
        // just for get the offset
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple10MirroredClass = EtsTuple<10>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple10MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple10MirroredClass->GetElementOffset(1), "$1"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple10MirroredClass->GetElementOffset(2), "$2"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple10MirroredClass->GetElementOffset(3), "$3"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple10MirroredClass->GetElementOffset(4), "$4"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple10MirroredClass->GetElementOffset(5), "$5"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple10MirroredClass->GetElementOffset(6), "$6"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple10MirroredClass->GetElementOffset(7), "$7"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple10MirroredClass->GetElementOffset(8), "$8"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple10MirroredClass->GetElementOffset(9), "$9"},
        };
    }());
}
TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple11)
{
    CheckOffsetOfFields(descriptors::TUPLE11.data(), []() -> std::vector<MemberInfo> {
        // just for get the offset
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple11MirroredClass = EtsTuple<11>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple11MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple11MirroredClass->GetElementOffset(1), "$1"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple11MirroredClass->GetElementOffset(2), "$2"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple11MirroredClass->GetElementOffset(3), "$3"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple11MirroredClass->GetElementOffset(4), "$4"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple11MirroredClass->GetElementOffset(5), "$5"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple11MirroredClass->GetElementOffset(6), "$6"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple11MirroredClass->GetElementOffset(7), "$7"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple11MirroredClass->GetElementOffset(8), "$8"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple11MirroredClass->GetElementOffset(9), "$9"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple11MirroredClass->GetElementOffset(10), "$10"},
        };
    }());
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple12)
{
    CheckOffsetOfFields(descriptors::TUPLE12.data(), []() -> std::vector<MemberInfo> {
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple12MirroredClass = EtsTuple<12>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple12MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple12MirroredClass->GetElementOffset(1), "$1"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple12MirroredClass->GetElementOffset(2), "$2"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple12MirroredClass->GetElementOffset(3), "$3"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple12MirroredClass->GetElementOffset(4), "$4"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple12MirroredClass->GetElementOffset(5), "$5"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple12MirroredClass->GetElementOffset(6), "$6"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple12MirroredClass->GetElementOffset(7), "$7"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple12MirroredClass->GetElementOffset(8), "$8"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple12MirroredClass->GetElementOffset(9), "$9"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple12MirroredClass->GetElementOffset(10), "$10"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple12MirroredClass->GetElementOffset(11), "$11"},
        };
    }());
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple13)
{
    CheckOffsetOfFields(descriptors::TUPLE13.data(), []() -> std::vector<MemberInfo> {
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple13MirroredClass = EtsTuple<13>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple13MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple13MirroredClass->GetElementOffset(1), "$1"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple13MirroredClass->GetElementOffset(2), "$2"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple13MirroredClass->GetElementOffset(3), "$3"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple13MirroredClass->GetElementOffset(4), "$4"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple13MirroredClass->GetElementOffset(5), "$5"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple13MirroredClass->GetElementOffset(6), "$6"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple13MirroredClass->GetElementOffset(7), "$7"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple13MirroredClass->GetElementOffset(8), "$8"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple13MirroredClass->GetElementOffset(9), "$9"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple13MirroredClass->GetElementOffset(10), "$10"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple13MirroredClass->GetElementOffset(11), "$11"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple13MirroredClass->GetElementOffset(12), "$12"},
        };
    }());
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple14)
{
    CheckOffsetOfFields(descriptors::TUPLE14.data(), []() -> std::vector<MemberInfo> {
        // just for get the offset
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple14MirroredClass = EtsTuple<14>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple14MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple14MirroredClass->GetElementOffset(1), "$1"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple14MirroredClass->GetElementOffset(2), "$2"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple14MirroredClass->GetElementOffset(3), "$3"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple14MirroredClass->GetElementOffset(4), "$4"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple14MirroredClass->GetElementOffset(5), "$5"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple14MirroredClass->GetElementOffset(6), "$6"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple14MirroredClass->GetElementOffset(7), "$7"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple14MirroredClass->GetElementOffset(8), "$8"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple14MirroredClass->GetElementOffset(9), "$9"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple14MirroredClass->GetElementOffset(10), "$10"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple14MirroredClass->GetElementOffset(11), "$11"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple14MirroredClass->GetElementOffset(12), "$12"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple14MirroredClass->GetElementOffset(13), "$13"},
        };
    }());
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple15)
{
    CheckOffsetOfFields(descriptors::TUPLE15.data(), []() -> std::vector<MemberInfo> {
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple15MirroredClass = EtsTuple<15>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(1), "$1"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(2), "$2"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(3), "$3"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(4), "$4"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(5), "$5"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(6), "$6"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(7), "$7"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(8), "$8"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(9), "$9"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(10), "$10"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(11), "$11"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(12), "$12"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(13), "$13"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple15MirroredClass->GetElementOffset(14), "$14"},
        };
    }());
}

TEST_F(EtsInteropTupleMirroredClassTest, Filed_Tuple16)
{
    CheckOffsetOfFields(descriptors::TUPLE16.data(), []() -> std::vector<MemberInfo> {
        // just for get the offset
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto tuple16MirroredClass = EtsTuple<16>::FromEtsObject(nullptr);
        return std::vector<MemberInfo> {
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(0), "$0"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(1), "$1"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(2), "$2"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(3), "$3"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(4), "$4"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(5), "$5"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(6), "$6"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(7), "$7"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(8), "$8"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(9), "$9"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(10), "$10"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(11), "$11"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(12), "$12"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(13), "$13"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(14), "$14"},
            // NOLINTNEXTLINE(readability-magic-numbers)
            MemberInfo {tuple16MirroredClass->GetElementOffset(15), "$15"},
        };
    }());
}

}  // namespace ark::ets::interop::js::testing
