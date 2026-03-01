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

#include <filesystem>
#include <iostream>

namespace {
using Config = ark::static_linker::Config;
using Result = ark::static_linker::Result;
using ark::static_linker::DefaultConfig;
using ark::static_linker::Link;

constexpr std::string_view ABC_FILE_EXTENSION = ".abc";
constexpr size_t ABC_FILE_EXTENSION_LENGTH = 4;

constexpr auto ARK_LINK_PATH = "../../../../bin/ark_link";
constexpr auto ETSSTDLIB_PATH = "../../../../plugins/ets/etsstdlib.abc";

std::pair<int, std::string> ExecPanda(const std::string &file, const std::string &verificationMode = "ahead-of-time",
                                      const std::string &loadRuntimes = "core",
                                      const std::string &entryPoint = "_GLOBAL::main")
{
    auto opts = ark::RuntimeOptions {};
    if (loadRuntimes == "ets") {
        opts.SetBootPandaFiles({ETSSTDLIB_PATH});
    } else if (loadRuntimes == "core") {
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

struct TestData {
    std::string pathPrefix;
    bool isGood = false;
    bool executable = true;
    Result *expected = nullptr;
    std::string gold;
};

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct StripOptions {
    bool stripUnused = false;
    std::optional<std::string> stripUnusedSkiplist;
    bool IsEmpty() const
    {
        return !stripUnused && !stripUnusedSkiplist.has_value();
    }
    StripOptions() = default;
};

struct TestStsConfig {
    std::string entryPoint = "1/ETSGLOBAL::main";
    std::string outputFileName = "out.gold";
    std::string verificationMode = "ahead-of-time";
    TestStsConfig() = default;
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

std::string BuildLinkerCommand(const std::string &path, const std::vector<std::string> &files,
                               const StripOptions &stripOptions, const std::string &targetPathPrefix)
{
    std::string linkerCommand = ARK_LINK_PATH;

    if (stripOptions.stripUnused) {
        linkerCommand.append(" --strip-unused");
    }

    if (stripOptions.stripUnusedSkiplist.has_value()) {
        linkerCommand.append(" --strip-unused-skiplist=");
        linkerCommand.append(stripOptions.stripUnusedSkiplist.value());
    }

    linkerCommand.append(" --output " + targetPathPrefix + "linked.abc");
    linkerCommand.append(" -- ");
    for (const auto &f : files) {
        if (f.length() < ABC_FILE_EXTENSION_LENGTH ||
            f.substr(f.length() - ABC_FILE_EXTENSION_LENGTH) != ABC_FILE_EXTENSION) {
            linkerCommand.push_back('@');  // test filesinfo
        }
        linkerCommand.append(path + f);
        linkerCommand.push_back(' ');
    }

    return linkerCommand;
}

void VerifyDeletedDependencies(const std::string &abcFilePath, const std::unordered_set<std::string> &allowedClasses,
                               const std::unordered_set<std::string> &notAllowedClasses = {})
{
    auto file = ark::panda_file::File::Open(abcFilePath.c_str(), ark::panda_file::File::READ_ONLY);
    ASSERT_TRUE(file != nullptr);
    const auto &classIdx = file->GetClasses();
    std::unordered_map<std::string, bool> allowedFound;

    for (const auto &cls : allowedClasses) {
        allowedFound[cls] = false;
    }
    for (uint32_t idx : classIdx) {
        auto entityId = ark::panda_file::File::EntityId(idx);
        auto strData = file->GetStringData(entityId);
        if (strData.data == nullptr) {
            continue;
        }
        std::string className(reinterpret_cast<const char *>(strData.data));

        if (!notAllowedClasses.empty()) {
            EXPECT_TRUE(notAllowedClasses.find(className) == notAllowedClasses.end());
        }

        if (allowedClasses.find(className) != allowedClasses.end()) {
            allowedFound[className] = true;
        }
    }
    for (const auto &cls : allowedClasses) {
        EXPECT_TRUE(allowedFound[cls]);
    }
}

void TestSts(const std::string &path, const std::vector<std::string> &files, bool isGood = true,
             const StripOptions &deleteOptions = {}, const TestStsConfig &config = {})
{
    const auto sourcePathPrefix = "data/ets/" + path + "/";
    const auto targetPathPrefix = "data/output/" + path + "/";

    std::string linkerCommand = BuildLinkerCommand(sourcePathPrefix, files, deleteOptions, targetPathPrefix);
    // NOLINTNEXTLINE(cert-env33-c)
    auto linkRes = std::system(linkerCommand.c_str());

    if (isGood) {
        ASSERT_EQ(linkRes, 0);
        auto gold = std::string {};
        ASSERT_TRUE(ReadFile<false>(sourcePathPrefix + config.outputFileName, gold));
        NormalizeGold(gold);
        auto ret = ExecPanda(targetPathPrefix + "linked.abc", config.verificationMode, "ets", config.entryPoint);
        ASSERT_EQ(ret.first, 0);
        ASSERT_EQ(ret.second, gold);
    } else {
        ASSERT_NE(linkRes, 0);
    }
}

TEST(linkertests, ForeignBase)
{
    constexpr auto LANG = ark::panda_file::SourceLang::ETS;
    auto makeRecord = [](ark::pandasm::Program &prog, const std::string &name) {
        return &prog.recordTable.emplace(name, ark::pandasm::Record(name, LANG)).first->second;
    };

    const std::string basePath = "data/multi/ForeignBase.1.abc";
    const std::string dervPath = "data/multi/ForeignBase.2.abc";

    {
        ark::pandasm::Program progBase;
        auto base = makeRecord(progBase, "Base");
        auto fld = ark::pandasm::Field(LANG);
        fld.name = "fld";
        fld.type = ark::pandasm::Type("i32", 0);
        base->fieldList.push_back(std::move(fld));

        ASSERT_TRUE(ark::pandasm::AsmEmitter::Emit(basePath, progBase));
    }

    {
        ark::pandasm::Program progDer;
        auto base = makeRecord(progDer, "Base");
        base->metadata->SetAttribute("external");

        auto derv = makeRecord(progDer, "Derv");
        ASSERT_EQ(derv->metadata->SetAttributeValue("ets.extends", "Base"), std::nullopt);
        std::ignore = derv;
        auto fld = ark::pandasm::Field(LANG);
        fld.name = "fld";
        fld.type = ark::pandasm::Type("i32", 0);
        fld.metadata->SetAttribute("external");
        derv->fieldList.push_back(std::move(fld));

        auto func = ark::pandasm::Function("main", LANG);
        func.regsNum = 1U;
        func.returnType = ark::pandasm::Type("void", 0);
        func.AddInstruction(ark::pandasm::Create_NEWOBJ(0, "Derv"));
        func.AddInstruction(ark::pandasm::Create_LDOBJ(0, "Derv.fld"));
        func.AddInstruction(ark::pandasm::Create_RETURN_VOID());

        progDer.functionInstanceTable.emplace(func.name, std::move(func));

        ASSERT_TRUE(ark::pandasm::AsmEmitter::Emit(dervPath, progDer));
    }

    auto res = Link(DefaultConfig(), "data/multi/ForeignBase.linked.abc", {basePath, dervPath});
    ASSERT_TRUE(res.errors.empty()) << res.errors.front();
}

TEST(linkertests, StsHelloWorld)
{
    TestSts("hello_world", {"1.ets.abc"});
}

TEST(linkertests, StsForeignClass)
{
    TestSts("fclass", {"1.ets.abc", "2.ets.abc"});
}

TEST(linkertests, StsForeignMethod)
{
    TestSts("fmethod", {"1.ets.abc", "2.ets.abc"});
}

TEST(linkertests, StsFMethodOverloaded)
{
    TestSts("fmethod_overloaded", {"1.ets.abc", "2.ets.abc"});
}

TEST(linkertests, StsFMethodOverloaded2)
{
    TestSts("fmethod_overloaded_2", {"1.ets.abc", "2.ets.abc", "3.ets.abc", "4.ets.abc"});
}

TEST(linkertests, FilesInfo)
{
    TestSts("filesinfo", {"filesinfo.txt"});
}

TEST(linkertests, Mix)
{
    TestSts("mix", {"1.ets.abc", "2.ets.abc"});
}

TEST(linkertests, ClassCallNoDeleteDependency)
{
    TestStsConfig config;
    config.entryPoint = "dependency/ETSGLOBAL::main";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, true, {}, config);
}

TEST(linkertests, ClassCallDeleteDependency)
{
    StripOptions opts;
    opts.stripUnused = true;
    TestStsConfig config;
    config.entryPoint = "dependency/ETSGLOBAL::main";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, true, opts, config);
    std::unordered_set<std::string> allowedClasses = {
        "Ldependency/ETSGLOBAL;",        "Lbedependent/ETSGLOBAL;",    "Ldependency/AnotherClass;", "Ldependency/User;",
        "Lbedependent/DependencyClass;", "Lbedependent/UnusedClass1;", "Lbedependent/UnusedClass2;"};
    const std::string abcFilePathPrefix = "data/output/classcall_doublefile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses);
}

TEST(linkertests, ClassCallDeleteDependencyFromEntry)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"dependency/ETSGLOBAL\"";
    TestStsConfig config;
    config.entryPoint = "dependency/ETSGLOBAL::main";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, true, opts, config);
    std::unordered_set<std::string> allowedClasses = {"Ldependency/ETSGLOBAL;", "Ldependency/User;"};
    std::unordered_set<std::string> notAllowedClasses = {"Lbedependent/ETSGLOBAL;", "Lbedependent/UnusedClass1;",
                                                         "Lbedependent/UnusedClass2;"};
    const std::string abcFilePathPrefix = "data/output/classcall_doublefile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses, notAllowedClasses);
}

TEST(linkertests, ClassCallDeleteDependencyFromEntryInputError)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"1/ETSGLOBAL\"";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, false, opts);
}

TEST(linkertests, ClassCallDeleteDependencyFromConfig)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "@data/ets/classcall_doublefile/configfile.txt";
    TestStsConfig config;
    config.entryPoint = "dependency/ETSGLOBAL::main";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, true, opts, config);
    std::unordered_set<std::string> allowedClasses = {"Ldependency/ETSGLOBAL;", "Ldependency/User;"};
    std::unordered_set<std::string> notAllowedClasses = {"Lbedependent/ETSGLOBAL;", "bedependent/UnusedClass1;",
                                                         "Lbedependent/UnusedClass2;"};
    const std::string abcFilePathPrefix = "data/output/classcall_doublefile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses, notAllowedClasses);
}

TEST(linkertests, MethodCallDeleteDependencyFromEntry)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"dependency/ETSGLOBAL/main\"";
    TestStsConfig config;
    config.entryPoint = "dependency/ETSGLOBAL::main";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, true, opts, config);
    std::unordered_set<std::string> allowedClasses = {"Ldependency/ETSGLOBAL;", "Ldependency/User;"};
    std::unordered_set<std::string> notAllowedClasses = {"Lbedependent/ETSGLOBAL;", "Lbedependent/UnusedClass1;",
                                                         "Lbedependent/UnusedClass2;"};
    const std::string abcFilePathPrefix = "data/output/classcall_doublefile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses, notAllowedClasses);
}

TEST(linkertests, MethodCallDeleteDependencyFromEntryInputError)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"1/ETSGLOBAL/main\"";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, false, opts);
}

TEST(linkertests, MethodCallDeleteDependencyFromConfig)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "@data/ets/classcall_doublefile/methodconfig.txt";
    TestStsConfig config;
    config.entryPoint = "dependency/ETSGLOBAL::main";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, true, opts, config);
    std::unordered_set<std::string> allowedClasses = {"Ldependency/ETSGLOBAL;", "Ldependency/User;"};
    std::unordered_set<std::string> notAllowedClasses = {"Lbedependent/ETSGLOBAL;", "Lbedependent/UnusedClass1;",
                                                         "Lbedependent/UnusedClass2;"};
    const std::string abcFilePathPrefix = "data/output/classcall_doublefile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses, notAllowedClasses);
}

TEST(linkertests, CallDeleteDependencyFromConfigFileNotExist)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "@data/ets/classcall_doublefile/methodconfig1.txt";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, false, opts);
}

TEST(linkertests, CallDeleteDependencyFromEntryFile)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"dependency.ets\"";
    TestStsConfig config;
    config.entryPoint = "dependency/ETSGLOBAL::main";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, true, opts, config);
    std::unordered_set<std::string> allowedClasses = {"Ldependency/ETSGLOBAL;", "Ldependency/AnotherClass;",
                                                      "Lbedependent/DependencyClass;", "Ldependency/User;"};
    std::unordered_set<std::string> notAllowedClasses = {"Lbedependent/ETSGLOBAL;", "Lbedependent/UnusedClass1;",
                                                         "Lbedependent/UnusedClass2;"};
    const std::string abcFilePathPrefix = "data/output/classcall_doublefile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses, notAllowedClasses);
}

TEST(linkertests, CallDeleteDependencyFromEntryFileNotExist)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"1.ets\"";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, false, opts);
}

TEST(linkertests, MethodAnnotationDeleteDependency)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"method/ETSGLOBAL/main\"";
    TestStsConfig config;
    config.entryPoint = "method/ETSGLOBAL::main";
    TestSts("annotation", {"method.ets.abc"}, true, opts, config);
    std::unordered_set<std::string> allowedClasses = {"Lmethod/ETSGLOBAL;", "Lmethod/ClassAuthor;"};
    std::unordered_set<std::string> notAllowedClasses = {"Lmethod/C3;", "Lmethod/ClassPreamble;"};
    const std::string abcFilePathPrefix = "data/output/annotation/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses, notAllowedClasses);
}

TEST(linkertests, FieldAnnotationDeleteDependency)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"field/ETSGLOBAL/main\"";
    TestStsConfig config;
    config.entryPoint = "field/ETSGLOBAL::main";
    TestSts("annotation", {"field.ets.abc"}, true, opts, config);
    std::unordered_set<std::string> allowedClasses = {"Lfield/ETSGLOBAL;", "Lfield/PropertyAuthor;"};
    std::unordered_set<std::string> notAllowedClasses = {"Lfield/C3;", "Lfield/ClassPreamble;",
                                                         "Lfield/DeprecatedProperty;"};
    const std::string abcFilePathPrefix = "data/output/annotation/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses, notAllowedClasses);
}

TEST(linkertests, ClassExtensionDeleteDependency)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"classExtension/ETSGLOBAL/main\"";
    TestStsConfig config;
    config.entryPoint = "classExtension/ETSGLOBAL::main";
    config.outputFileName = "classExtension.gold";
    TestSts("singlefile", {"classExtension.ets.abc"}, true, opts, config);
    std::unordered_set<std::string> allowedClasses = {"LclassExtension/ETSGLOBAL;", "LclassExtension/Animal;",
                                                      "LclassExtension/Dog;"};
    const std::string abcFilePathPrefix = "data/output/singlefile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses);
}

TEST(linkertests, InterImplDeleteDependency)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"interImpl/ETSGLOBAL/main\"";
    TestStsConfig config;
    config.entryPoint = "interImpl/ETSGLOBAL::main";
    config.outputFileName = "interImpl.gold";
    TestSts("singlefile", {"interImpl.ets.abc"}, true, opts, config);
    std::unordered_set<std::string> allowedClasses = {"LinterImpl/ETSGLOBAL;", "LinterImpl/Circle;",
                                                      "LinterImpl/Rectangle;", "LinterImpl/Shape;"};
    const std::string abcFilePathPrefix = "data/output/singlefile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses);
}

TEST(linkertests, ObjectLiteralDeleteDependency)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"ObjectLiteral/ETSGLOBAL/main\"";
    TestStsConfig config;
    config.entryPoint = "ObjectLiteral/ETSGLOBAL::main";
    config.outputFileName = "ObjectLiteral.gold";
    TestSts("singlefile", {"ObjectLiteral.ets.abc"}, true, opts, config);
    std::unordered_set<std::string> allowedClasses = {"LObjectLiteral/ETSGLOBAL;", "LObjectLiteral/Person;"};
    const std::string abcFilePathPrefix = "data/output/singlefile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses);
}

TEST(linkertests, TesErrorDeleteDependency)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"teserror/ETSGLOBAL/main\"";
    TestStsConfig config;
    config.entryPoint = "teserror/ETSGLOBAL::main";
    config.outputFileName = "teserror.gold";
    TestSts("singlefile", {"teserror.ets.abc"}, true, opts, config);
    std::unordered_set<std::string> allowedClasses = {"Lteserror/ETSGLOBAL;", "Lteserror/TestA;"};
    const std::string abcFilePathPrefix = "data/output/singlefile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses);
}

TEST(linkertests, DebugInfoClassCallDeleteDependency)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "dependency_debug/ETSGLOBAL/main";
    TestStsConfig config;
    config.entryPoint = "dependency_debug/ETSGLOBAL::main";
    config.outputFileName = "outDebug.gold";
    TestSts("classcall_doublefile", {"dependency_debug.abc"}, true, opts, config);
    std::unordered_set<std::string> allowedClasses = {"Ldependency_debug/ETSGLOBAL;",
                                                      "Ldependency_debug/DependencyClass;"};
    std::unordered_set<std::string> notAllowedClasses = {};
    const std::string abcFilePathPrefix = "data/output/classcall_doublefile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses, notAllowedClasses);
}

TEST(linkertests, MultiClassCallNoDeleteDependency)
{
    TestStsConfig config;
    config.entryPoint = "main_dependencyUser/ETSGLOBAL::main";
    TestSts("classcall_multifile",
            {"main_dependencyUser.ets.abc", "dependency.ets.abc", "user_dependency.ets.abc", "product_user.ets.abc"},
            true, {}, config);
}

TEST(linkertests, MultiClassCallDeleteDependency)
{
    StripOptions opts;
    opts.stripUnused = true;
    TestStsConfig config;
    config.entryPoint = "main_dependencyUser/ETSGLOBAL::main";
    TestSts("classcall_multifile",
            {"main_dependencyUser.ets.abc", "dependency.ets.abc", "user_dependency.ets.abc", "product_user.ets.abc"},
            true, opts, config);
    std::unordered_set<std::string> allowedClasses = {"Lmain_dependencyUser/ETSGLOBAL;",
                                                      "Lmain_dependencyUser/MainApplication;",
                                                      "Ldependency/ETSGLOBAL;",
                                                      "Ldependency/DependencyClass;",
                                                      "Luser_dependency/User;",
                                                      "Ldependency/UnusedDependencyClass;",
                                                      "Luser_dependency/UnusedUserClass;",
                                                      "Lproduct_user/Product;",
                                                      "Lproduct_user/UnusedProductClass;",
                                                      "Lproduct_user/ETSGLOBAL;",
                                                      "Luser_dependency/ETSGLOBAL;"};
    const std::string abcFilePathPrefix = "data/output/classcall_multifile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses);
}
TEST(linkertests, MultiClassCallDeleteDependencyFromEntry)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"main_dependencyUser/ETSGLOBAL\"";
    TestStsConfig config;
    config.entryPoint = "main_dependencyUser/ETSGLOBAL::main";
    TestSts("classcall_multifile",
            {"main_dependencyUser.ets.abc", "dependency.ets.abc", "user_dependency.ets.abc", "product_user.ets.abc"},
            true, opts, config);
    std::unordered_set<std::string> allowedClasses = {"Lmain_dependencyUser/ETSGLOBAL;",
                                                      "Lmain_dependencyUser/MainApplication;",
                                                      "Ldependency/DependencyClass;", "Luser_dependency/User;"};
    std::unordered_set<std::string> notAllowedClasses = {
        "Ldependency/UnusedDependencyClass;", "Luser_dependency/UnusedUserClass;", "Lproduct_user/Product;",
        "Lproduct_user/UnusedProductClass;", "Lproduct_user/ETSGLOBAL;"};
    const std::string abcFilePathPrefix = "data/output/classcall_multifile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses, notAllowedClasses);
}

TEST(linkertests, MultiClassCallDeleteDependencyFromConfig)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "@data/ets/classcall_multifile/configfile.txt";
    TestStsConfig config;
    config.entryPoint = "main_dependencyUser/ETSGLOBAL::main";
    TestSts("classcall_multifile",
            {"main_dependencyUser.ets.abc", "dependency.ets.abc", "user_dependency.ets.abc", "product_user.ets.abc"},
            true, opts, config);
    std::unordered_set<std::string> allowedClasses = {
        "Lmain_dependencyUser/ETSGLOBAL;", "Lmain_dependencyUser/MainApplication;", "Ldependency/DependencyClass;",
        "Luser_dependency/User;", "Lproduct_user/Product;"};
    std::unordered_set<std::string> notAllowedClasses = {"Luser_dependency/UnusedUserClass;",
                                                         "Lproduct_user/ETSGLOBAL;", "Luser_dependency/ETSGLOBAL;"};
    const std::string abcFilePathPrefix = "data/output/classcall_multifile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses, notAllowedClasses);
}

TEST(linkertests, ClassCallDeleteDependencyAll)
{
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "*";
    TestStsConfig config;
    config.entryPoint = "dependency/ETSGLOBAL::main";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, true, opts, config);
    std::unordered_set<std::string> allowedClasses = {
        "Ldependency/ETSGLOBAL;",        "Lbedependent/ETSGLOBAL;",    "Ldependency/AnotherClass;", "Ldependency/User;",
        "Lbedependent/DependencyClass;", "Lbedependent/UnusedClass1;", "Lbedependent/UnusedClass2;"};
    const std::string abcFilePathPrefix = "data/output/classcall_doublefile/";
    VerifyDeletedDependencies(abcFilePathPrefix + "linked.abc", allowedClasses);
}

TEST(linkertests, CallDeleteDependencyNoStripUnusedArg)
{
    StripOptions opts;
    opts.stripUnusedSkiplist = "\"dependency/ETSGLOBAL/main\"";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, false, opts);
}

std::string GenLinkCmd(const std::string &param)
{
    std::string prefix = ARK_LINK_PATH;
    std::string suffix = " > /dev/null 2>&1";
    return prefix + param + suffix;
}

TEST(linkertests, TestForCoverage)
{
    std::string cmd = GenLinkCmd(" --error-option");
    // NOLINTNEXTLINE(cert-env33-c)
    auto linkRes = std::system(cmd.c_str());
    ASSERT_NE(linkRes, 0);

    cmd = GenLinkCmd(" --output data/ets/sys/target.abc");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_NE(linkRes, 0);

    cmd = GenLinkCmd(" -- data/ets/sys/1.ets.abc data/ets/sys/2.ets.abc");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_EQ(linkRes, 0);

    cmd = GenLinkCmd(" -- data/ets/sys/no_exist.abc");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_NE(linkRes, 0);

    cmd = GenLinkCmd(" -- data/ets/sys/1.ets.abc data/ets/sys/2.ets.abc");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_EQ(linkRes, 0);

    cmd = GenLinkCmd(" --output data/ets/sys/target.abc -- data/ets/sys/1.ets.abc data/ets/sys/2.ets.abc");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_EQ(linkRes, 0);

    std::string opt = " --show-stats --log-level info";
    std::string dst = " --output data/ets/sys/target.abc -- data/ets/sys/1.ets.abc data/ets/sys/2.ets.abc";
    cmd = GenLinkCmd(opt + dst);
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_EQ(linkRes, 0);

    cmd = GenLinkCmd(" --output data/ets/sys/target.abc -- @data/ets/sys/no_exist.txt");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_NE(linkRes, 0);

    cmd = GenLinkCmd(" --output data/ets/sys/target.abc -- @data/ets/sys/file_list_exist.txt");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_EQ(linkRes, 0);

    cmd = GenLinkCmd(" --output data/ets/sys/target.abc -- @data/ets/sys/file_list_noexist.txt");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_NE(linkRes, 0);

    cmd = GenLinkCmd(" --output data/ets/sys/target.abc -- @data/ets/sys/file_list_errorformat.txt");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_NE(linkRes, 0);
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(linkertests, TestLoopSuperClass)
{
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
}

}  // namespace
