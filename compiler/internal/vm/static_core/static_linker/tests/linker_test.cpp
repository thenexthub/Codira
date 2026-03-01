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
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

#include "assembler/assembly-field.h"
#include "assembler/assembly-function.h"
#include "assembler/assembly-program.h"
#include "assembler/assembly-record.h"
#include "assembler/assembly-emitter.h"
#include "assembler/assembly-parser.h"
#include "assembler/ins_create_api.h"
#include "libarkfile/file_item_container.h"
#include "libarkfile/file_writer.h"
#include "include/runtime_options.h"
#include "linker.h"
#include "linker_context.h"
#include "linker_context_misc.cpp"
#include "runtime/include/runtime.h"
#include <libarkfile/include/source_lang_enum.h>

namespace {
using Config = ark::static_linker::Config;
using Result = ark::static_linker::Result;
using ark::static_linker::DefaultConfig;
using ark::static_linker::Link;

constexpr size_t TEST_REPEAT_COUNT = 10;

std::pair<int, std::string> ExecPanda(const std::string &file, const std::string &verificationMode = "ahead-of-time",
                                      const std::string &loadRuntimes = "core",
                                      const std::string &entryPoint = "_GLOBAL::main")
{
    auto opts = ark::RuntimeOptions {};
    if (loadRuntimes == "core") {
        auto boot = opts.GetBootPandaFiles();
        for (auto &a : boot) {
            a.insert(0, "../");
        }
        opts.SetBootPandaFiles(std::move(boot));
    } else {
        return {1, "unknown loadRuntimes " + loadRuntimes};
    }

    opts.SetLoadRuntimes({loadRuntimes});
    opts.SetVerificationMode(verificationMode);

    opts.SetPandaFiles({file});
    if (!ark::Runtime::Create(opts)) {
        return {1, "can't create runtime"};
    }

    auto *runtime = ark::Runtime::GetCurrent();

    std::stringstream strBuf;
    auto old = std::cout.rdbuf(strBuf.rdbuf());
    auto reset = [&old](auto *cout) { cout->rdbuf(old); };
    auto guard = std::unique_ptr<std::ostream, decltype(reset)>(&std::cout, reset);

    auto res = runtime->ExecutePandaFile(file, entryPoint, {});
    auto ret = std::pair<int, std::string> {};
    if (!res) {
        ret = {1, "error " + std::to_string((int)res.Error())};
    } else {
        ret = {0, strBuf.str()};
    }

    if (!ark::Runtime::Destroy()) {
        return {1, "can't destroy runtime"};
    }

    return ret;
}

template <bool IS_BINARY>
bool ReadFile(const std::string &path, std::conditional_t<IS_BINARY, std::vector<char>, std::string> &out)
{
    auto f = std::ifstream(path, IS_BINARY ? std::ios_base::binary : std::ios_base::in);
    if (!f.is_open() || f.bad()) {
        return false;
    }

    out.clear();

    out.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    return true;
}

// removes comments
void NormalizeGold(std::string &gold)
{
    std::string_view in = gold;
    std::string out;
    out.reserve(gold.size());
    while (!in.empty()) {
        auto nxtNl = in.find('\n');
        if (in[0] == '#') {
            if (nxtNl == std::string::npos) {
                break;
            }
            in = in.substr(nxtNl + 1);
            continue;
        }
        if (nxtNl == std::string::npos) {
            out += in;
            break;
        }
        out += in.substr(0, nxtNl + 1);
        in = in.substr(nxtNl + 1);
    }
    gold = std::move(out);
}

std::optional<std::string> Build(const std::string &path)
{
    std::string prog;

    if (!ReadFile<false>(path + ".pa", prog)) {
        return "can't read file " + path + ".pa";
    }
    ark::pandasm::Parser p;
    auto res = p.Parse(prog, path + ".pa");
    if (p.ShowError().err != ark::pandasm::Error::ErrorType::ERR_NONE) {
        return p.ShowError().message + "\n" + p.ShowError().wholeLine;
    }

    if (!res.HasValue()) {
        return "no parsed value";
    }

    auto writer = ark::panda_file::FileWriter(path + ".abc");
    if (!ark::pandasm::AsmEmitter::Emit(&writer, res.Value())) {
        return "can't emit";
    }

    return std::nullopt;
}

// CC-OFFNXT(G.FUN.01-CPP, readability-function-size_parameters) function for testing
void TestSingle(const std::string &path, bool isGood = true, const Config &conf = ark::static_linker::DefaultConfig(),
                bool *succeded = nullptr, Result *saveResult = nullptr,
                const std::string &verificationMode = "ahead-of-time")
{
    const auto pathPrefix = "data/single/";
    ASSERT_EQ(Build(pathPrefix + path), std::nullopt);
    auto gold = std::string {};
    ASSERT_TRUE(ReadFile<false>(pathPrefix + path + ".gold", gold));

    NormalizeGold(gold);

    const auto out = pathPrefix + path + ".linked.abc";
    auto linkRes = Link(conf, out, {pathPrefix + path + ".abc"});

    ASSERT_EQ(linkRes.errors.empty(), isGood);

    if (isGood) {
        auto res = ExecPanda(out, verificationMode);
        ASSERT_EQ(res.first, 0);
        ASSERT_EQ(res.second, gold);
    }

    if (succeded != nullptr) {
        *succeded = true;
    }

    if (saveResult != nullptr) {
        *saveResult = std::move(linkRes);
    }
}

struct TestData {
    std::string pathPrefix;
    bool isGood = false;
    bool executable = true;
    Result *expected = nullptr;
    std::string gold;
};

void PerformTest(TestData *data, const std::vector<std::string> &perms, const Config &conf,
                 std::optional<std::vector<char>> expectedFile, size_t iteration)
{
    auto out = data->pathPrefix + "linked.";
    auto files = std::vector<std::string> {};

    for (const auto &f : perms) {
        out += f;
        out += ".";
        files.emplace_back(data->pathPrefix + f + ".abc");
    }
    out += "it";
    out += std::to_string(iteration);
    out += ".abc";

    SCOPED_TRACE(out);

    auto linkRes = Link(conf, out, files);
    if (linkRes.errors.empty() != data->isGood) {
        auto errs = std::string();
        for (auto &err : linkRes.errors) {
            errs += err;
            errs += "\n";
        }
        ASSERT_EQ(linkRes.errors.empty(), data->isGood) << errs;
    }

    if (data->expected != nullptr) {
        ASSERT_EQ(linkRes.stats.deduplicatedForeigners, data->expected->stats.deduplicatedForeigners);
    }

    if (data->isGood) {
        std::vector<char> gotFile;
        ASSERT_TRUE(ReadFile<true>(out, gotFile));
        if (!expectedFile.has_value()) {
            expectedFile = std::move(gotFile);
        } else {
            (void)iteration;
            ASSERT_EQ(expectedFile.value(), gotFile) << "on iteration: " << iteration;
        }
        if (data->executable) {
            auto res = ExecPanda(out);
            ASSERT_EQ(res.second, data->gold);
            ASSERT_EQ(res.first, 0);
        }
    }
}

// CC-OFFNXT(G.FUN.01-CPP, readability-function-size_parameters) function for testing
void TestMultiple(const std::string &path, std::vector<std::string> perms, bool isGood = true,
                  const Config &conf = ark::static_linker::DefaultConfig(), Result *expected = nullptr,
                  bool executable = true)
{
    std::sort(perms.begin(), perms.end());

    const auto pathPrefix = "data/multi/" + path + "/";

    for (const auto &p : perms) {
        ASSERT_EQ(Build(pathPrefix + p), std::nullopt);
    }

    auto gold = std::string {};

    if (isGood) {
        ASSERT_TRUE(ReadFile<false>(pathPrefix + "out.gold", gold));
        NormalizeGold(gold);
    }

    std::optional<std::vector<char>> expectedFile;

    do {
        expectedFile = std::nullopt;
        TestData data;
        data.pathPrefix = pathPrefix;
        data.isGood = isGood;
        data.executable = executable;
        data.expected = expected;
        data.gold = gold;
        for (size_t iteration = 0; iteration < TEST_REPEAT_COUNT; iteration++) {
            PerformTest(&data, perms, conf, expectedFile, iteration);
        }
    } while (std::next_permutation(perms.begin(), perms.end()));
}

TEST(linkertests, HelloWorld)
{
    TestSingle("hello_world");
}

TEST(linkertests, LitArray)
{
    TestSingle("lit_array");
}

TEST(linkertests, Exceptions)
{
    // NOTE:(mgroshev): #29225 expected rt error not a verifier's
    TestSingle("exceptions", true, {}, nullptr, nullptr, "disabled");
}

TEST(linkertests, ForeignMethod)
{
    TestMultiple("fmethod", {"1", "2"});
}

TEST(linkertests, ForeignField)
{
    TestMultiple("ffield", {"1", "2"});
}

TEST(linkertests, BadForeignField)
{
    TestMultiple("bad_ffield", {"1", "2"}, false);
}

TEST(linkertests, BadClassRedefinition)
{
    TestMultiple("bad_class_redefinition", {"1", "2"}, false);
}

TEST(linkertests, BadFFieldType)
{
    TestMultiple("bad_ffield_type", {"1", "2"}, false);
}

TEST(linkertests, FMethodOverloaded)
{
    TestMultiple("fmethod_overloaded", {"1", "2"});
}

TEST(linkertests, FMethodOverloaded2)
{
    TestMultiple("fmethod_overloaded_2", {"1", "2", "3", "4"});
}

TEST(linkertests, BadFMethodOverloaded)
{
    TestMultiple("bad_fmethod_overloaded", {"1", "2"}, false);
}

TEST(linkertests, DeduplicatedField)
{
    auto res = Result {};
    res.stats.deduplicatedForeigners = 1;
    TestMultiple("dedup_field", {"1", "2"}, true, DefaultConfig(), &res, false);
}

TEST(linkertests, DeduplicatedMethod)
{
    auto res = Result {};
    res.stats.deduplicatedForeigners = 1;
    TestMultiple("dedup_method", {"1", "2"}, true, DefaultConfig(), &res, false);
}

TEST(linkertests, UnresolvedInGlobal)
{
    TestSingle("unresolved_global", false);
    auto conf = DefaultConfig();
    conf.remainsPartial = {std::string(ark::panda_file::ItemContainer::GetGlobalClassName())};
    // NOTE:(mgroshev): #29225 verification hare is disabled because we are checking behavior< when method is unresolved
    TestSingle("unresolved_global", true, conf, nullptr, nullptr, "disabled");
}

TEST(linkertests, DeduplicateLineNumberNrogram)
{
    auto succ = false;
    auto res = Result {};
    TestSingle("lnp_dedup", true, DefaultConfig(), &succ, &res);
    ASSERT_TRUE(succ);
    ASSERT_EQ(res.stats.debugCount, 1);
}

TEST(linkertests, StripDebugInfo)
{
    auto succ = false;
    auto res = Result {};
    auto conf = DefaultConfig();
    conf.stripDebugInfo = true;
    TestSingle("hello_world", true, conf, &succ, &res);
    ASSERT_TRUE(succ);
    ASSERT_EQ(res.stats.debugCount, 0);
}

TEST(linkertests, FieldOverload)
{
    auto conf = DefaultConfig();
    conf.partial.emplace("LFor;");
    TestMultiple("ffield_overloaded", {"1", "2"}, true, conf);
}

constexpr uint32_t I = 32;
constexpr uint64_t L = 64;
constexpr float F = 11.1;
constexpr double D = 22.2;

TEST(linkertests, TestForReprValueItem)
{
    std::string sv1Expect = "32 as int";
    std::stringstream ss1;
    auto *sv1 = new ark::panda_file::ScalarValueItem(I);
    ark::static_linker::ReprValueItem(ss1, sv1);
    std::string sv1Res = ss1.str();
    delete sv1;
    ASSERT_TRUE(sv1Res == sv1Expect);

    std::string sv2Expect = "64 as long";
    std::stringstream ss2;
    auto *sv2 = new ark::panda_file::ScalarValueItem(L);
    ark::static_linker::ReprValueItem(ss2, sv2);
    std::string sv2Res = ss2.str();
    delete sv2;
    ASSERT_TRUE(sv2Res == sv2Expect);

    std::string sv3Expect = "11.1 as float";
    std::stringstream ss3;
    auto *sv3 = new ark::panda_file::ScalarValueItem(F);
    ark::static_linker::ReprValueItem(ss3, sv3);
    std::string sv3Res = ss3.str();
    delete sv3;
    ASSERT_TRUE(sv3Res == sv3Expect);

    std::string sv4Expect = "22.2 as double";
    std::stringstream ss4;
    auto *sv4 = new ark::panda_file::ScalarValueItem(D);
    ark::static_linker::ReprValueItem(ss4, sv4);
    std::string sv4Res = ss4.str();
    delete sv4;
    ASSERT_TRUE(sv4Res == sv4Expect);

    auto *sv5p = new ark::panda_file::ScalarValueItem(D);
    auto *sv5 = new ark::panda_file::ScalarValueItem(sv5p);
    std::stringstream ss5;
    ark::static_linker::ReprValueItem(ss5, sv5);
    std::string sv5Res = ss5.str();
    delete sv5p;
    delete sv5;
    ASSERT_EQ(sv5Res, sv4Expect);
}

TEST(linkertests, TestForReprArrayValueItem)
{
    std::string av1Expect = "[1 as int, 2 as int, 3 as int]";
    std::stringstream ss6;
    std::vector<ark::panda_file::ScalarValueItem> items;
    items.emplace_back(static_cast<uint32_t>(1));
    items.emplace_back(static_cast<uint32_t>(2));
    items.emplace_back(static_cast<uint32_t>(3));
    auto *av1 = new ark::panda_file::ArrayValueItem(ark::panda_file::Type(ark::panda_file::Type::TypeId::I32), items);
    ark::static_linker::ReprValueItem(ss6, av1);
    std::string av1Res = ss6.str();
    delete av1;
    ASSERT_TRUE(av1Res == av1Expect);
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(linkertests, TestLoopSuperClass)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    // Write panda file to memory
    ark::panda_file::ItemContainer container;

    // set current class's superclass to self
    ark::panda_file::ClassItem *classItem = container.GetOrCreateClassItem("Bar");
    classItem->SetAccessFlags(ark::ACC_PUBLIC);
    classItem->SetSuperClass(classItem);

    // Add interface
    ark::panda_file::ClassItem *ifaceItem = container.GetOrCreateClassItem("Iface");
    ifaceItem->SetAccessFlags(ark::ACC_PUBLIC);
    classItem->AddInterface(ifaceItem);

    // Add method
    ark::panda_file::StringItem *methodName = container.GetOrCreateStringItem("foo");
    ark::panda_file::PrimitiveTypeItem *retType =
        container.GetOrCreatePrimitiveTypeItem(ark::panda_file::Type::TypeId::VOID);
    std::vector<ark::panda_file::MethodParamItem> params;
    ark::panda_file::ProtoItem *protoItem = container.GetOrCreateProtoItem(retType, params);
    classItem->AddMethod(methodName, protoItem, ark::ACC_PUBLIC | ark::ACC_STATIC, params);

    // Add field
    ark::panda_file::StringItem *fieldName = container.GetOrCreateStringItem("field");
    ark::panda_file::PrimitiveTypeItem *fieldType =
        container.GetOrCreatePrimitiveTypeItem(ark::panda_file::Type::TypeId::I32);
    classItem->AddField(fieldName, fieldType, ark::ACC_PUBLIC);

    // Add source file
    ark::panda_file::StringItem *sourceFile = container.GetOrCreateStringItem("source_file");
    classItem->SetSourceFile(sourceFile);
    auto writer = ark::panda_file::FileWriter("loop_super_class.abc");
    ASSERT_TRUE(container.Write(&writer));

    std::string cmd = GenLinkCmd(" -- loop_super_class.abc");
    // NOLINTNEXTLINE(cert-env33-c)
    auto linkRes = std::system(cmd.c_str());
    ASSERT_NE(linkRes, 0);
#endif
}

}  // namespace
