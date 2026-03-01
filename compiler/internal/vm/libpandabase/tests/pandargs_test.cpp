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

#include "utils/pandargs.h"

#include <gtest/gtest.h>

namespace panda::test {

static const bool REF_DEF_BOOL = false;
static const int REF_DEF_INT = 0;
static const double REF_DEF_DOUBLE = 1.0;
static const std::string REF_DEF_STRING = "noarg";
static const uint32_t REF_DEF_UINT32 = 0;
static const uint64_t REF_DEF_UINT64 = 0;
static const arg_list_t REF_DEF_DLIST = arg_list_t();
static const arg_list_t REF_DEF_LIST = arg_list_t();

PandArg<bool> PAB("bool", REF_DEF_BOOL, "Sample boolean argument");
PandArg<int> PAI("int", REF_DEF_INT, "Sample integer argument");
PandArg<double> PAD("double", REF_DEF_DOUBLE, "Sample rational argument");
PandArg<std::string> PAS("string", REF_DEF_STRING, "Sample string argument");
PandArg<uint32_t> PAU32("uint32", REF_DEF_UINT32, "Sample uint32 argument");
PandArg<uint64_t> PAU64("uint64", REF_DEF_UINT64, "Sample uint64 argument");
PandArg<arg_list_t> PALD("dlist", REF_DEF_DLIST, "Sample delimiter list argument", ":");
PandArg<arg_list_t> PAL("list", REF_DEF_LIST, "Sample list argument");

// The numbers -100 and 100 are used to initialize the range of the variable PAIR
PandArg<int> PAIR("rint", REF_DEF_INT, "Integer argument with range", -100, 100);

// The numbers 0 and 1000000000 are used to initialize the range of the variable PAUR32
PandArg<uint32_t> PAUR32("ruint32", REF_DEF_UINT64, "uint32 argument with range", 0, 1000000000);

// The numbers 0 and 100000000000 are used to initialize the range of the variable PAUR64
PandArg<uint64_t> PAUR64("ruint64", REF_DEF_UINT64, "uint64 argument with range", 0, 100000000000);

PandArgParser PA_PARSER;

HWTEST(libpandargs, TestAdd, testing::ext::TestSize.Level0)
{
    EXPECT_FALSE(PA_PARSER.Add(nullptr));
    EXPECT_TRUE(PA_PARSER.Add(&PAB));
    EXPECT_TRUE(PA_PARSER.Add(&PAI));
    EXPECT_TRUE(PA_PARSER.Add(&PAD));
    EXPECT_TRUE(PA_PARSER.Add(&PAS));
    EXPECT_TRUE(PA_PARSER.Add(&PAU32));
    EXPECT_TRUE(PA_PARSER.Add(&PAU64));
    EXPECT_TRUE(PA_PARSER.Add(&PALD));
    EXPECT_TRUE(PA_PARSER.Add(&PAL));
    EXPECT_TRUE(PA_PARSER.Add(&PAIR));
    EXPECT_TRUE(PA_PARSER.Add(&PAUR32));
    EXPECT_TRUE(PA_PARSER.Add(&PAUR64));
}

PandArg<bool> T_PAB("tail_bool", REF_DEF_BOOL, "Sample tail boolean argument");
PandArg<int> T_PAI("tail_int", REF_DEF_INT, "Sample tail integer argument");
PandArg<double> T_PAD("tail_double", REF_DEF_DOUBLE, "Sample tail rational argument");
PandArg<std::string> T_PAS("tail_string", REF_DEF_STRING, "Sample tail string argument");
PandArg<uint32_t> T_PAU32("tail_uint32", REF_DEF_UINT32, "Sample tail uint32 argument");
PandArg<uint64_t> T_PAU64("tail_uint64", REF_DEF_UINT64, "Sample tail uint64 argument");

// expect all arguments are set in parser
HWTEST(libpandargs, TestIsArgSet, testing::ext::TestSize.Level0)
{
    EXPECT_TRUE(PA_PARSER.IsArgSet(&PAB));
    EXPECT_TRUE(PA_PARSER.IsArgSet(PAB.GetName()));
    EXPECT_TRUE(PA_PARSER.IsArgSet(&PAI));
    EXPECT_TRUE(PA_PARSER.IsArgSet(PAI.GetName()));
    EXPECT_TRUE(PA_PARSER.IsArgSet(&PAD));
    EXPECT_TRUE(PA_PARSER.IsArgSet(PAD.GetName()));
    EXPECT_TRUE(PA_PARSER.IsArgSet(&PAS));
    EXPECT_TRUE(PA_PARSER.IsArgSet(PAS.GetName()));
    EXPECT_TRUE(PA_PARSER.IsArgSet(&PAU32));
    EXPECT_TRUE(PA_PARSER.IsArgSet(PAU32.GetName()));
    EXPECT_TRUE(PA_PARSER.IsArgSet(&PAU64));
    EXPECT_TRUE(PA_PARSER.IsArgSet(PAU64.GetName()));
    EXPECT_TRUE(PA_PARSER.IsArgSet(&PALD));
    EXPECT_TRUE(PA_PARSER.IsArgSet(PALD.GetName()));
    EXPECT_TRUE(PA_PARSER.IsArgSet(&PAL));
    EXPECT_TRUE(PA_PARSER.IsArgSet(PAL.GetName()));
    EXPECT_TRUE(PA_PARSER.IsArgSet(&PAIR));
    EXPECT_TRUE(PA_PARSER.IsArgSet(PAIR.GetName()));
    EXPECT_TRUE(PA_PARSER.IsArgSet(&PAUR32));
    EXPECT_TRUE(PA_PARSER.IsArgSet(PAUR32.GetName()));
    EXPECT_TRUE(PA_PARSER.IsArgSet(&PAUR64));
    EXPECT_TRUE(PA_PARSER.IsArgSet(PAUR64.GetName()));
}

// expect default values and types are consistent
HWTEST(libpandargs, TestConsistent, testing::ext::TestSize.Level0)
{
    EXPECT_EQ(PAB.GetDefaultValue(), REF_DEF_BOOL);
    EXPECT_EQ(PAB.GetDefaultValue(), PAB.GetValue());
    EXPECT_EQ(PAB.GetType(), PandArgType::BOOL);

    EXPECT_EQ(PAI.GetDefaultValue(), REF_DEF_INT);
    EXPECT_EQ(PAI.GetDefaultValue(), PAI.GetValue());
    EXPECT_EQ(PAI.GetType(), PandArgType::INTEGER);

    EXPECT_DOUBLE_EQ(PAD.GetValue(), REF_DEF_DOUBLE);
    EXPECT_DOUBLE_EQ(PAD.GetDefaultValue(), PAD.GetValue());
    EXPECT_EQ(PAD.GetType(), PandArgType::DOUBLE);

    EXPECT_EQ(PAS.GetDefaultValue(), REF_DEF_STRING);
    EXPECT_EQ(PAS.GetDefaultValue(), PAS.GetValue());
    EXPECT_EQ(PAS.GetType(), PandArgType::STRING);

    EXPECT_EQ(PAU32.GetDefaultValue(), REF_DEF_UINT32);
    EXPECT_EQ(PAU32.GetDefaultValue(), PAU32.GetValue());
    EXPECT_EQ(PAU32.GetType(), PandArgType::UINT32);

    EXPECT_EQ(PAU64.GetDefaultValue(), REF_DEF_UINT64);
    EXPECT_EQ(PAU64.GetDefaultValue(), PAU64.GetValue());
    EXPECT_EQ(PAU64.GetType(), PandArgType::UINT64);

    EXPECT_TRUE(PALD.GetValue().empty());
    EXPECT_EQ(PALD.GetDefaultValue(), PALD.GetValue());
    EXPECT_EQ(PALD.GetType(), PandArgType::LIST);

    EXPECT_TRUE(PAL.GetValue().empty());
    EXPECT_EQ(PAL.GetDefaultValue(), PAL.GetValue());
    EXPECT_EQ(PAL.GetType(), PandArgType::LIST);

    EXPECT_EQ(PAIR.GetDefaultValue(), REF_DEF_INT);
    EXPECT_EQ(PAIR.GetDefaultValue(), PAIR.GetValue());
    EXPECT_EQ(PAIR.GetType(), PandArgType::INTEGER);

    EXPECT_EQ(PAUR32.GetDefaultValue(), REF_DEF_UINT64);
    EXPECT_EQ(PAUR32.GetDefaultValue(), PAUR32.GetValue());
    EXPECT_EQ(PAUR32.GetType(), PandArgType::UINT32);

    EXPECT_EQ(PAUR64.GetDefaultValue(), REF_DEF_UINT64);
    EXPECT_EQ(PAUR64.GetDefaultValue(), PAUR64.GetValue());
    EXPECT_EQ(PAUR64.GetType(), PandArgType::UINT64);
}

// expect false on duplicate argument
HWTEST(libpandargs, TestDuplicate, testing::ext::TestSize.Level0)
{
    PandArg<int> pai_dup("int", 0, "Integer number 0");
    EXPECT_TRUE(PA_PARSER.IsArgSet(pai_dup.GetName()));
    EXPECT_FALSE(PA_PARSER.Add(&pai_dup));
}

// add tail argument, expect false on duplicate
// erase tail, expect 0 tail size
HWTEST(libpandargs, TestTail, testing::ext::TestSize.Level0)
{
    {
        EXPECT_EQ(PA_PARSER.GetTailSize(), 0U);
        EXPECT_TRUE(PA_PARSER.PushBackTail(&T_PAI));
        EXPECT_EQ(PA_PARSER.GetTailSize(), 1U);
        EXPECT_FALSE(PA_PARSER.PushBackTail(&T_PAI));
        PA_PARSER.PopBackTail();
        EXPECT_EQ(PA_PARSER.GetTailSize(), 0U);
    }

    {
        ASSERT_EQ(PA_PARSER.GetTailSize(), 0U);
        ASSERT_FALSE(PA_PARSER.PushBackTail(nullptr));
        ASSERT_EQ(PA_PARSER.GetTailSize(), 0U);
        ASSERT_FALSE(PA_PARSER.PopBackTail());
    }
}

// expect help string formed right
HWTEST(libpandargs, TestString, testing::ext::TestSize.Level0)
{
    std::string ref_string = "--" + PAB.GetName() + ": " + PAB.GetDesc() + "\n";
    ref_string += "--" + PALD.GetName() + ": " + PALD.GetDesc() + "\n";
    ref_string += "--" + PAD.GetName() + ": " + PAD.GetDesc() + "\n";
    ref_string += "--" + PAI.GetName() + ": " + PAI.GetDesc() + "\n";
    ref_string += "--" + PAL.GetName() + ": " + PAL.GetDesc() + "\n";
    ref_string += "--" + PAIR.GetName() + ": " + PAIR.GetDesc() + "\n";
    ref_string += "--" + PAUR32.GetName() + ": " + PAUR32.GetDesc() + "\n";
    ref_string += "--" + PAUR64.GetName() + ": " + PAUR64.GetDesc() + "\n";
    ref_string += "--" + PAS.GetName() + ": " + PAS.GetDesc() + "\n";
    ref_string += "--" + PAU32.GetName() + ": " + PAU32.GetDesc() + "\n";
    ref_string += "--" + PAU64.GetName() + ": " + PAU64.GetDesc() + "\n";
    EXPECT_EQ(PA_PARSER.GetHelpString(), ref_string);
}

// expect regular args list formed right
HWTEST(libpandargs, TestRegular, testing::ext::TestSize.Level0)
{
    arg_list_t ref_arg_dlist = PALD.GetValue();
    arg_list_t ref_arg_list = PAL.GetValue();
    std::string ref_string = "--" + PAB.GetName() + "=" + std::to_string(PAB.GetValue()) + "\n";
    ref_string += "--" + PALD.GetName() + "=";
    for (const auto &i : ref_arg_dlist) {
        ref_string += i + ", ";
    }
    ref_string += "\n";
    ref_string += "--" + PAD.GetName() + "=" + std::to_string(PAD.GetValue()) + "\n";
    ref_string += "--" + PAI.GetName() + "=" + std::to_string(PAI.GetValue()) + "\n";
    ref_string += "--" + PAL.GetName() + "=";
    for (const auto &i : ref_arg_list) {
        ref_string += i + ", ";
    }
    ref_string += "\n";
    ref_string += "--" + PAIR.GetName() + "=" + std::to_string(PAIR.GetValue()) + "\n";
    ref_string += "--" + PAUR32.GetName() + "=" + std::to_string(PAUR32.GetValue()) + "\n";
    ref_string += "--" + PAUR64.GetName() + "=" + std::to_string(PAUR64.GetValue()) + "\n";
    ref_string += "--" + PAS.GetName() + "=" + PAS.GetValue() + "\n";
    ref_string += "--" + PAU32.GetName() + "=" + std::to_string(PAU32.GetValue()) + "\n";
    ref_string += "--" + PAU64.GetName() + "=" + std::to_string(PAU64.GetValue()) + "\n";
    EXPECT_EQ(PA_PARSER.GetRegularArgs(), ref_string);
}

// expect all boolean values processed right
HWTEST(libpandargs, TestAllBoolean, testing::ext::TestSize.Level0)
{
    static const char *true_values[] = {"true", "on", "1"};
    static const char *false_values[] = {"false", "off", "0"};
    static const int argc_bool_only = 3;
    static const char *argv_bool_only[argc_bool_only];
    argv_bool_only[0] = "gtest_app";
    std::string s = "--" + PAB.GetName();
    argv_bool_only[1] = s.c_str();

    for (int i = 0; i < 3; i++) {
        argv_bool_only[2] = true_values[i];
        EXPECT_TRUE(PA_PARSER.Parse(argc_bool_only, argv_bool_only));
        EXPECT_TRUE(PAB.GetValue());
    }
    for (int i = 0; i < 3; i++) {
        argv_bool_only[2] = false_values[i];
        EXPECT_TRUE(PA_PARSER.Parse(argc_bool_only, argv_bool_only));
        EXPECT_FALSE(PAB.GetValue());
    }
}

// expect wrong boolean arguments with "=" processed right
HWTEST(libpandargs, TestWrongBoolean, testing::ext::TestSize.Level0)
{
    static const int argc_bool_only = 2;
    static const char *argv_bool_only[argc_bool_only];
    argv_bool_only[0] = "gtest_app";
    std::string s = "--" + PAB.GetName() + "=";
    argv_bool_only[1] = s.c_str();
    EXPECT_FALSE(PA_PARSER.Parse(argc_bool_only, argv_bool_only));
}

// expect boolean at the end of arguments line is true
HWTEST(libpandargs, TestBooleanEnd, testing::ext::TestSize.Level0)
{
    static const int argc_bool_only = 2;
    static const char *argv_bool_only[argc_bool_only];
    argv_bool_only[0] = "gtest_app";
    std::string s = "--" + PAB.GetName();
    argv_bool_only[1] = s.c_str();
    EXPECT_TRUE(PA_PARSER.Parse(argc_bool_only, argv_bool_only));
    EXPECT_TRUE(PAB.GetValue());
}

// expect positive and negative integer values processed right
HWTEST(libpandargs, TestInteger, testing::ext::TestSize.Level0)
{
    static const int ref_int_pos = 42422424;
    static const int ref_int_neg = -42422424;
    static const int argc_int_only = 3;
    static const char *argv_int_only[argc_int_only];
    argv_int_only[0] = "gtest_app";
    std::string s = "--" + PAI.GetName();
    argv_int_only[1] = s.c_str();
    argv_int_only[2] = "42422424";
    EXPECT_TRUE(PA_PARSER.Parse(argc_int_only, argv_int_only));
    EXPECT_EQ(PAI.GetValue(), ref_int_pos);
    argv_int_only[2] = "-42422424";
    EXPECT_TRUE(PA_PARSER.Parse(argc_int_only, argv_int_only));
    EXPECT_EQ(PAI.GetValue(), ref_int_neg);
}

// expect positive and negative double values processed right
HWTEST(libpandargs, TestDouble, testing::ext::TestSize.Level0)
{
    static const double ref_double_pos = 4242.2424;
    static const double ref_double_neg = -4242.2424;
    static const int argc_double_only = 3;
    static const char *argv_double_only[argc_double_only];
    argv_double_only[0] = "gtest_app";
    std::string s = "--" + PAD.GetName();
    argv_double_only[1] = s.c_str();
    argv_double_only[2] = "4242.2424";
    EXPECT_TRUE(PA_PARSER.Parse(argc_double_only, argv_double_only));
    EXPECT_EQ(PAD.GetValue(), ref_double_pos);
    argv_double_only[2] = "-4242.2424";
    EXPECT_TRUE(PA_PARSER.Parse(argc_double_only, argv_double_only));
    EXPECT_EQ(PAD.GetValue(), ref_double_neg);
}

// expect hex values processed right
HWTEST(libpandargs, TestHex, testing::ext::TestSize.Level0)
{
    static const uint64_t refUint64 = 274877906959;
    static const uint64_t refInt = 64;
    static const int argcUint64Int = 3;
    static const char* argvUint64Int[argcUint64Int];
    argvUint64Int[0] = "gtest_app";
    std::string s = "--" + PAU64.GetName();
    argvUint64Int[1] = s.c_str();
    argvUint64Int[2] = "0x400000000f";
    EXPECT_TRUE(PA_PARSER.Parse(argcUint64Int, argvUint64Int));
    EXPECT_EQ(PAU64.GetValue(), refUint64);
    argvUint64Int[2] = "0x40";
    EXPECT_TRUE(PA_PARSER.Parse(argcUint64Int, argvUint64Int));
    EXPECT_EQ(PAU64.GetValue(), refInt);
}

// expect uint32_t values processed right
HWTEST(libpandargs, TestUint32, testing::ext::TestSize.Level0)
{
    static const uint32_t ref_uint32_pos = 4242422424;
    static const int argc_uint32_only = 3;
    static const char *argv_uint32_only[argc_uint32_only];
    argv_uint32_only[0] = "gtest_app";
    std::string s = "--" + PAU32.GetName();
    argv_uint32_only[1] = s.c_str();
    argv_uint32_only[2] = "4242422424";
    EXPECT_TRUE(PA_PARSER.Parse(argc_uint32_only, argv_uint32_only));
    EXPECT_EQ(PAU32.GetValue(), ref_uint32_pos);
}

// expect uint64_t values processed right
HWTEST(libpandargs, TestUint64, testing::ext::TestSize.Level0)
{
    static const uint64_t ref_uint64_pos = 424242422424;
    static const int argc_uint64_only = 3;
    static const char *argv_uint64_only[argc_uint64_only];
    argv_uint64_only[0] = "gtest_app";
    std::string s = "--" + PAU64.GetName();
    argv_uint64_only[1] = s.c_str();
    argv_uint64_only[2] = "424242422424";
    EXPECT_TRUE(PA_PARSER.Parse(argc_uint64_only, argv_uint64_only));
    EXPECT_EQ(PAU64.GetValue(), ref_uint64_pos);
}

// expect hex values processed right
HWTEST(libpandargs, TestHexValues, testing::ext::TestSize.Level0)
{
    static const uint64_t ref_uint64 = 274877906944;
    static const uint64_t ref_int = 64;
    static const int argc_uint64_int = 3;
    static const char *argv_uint64_int[argc_uint64_int];
    argv_uint64_int[0] = "gtest_app";
    std::string s = "--" + PAU64.GetName();
    argv_uint64_int[1] = s.c_str();
    argv_uint64_int[2] = "0x4000000000";
    EXPECT_TRUE(PA_PARSER.Parse(argc_uint64_int, argv_uint64_int));
    EXPECT_EQ(PAU64.GetValue(), ref_uint64);
    argv_uint64_int[2] = "0x40";
    EXPECT_TRUE(PA_PARSER.Parse(argc_uint64_int, argv_uint64_int));
    EXPECT_EQ(PAU64.GetValue(), ref_int);
}

// expect out of range uint32_t values processed right
HWTEST(libpandargs, TestOutUint32, testing::ext::TestSize.Level0)
{
    static const int argc_uint32_only = 3;
    static const char *argv_uint32_only[argc_uint32_only];
    argv_uint32_only[0] = "gtest_app";
    std::string s = "--" + PAU32.GetName();
    argv_uint32_only[1] = s.c_str();
    argv_uint32_only[2] = "424224244242242442422424";
    EXPECT_FALSE(PA_PARSER.Parse(argc_uint32_only, argv_uint32_only));
    argv_uint32_only[2] = "0xffffffffffffffffffffffffff";
    EXPECT_FALSE(PA_PARSER.Parse(argc_uint32_only, argv_uint32_only));
}

// expect out of range uint64_t values processed right
HWTEST(libpandargs, TestOutUint64, testing::ext::TestSize.Level0)
{
    static const int argc_uint64_only = 3;
    static const char *argv_uint64_only[argc_uint64_only];
    argv_uint64_only[0] = "gtest_app";
    std::string s = "--" + PAU64.GetName();
    argv_uint64_only[1] = s.c_str();
    argv_uint64_only[2] = "424224244242242442422424";
    EXPECT_FALSE(PA_PARSER.Parse(argc_uint64_only, argv_uint64_only));
    argv_uint64_only[2] = "0xffffffffffffffffffffffffff";
    EXPECT_FALSE(PA_PARSER.Parse(argc_uint64_only, argv_uint64_only));
}

// expect string argument of one word and multiple word processed right
HWTEST(libpandargs, TestStringRange, testing::ext::TestSize.Level0)
{
    static const std::string ref_one_string = "string";
    static const std::string ref_multiple_string = "this is a string";
    static const char *str_argname = "--string";
    static const int argc_one_string = 3;
    static const char *argv_one_string[argc_one_string] = {"gtest_app", str_argname, "string"};
    static const int argc_multiple_string = 3;
    static const char *argv_multiple_string[argc_multiple_string] = {"gtest_app", str_argname, "this is a string"};
    EXPECT_TRUE(PA_PARSER.Parse(argc_multiple_string, argv_multiple_string));
    EXPECT_EQ(PAS.GetValue(), ref_multiple_string);
    EXPECT_TRUE(PA_PARSER.Parse(argc_one_string, argv_one_string));
    EXPECT_EQ(PAS.GetValue(), ref_one_string);
}

// expect string at the end of line is an empty string
HWTEST(libpandargs, TestStringLine, testing::ext::TestSize.Level0)
{
    static const int argc_string_only = 2;
    static const char *argv_string_only[argc_string_only];
    argv_string_only[0] = "gtest_app";
    std::string s = "--" + PAS.GetName();
    argv_string_only[1] = s.c_str();
    EXPECT_TRUE(PA_PARSER.Parse(argc_string_only, argv_string_only));
    EXPECT_EQ(PAS.GetValue(), "");
}

// expect list argument processed right
HWTEST(libpandargs, TestList, testing::ext::TestSize.Level0)
{
    PALD.ResetDefaultValue();
    static const arg_list_t ref_list = {"list1", "list2", "list3"};
    std::string s = "--" + PALD.GetName();
    static const char *list_argname = s.c_str();
    static const int argc_list_only = 7;
    static const char *argv_list_only[argc_list_only] = {"gtest_app", list_argname, "list1",
        list_argname, "list2", list_argname, "list3"};
    EXPECT_TRUE(PA_PARSER.Parse(argc_list_only, argv_list_only));
    ASSERT_EQ(PALD.GetValue().size(), ref_list.size());
    for (std::size_t i = 0; i < ref_list.size(); ++i) {
        EXPECT_EQ(PALD.GetValue()[i], ref_list[i]);
    }
}

// expect list argument without delimiter processed right
HWTEST(libpandargs, TestListDelimiter, testing::ext::TestSize.Level0)
{
    PAL.ResetDefaultValue();
    static const arg_list_t ref_list = {"list1", "list2", "list3", "list4"};
    std::string s = "--" + PAL.GetName();
    static const char *list_argname = s.c_str();
    static const int argc_list_only = 9;
    static const char *argv_list_only[argc_list_only] = {"gtest_app",
        list_argname, "list1", list_argname, "list2", list_argname, "list3", list_argname, "list4"};
    EXPECT_TRUE(PA_PARSER.Parse(argc_list_only, argv_list_only));
    ASSERT_EQ(PAL.GetValue().size(), ref_list.size());
    for (std::size_t i = 0; i < ref_list.size(); ++i) {
        EXPECT_EQ(PAL.GetValue()[i], ref_list[i]);
    }
}

// expect delimiter list argument processed right
HWTEST(libpandargs, TestDelimiter, testing::ext::TestSize.Level0)
{
    PALD.ResetDefaultValue();
    static const arg_list_t ref_dlist = {"dlist1", "dlist2", "dlist3"};
    std::string s = "--" + PALD.GetName();
    static const char *list_argname = s.c_str();
    static const int argc_dlist_only = 3;
    static const char *argv_dlist_only[argc_dlist_only] = {"gtest_app", list_argname, "dlist1:dlist2:dlist3"};
    EXPECT_TRUE(PA_PARSER.Parse(argc_dlist_only, argv_dlist_only));
    ASSERT_EQ(PALD.GetValue().size(), ref_dlist.size());
    for (std::size_t i = 0; i < ref_dlist.size(); ++i) {
        EXPECT_EQ(PALD.GetValue()[i], ref_dlist[i]);
    }
}

// expect delimiter and multiple list argument processed right
HWTEST(libpandargs, TestDelimiterList, testing::ext::TestSize.Level0)
{
    PALD.ResetDefaultValue();
    static const arg_list_t ref_list = {"dlist1", "dlist2", "list1", "list2", "dlist3", "dlist4"};
    std::string s = "--" + PALD.GetName();
    static const char *list_argname = s.c_str();
    static const int argc_list = 9;
    static const char *argv_list[argc_list] = {"gtest_app",  list_argname, "dlist1:dlist2", list_argname, "list1",
        list_argname, "list2", list_argname, "dlist3:dlist4"};
    EXPECT_TRUE(PA_PARSER.Parse(argc_list, argv_list));
    ASSERT_EQ(PALD.GetValue().size(), ref_list.size());
    for (std::size_t i = 0; i < ref_list.size(); ++i) {
        EXPECT_EQ(PALD.GetValue()[i], ref_list[i]);
    }
}

// expect positive and negative integer values with range processed right
HWTEST(libpandargs, TestIntegerRange, testing::ext::TestSize.Level0)
{
    static const int ref_int_pos = 99;
    static const int ref_int_neg = -99;
    static const int argc_int_only = 3;
    static const char *argv_int_only[argc_int_only];
    argv_int_only[0] = "gtest_app";
    std::string s = "--" + PAIR.GetName();
    argv_int_only[1] = s.c_str();
    argv_int_only[2] = "99";
    EXPECT_TRUE(PA_PARSER.Parse(argc_int_only, argv_int_only));
    EXPECT_EQ(PAIR.GetValue(), ref_int_pos);
    argv_int_only[2] = "-99";
    EXPECT_TRUE(PA_PARSER.Parse(argc_int_only, argv_int_only));
    EXPECT_EQ(PAIR.GetValue(), ref_int_neg);
}

// expect wrong positive and negative integer values with range processed right
HWTEST(libpandargs, TestWrongInteger, testing::ext::TestSize.Level0)
{
    static const int argc_int_only = 3;
    static const char *argv_int_only[argc_int_only];
    argv_int_only[0] = "gtest_app";
    std::string s = "--" + PAIR.GetName();
    argv_int_only[1] = s.c_str();
    argv_int_only[2] = "101";
    EXPECT_FALSE(PA_PARSER.Parse(argc_int_only, argv_int_only));
    argv_int_only[2] = "-101";
    EXPECT_FALSE(PA_PARSER.Parse(argc_int_only, argv_int_only));
}

// expect uint32_t values with range processed right
HWTEST(libpandargs, TestUint32Range, testing::ext::TestSize.Level0)
{
    static const uint32_t ref_int_min = 1;
    static const uint32_t ref_int_max = 990000000;
    static const int argc_int_only = 3;
    static const char *argv_int_only[argc_int_only];
    argv_int_only[0] = "gtest_app";
    std::string s = "--" + PAUR32.GetName();
    argv_int_only[1] = s.c_str();
    argv_int_only[2] = "1";
    EXPECT_TRUE(PA_PARSER.Parse(argc_int_only, argv_int_only));
    EXPECT_EQ(PAUR32.GetValue(), ref_int_min);
    argv_int_only[2] = "990000000";
    EXPECT_TRUE(PA_PARSER.Parse(argc_int_only, argv_int_only));
    EXPECT_EQ(PAUR32.GetValue(), ref_int_max);
}

// expect wrong uint32_t values with range processed right
HWTEST(libpandargs, TestWrongUint32, testing::ext::TestSize.Level0)
{
    static const int argc_int_only = 3;
    static const char *argv_int_only[argc_int_only];
    argv_int_only[0] = "gtest_app";
    std::string s = "--" + PAUR32.GetName();
    argv_int_only[1] = s.c_str();
    argv_int_only[2] = "-1";
    EXPECT_FALSE(PA_PARSER.Parse(argc_int_only, argv_int_only));
    argv_int_only[2] = "1000000001";
    EXPECT_FALSE(PA_PARSER.Parse(argc_int_only, argv_int_only));
}

// expect uint64_t values with range processed right
HWTEST(libpandargs, TestUint64Range, testing::ext::TestSize.Level0)
{
    static const uint64_t ref_int_min = 1;
    static const uint64_t ref_int_max = 99000000000;
    static const int argc_int_only = 3;
    static const char *argv_int_only[argc_int_only];
    argv_int_only[0] = "gtest_app";
    std::string s = "--" + PAUR64.GetName();
    argv_int_only[1] = s.c_str();
    argv_int_only[2] = "1";
    EXPECT_TRUE(PA_PARSER.Parse(argc_int_only, argv_int_only));
    EXPECT_EQ(PAUR64.GetValue(), ref_int_min);
    argv_int_only[2] = "99000000000";
    EXPECT_TRUE(PA_PARSER.Parse(argc_int_only, argv_int_only));
    EXPECT_EQ(PAUR64.GetValue(), ref_int_max);
}

// expect wrong uint64_t values with range processed right
HWTEST(libpandargs, TestWrongUint364, testing::ext::TestSize.Level0)
{
    static const int argc_int_only = 3;
    static const char *argv_int_only[argc_int_only];
    argv_int_only[0] = "gtest_app";
    std::string s = "--" + PAUR64.GetName();
    argv_int_only[1] = s.c_str();
    argv_int_only[2] = "-1";
    EXPECT_FALSE(PA_PARSER.Parse(argc_int_only, argv_int_only));
    argv_int_only[2] = "100000000001";
    EXPECT_FALSE(PA_PARSER.Parse(argc_int_only, argv_int_only));
}

// expect list at the end of line is a list with empty string
HWTEST(libpandargs, TestListRange, testing::ext::TestSize.Level0)
{
    PALD.ResetDefaultValue();
    static const arg_list_t ref_list = {""};
    static const int argc_list_only = 2;
    static const char *argv_list_only[argc_list_only];
    argv_list_only[0] = "gtest_app";
    std::string s = "--" + PALD.GetName();
    argv_list_only[1] = s.c_str();
    EXPECT_TRUE(PA_PARSER.Parse(argc_list_only, argv_list_only));
    EXPECT_EQ(PALD.GetValue(), ref_list);
}

// expect true on IsTailEnabled when tail is enabled, false otherwise
HWTEST(libpandargs, TestIsTailEnabled, testing::ext::TestSize.Level0)
{
    PA_PARSER.EnableTail();
    EXPECT_TRUE(PA_PARSER.IsTailEnabled());
    PA_PARSER.DisableTail();
    EXPECT_FALSE(PA_PARSER.IsTailEnabled());
}

// expect tail only argument is consistent
HWTEST(libpandargs, TestTailConsistent, testing::ext::TestSize.Level0)
{
    static const int argc_tail_only = 2;
    static const char *argv_tail_only[] = {"gtest_app", "tail1"};
    static const std::string ref_str_tail = "tail1";
    PA_PARSER.EnableTail();
    PA_PARSER.PushBackTail(&T_PAS);
    EXPECT_TRUE(PA_PARSER.Parse(argc_tail_only, argv_tail_only));
    ASSERT_EQ(T_PAS.GetValue(), ref_str_tail);
    PA_PARSER.DisableTail();
    PA_PARSER.EraseTail();
}

// expect multiple tail only argument is consistent
HWTEST(libpandargs, TestMultipleTail, testing::ext::TestSize.Level0)
{
    static const int argc_tail_only = 7;
    static const char *argv_tail_only[] = {"gtest_app", "str_tail", "off", "-4", "3.14", "2", "4"};
    static const std::string str_ref = "str_tail";
    static const bool bool_ref = false;
    static const int int_ref = -4;
    static const double double_ref = 3.14;
    static const uint32_t uint32_ref = 2;
    static const uint64_t uint64_ref = 4;
    PA_PARSER.EnableTail();
    PA_PARSER.PushBackTail(&T_PAS);
    PA_PARSER.PushBackTail(&T_PAB);
    PA_PARSER.PushBackTail(&T_PAI);
    PA_PARSER.PushBackTail(&T_PAD);
    PA_PARSER.PushBackTail(&T_PAU32);
    PA_PARSER.PushBackTail(&T_PAU64);
    EXPECT_EQ(PA_PARSER.GetTailSize(), 6U);
    EXPECT_TRUE(PA_PARSER.Parse(argc_tail_only, argv_tail_only));
    EXPECT_EQ(T_PAS.GetValue(), str_ref);
    EXPECT_EQ(T_PAB.GetValue(), bool_ref);
    EXPECT_EQ(T_PAI.GetValue(), int_ref);
    EXPECT_DOUBLE_EQ(T_PAD.GetValue(), double_ref);
    EXPECT_EQ(T_PAU32.GetValue(), uint32_ref);
    EXPECT_EQ(T_PAU64.GetValue(), uint64_ref);
    PA_PARSER.DisableTail();
    PA_PARSER.EraseTail();
    EXPECT_EQ(PA_PARSER.GetTailSize(), 0U);
}

// expect parse fail on wrong tail argument type
HWTEST(libpandargs, TestRrongTail, testing::ext::TestSize.Level0)
{
    PA_PARSER.EnableTail();
    static const int argc_tail_only = 3;
    // boolean value instead of integer
    static const char *argv_tail_only[] = {"gtest_app", "str_tail", "off"};
    static const std::string str_ref = "str_tail";
    PA_PARSER.PushBackTail(&T_PAS);
    PA_PARSER.PushBackTail(&T_PAI);
    EXPECT_EQ(PA_PARSER.GetTailSize(), 2U);
    EXPECT_FALSE(PA_PARSER.Parse(argc_tail_only, argv_tail_only));
    EXPECT_EQ(T_PAS.GetValue(), str_ref);
    PA_PARSER.DisableTail();
    PA_PARSER.EraseTail();
    EXPECT_EQ(PA_PARSER.GetTailSize(), 0U);
}

// expect right tail argument processing after preceiding string argument
HWTEST(libpandargs, TestRightTail, testing::ext::TestSize.Level0)
{
    PA_PARSER.EnableTail();
    static const char *str_argname = "--string";
    static const std::string ref_string = "this is a reference string";
    static const std::string ref_t_str = "string";
    static const double ref_t_double = 0.1;
    static const bool ref_t_bool = true;
    static const uint32_t ref_t_uint32 = 32;
    static const uint64_t ref_t_uint64 = 64;
    static const int argc_tail_string = 8;
    static const char *argv_tail_string[] = {"gtest_app",
        str_argname, "this is a reference string", "string", ".1", "on", "32", "64"};
    PA_PARSER.PushBackTail(&T_PAS);
    PA_PARSER.PushBackTail(&T_PAD);
    PA_PARSER.PushBackTail(&T_PAB);
    PA_PARSER.PushBackTail(&T_PAU32);
    PA_PARSER.PushBackTail(&T_PAU64);
    EXPECT_TRUE(PA_PARSER.Parse(argc_tail_string, argv_tail_string));
    EXPECT_EQ(PAS.GetValue(), ref_string);
    EXPECT_EQ(T_PAS.GetValue(), ref_t_str);
    EXPECT_EQ(T_PAD.GetValue(), ref_t_double);
    EXPECT_EQ(T_PAB.GetValue(), ref_t_bool);
    EXPECT_EQ(T_PAU32.GetValue(), ref_t_uint32);
    EXPECT_EQ(T_PAU64.GetValue(), ref_t_uint64);
    PA_PARSER.DisableTail();
    PA_PARSER.EraseTail();
}

// expect right tail argument processing after preceiding list argument
HWTEST(libpandargs, TestTailProceiding, testing::ext::TestSize.Level0)
{
    PALD.ResetDefaultValue();
    PA_PARSER.EnableTail();
    static const char *list_argname = "--dlist";
    static const arg_list_t ref_list = {"list1", "list2", "list3", "list4", "list5"};
    static const double ref_t_double = -7;
    static const bool ref_t_bool = true;
    static const int ref_t_int = 255;
    static const uint32_t ref_t_uint32 = 32;
    static const uint64_t ref_t_uint64 = 64;
    static const int argc_tail_list = 16;
    static const char *argv_tail_list[] = {"gtest_app", list_argname, "list1", list_argname, "list2", list_argname,
        "list3", list_argname, "list4", list_argname, "list5", "true", "255", "-7", "32", "64"};
    PA_PARSER.PushBackTail(&T_PAB);
    PA_PARSER.PushBackTail(&T_PAI);
    PA_PARSER.PushBackTail(&T_PAD);
    PA_PARSER.PushBackTail(&T_PAU32);
    PA_PARSER.PushBackTail(&T_PAU64);
    EXPECT_TRUE(PA_PARSER.Parse(argc_tail_list, argv_tail_list));
    ASSERT_EQ(PALD.GetValue().size(), ref_list.size());
    for (std::size_t i = 0; i < ref_list.size(); i++) {
        EXPECT_EQ(PALD.GetValue()[i], ref_list[i]);
    }
    EXPECT_EQ(T_PAB.GetValue(), ref_t_bool);
    EXPECT_EQ(T_PAI.GetValue(), ref_t_int);
    EXPECT_DOUBLE_EQ(T_PAD.GetValue(), ref_t_double);
    EXPECT_EQ(T_PAU32.GetValue(), ref_t_uint32);
    EXPECT_EQ(T_PAU64.GetValue(), ref_t_uint64);

    PA_PARSER.DisableTail();
    PA_PARSER.EraseTail();
}

// expect right tail argument processing after noparam boolean argument
HWTEST(libpandargs, TestTailBoolean, testing::ext::TestSize.Level0)
{
    PA_PARSER.EnableTail();
    PandArg<std::string> t_pas0("tail_string0", REF_DEF_STRING, "Sample tail string argument 0");
    PandArg<std::string> t_pas1("tail_string1", REF_DEF_STRING, "Sample tail string argument 1");
    static const std::string ref_t_str1 = "offtail1";
    static const std::string ref_t_str2 = "offtail2";
    static const std::string ref_t_str3 = "offtail3";
    static const int argc_tail_bool = 5;
    static const char *argv_tail_bool[] = {"gtest_app", "--bool", "offtail1", "offtail2", "offtail3"};
    PA_PARSER.PushBackTail(&T_PAS);
    PA_PARSER.PushBackTail(&t_pas0);
    PA_PARSER.PushBackTail(&t_pas1);
    EXPECT_TRUE(PA_PARSER.Parse(argc_tail_bool, argv_tail_bool));
    EXPECT_TRUE(PAB.GetValue());
    EXPECT_EQ(T_PAS.GetValue(), ref_t_str1);
    EXPECT_EQ(t_pas0.GetValue(), ref_t_str2);
    EXPECT_EQ(t_pas1.GetValue(), ref_t_str3);
    PA_PARSER.DisableTail();
    PA_PARSER.EraseTail();
}

// expect fail on amount of tail arguments more then PA_PARSER may have
HWTEST(libpandargs, TestTailPa_parser, testing::ext::TestSize.Level0)
{
    PA_PARSER.EnableTail();
    static const int argc_tail = 5;
    static const char *argv_tail[] = {"gtest_app", "gdb", "--args", "file.bin", "entry"};

    PandArg<std::string> t_pas1("tail_string1", REF_DEF_STRING, "Sample tail string argument 1");
    PA_PARSER.PushBackTail(&T_PAS);
    PA_PARSER.PushBackTail(&t_pas1);

    EXPECT_EQ(PA_PARSER.GetTailSize(), 2U);
    EXPECT_FALSE(PA_PARSER.Parse(argc_tail, argv_tail));
    PA_PARSER.DisableTail();
    PA_PARSER.EraseTail();
}

// expect remainder arguments only parsed as expected
HWTEST(libpandargs, TestRemainder, testing::ext::TestSize.Level0)
{
    PA_PARSER.EnableRemainder();
    static const arg_list_t ref_rem = {"rem1", "rem2", "rem3"};
    static int argc_rem = 5;
    static const char *argv_rem[] = {"gtest_app", "--", "rem1", "rem2", "rem3"};
    PA_PARSER.Parse(argc_rem, argv_rem);
    arg_list_t remainder = PA_PARSER.GetRemainder();
    EXPECT_EQ(remainder.size(), ref_rem.size());
    for (std::size_t i = 0; i < remainder.size(); i++) {
        EXPECT_EQ(remainder[i], ref_rem[i]);
    }
    PA_PARSER.DisableRemainder();
}

// expect regular argument before remainder parsed right
HWTEST(libpandargs, TestRegularRemainder, testing::ext::TestSize.Level0)
{
    PA_PARSER.EnableRemainder();
    static const arg_list_t ref_rem = {"rem1", "rem2", "rem3"};
    std::string bool_name = "--" + PAB.GetName();
    static int argc_rem = 6;
    static const char *argv_rem[] = {"gtest_app", bool_name.c_str(), "--", "rem1", "rem2", "rem3"};
    PA_PARSER.Parse(argc_rem, argv_rem);
    EXPECT_TRUE(PAB.GetValue());
    arg_list_t remainder = PA_PARSER.GetRemainder();
    EXPECT_EQ(remainder.size(), ref_rem.size());
    for (std::size_t i = 0; i < remainder.size(); i++) {
        EXPECT_EQ(remainder[i], ref_rem[i]);
    }
    PA_PARSER.DisableRemainder();
}

static const char *ARGV_CONSISTENT[] = {"gtest_app", "--bool", "on", "--int=42", "--string", "this is a string",
    "--double", ".42", "--uint32=4294967295", "--uint64=18446744073709551615", "--dlist=dlist1:dlist2:dlist3:dlist4",
    "--rint=42", "--ruint32=990000000", "--ruint64=99000000000", "tail1", "tail2 tail3", "tail4", "--", "rem1",
    "rem2", "rem3"};

// expect that all arguments parsed as expected
HWTEST(libpandargs, TestAll, testing::ext::TestSize.Level0)
{
    PALD.ResetDefaultValue();
    PA_PARSER.EnableTail();
    PA_PARSER.EnableRemainder();
    static const arg_list_t ref_rem = {"rem1", "rem2", "rem3"};
    PandArg<std::string> t_pas0("tail_string0", REF_DEF_STRING, "Sample tail string argument 0");
    PandArg<std::string> t_pas1("tail_string1", REF_DEF_STRING, "Sample tail string argument 1");
    static const bool ref_bool = true;
    static const int ref_int = 42;
    static const arg_list_t ref_dlist = {"dlist1", "dlist2", "dlist3", "dlist4"};
    static const std::string ref_t_str1 = "tail1";
    static const std::string ref_t_str2 = "tail2 tail3";
    static const std::string ref_t_str3 = "tail4";
    static const std::string ref_str = "this is a string";
    static const double ref_dbl = 0.42;
    static const uint32_t ref_uint32 = std::numeric_limits<std::uint32_t>::max();
    static const uint32_t ref_uint32r = 990000000;
    static const uint64_t ref_uint64 = std::numeric_limits<std::uint64_t>::max();
    static const uint64_t ref_uint64r = 99000000000;
    static int argc_consistent = 21;
    PA_PARSER.PushBackTail(&T_PAS);
    PA_PARSER.PushBackTail(&t_pas0);
    PA_PARSER.PushBackTail(&t_pas1);
    EXPECT_TRUE(PA_PARSER.Parse(argc_consistent, ARGV_CONSISTENT));
    EXPECT_EQ(PAB.GetValue(), ref_bool);
    EXPECT_EQ(PAI.GetValue(), ref_int);
    EXPECT_EQ(PAS.GetValue(), ref_str);
    EXPECT_DOUBLE_EQ(PAD.GetValue(), ref_dbl);
    EXPECT_EQ(PAU32.GetValue(), ref_uint32);
    EXPECT_EQ(PAU64.GetValue(), ref_uint64);
    ASSERT_EQ(PALD.GetValue().size(), ref_dlist.size());
    for (std::size_t i = 0; i < ref_dlist.size(); ++i) {
        EXPECT_EQ(PALD.GetValue()[i], ref_dlist[i]);
    }
    EXPECT_EQ(PAIR.GetValue(), ref_int);
    EXPECT_EQ(PAUR32.GetValue(), ref_uint32r);
    EXPECT_EQ(PAUR64.GetValue(), ref_uint64r);
    EXPECT_EQ(T_PAS.GetValue(), ref_t_str1);
    EXPECT_EQ(t_pas0.GetValue(), ref_t_str2);
    EXPECT_EQ(t_pas1.GetValue(), ref_t_str3);
    arg_list_t remainder = PA_PARSER.GetRemainder();
    EXPECT_EQ(remainder.size(), ref_rem.size());
    for (std::size_t i = 0; i < remainder.size(); i++) {
        EXPECT_EQ(remainder[i], ref_rem[i]);
    }
    PA_PARSER.DisableRemainder();
    PA_PARSER.DisableTail();
    PA_PARSER.EraseTail();
}

PandArg<bool> SUB_BOOL_ARG("bool", false, "Sample boolean argument");

// The number 12 is the default parameter for variable SUB_INT_ARG
PandArg<int> SUB_INT_ARG("int", 12, "Sample integer argument");

// The number 123.45 is the default parameter for variable SUB_DOUBLE_ARG
PandArg<double> SUB_DOUBLE_ARG("double", 123.45, "Sample rational argument");

PandArg<std::string> SUB_STRING_ARG("string", "Hello", "Sample string argument");

// The number 123 is the default parameter for variable INT_ARG
PandArg<int> INT_ARG("global_int", 123, "Global integer argument");

PandArgCompound PARENT("compound", "Sample boolean argument", {
    &SUB_BOOL_ARG, &SUB_INT_ARG, &SUB_DOUBLE_ARG, &SUB_STRING_ARG});

PandArgParser C_PARSER;

HWTEST(libpandargs, CompoundArgsAdd, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(C_PARSER.Add(&INT_ARG));
    ASSERT_TRUE(C_PARSER.Add(&PARENT));
}

// Should work well with no sub arguments
HWTEST(libpandargs, CompoundArgsNoSub, testing::ext::TestSize.Level0)
{
    {
        PARENT.ResetDefaultValue();
        static const char *argv[] = {"gtest_app", "--compound"};
        ASSERT_TRUE(C_PARSER.Parse(2, argv)) << C_PARSER.GetErrorString();
        ASSERT_EQ(PARENT.GetValue(), true);
        ASSERT_EQ(SUB_BOOL_ARG.GetValue(), false);
        ASSERT_EQ(SUB_INT_ARG.GetValue(), 12);
        ASSERT_EQ(SUB_DOUBLE_ARG.GetValue(), 123.45);
        ASSERT_EQ(SUB_STRING_ARG.GetValue(), "Hello");
    }

    {
        PARENT.ResetDefaultValue();
        static const char *argv[] = {"gtest_app", "--compound:bool,int=2,double=54.321,string=World"};
        ASSERT_TRUE(C_PARSER.Parse(2, argv)) << C_PARSER.GetErrorString();
        ASSERT_EQ(PARENT.GetValue(), true);
        ASSERT_EQ(SUB_BOOL_ARG.GetValue(), true);
        ASSERT_EQ(SUB_INT_ARG.GetValue(), 2);
        ASSERT_EQ(SUB_DOUBLE_ARG.GetValue(), 54.321);
        ASSERT_EQ(SUB_STRING_ARG.GetValue(), "World");
    }
}

// ResetDefaultValue should reset all sub arguments
HWTEST(libpandargs, CompoundArgsResetDefaultValue, testing::ext::TestSize.Level0)
{
    {
        PARENT.ResetDefaultValue();
        ASSERT_EQ(PARENT.GetValue(), false);
        ASSERT_EQ(SUB_BOOL_ARG.GetValue(), false);
        ASSERT_EQ(SUB_INT_ARG.GetValue(), 12);
        ASSERT_EQ(SUB_DOUBLE_ARG.GetValue(), 123.45);
        ASSERT_EQ(SUB_STRING_ARG.GetValue(), "Hello");
    }

    {
        static const char *argv[] = {"gtest_app", "--compound:bool=true"};
        ASSERT_TRUE(C_PARSER.Parse(2, argv)) << C_PARSER.GetErrorString();
        ASSERT_EQ(PARENT.GetValue(), true);
        ASSERT_EQ(SUB_BOOL_ARG.GetValue(), true);
    }

    {
        PARENT.ResetDefaultValue();
        static const char *argv[] = {"gtest_app", "--compound:bool"};
        ASSERT_TRUE(C_PARSER.Parse(2, argv)) << C_PARSER.GetErrorString();
        ASSERT_EQ(PARENT.GetValue(), true);
        ASSERT_EQ(SUB_BOOL_ARG.GetValue(), true);
    }

    {
        static const char *argv[] = {"gtest_app", "--compound:bool=false"};
        ASSERT_TRUE(C_PARSER.Parse(2, argv)) << C_PARSER.GetErrorString();
        ASSERT_EQ(PARENT.GetValue(), true);
        ASSERT_EQ(SUB_BOOL_ARG.GetValue(), false);
    }

    {
        PARENT.ResetDefaultValue();
        static const char *argv[] = {"gtest_app", "--global_int=321"};
        ASSERT_TRUE(C_PARSER.Parse(2, argv)) << C_PARSER.GetErrorString();
        ASSERT_EQ(PARENT.GetValue(), false);
        ASSERT_EQ(INT_ARG.GetValue(), 321);
    }

    {
        PARENT.ResetDefaultValue();
        static const char *argv[] = {"gtest_app", "--compound", "--global_int", "321"};
        ASSERT_TRUE(C_PARSER.Parse(4, argv)) << C_PARSER.GetErrorString();
        ASSERT_EQ(PARENT.GetValue(), true);
        ASSERT_EQ(INT_ARG.GetValue(), 321);
    }
}

// Test that sub arguments are not visible in the global space
HWTEST(libpandargs, CompoundArgsSubArguments, testing::ext::TestSize.Level0)
{
    {
        static const char *argv[] = {"gtest_app", "--bool"};
        ASSERT_FALSE(C_PARSER.Parse(2, argv));
    }
    {
        static const char *argv[] = {"gtest_app", "--int=2"};
        ASSERT_FALSE(C_PARSER.Parse(2, argv));
    }
    {
        static const char *argv[] = {"gtest_app", "--double=54.321"};
        ASSERT_FALSE(C_PARSER.Parse(2, argv));
    }
    {
        static const char *argv[] = {"gtest_app", "--string=World"};
        ASSERT_FALSE(C_PARSER.Parse(2, argv));
    }
}

HWTEST(libpandargs, GetHelpString, testing::ext::TestSize.Level0)
{
    PandArg<bool> g_pab("bool", false, "Sample boolean argument");
    PandArg<int> g_pai("int", 0, "Sample integer argument");
    PandArg<std::string> g_pas("tail_string", "arg", "Sample tail string argument");
    PandArgCompound g_pac("compound", "Sample compound argument", {&g_pab, &g_pai});

    PandArgParser g_parser;
    ASSERT_TRUE(g_parser.Add(&g_pab));
    ASSERT_TRUE(g_parser.Add(&g_pai));
    ASSERT_TRUE(g_parser.PushBackTail(&g_pas));
    ASSERT_TRUE(g_parser.Add(&g_pac));

    std::string ref_string = "--" + g_pab.GetName() + ": " + g_pab.GetDesc() + "\n";
    ref_string += "--" + g_pac.GetName() + ": " + g_pac.GetDesc() + "\n";
    ref_string += "  Sub arguments:\n";
    ref_string += "    " + g_pab.GetName() + ": " + g_pab.GetDesc() + "\n";
    ref_string += "    " + g_pai.GetName() + ": " + g_pai.GetDesc() + "\n";
    ref_string += "--" + g_pai.GetName() + ": " + g_pai.GetDesc() + "\n";
    ref_string += "Tail arguments:\n";
    ref_string += g_pas.GetName() + ": " + g_pas.GetDesc() + "\n";
    ASSERT_EQ(g_parser.GetHelpString(), ref_string);
}

}  // namespace panda::test

