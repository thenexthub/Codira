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

#include "libarkbase/utils/pandargs.h"

#include <gtest/gtest.h>

namespace ark::test {

static constexpr const bool REF_DEF_BOOL = false;
static constexpr const int REF_DEF_INT = 0;
static constexpr const double REF_DEF_DOUBLE = 1.0;
static constexpr const uint32_t REF_DEF_UINT32 = 0;
static constexpr const uint64_t REF_DEF_UINT64 = 0;

// NOLINTNEXTLINE(readability-magic-numbers)
static constexpr int ARGC_CONSISTENT = 21;
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static constexpr const char *ARGV_CONSISTENT[] = {"gtest_app",
                                                  "--bool",
                                                  "on",
                                                  "--int=42",
                                                  "--string",
                                                  "this is a string",
                                                  "--double",
                                                  ".42",
                                                  "--uint32=4294967295",
                                                  "--uint64=18446744073709551615",
                                                  "--dlist=dlist1:dlist2:dlist3:dlist4",
                                                  "--rint=42",
                                                  "--ruint32=990000000",
                                                  "--ruint64=99000000000",
                                                  "tail1",
                                                  "tail2 tail3",
                                                  "tail4",
                                                  "--",
                                                  "rem1",
                                                  "rem2",
                                                  "rem3"};

struct TestPandArgs {
    PandArg<bool> *pab;
    PandArg<int> *pai;
    PandArg<double> *pad;
    PandArg<std::string> *pas;
    PandArg<uint32_t> *pau32;
    PandArg<uint64_t> *pau64;
    PandArg<arg_list_t> *pald;
    PandArg<arg_list_t> *pal;
    PandArg<int> *pair;
    PandArg<uint32_t> *paur32;
    PandArg<uint64_t> *paur64;
};

struct TestTailPandArgs {
    PandArg<bool> *tPab;
    PandArg<int> *tPai;
    PandArg<double> *tPad;
    PandArg<std::string> *tPas;
    PandArg<uint32_t> *tPau32;
    PandArg<uint64_t> *tPau64;
};

static void TestArgSet(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect all arguments are set in parser
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pab));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pab->GetName()));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pai));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pai->GetName()));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pad));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pad->GetName()));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pas));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pas->GetName()));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pau32));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pau32->GetName()));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pau64));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pau64->GetName()));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pald));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pald->GetName()));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pal));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pal->GetName()));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pair));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.pair->GetName()));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.paur32));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.paur32->GetName()));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.paur64));
    EXPECT_TRUE(paParser.IsArgSet(pandArgs.paur64->GetName()));
}

static void TestTypeConsistency(TestPandArgs &pandArgs, const std::string &refDefStr)
{
    // expect default values and types are consistent
    EXPECT_EQ(pandArgs.pab->GetDefaultValue(), REF_DEF_BOOL);
    EXPECT_EQ(pandArgs.pab->GetDefaultValue(), pandArgs.pab->GetValue());
    EXPECT_EQ(pandArgs.pab->GetType(), PandArgType::BOOL);

    EXPECT_EQ(pandArgs.pai->GetDefaultValue(), REF_DEF_INT);
    EXPECT_EQ(pandArgs.pai->GetDefaultValue(), pandArgs.pai->GetValue());
    EXPECT_EQ(pandArgs.pai->GetType(), PandArgType::INTEGER);

    EXPECT_DOUBLE_EQ(pandArgs.pad->GetValue(), REF_DEF_DOUBLE);
    EXPECT_DOUBLE_EQ(pandArgs.pad->GetDefaultValue(), pandArgs.pad->GetValue());
    EXPECT_EQ(pandArgs.pad->GetType(), PandArgType::DOUBLE);

    EXPECT_EQ(pandArgs.pas->GetDefaultValue(), refDefStr);
    EXPECT_EQ(pandArgs.pas->GetDefaultValue(), pandArgs.pas->GetValue());
    EXPECT_EQ(pandArgs.pas->GetType(), PandArgType::STRING);

    EXPECT_EQ(pandArgs.pau32->GetDefaultValue(), REF_DEF_UINT32);
    EXPECT_EQ(pandArgs.pau32->GetDefaultValue(), pandArgs.pau32->GetValue());
    EXPECT_EQ(pandArgs.pau32->GetType(), PandArgType::UINT32);

    EXPECT_EQ(pandArgs.pau64->GetDefaultValue(), REF_DEF_UINT64);
    EXPECT_EQ(pandArgs.pau64->GetDefaultValue(), pandArgs.pau64->GetValue());
    EXPECT_EQ(pandArgs.pau64->GetType(), PandArgType::UINT64);

    EXPECT_TRUE(pandArgs.pald->GetValue().empty());
    EXPECT_EQ(pandArgs.pald->GetDefaultValue(), pandArgs.pald->GetValue());
    EXPECT_EQ(pandArgs.pald->GetType(), PandArgType::LIST);

    EXPECT_TRUE(pandArgs.pal->GetValue().empty());
    EXPECT_EQ(pandArgs.pal->GetDefaultValue(), pandArgs.pal->GetValue());
    EXPECT_EQ(pandArgs.pal->GetType(), PandArgType::LIST);

    EXPECT_EQ(pandArgs.pair->GetDefaultValue(), REF_DEF_INT);
    EXPECT_EQ(pandArgs.pair->GetDefaultValue(), pandArgs.pair->GetValue());
    EXPECT_EQ(pandArgs.pair->GetType(), PandArgType::INTEGER);

    EXPECT_EQ(pandArgs.paur32->GetDefaultValue(), REF_DEF_UINT64);
    EXPECT_EQ(pandArgs.paur32->GetDefaultValue(), pandArgs.paur32->GetValue());
    EXPECT_EQ(pandArgs.paur32->GetType(), PandArgType::UINT32);

    EXPECT_EQ(pandArgs.paur64->GetDefaultValue(), REF_DEF_UINT64);
    EXPECT_EQ(pandArgs.paur64->GetDefaultValue(), pandArgs.paur64->GetValue());
    EXPECT_EQ(pandArgs.paur64->GetType(), PandArgType::UINT64);
}

static void TestDublicate(PandArgParser &paParser)
{
    // expect false on duplicate argument
    PandArg<int> paiDup("int", 0U, "Integer number 0");
    EXPECT_TRUE(paParser.IsArgSet(paiDup.GetName()));
    EXPECT_FALSE(paParser.Add(&paiDup));
}

static void TestTailFunctions(PandArgParser &paParser, TestTailPandArgs &tPandArgs)
{
    // add tail argument, expect false on duplicate
    // erase tail, expect 0 tail size
    EXPECT_EQ(paParser.GetTailSize(), 0U);
    EXPECT_TRUE(paParser.PushBackTail(tPandArgs.tPai));
    EXPECT_EQ(paParser.GetTailSize(), 1U);
    EXPECT_FALSE(paParser.PushBackTail(tPandArgs.tPai));
    paParser.PopBackTail();
    EXPECT_EQ(paParser.GetTailSize(), 0U);
}

static void TestHelpStringAndArgsList(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect help string formed right
    {
        std::string refString = "--" + pandArgs.pab->GetName() + ": " + pandArgs.pab->GetDesc() + "\n";
        refString += "--" + pandArgs.pald->GetName() + ": " + pandArgs.pald->GetDesc() + "\n";
        refString += "--" + pandArgs.pad->GetName() + ": " + pandArgs.pad->GetDesc() + "\n";
        refString += "--" + pandArgs.pai->GetName() + ": " + pandArgs.pai->GetDesc() + "\n";
        refString += "--" + pandArgs.pal->GetName() + ": " + pandArgs.pal->GetDesc() + "\n";
        refString += "--" + pandArgs.pair->GetName() + ": " + pandArgs.pair->GetDesc() + "\n";
        refString += "--" + pandArgs.paur32->GetName() + ": " + pandArgs.paur32->GetDesc() + "\n";
        refString += "--" + pandArgs.paur64->GetName() + ": " + pandArgs.paur64->GetDesc() + "\n";
        refString += "--" + pandArgs.pas->GetName() + ": " + pandArgs.pas->GetDesc() + "\n";
        refString += "--" + pandArgs.pau32->GetName() + ": " + pandArgs.pau32->GetDesc() + "\n";
        refString += "--" + pandArgs.pau64->GetName() + ": " + pandArgs.pau64->GetDesc() + "\n";
        EXPECT_EQ(paParser.GetHelpString(), refString);
    }

    // expect regular args list formed right
    {
        arg_list_t refArgDlist = pandArgs.pald->GetValue();
        arg_list_t refArgList = pandArgs.pal->GetValue();
        std::string refString =
            "--" + pandArgs.pab->GetName() + "=" + std::to_string(static_cast<int>(pandArgs.pab->GetValue())) + "\n";
        refString += "--" + pandArgs.pald->GetName() + "=";
        for (const auto &i : refArgDlist) {
            refString += i + ", ";
        }
        refString += "\n";
        refString += "--" + pandArgs.pad->GetName() + "=" + std::to_string(pandArgs.pad->GetValue()) + "\n";
        refString += "--" + pandArgs.pai->GetName() + "=" + std::to_string(pandArgs.pai->GetValue()) + "\n";
        refString += "--" + pandArgs.pal->GetName() + "=";
        for (const auto &i : refArgList) {
            refString += i + ", ";
        }
        refString += "\n";
        refString += "--" + pandArgs.pair->GetName() + "=" + std::to_string(pandArgs.pair->GetValue()) + "\n";
        refString += "--" + pandArgs.paur32->GetName() + "=" + std::to_string(pandArgs.paur32->GetValue()) + "\n";
        refString += "--" + pandArgs.paur64->GetName() + "=" + std::to_string(pandArgs.paur64->GetValue()) + "\n";
        refString += "--" + pandArgs.pas->GetName() + "=" + pandArgs.pas->GetValue() + "\n";
        refString += "--" + pandArgs.pau32->GetName() + "=" + std::to_string(pandArgs.pau32->GetValue()) + "\n";
        refString += "--" + pandArgs.pau64->GetName() + "=" + std::to_string(pandArgs.pau64->GetValue()) + "\n";
        EXPECT_EQ(paParser.GetRegularArgs(), refString);
    }
}

// NOLINTBEGIN(modernize-avoid-c-arrays)
static void TestBoolean(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect all boolean values processed right
    {
        static const char *trueValues[] = {"true", "on", "1"};
        static const char *falseValues[] = {"false", "off", "0"};
        static constexpr const int ARGC_BOOL_ONLY = 3;
        static const char *argvBoolOnly[ARGC_BOOL_ONLY];
        argvBoolOnly[0U] = "gtest_app";
        std::string s = "--" + pandArgs.pab->GetName();
        argvBoolOnly[1U] = s.c_str();

        // NOLINTNEXTLINE(modernize-loop-convert)
        for (size_t i = 0; i < 3U; i++) {
            argvBoolOnly[2U] = trueValues[i];
            EXPECT_TRUE(paParser.Parse(ARGC_BOOL_ONLY, argvBoolOnly));
            EXPECT_TRUE(pandArgs.pab->GetValue());
        }
        // NOLINTNEXTLINE(modernize-loop-convert)
        for (size_t i = 0; i < 3U; i++) {
            argvBoolOnly[2U] = falseValues[i];
            EXPECT_TRUE(paParser.Parse(ARGC_BOOL_ONLY, argvBoolOnly));
            EXPECT_FALSE(pandArgs.pab->GetValue());
        }
    }

    // expect wrong boolean arguments with "=" processed right
    {
        static const int ARGC_BOOL_ONLY = 2;
        static const char *argvBoolOnly[ARGC_BOOL_ONLY];
        argvBoolOnly[0U] = "gtest_app";
        std::string s = "--" + pandArgs.pab->GetName() + "=";
        argvBoolOnly[1U] = s.c_str();
        EXPECT_FALSE(paParser.Parse(ARGC_BOOL_ONLY, argvBoolOnly));
    }

    // expect boolean at the end of arguments line is true
    {
        static const int ARGC_BOOL_ONLY = 2;
        static const char *argvBoolOnly[ARGC_BOOL_ONLY];
        argvBoolOnly[0U] = "gtest_app";
        std::string s = "--" + pandArgs.pab->GetName();
        argvBoolOnly[1U] = s.c_str();
        EXPECT_TRUE(paParser.Parse(ARGC_BOOL_ONLY, argvBoolOnly));
        EXPECT_TRUE(pandArgs.pab->GetValue());
    }
}

static void TestSignedValues(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect positive and negative integer values processed right
    {
        static constexpr const int REF_INT_POS = 42422424;
        static constexpr const int REF_INT_NEG = -42422424;
        static constexpr const int ARGC_INT_ONLY = 3;
        static const char *argvIntOnly[ARGC_INT_ONLY];
        argvIntOnly[0U] = "gtest_app";
        std::string s = "--" + pandArgs.pai->GetName();
        argvIntOnly[1U] = s.c_str();
        argvIntOnly[2U] = "42422424";
        EXPECT_TRUE(paParser.Parse(ARGC_INT_ONLY, argvIntOnly));
        EXPECT_EQ(pandArgs.pai->GetValue(), REF_INT_POS);
        argvIntOnly[2U] = "-42422424";
        EXPECT_TRUE(paParser.Parse(ARGC_INT_ONLY, argvIntOnly));
        EXPECT_EQ(pandArgs.pai->GetValue(), REF_INT_NEG);
    }

    // expect positive and negative double values processed right
    {
        static constexpr const double REF_DOUBLE_POS = 4242.2424;
        static constexpr const double REF_DOUBLE_NEG = -4242.2424;
        static constexpr const int ARGC_DOUBLE_ONLY = 3;
        static const char *argvDoubleOnly[ARGC_DOUBLE_ONLY];
        argvDoubleOnly[0U] = "gtest_app";
        std::string s = "--" + pandArgs.pad->GetName();
        argvDoubleOnly[1U] = s.c_str();
        argvDoubleOnly[2U] = "4242.2424";
        EXPECT_TRUE(paParser.Parse(ARGC_DOUBLE_ONLY, argvDoubleOnly));
        EXPECT_EQ(pandArgs.pad->GetValue(), REF_DOUBLE_POS);
        argvDoubleOnly[2U] = "-4242.2424";
        EXPECT_TRUE(paParser.Parse(ARGC_DOUBLE_ONLY, argvDoubleOnly));
        EXPECT_EQ(pandArgs.pad->GetValue(), REF_DOUBLE_NEG);
    }
}

static void TestIntValues(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect uint32_t values processed right
    {
        static constexpr const uint32_t REF_UINT32_POS = 4242422424;
        static constexpr const int ARGC_UINT32_ONLY = 3;
        static const char *argvUint32Only[ARGC_UINT32_ONLY];
        argvUint32Only[0U] = "gtest_app";
        std::string s = "--" + pandArgs.pau32->GetName();
        argvUint32Only[1U] = s.c_str();
        argvUint32Only[2U] = "4242422424";
        EXPECT_TRUE(paParser.Parse(ARGC_UINT32_ONLY, argvUint32Only));
        EXPECT_EQ(pandArgs.pau32->GetValue(), REF_UINT32_POS);
    }

    // expect uint64_t values processed right
    {
        static constexpr const uint64_t REF_UINT64_POS = 424242422424;
        static constexpr const int ARGC_UINT64_ONLY = 3;
        static const char *argvUint64Only[ARGC_UINT64_ONLY];
        argvUint64Only[0U] = "gtest_app";
        std::string s = "--" + pandArgs.pau64->GetName();
        argvUint64Only[1U] = s.c_str();
        argvUint64Only[2U] = "424242422424";
        EXPECT_TRUE(paParser.Parse(ARGC_UINT64_ONLY, argvUint64Only));
        EXPECT_EQ(pandArgs.pau64->GetValue(), REF_UINT64_POS);
    }
}

static void TestOutOfRangeInt(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect out of range uint32_t values processed right
    {
        static constexpr const int ARGC_UINT32_ONLY = 3;
        static const char *argvUint32Only[ARGC_UINT32_ONLY];
        argvUint32Only[0U] = "gtest_app";
        std::string s = "--" + pandArgs.pau32->GetName();
        argvUint32Only[1U] = s.c_str();
        argvUint32Only[2U] = "424224244242242442422424";
        EXPECT_FALSE(paParser.Parse(ARGC_UINT32_ONLY, argvUint32Only));
        argvUint32Only[2U] = "0xffffffffffffffffffffffffff";
        EXPECT_FALSE(paParser.Parse(ARGC_UINT32_ONLY, argvUint32Only));
    }

    // expect out of range uint64_t values processed right
    {
        static const int ARGC_UINT64_ONLY = 3;
        static const char *argvUint64Only[ARGC_UINT64_ONLY];
        argvUint64Only[0U] = "gtest_app";
        std::string s = "--" + pandArgs.pau64->GetName();
        argvUint64Only[1U] = s.c_str();
        argvUint64Only[2U] = "424224244242242442422424";
        EXPECT_FALSE(paParser.Parse(ARGC_UINT64_ONLY, argvUint64Only));
        argvUint64Only[2U] = "0xffffffffffffffffffffffffff";
        EXPECT_FALSE(paParser.Parse(ARGC_UINT64_ONLY, argvUint64Only));
    }
}

static void TestStringArg(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect string argument of one word and multiple word processed right
    {
        static const std::string REF_ONE_STRING = "string";
        static const std::string REF_MULTIPLE_STRING = "this is a string";
        static const char *strArgname = "--string";
        static constexpr const int ARGC_ONE_STRING = 3;
        static const char *argvOneString[ARGC_ONE_STRING] = {"gtest_app", strArgname, "string"};
        static constexpr const int ARGC_MULTIPLE_STRING = 3;
        static const char *argvMultipleString[ARGC_MULTIPLE_STRING] = {"gtest_app", strArgname, "this is a string"};
        EXPECT_TRUE(paParser.Parse(ARGC_MULTIPLE_STRING, argvMultipleString));
        EXPECT_EQ(pandArgs.pas->GetValue(), REF_MULTIPLE_STRING);
        EXPECT_TRUE(paParser.Parse(ARGC_ONE_STRING, argvOneString));
        EXPECT_EQ(pandArgs.pas->GetValue(), REF_ONE_STRING);
    }

    // expect string at the end of line is an empty string
    {
        static constexpr const int ARGC_STRING_ONLY = 2;
        static const char *argvStringOnly[ARGC_STRING_ONLY];
        argvStringOnly[0U] = "gtest_app";
        std::string s = "--" + pandArgs.pas->GetName();
        argvStringOnly[1U] = s.c_str();
        EXPECT_TRUE(paParser.Parse(ARGC_STRING_ONLY, argvStringOnly));
        EXPECT_EQ(pandArgs.pas->GetValue(), "");
    }
}

static void TestListArg(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect list argument processed right
    {
        pandArgs.pald->ResetDefaultValue();
        static const arg_list_t REF_LIST = {"list1", "list2", "list3"};
        std::string s = "--" + pandArgs.pald->GetName();
        static const char *listArgname = s.c_str();
        static constexpr const int ARGC_LIST_ONLY = 7;
        static const char *argvListOnly[ARGC_LIST_ONLY] = {"gtest_app", listArgname, "list1", listArgname,
                                                           "list2",     listArgname, "list3"};
        EXPECT_TRUE(paParser.Parse(ARGC_LIST_ONLY, argvListOnly));
        ASSERT_EQ(pandArgs.pald->GetValue().size(), REF_LIST.size());
        for (std::size_t i = 0; i < REF_LIST.size(); ++i) {
            EXPECT_EQ(pandArgs.pald->GetValue()[i], REF_LIST[i]);
        }
    }

    // expect list argument without delimiter processed right
    {
        pandArgs.pal->ResetDefaultValue();
        static const arg_list_t REF_LIST = {"list1", "list2", "list3", "list4"};
        std::string s = "--" + pandArgs.pal->GetName();
        static const char *listArgname = s.c_str();
        static constexpr const int ARGC_LIST_ONLY = 9;
        static const char *argvListOnly[ARGC_LIST_ONLY] = {"gtest_app", listArgname, "list1",     listArgname, "list2",
                                                           listArgname, "list3",     listArgname, "list4"};
        EXPECT_TRUE(paParser.Parse(ARGC_LIST_ONLY, argvListOnly));
        ASSERT_EQ(pandArgs.pal->GetValue().size(), REF_LIST.size());
        for (std::size_t i = 0; i < REF_LIST.size(); ++i) {
            EXPECT_EQ(pandArgs.pal->GetValue()[i], REF_LIST[i]);
        }
    }
}

static void TestDelimiterListArg(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect delimiter list argument processed right
    {
        pandArgs.pald->ResetDefaultValue();
        static const arg_list_t REF_DLIST = {"dlist1", "dlist2", "dlist3"};
        std::string s = "--" + pandArgs.pald->GetName();
        static const char *listArgname = s.c_str();
        static constexpr const int ARGC_DLIST_ONLY = 3;
        static const char *argvDlistOnly[ARGC_DLIST_ONLY] = {"gtest_app", listArgname, "dlist1:dlist2:dlist3"};
        EXPECT_TRUE(paParser.Parse(ARGC_DLIST_ONLY, argvDlistOnly));
        ASSERT_EQ(pandArgs.pald->GetValue().size(), REF_DLIST.size());
        for (std::size_t i = 0; i < REF_DLIST.size(); ++i) {
            EXPECT_EQ(pandArgs.pald->GetValue()[i], REF_DLIST[i]);
        }
    }

    // expect delimiter and multiple list argument processed right
    {
        pandArgs.pald->ResetDefaultValue();
        static const arg_list_t REF_LIST = {"dlist1", "dlist2", "list1", "list2", "dlist3", "dlist4"};
        std::string s = "--" + pandArgs.pald->GetName();
        static const char *listArgname = s.c_str();
        static constexpr const int ARGC_LIST = 9;
        static const char *argvList[ARGC_LIST] = {"gtest_app", listArgname, "dlist1:dlist2", listArgname,    "list1",
                                                  listArgname, "list2",     listArgname,     "dlist3:dlist4"};
        EXPECT_TRUE(paParser.Parse(ARGC_LIST, argvList));
        ASSERT_EQ(pandArgs.pald->GetValue().size(), REF_LIST.size());
        for (std::size_t i = 0; i < REF_LIST.size(); ++i) {
            EXPECT_EQ(pandArgs.pald->GetValue()[i], REF_LIST[i]);
        }
    }
}

static void TestSignedIntWihRange(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect positive and negative integer values with range processed right
    {
        static constexpr const int REF_INT_POS = 99;
        static constexpr const int REF_INT_NEG = -99;
        static constexpr const int ARGC_INT_ONLY = 3;
        static const char *argvIntOnly[ARGC_INT_ONLY];
        argvIntOnly[0U] = "gtest_app";
        std::string s = "--" + pandArgs.pair->GetName();
        argvIntOnly[1U] = s.c_str();
        argvIntOnly[2U] = "99";
        EXPECT_TRUE(paParser.Parse(ARGC_INT_ONLY, argvIntOnly));
        EXPECT_EQ(pandArgs.pair->GetValue(), REF_INT_POS);
        argvIntOnly[2U] = "-99";
        EXPECT_TRUE(paParser.Parse(ARGC_INT_ONLY, argvIntOnly));
        EXPECT_EQ(pandArgs.pair->GetValue(), REF_INT_NEG);
    }

    // expect wrong positive and negative integer values with range processed right
    {
        static constexpr const int ARGC_INT_ONLY = 3;
        static const char *argvIntOnly[ARGC_INT_ONLY];
        argvIntOnly[0U] = "gtest_app";
        std::string s = "--" + pandArgs.pair->GetName();
        argvIntOnly[1U] = s.c_str();
        argvIntOnly[2U] = "101";
        EXPECT_FALSE(paParser.Parse(ARGC_INT_ONLY, argvIntOnly));
        argvIntOnly[2U] = "-101";
        EXPECT_FALSE(paParser.Parse(ARGC_INT_ONLY, argvIntOnly));
    }
}

static void TestUInt32WihRange(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect uint32_t values with range processed right
    {
        static constexpr const uint32_t REF_INT_MIN = 1;
        static constexpr const uint32_t REF_INT_MAX = 990000000;
        static constexpr const int ARGC_INT_ONLY = 3;
        static const char *argvIntOnly[ARGC_INT_ONLY];
        argvIntOnly[0U] = "gtest_app";
        std::string s = "--" + pandArgs.paur32->GetName();
        argvIntOnly[1U] = s.c_str();
        argvIntOnly[2U] = "1";
        EXPECT_TRUE(paParser.Parse(ARGC_INT_ONLY, argvIntOnly));
        EXPECT_EQ(pandArgs.paur32->GetValue(), REF_INT_MIN);
        argvIntOnly[2U] = "990000000";
        EXPECT_TRUE(paParser.Parse(ARGC_INT_ONLY, argvIntOnly));
        EXPECT_EQ(pandArgs.paur32->GetValue(), REF_INT_MAX);
    }

    // expect wrong uint32_t values with range processed right
    {
        static constexpr const int ARGC_INT_ONLY = 3;
        static const char *argvIntOnly[ARGC_INT_ONLY];
        argvIntOnly[0U] = "gtest_app";
        std::string s = "--" + pandArgs.paur32->GetName();
        argvIntOnly[1U] = s.c_str();
        argvIntOnly[2U] = "-1";
        EXPECT_FALSE(paParser.Parse(ARGC_INT_ONLY, argvIntOnly));
        argvIntOnly[2U] = "1000000001";
        EXPECT_FALSE(paParser.Parse(ARGC_INT_ONLY, argvIntOnly));
    }
}

static void TestEmptyString(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect list at the end of line is a list with empty string
    pandArgs.pald->ResetDefaultValue();
    static const arg_list_t REF_LIST = {""};
    static constexpr const int ARGC_LIST_ONLY = 2;
    static const char *argvListOnly[ARGC_LIST_ONLY];
    argvListOnly[0U] = "gtest_app";
    std::string s = "--" + pandArgs.pald->GetName();
    argvListOnly[1U] = s.c_str();
    EXPECT_TRUE(paParser.Parse(ARGC_LIST_ONLY, argvListOnly));
    EXPECT_EQ(pandArgs.pald->GetValue(), REF_LIST);
}

static void TestTailConsistency(PandArgParser &paParser, TestTailPandArgs &tPandArgs)
{
    // expect true on IsTailEnabled when tail is enabled, false otherwise
    {
        paParser.EnableTail();
        EXPECT_TRUE(paParser.IsTailEnabled());
        paParser.DisableTail();
        EXPECT_FALSE(paParser.IsTailEnabled());
    }

    // expect tail only argument is consistent
    {
        static constexpr const int ARGC_TAIL_ONLY = 2;
        static const char *argvTailOnly[] = {"gtest_app", "tail1"};
        static const std::string REF_STR_TAIL = "tail1";
        paParser.EnableTail();
        paParser.PushBackTail(tPandArgs.tPas);
        EXPECT_TRUE(paParser.Parse(ARGC_TAIL_ONLY, argvTailOnly));
        ASSERT_EQ(tPandArgs.tPas->GetValue(), REF_STR_TAIL);
        paParser.DisableTail();
        paParser.EraseTail();
    }

    // expect multiple tail only argument is consistent
    {
        static constexpr const int ARGC_TAIL_ONLY = 7;
        static const char *argvTailOnly[] = {"gtest_app", "str_tail", "off", "-4", "3.14", "2", "4"};
        static const std::string STR_REF = "str_tail";
        static constexpr const bool BOOL_REF = false;
        static constexpr const int INT_REF = -4;
        static constexpr const double DOUBLE_REF = 3.14;
        static constexpr const uint32_t UINT32_REF = 2;
        static constexpr const uint64_t UINT64_REF = 4;
        paParser.EnableTail();
        paParser.PushBackTail(tPandArgs.tPas);
        paParser.PushBackTail(tPandArgs.tPab);
        paParser.PushBackTail(tPandArgs.tPai);
        paParser.PushBackTail(tPandArgs.tPad);
        paParser.PushBackTail(tPandArgs.tPau32);
        paParser.PushBackTail(tPandArgs.tPau64);
        EXPECT_EQ(paParser.GetTailSize(), 6U);
        EXPECT_TRUE(paParser.Parse(ARGC_TAIL_ONLY, argvTailOnly));
        EXPECT_EQ(tPandArgs.tPas->GetValue(), STR_REF);
        EXPECT_EQ(tPandArgs.tPab->GetValue(), BOOL_REF);
        EXPECT_EQ(tPandArgs.tPai->GetValue(), INT_REF);
        EXPECT_DOUBLE_EQ(tPandArgs.tPad->GetValue(), DOUBLE_REF);
        EXPECT_EQ(tPandArgs.tPau32->GetValue(), UINT32_REF);
        EXPECT_EQ(tPandArgs.tPau64->GetValue(), UINT64_REF);
        paParser.DisableTail();
        paParser.EraseTail();
        EXPECT_EQ(paParser.GetTailSize(), 0U);
    }
}
static void TestUInt64WihRange(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect uint64_t values with range processed right
    {
        static const uint64_t REF_INT_MIN = 1;
        static const uint64_t REF_INT_MAX = 99000000000;
        static const int ARGC_INT_ONLY = 3;
        static const char *argvIntOnly[ARGC_INT_ONLY];
        argvIntOnly[0U] = "gtest_app";
        std::string s = "--" + pandArgs.paur64->GetName();
        argvIntOnly[1U] = s.c_str();
        argvIntOnly[2U] = "1";
        EXPECT_TRUE(paParser.Parse(ARGC_INT_ONLY, argvIntOnly));
        EXPECT_EQ(pandArgs.paur64->GetValue(), REF_INT_MIN);
        argvIntOnly[2U] = "99000000000";
        EXPECT_TRUE(paParser.Parse(ARGC_INT_ONLY, argvIntOnly));
        EXPECT_EQ(pandArgs.paur64->GetValue(), REF_INT_MAX);
    }

    // expect wrong uint64_t values with range processed right
    {
        static constexpr const int ARGC_INT_ONLY = 3;
        static const char *argvIntOnly[ARGC_INT_ONLY];
        argvIntOnly[0U] = "gtest_app";
        std::string s = "--" + pandArgs.paur64->GetName();
        argvIntOnly[1U] = s.c_str();
        argvIntOnly[2U] = "-1";
        EXPECT_FALSE(paParser.Parse(ARGC_INT_ONLY, argvIntOnly));
        argvIntOnly[2U] = "100000000001";
        EXPECT_FALSE(paParser.Parse(ARGC_INT_ONLY, argvIntOnly));
    }
}

static void TestWrongTailArg(PandArgParser &paParser, TestTailPandArgs &tPandArgs)
{
    // expect parse fail on wrong tail argument type
    paParser.EnableTail();
    static constexpr const int ARGC_TAIL_ONLY = 3;
    // boolean value instead of integer
    static const char *argvTailOnly[] = {"gtest_app", "str_tail", "off"};
    static const std::string STR_REF = "str_tail";
    paParser.PushBackTail(tPandArgs.tPas);
    paParser.PushBackTail(tPandArgs.tPai);
    EXPECT_EQ(paParser.GetTailSize(), 2U);
    EXPECT_FALSE(paParser.Parse(ARGC_TAIL_ONLY, argvTailOnly));
    EXPECT_EQ(tPandArgs.tPas->GetValue(), STR_REF);
    paParser.DisableTail();
    paParser.EraseTail();
    EXPECT_EQ(paParser.GetTailSize(), 0U);
}

static void TestTailAfterString(PandArgParser &paParser, TestPandArgs &pandArgs, TestTailPandArgs &tPandArgs)
{
    // expect right tail argument processing after preceiding string argument
    paParser.EnableTail();
    static const char *strArgname = "--string";
    static const std::string REF_STRING = "this is a reference string";
    static const std::string REF_T_STR = "string";
    static constexpr const double REF_T_DOUBLE = 0.1;
    static constexpr const bool REF_T_BOOL = true;
    static constexpr const uint32_t REF_T_UINT32 = 32;
    static constexpr const uint64_t REF_T_UINT64 = 64;
    static constexpr const int ARGC_TAIL_STRING = 8;
    static const char *argvTailString[] = {"gtest_app", strArgname, "this is a reference string", "string", ".1", "on",
                                           "32",        "64"};
    paParser.PushBackTail(tPandArgs.tPas);
    paParser.PushBackTail(tPandArgs.tPad);
    paParser.PushBackTail(tPandArgs.tPab);
    paParser.PushBackTail(tPandArgs.tPau32);
    paParser.PushBackTail(tPandArgs.tPau64);
    EXPECT_TRUE(paParser.Parse(ARGC_TAIL_STRING, argvTailString));
    EXPECT_EQ(pandArgs.pas->GetValue(), REF_STRING);
    EXPECT_EQ(tPandArgs.tPas->GetValue(), REF_T_STR);
    EXPECT_EQ(tPandArgs.tPad->GetValue(), REF_T_DOUBLE);
    EXPECT_EQ(tPandArgs.tPab->GetValue(), REF_T_BOOL);
    EXPECT_EQ(tPandArgs.tPau32->GetValue(), REF_T_UINT32);
    EXPECT_EQ(tPandArgs.tPau64->GetValue(), REF_T_UINT64);
    paParser.DisableTail();
    paParser.EraseTail();
}

static void TestTailAfterList(PandArgParser &paParser, TestPandArgs &pandArgs, TestTailPandArgs &tPandArgs)
{
    // expect right tail argument processing after preceiding list argument
    pandArgs.pald->ResetDefaultValue();
    paParser.EnableTail();
    static const char *listArgname = "--dlist";
    static const arg_list_t REF_LIST = {"list1", "list2", "list3", "list4", "list5"};
    static constexpr const double REF_T_DOUBLE = -7;
    static constexpr const bool REF_T_BOOL = true;
    static constexpr const int REF_T_INT = 255;
    static constexpr const uint32_t REF_T_UINT32 = 32;
    static constexpr const uint64_t REF_T_UINT64 = 64;
    static constexpr const int ARGC_TAIL_LIST = 16;
    static const char *argvTailList[] = {"gtest_app", listArgname, "list1", listArgname, "list2", listArgname,
                                         "list3",     listArgname, "list4", listArgname, "list5", "true",
                                         "255",       "-7",        "32",    "64"};
    paParser.PushBackTail(tPandArgs.tPab);
    paParser.PushBackTail(tPandArgs.tPai);
    paParser.PushBackTail(tPandArgs.tPad);
    paParser.PushBackTail(tPandArgs.tPau32);
    paParser.PushBackTail(tPandArgs.tPau64);
    EXPECT_TRUE(paParser.Parse(ARGC_TAIL_LIST, argvTailList));
    ASSERT_EQ(pandArgs.pald->GetValue().size(), REF_LIST.size());
    for (std::size_t i = 0; i < REF_LIST.size(); i++) {
        EXPECT_EQ(pandArgs.pald->GetValue()[i], REF_LIST[i]);
    }
    EXPECT_EQ(tPandArgs.tPab->GetValue(), REF_T_BOOL);
    EXPECT_EQ(tPandArgs.tPai->GetValue(), REF_T_INT);
    EXPECT_DOUBLE_EQ(tPandArgs.tPad->GetValue(), REF_T_DOUBLE);
    EXPECT_EQ(tPandArgs.tPau32->GetValue(), REF_T_UINT32);
    EXPECT_EQ(tPandArgs.tPau64->GetValue(), REF_T_UINT64);

    paParser.DisableTail();
    paParser.EraseTail();
}

static void TestTailAfterNoparamBoolean(PandArgParser &paParser, TestPandArgs &pandArgs, TestTailPandArgs &tPandArgs,
                                        const std::string &refDefStr)
{
    // expect right tail argument processing after noparam boolean argument
    paParser.EnableTail();
    PandArg<std::string> tPas0("tail_string0", refDefStr, "Sample tail string argument 0");
    PandArg<std::string> tPas1("tail_string1", refDefStr, "Sample tail string argument 1");
    static const std::string REF_T_STR1 = "offtail1";
    static const std::string REF_T_STR2 = "offtail2";
    static const std::string REF_T_STR3 = "offtail3";
    static constexpr const int ARGC_TAIL_BOOL = 5;
    static const char *argvTailBool[] = {"gtest_app", "--bool", "offtail1", "offtail2", "offtail3"};
    paParser.PushBackTail(tPandArgs.tPas);
    paParser.PushBackTail(&tPas0);
    paParser.PushBackTail(&tPas1);
    EXPECT_TRUE(paParser.Parse(ARGC_TAIL_BOOL, argvTailBool));
    EXPECT_TRUE(pandArgs.pab->GetValue());
    EXPECT_EQ(tPandArgs.tPas->GetValue(), REF_T_STR1);
    EXPECT_EQ(tPas0.GetValue(), REF_T_STR2);
    EXPECT_EQ(tPas1.GetValue(), REF_T_STR3);
    paParser.DisableTail();
    paParser.EraseTail();
}

static void TestTailExceedsArgsNum(PandArgParser &paParser, TestTailPandArgs &tPandArgs, const std::string &refDefStr)
{
    // expect fail on amount of tail arguments more then pa_parser may have
    paParser.EnableTail();
    static const int ARGC_TAIL = 5;
    static const char *argvTail[] = {"gtest_app", "gdb", "--args", "file.bin", "entry"};

    PandArg<std::string> tPas1("tail_string1", refDefStr, "Sample tail string argument 1");
    paParser.PushBackTail(tPandArgs.tPas);
    paParser.PushBackTail(&tPas1);

    EXPECT_EQ(paParser.GetTailSize(), 2U);
    EXPECT_FALSE(paParser.Parse(ARGC_TAIL, argvTail));
    paParser.DisableTail();
    paParser.EraseTail();
}

static void TestTailRemainder(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect remainder arguments only parsed as expected
    {
        paParser.EnableRemainder();
        static const arg_list_t REF_REM = {"rem1", "rem2", "rem3"};
        static int argcRem = 5;
        static const char *argvRem[] = {"gtest_app", "--", "rem1", "rem2", "rem3"};
        paParser.Parse(argcRem, argvRem);
        arg_list_t remainder = paParser.GetRemainder();
        EXPECT_EQ(remainder.size(), REF_REM.size());
        for (std::size_t i = 0; i < remainder.size(); i++) {
            EXPECT_EQ(remainder[i], REF_REM[i]);
        }
        paParser.DisableRemainder();
    }

    // expect regular argument before remainder parsed right
    {
        paParser.EnableRemainder();
        static const arg_list_t REF_REM = {"rem1", "rem2", "rem3"};
        std::string boolName = "--" + pandArgs.pab->GetName();
        static int argcRem = 6;
        static const char *argvRem[] = {"gtest_app", boolName.c_str(), "--", "rem1", "rem2", "rem3"};
        paParser.Parse(argcRem, argvRem);
        EXPECT_TRUE(pandArgs.pab->GetValue());
        arg_list_t remainder = paParser.GetRemainder();
        EXPECT_EQ(remainder.size(), REF_REM.size());
        for (std::size_t i = 0; i < remainder.size(); i++) {
            EXPECT_EQ(remainder[i], REF_REM[i]);
        }
        paParser.DisableRemainder();
    }
}

static void TestAllArgs(PandArgParser &paParser, TestPandArgs &pandArgs, TestTailPandArgs &tPandArgs,
                        const std::string &refDefStr)
{
    // expect that all arguments parsed as expected
    pandArgs.pald->ResetDefaultValue();
    paParser.EnableTail();
    paParser.EnableRemainder();
    static const arg_list_t REF_REM = {"rem1", "rem2", "rem3"};
    PandArg<std::string> tPas0("tail_string0", refDefStr, "Sample tail string argument 0");
    PandArg<std::string> tPas1("tail_string1", refDefStr, "Sample tail string argument 1");
    static constexpr const bool REF_BOOL = true;
    static constexpr const int REF_INT = 42;
    static const arg_list_t REF_DLIST = {"dlist1", "dlist2", "dlist3", "dlist4"};
    static const std::string REF_T_STR1 = "tail1";
    static const std::string REF_T_STR2 = "tail2 tail3";
    static const std::string REF_T_STR3 = "tail4";
    static const std::string REF_STR = "this is a string";
    static constexpr const double REF_DBL = 0.42;
    static constexpr const uint32_t REF_UINT32 = std::numeric_limits<std::uint32_t>::max();
    static constexpr const uint32_t REF_UINT32R = 990000000;
    static constexpr const uint64_t REF_UINT64 = std::numeric_limits<std::uint64_t>::max();
    static constexpr const uint64_t REF_UINT64R = 99000000000;

    paParser.PushBackTail(tPandArgs.tPas);
    paParser.PushBackTail(&tPas0);
    paParser.PushBackTail(&tPas1);
    EXPECT_TRUE(paParser.Parse(ARGC_CONSISTENT, ARGV_CONSISTENT));
    EXPECT_EQ(pandArgs.pab->GetValue(), REF_BOOL);
    EXPECT_EQ(pandArgs.pai->GetValue(), REF_INT);
    EXPECT_EQ(pandArgs.pas->GetValue(), REF_STR);
    EXPECT_DOUBLE_EQ(pandArgs.pad->GetValue(), REF_DBL);
    EXPECT_EQ(pandArgs.pau32->GetValue(), REF_UINT32);
    EXPECT_EQ(pandArgs.pau64->GetValue(), REF_UINT64);
    ASSERT_EQ(pandArgs.pald->GetValue().size(), REF_DLIST.size());
    for (std::size_t i = 0; i < REF_DLIST.size(); ++i) {
        EXPECT_EQ(pandArgs.pald->GetValue()[i], REF_DLIST[i]);
    }
    EXPECT_EQ(pandArgs.pair->GetValue(), REF_INT);
    EXPECT_EQ(pandArgs.paur32->GetValue(), REF_UINT32R);
    EXPECT_EQ(pandArgs.paur64->GetValue(), REF_UINT64R);
    EXPECT_EQ(tPandArgs.tPas->GetValue(), REF_T_STR1);
    EXPECT_EQ(tPas0.GetValue(), REF_T_STR2);
    EXPECT_EQ(tPas1.GetValue(), REF_T_STR3);
    arg_list_t remainder = paParser.GetRemainder();
    EXPECT_EQ(remainder.size(), REF_REM.size());
    for (std::size_t i = 0; i < remainder.size(); i++) {
        EXPECT_EQ(remainder[i], REF_REM[i]);
    }
    paParser.DisableRemainder();
    paParser.DisableTail();
    paParser.EraseTail();
}

static void TestHexValues(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    // expect hex values processed right
    {
        static constexpr const uint64_t REF_UINT64 = 274877906959;
        static constexpr const int REF_INT = 64;
        static constexpr const int ARGC_UINT64_INT = 3;
        static const char *argvUint64Int[ARGC_UINT64_INT];
        argvUint64Int[0U] = "gtest_app";
        std::string s = "--" + pandArgs.pau64->GetName();
        argvUint64Int[1U] = s.c_str();
        argvUint64Int[2U] = "0x400000000f";
        EXPECT_TRUE(paParser.Parse(ARGC_UINT64_INT, argvUint64Int));
        EXPECT_EQ(pandArgs.pau64->GetValue(), REF_UINT64);
        argvUint64Int[2U] = "0x40";
        EXPECT_TRUE(paParser.Parse(ARGC_UINT64_INT, argvUint64Int));
        EXPECT_EQ(pandArgs.pau64->GetValue(), REF_INT);
    }

    {
        static constexpr const uint64_t REF_UINT64 = 274877906944;
        static constexpr const int REF_INT = 64;
        static constexpr const int ARGC_UINT64_INT = 3;
        static const char *argvUint64Int[ARGC_UINT64_INT];
        argvUint64Int[0U] = "gtest_app";
        std::string s = "--" + pandArgs.pau64->GetName();
        argvUint64Int[1U] = s.c_str();
        argvUint64Int[2U] = "0x4000000000";
        EXPECT_TRUE(paParser.Parse(ARGC_UINT64_INT, argvUint64Int));
        EXPECT_EQ(pandArgs.pau64->GetValue(), REF_UINT64);
        argvUint64Int[2U] = "0x40";
        EXPECT_TRUE(paParser.Parse(ARGC_UINT64_INT, argvUint64Int));
        EXPECT_EQ(pandArgs.pau64->GetValue(), REF_INT);
    }
}

static void RunAPITests(PandArgParser &paParser, TestPandArgs &pandArgs, TestTailPandArgs &tPandArgs,
                        const std::string &refDefStr)
{
    TestArgSet(paParser, pandArgs);

    TestTypeConsistency(pandArgs, refDefStr);

    TestDublicate(paParser);

    TestTailFunctions(paParser, tPandArgs);

    TestHelpStringAndArgsList(paParser, pandArgs);

    TestBoolean(paParser, pandArgs);

    TestSignedValues(paParser, pandArgs);

    TestIntValues(paParser, pandArgs);

    TestHexValues(paParser, pandArgs);

    TestOutOfRangeInt(paParser, pandArgs);

    TestStringArg(paParser, pandArgs);

    TestListArg(paParser, pandArgs);

    TestDelimiterListArg(paParser, pandArgs);

    TestSignedIntWihRange(paParser, pandArgs);

    TestUInt32WihRange(paParser, pandArgs);

    TestUInt64WihRange(paParser, pandArgs);

    TestEmptyString(paParser, pandArgs);

    TestTailConsistency(paParser, tPandArgs);

    TestWrongTailArg(paParser, tPandArgs);

    TestTailAfterString(paParser, pandArgs, tPandArgs);

    TestTailAfterList(paParser, pandArgs, tPandArgs);

    TestTailAfterNoparamBoolean(paParser, pandArgs, tPandArgs, refDefStr);

    TestTailExceedsArgsNum(paParser, tPandArgs, refDefStr);

    TestTailRemainder(paParser, pandArgs);

    TestAllArgs(paParser, pandArgs, tPandArgs, refDefStr);
}

static void AddArgsToParser(PandArgParser &paParser, TestPandArgs &pandArgs)
{
    EXPECT_TRUE(paParser.Add(pandArgs.pab));
    EXPECT_TRUE(paParser.Add(pandArgs.pai));
    EXPECT_TRUE(paParser.Add(pandArgs.pad));
    EXPECT_TRUE(paParser.Add(pandArgs.pas));
    EXPECT_TRUE(paParser.Add(pandArgs.pau32));
    EXPECT_TRUE(paParser.Add(pandArgs.pau64));
    EXPECT_TRUE(paParser.Add(pandArgs.pald));
    EXPECT_TRUE(paParser.Add(pandArgs.pal));
    EXPECT_TRUE(paParser.Add(pandArgs.pair));
    EXPECT_TRUE(paParser.Add(pandArgs.paur32));
    EXPECT_TRUE(paParser.Add(pandArgs.paur64));
}

TEST(libpandargs, TestAPI)
{
    static const std::string REF_DEF_STRING = "noarg";
    static const arg_list_t REF_DEF_DLIST = arg_list_t();
    static const arg_list_t REF_DEF_LIST = arg_list_t();

    PandArg<bool> pab("bool", REF_DEF_BOOL, "Sample boolean argument");
    PandArg<int> pai("int", REF_DEF_INT, "Sample integer argument");
    PandArg<double> pad("double", REF_DEF_DOUBLE, "Sample rational argument");
    PandArg<std::string> pas("string", REF_DEF_STRING, "Sample string argument");
    PandArg<uint32_t> pau32("uint32", REF_DEF_UINT32, "Sample uint32 argument");
    PandArg<uint64_t> pau64("uint64", REF_DEF_UINT64, "Sample uint64 argument");
    PandArg<arg_list_t> pald("dlist", REF_DEF_DLIST, "Sample delimiter list argument", ":");
    PandArg<arg_list_t> pal("list", REF_DEF_LIST, "Sample list argument");
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<int> pair("rint", REF_DEF_INT, "Integer argument with range", -100L, 100U);
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<uint32_t> paur32("ruint32", REF_DEF_UINT64, "uint32 argument with range", 0U, 1000000000U);
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<uint64_t> paur64("ruint64", REF_DEF_UINT64, "uint64 argument with range", 0U, 100000000000U);

    TestPandArgs pandArgs {};
    pandArgs.pab = &pab;
    pandArgs.pai = &pai;
    pandArgs.pad = &pad;
    pandArgs.pas = &pas;
    pandArgs.pau32 = &pau32;
    pandArgs.pau64 = &pau64;
    pandArgs.pald = &pald;
    pandArgs.pal = &pal;
    pandArgs.pair = &pair;
    pandArgs.paur32 = &paur32;
    pandArgs.paur64 = &paur64;

    PandArgParser paParser;
    AddArgsToParser(paParser, pandArgs);

    PandArg<bool> tPab("tail_bool", REF_DEF_BOOL, "Sample tail boolean argument");
    PandArg<int> tPai("tail_int", REF_DEF_INT, "Sample tail integer argument");
    PandArg<double> tPad("tail_double", REF_DEF_DOUBLE, "Sample tail rational argument");
    PandArg<std::string> tPas("tail_string", REF_DEF_STRING, "Sample tail string argument");
    PandArg<uint32_t> tPau32("tail_uint32", REF_DEF_UINT32, "Sample tail uint32 argument");
    PandArg<uint64_t> tPau64("tail_uint64", REF_DEF_UINT64, "Sample tail uint64 argument");

    TestTailPandArgs tPandArgs {};
    tPandArgs.tPab = &tPab;
    tPandArgs.tPai = &tPai;
    tPandArgs.tPad = &tPad;
    tPandArgs.tPas = &tPas;
    tPandArgs.tPau32 = &tPau32;
    tPandArgs.tPau64 = &tPau64;

    RunAPITests(paParser, pandArgs, tPandArgs, REF_DEF_STRING);
}

TEST(libpandargs, NoSubArgs)
{
    PandArg<bool> subBoolArg("bool", false, "Sample boolean argument");
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<int> subIntArg("int", 12U, "Sample integer argument");
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<double> subDoubleArg("double", 123.45, "Sample rational argument");
    PandArg<std::string> subStringArg("string", "Hello", "Sample string argument");
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<int> intArg("global_int", 123U, "Global integer argument");
    PandArgCompound parent("compound", "Sample boolean argument",
                           {&subBoolArg, &subIntArg, &subDoubleArg, &subStringArg});

    PandArgParser paParser;
    ASSERT_TRUE(paParser.Add(&intArg));
    ASSERT_TRUE(paParser.Add(&parent));

    /* Should work well with no sub arguments */
    {
        parent.ResetDefaultValue();
        static const char *argv[] = {"gtest_app", "--compound"};
        ASSERT_TRUE(paParser.Parse(2U, argv)) << paParser.GetErrorString();
        ASSERT_EQ(parent.GetValue(), true);
        ASSERT_EQ(subBoolArg.GetValue(), false);
        ASSERT_EQ(subIntArg.GetValue(), 12U);
        ASSERT_EQ(subDoubleArg.GetValue(), 123.45);
        ASSERT_EQ(subStringArg.GetValue(), "Hello");
    }

    {
        parent.ResetDefaultValue();
        static const char *argv[] = {"gtest_app", "--compound:bool,int=2,double=54.321,string=World"};
        ASSERT_TRUE(paParser.Parse(2U, argv)) << paParser.GetErrorString();
        ASSERT_EQ(parent.GetValue(), true);
        ASSERT_EQ(subBoolArg.GetValue(), true);
        ASSERT_EQ(subIntArg.GetValue(), 2U);
        ASSERT_EQ(subDoubleArg.GetValue(), 54.321);
        ASSERT_EQ(subStringArg.GetValue(), "World");
    }
}

TEST(libpandargs, ResetSubArgs)
{
    PandArg<bool> subBoolArg("bool", false, "Sample boolean argument");
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<int> subIntArg("int", 12U, "Sample integer argument");
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<double> subDoubleArg("double", 123.45, "Sample rational argument");
    PandArg<std::string> subStringArg("string", "Hello", "Sample string argument");
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<int> intArg("global_int", 123U, "Global integer argument");
    PandArgCompound parent("compound", "Sample boolean argument",
                           {&subBoolArg, &subIntArg, &subDoubleArg, &subStringArg});

    PandArgParser paParser;
    ASSERT_TRUE(paParser.Add(&intArg));
    ASSERT_TRUE(paParser.Add(&parent));

    /* ResetDefaultValue should reset all sub arguments */
    parent.ResetDefaultValue();
    ASSERT_EQ(parent.GetValue(), false);
    ASSERT_EQ(subBoolArg.GetValue(), false);
    ASSERT_EQ(subIntArg.GetValue(), 12U);
    ASSERT_EQ(subDoubleArg.GetValue(), 123.45);
    ASSERT_EQ(subStringArg.GetValue(), "Hello");

    static const char *argv1[] = {"gtest_app", "--compound:bool=true"};
    ASSERT_TRUE(paParser.Parse(2U, argv1)) << paParser.GetErrorString();
    ASSERT_EQ(parent.GetValue(), true);
    ASSERT_EQ(subBoolArg.GetValue(), true);

    parent.ResetDefaultValue();
    static const char *argv2[] = {"gtest_app", "--compound:bool"};
    ASSERT_TRUE(paParser.Parse(2U, argv2)) << paParser.GetErrorString();
    ASSERT_EQ(parent.GetValue(), true);
    ASSERT_EQ(subBoolArg.GetValue(), true);

    static const char *argv3[] = {"gtest_app", "--compound:bool=false"};
    ASSERT_TRUE(paParser.Parse(2U, argv3)) << paParser.GetErrorString();
    ASSERT_EQ(parent.GetValue(), true);
    ASSERT_EQ(subBoolArg.GetValue(), false);

    parent.ResetDefaultValue();
    static const char *argv4[] = {"gtest_app", "--global_int=321"};
    ASSERT_TRUE(paParser.Parse(2U, argv4)) << paParser.GetErrorString();
    ASSERT_EQ(parent.GetValue(), false);
    ASSERT_EQ(intArg.GetValue(), 321U);

    parent.ResetDefaultValue();
    static const char *argv5[] = {"gtest_app", "--compound", "--global_int", "321"};
    ASSERT_TRUE(paParser.Parse(4U, argv5)) << paParser.GetErrorString();
    ASSERT_EQ(parent.GetValue(), true);
    ASSERT_EQ(intArg.GetValue(), 321U);
}

TEST(libpandargs, SubArgsVisibility)
{
    PandArg<bool> subBoolArg("bool", false, "Sample boolean argument");
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<int> subIntArg("int", 12U, "Sample integer argument");
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<double> subDoubleArg("double", 123.45, "Sample rational argument");
    PandArg<std::string> subStringArg("string", "Hello", "Sample string argument");
    // NOLINTNEXTLINE(readability-magic-numbers)
    PandArg<int> intArg("global_int", 123U, "Global integer argument");
    PandArgCompound parent("compound", "Sample boolean argument",
                           {&subBoolArg, &subIntArg, &subDoubleArg, &subStringArg});

    PandArgParser paParser;
    ASSERT_TRUE(paParser.Add(&intArg));
    ASSERT_TRUE(paParser.Add(&parent));

    /* Test that sub arguments are not visible in the global space */
    {
        static const char *argv[] = {"gtest_app", "--bool"};
        ASSERT_FALSE(paParser.Parse(2U, argv));
    }
    {
        static const char *argv[] = {"gtest_app", "--int=2"};
        ASSERT_FALSE(paParser.Parse(2U, argv));
    }
    {
        static const char *argv[] = {"gtest_app", "--double=54.321"};
        ASSERT_FALSE(paParser.Parse(2U, argv));
    }
    {
        static const char *argv[] = {"gtest_app", "--string=World"};
        ASSERT_FALSE(paParser.Parse(2U, argv));
    }
}

TEST(libpandargs, IncorrectCompoundArgs)
{
    static const arg_list_t REF_DEF_LIST = arg_list_t();
    PandArg<arg_list_t> pal("list", REF_DEF_LIST, "Sample list argument");

    PandArgParser paParser;
    EXPECT_TRUE(paParser.Add(&pal));
    pal.ResetDefaultValue();

    /* Test that incorrect using of compound argument syntax for non-compound argument produces error*/
    static const char *argv[] = {"gtest_app", "--list:list_arg1:list_arg2"};
    ASSERT_FALSE(paParser.Parse(2U, argv)) << paParser.GetErrorString();
}

TEST(libpandargs, IncorrectIntArgs)
{
    PandArg<int> pai("int", 0, "Sample int argument");
    PandArg<uint32_t> pau32("uint32", 0, "Sample uin32 argument");
    PandArg<uint64_t> pau64("uint64", 0, "Sample uin64 argument");

    PandArgParser paParser;
    EXPECT_TRUE(paParser.Add(&pai));
    EXPECT_TRUE(paParser.Add(&pau32));
    EXPECT_TRUE(paParser.Add(&pau64));

    {
        static const char *argv[] = {"int", "20000000000000000000"};  // > 2^64
        ASSERT_FALSE(paParser.Parse(2U, argv)) << paParser.GetErrorString();
    }
    {
        static const char *argv[] = {"uint32", "5000000000"};  // > 2^32
        ASSERT_FALSE(paParser.Parse(2U, argv)) << paParser.GetErrorString();
    }
    {
        static const char *argv[] = {"uint64", "20000000000000000000"};  // > 2^64
        ASSERT_FALSE(paParser.Parse(2U, argv)) << paParser.GetErrorString();
    }
}

// NOLINTEND(modernize-avoid-c-arrays)

}  // namespace ark::test
