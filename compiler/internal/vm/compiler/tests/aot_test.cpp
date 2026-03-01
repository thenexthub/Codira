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

#include <elf.h>
#include "unit_test.h"
#include "aot/aot_manager.h"
#include "aot/aot_builder/aot_builder.h"
#include "aot/compiled_method.h"
#include "compiler/code_info/code_info_builder.h"
#include "os/exec.h"
#include "assembly-parser.h"
#include "utils/string_helpers.h"
#include "events/events.h"
#include "mem/gc/gc_types.h"
#include "runtime/include/file_manager.h"

#include <regex>

using panda::panda_file::File;

namespace panda::compiler {
class AotTest : public AsmTest {
public:
    AotTest()
    {
        std::string exe_path = GetExecPath();
        auto pos = exe_path.rfind('/');
        paoc_path_ = exe_path.substr(0, pos) + "/../bin/ark_aot";
        aotdump_path_ = exe_path.substr(0, pos) + "/../bin/ark_aotdump";
    }

    std::string GetPaocDirectory() const
    {
        auto pos = paoc_path_.rfind('/');
        return paoc_path_.substr(0, pos);
    }

    const char *GetArchAsArgString() const
    {
        switch (target_arch) {
            case Arch::AARCH32:
                return "arm";
            case Arch::AARCH64:
                return "arm64";
            case Arch::X86:
                return "x86";
            case Arch::X86_64:
                return "x86_64";
            default:
                UNREACHABLE();
        }
    }

    void RunAotdump(const std::string &aot_filename)
    {
        TmpFile tmpfile("aotdump.tmp");

        auto res = os::exec::Exec(aotdump_path_.c_str(), "--show-code=disasm", "--output-file", tmpfile.GetFileName(),
                                  aot_filename.c_str());
        ASSERT_TRUE(res) << "aotdump failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0) << "aotdump return error code: " << res.Value();
    }

    std::string paoc_path_;
    std::string aotdump_path_;
protected:
    Arch target_arch = Arch::AARCH64;
};

#ifdef PANDA_COMPILER_TARGET_AARCH64
TEST_F(AotTest, PaocBootPandaFiles)
{
    // Test basic functionality only in host mode.
    if (RUNTIME_ARCH != Arch::X86_64) {
        return;
    }
    TmpFile panda_fname("test.pf");
    TmpFile aot_fname("./test.an");
    static const std::string location = "/data/local/tmp";
    static const std::string panda_file_path = location + "/" + panda_fname.GetFileName();

    auto source = R"(
        .function void dummy() {
            return.void
        }
    )";

    {
        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(panda_fname.GetFileName(), res.Value()));
    }

    // Correct path to arkstdlib.abc
    {
        auto pandastdlib_path = GetPaocDirectory() + "/../pandastdlib/arkstdlib.abc";
        auto res = os::exec::Exec(paoc_path_.c_str(), "--paoc-panda-files", panda_fname.GetFileName(), "--paoc-output",
                                  aot_fname.GetFileName(), "--paoc-location", location.c_str(), "--paoc-arch",
                                  GetArchAsArgString(), "--boot-panda-files", pandastdlib_path.c_str());
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0) << "Aot compiler failed with code " << res.Value();
        RunAotdump(aot_fname.GetFileName());
    }
}

TEST_F(AotTest, PaocLocation)
{
    // Test basic functionality only in host mode.
    if (RUNTIME_ARCH != Arch::X86_64) {
        return;
    }
    TmpFile panda_fname("test.pf");
    TmpFile aot_fname("./test.an");
    static const std::string location = "/data/local/tmp";
    static const std::string panda_file_path = location + "/" + panda_fname.GetFileName();

    auto source = R"(
        .function u32 add(u64 a0, u64 a1) {
            add a0, a1
            return
        }
    )";

    {
        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(panda_fname.GetFileName(), res.Value()));
    }

    {
        auto pandastdlib_path = GetPaocDirectory() + "/../pandastdlib/arkstdlib.abc";
        auto res = os::exec::Exec(paoc_path_.c_str(), "--paoc-panda-files", panda_fname.GetFileName(), "--paoc-output",
                                  aot_fname.GetFileName(), "--paoc-location", location.c_str(), "--paoc-arch=x86_64",
                                  "--gc-type=epsilon", "--paoc-use-cha=false");
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0) << "Aot compiler failed with code " << res.Value();
    }

    AotManager aot_manager;
    {
        auto res =
            aot_manager.AddFile(aot_fname.GetFileName(), nullptr, static_cast<uint32_t>(mem::GCType::EPSILON_GC));
        ASSERT_TRUE(res) << res.Error();
    }

    auto aot_file = aot_manager.GetFile(aot_fname.GetFileName());
    ASSERT_TRUE(aot_file);
    ASSERT_EQ(aot_file->GetFilesCount(), 1);
    ASSERT_TRUE(aot_file->FindPandaFile(panda_file_path));
}
#endif  // PANDA_COMPILER_TARGET_AARCH64

TEST_F(AotTest, BuildAndLoad)
{
    if (RUNTIME_ARCH == Arch::AARCH32) {
        // TODO(msherstennikov): for some reason dlopen cannot open aot file in qemu-arm
        return;
    }
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string tmpfile = helpers::string::Format("/tmp/tmpfile_%04x.pn", tid);
    static constexpr const char *tmpfile_pf = "test.pf";
    static constexpr const char *cmdline = "cmdline";
    static constexpr uint32_t method1_id = 42;
    static constexpr uint32_t method2_id = 43;
    const std::string class_name("Foo");
    std::string method_name(class_name + "::method");
    std::array<uint8_t, 4> x86_add = {
        0x8d, 0x04, 0x37,  // lea    eax,[rdi+rdi*1]
        0xc3               // ret
    };

    AotBuilder aot_builder;
    aot_builder.SetArch(RUNTIME_ARCH);
    aot_builder.SetGcType(2);
    RuntimeInterfaceMock iruntime;
    aot_builder.SetRuntime(&iruntime);

    aot_builder.StartFile(tmpfile_pf, 0x12345678);

    auto thread = MTManagedThread::GetCurrent();
    if (thread != nullptr) {
        thread->ManagedCodeBegin();
    }
    auto runtime = Runtime::GetCurrent();
    auto etx = runtime->GetClassLinker()->GetExtension(runtime->GetLanguageContext(runtime->GetRuntimeType()));
    auto klass = etx->CreateClass(reinterpret_cast<const uint8_t *>(class_name.data()), 0, 0,
                                  AlignUp(sizeof(Class), OBJECT_POINTER_SIZE));
    if (thread != nullptr) {
        thread->ManagedCodeEnd();
    }

    klass->SetFileId(panda_file::File::EntityId(13));
    aot_builder.StartClass(*klass);

    Method method1(klass, nullptr, File::EntityId(method1_id), File::EntityId(), 0, 1, nullptr);
    {
        CodeInfoBuilder code_builder(RUNTIME_ARCH, GetAllocator());
        ArenaVector<uint8_t> data(GetAllocator()->Adapter());
        code_builder.Encode(&data);
        CompiledMethod compiled_method1(RUNTIME_ARCH, &method1);
        compiled_method1.SetCode(Span(reinterpret_cast<const uint8_t *>(method_name.data()), method_name.size() + 1));
        compiled_method1.SetCodeInfo(Span(data).ToConst());
        aot_builder.AddMethod(compiled_method1, 0);
    }

    Method method2(klass, nullptr, File::EntityId(method2_id), File::EntityId(), 0, 1, nullptr);
    {
        CodeInfoBuilder code_builder(RUNTIME_ARCH, GetAllocator());
        ArenaVector<uint8_t> data(GetAllocator()->Adapter());
        code_builder.Encode(&data);
        CompiledMethod compiled_method2(RUNTIME_ARCH, &method2);
        compiled_method2.SetCode(Span(reinterpret_cast<const uint8_t *>(x86_add.data()), x86_add.size()));
        compiled_method2.SetCodeInfo(Span(data).ToConst());
        aot_builder.AddMethod(compiled_method2, 1);
    }

    aot_builder.EndClass();
    uint32_t hash = GetHash32String(reinterpret_cast<const uint8_t *>(class_name.data()));
    aot_builder.InsertEntityPairHeader(hash, 13);
    aot_builder.InsertClassHashTableSize(1);
    aot_builder.EndFile();

    aot_builder.Write(cmdline, tmpfile.c_str());

    AotManager aot_manager;
    auto res = aot_manager.AddFile(tmpfile.c_str(), nullptr, static_cast<uint32_t>(mem::GCType::STW_GC));
    ASSERT_TRUE(res) << res.Error();

    auto aot_file = aot_manager.GetFile(tmpfile.c_str());
    ASSERT_TRUE(aot_file);
    ASSERT_TRUE(strcmp(cmdline, aot_file->GetCommandLine()) == 0U);
    ASSERT_TRUE(strcmp(tmpfile.c_str(), aot_file->GetFileName()) == 0U);
    ASSERT_EQ(aot_file->GetFilesCount(), 1U);

    auto pfile = aot_manager.FindPandaFile(tmpfile_pf);
    ASSERT_NE(pfile, nullptr);
    auto cls = pfile->GetClass(13);
    ASSERT_TRUE(cls.IsValid());

    {
        auto code = cls.FindMethodCodeEntry(0);
        ASSERT_FALSE(code == nullptr);
        ASSERT_EQ(method_name, reinterpret_cast<const char *>(code));
    }

    {
        auto code = cls.FindMethodCodeEntry(1);
        ASSERT_FALSE(code == nullptr);
        ASSERT_EQ(std::memcmp(x86_add.data(), code, x86_add.size()), 0);
#ifdef PANDA_TARGET_AMD64
        auto func_add = (int (*)(int, int))code;
        ASSERT_EQ(func_add(2, 3), 5);
#endif
    }
}

TEST_F(AotTest, PaocSpecifyMethods)
{
#ifndef PANDA_EVENTS_ENABLED
    GTEST_SKIP();
#endif

    // Test basic functionality only in host mode.
    if (RUNTIME_ARCH != Arch::X86_64) {
        return;
    }
    TmpFile panda_fname("test.pf");
    TmpFile paoc_output_name("events-out.csv");

    static const std::string location = "/data/local/tmp";
    static const std::string panda_file_path = location + "/" + panda_fname.GetFileName();

    auto source = R"(
        .record A {}
        .record B {}

        .function i32 A.f1() {
            ldai 10
            return
        }

        .function i32 B.f1() {
            ldai 20
            return
        }

        .function i32 A.f2() {
            ldai 10
            return
        }

        .function i32 B.f2() {
            ldai 20
            return
        }

        .function i32 main() {
            ldai 0
            return
        }
    )";

    {
        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(panda_fname.GetFileName(), res.Value()));
    }

    {
        // paoc will try compiling all the methods from the panda-file that matches `--compiler-regex`
        auto res =
            os::exec::Exec(paoc_path_.c_str(), "--paoc-panda-files", panda_fname.GetFileName(),
                           "--compiler-regex", "B::f1",
                           "--paoc-mode=jit", "--events-output=csv",
                           "--events-file", paoc_output_name.GetFileName());
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0);

        std::ifstream infile(paoc_output_name.GetFileName());
        std::regex rgx("Compilation,B::f1.*,COMPILED");
        for (std::string line; std::getline(infile, line);) {
            if (line.rfind("Compilation", 0) == 0) {
                ASSERT_TRUE(std::regex_match(line, rgx));
            }
        }
    }
}

TEST_F(AotTest, PaocMultipleFiles)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile aot_fname("./test.an");
    TmpFile panda_fname1("test1.pf");
    TmpFile panda_fname2("test2.pf");

    {
        auto source = R"(
            .function f64 main() {
                fldai.64 3.1415926
                return.64
            }
        )";

        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(panda_fname1.GetFileName(), res.Value()));
    }

    {
        auto source = R"(
            .record MyMath {
            }

            .function f64 MyMath.getPi() <static> {
                fldai.64 3.1415926
                return.64
            }
        )";

        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(panda_fname2.GetFileName(), res.Value()));
    }

    {
        std::stringstream panda_files;
        panda_files << panda_fname1.GetFileName() << ',' << panda_fname2.GetFileName();
        auto res = os::exec::Exec(paoc_path_.c_str(), "--paoc-panda-files", panda_files.str().c_str(), "--paoc-output",
                                  aot_fname.GetFileName(), "--gc-type=epsilon", "--paoc-use-cha=false");
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0);
    }

    {
        AotManager aot_manager;
        auto res =
            aot_manager.AddFile(aot_fname.GetFileName(), nullptr, static_cast<uint32_t>(mem::GCType::EPSILON_GC));
        ASSERT_TRUE(res) << res.Error();

        auto aot_file = aot_manager.GetFile(aot_fname.GetFileName());
        ASSERT_TRUE(aot_file);
        ASSERT_EQ(aot_file->GetFilesCount(), 2U);
    }
    RunAotdump(aot_fname.GetFileName());
}

TEST_F(AotTest, PaocGcType)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile aot_fname("./test.pn");
    TmpFile panda_fname("test.pf");

    {
        auto source = R"(
            .function f64 main() {
                fldai.64 3.1415926
                return.64
            }
        )";

        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(panda_fname.GetFileName(), res.Value()));
    }

    {
        auto res = os::exec::Exec(paoc_path_.c_str(), "--paoc-panda-files", panda_fname.GetFileName(), "--paoc-output",
                                  aot_fname.GetFileName(), "--gc-type=epsilon", "--paoc-use-cha=false");
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0);
    }

    {
        // Wrong gc-type
        AotManager aot_manager;
        auto res = aot_manager.AddFile(aot_fname.GetFileName(), nullptr, static_cast<uint32_t>(mem::GCType::STW_GC));
        ASSERT_FALSE(res) << res.Error();
        std::string expected_string = "Wrong AotHeader gc-type: epsilon vs stw";
        ASSERT_NE(res.Error().find(expected_string), std::string::npos);
    }

    {
        AotManager aot_manager;
        auto res =
            aot_manager.AddFile(aot_fname.GetFileName(), nullptr, static_cast<uint32_t>(mem::GCType::EPSILON_GC));
        ASSERT_TRUE(res) << res.Error();

        auto aot_file = aot_manager.GetFile(aot_fname.GetFileName());
        ASSERT_TRUE(aot_file);
        ASSERT_EQ(aot_file->GetFilesCount(), 1U);
    }
    RunAotdump(aot_fname.GetFileName());
}

TEST_F(AotTest, FileManagerLoadAbc)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile aot_fname("./test.an");
    TmpFile panda_fname("./test.pf");

    {
        auto source = R"(
            .function f64 main() {
                fldai.64 3.1415926
                return.64
            }
        )";

        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(panda_fname.GetFileName(), res.Value()));
    }

    {
        auto runtime = Runtime::GetCurrent();
        auto gc_type_id = static_cast<uint32_t>(
            Runtime::GetGCType(runtime->GetOptions(), plugins::RuntimeTypeToLang(runtime->GetRuntimeType())));
        auto gc_type_name = "--gc-type=epsilon";
        if (gc_type_id == 2) {
            gc_type_name = "--gc-type=stw";
        } else if (gc_type_id == 4) {
            gc_type_name = "--gc-type=gen-gc";
        } else {
            ASSERT_EQ(gc_type_id, 1) << "Invalid GC type\n";
        }
        auto res = os::exec::Exec(paoc_path_.c_str(), "--paoc-panda-files", panda_fname.GetFileName(), "--paoc-output",
                                  aot_fname.GetFileName(), gc_type_name, "--paoc-use-cha=false");
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0);
    }

    {
        auto res = FileManager::LoadAbcFile(panda_fname.GetFileName(), panda_file::File::READ_ONLY);
        ASSERT_TRUE(res);
        auto aot_manager = Runtime::GetCurrent()->GetClassLinker()->GetAotManager();
        auto aot_file = aot_manager->GetFile(aot_fname.GetFileName());
        ASSERT_TRUE(aot_file);
        ASSERT_EQ(aot_file->GetFilesCount(), 1);
    }
    RunAotdump(aot_fname.GetFileName());
}

TEST_F(AotTest, FileManagerLoadAn)
{
    if (RUNTIME_ARCH == Arch::AARCH32) {
        // TODO(msherstennikov): for some reason dlopen cannot open aot file in qemu-arm
        return;
    }
    uint32_t tid = os::thread::GetCurrentThreadId();
    std::string tmpfile = helpers::string::Format("test.an", tid);
    static constexpr const char *tmpfile_pf = "test.pf";
    static constexpr const char *cmdline = "cmdline";
    static constexpr uint32_t method1_id = 42;
    static constexpr uint32_t method2_id = 43;
    const std::string class_name("Foo");
    std::string method_name(class_name + "::method");
    std::array<uint8_t, 4> x86_add = {
        0x8d, 0x04, 0x37,  // lea    eax,[rdi+rdi*1]
        0xc3               // ret
    };

    AotBuilder aot_builder;
    aot_builder.SetArch(RUNTIME_ARCH);
    RuntimeInterfaceMock iruntime;
    aot_builder.SetRuntime(&iruntime);
    auto runtime = Runtime::GetCurrent();
    auto gc_type = Runtime::GetGCType(runtime->GetOptions(), plugins::RuntimeTypeToLang(runtime->GetRuntimeType()));
    aot_builder.SetGcType(static_cast<uint32_t>(gc_type));

    aot_builder.StartFile(tmpfile_pf, 0x12345678);

    auto thread = MTManagedThread::GetCurrent();
    if (thread != nullptr) {
        thread->ManagedCodeBegin();
    }
    auto etx = runtime->GetClassLinker()->GetExtension(runtime->GetLanguageContext(runtime->GetRuntimeType()));
    auto klass = etx->CreateClass(reinterpret_cast<const uint8_t *>(class_name.data()), 0, 0,
                                  AlignUp(sizeof(Class), OBJECT_POINTER_SIZE));
    if (thread != nullptr) {
        thread->ManagedCodeEnd();
    }

    klass->SetFileId(panda_file::File::EntityId(13));
    aot_builder.StartClass(*klass);

    Method method1(klass, nullptr, File::EntityId(method1_id), File::EntityId(), 0, 1, nullptr);
    {
        CodeInfoBuilder code_builder(RUNTIME_ARCH, GetAllocator());
        ArenaVector<uint8_t> data(GetAllocator()->Adapter());
        code_builder.Encode(&data);
        CompiledMethod compiled_method1(RUNTIME_ARCH, &method1);
        compiled_method1.SetCode(Span(reinterpret_cast<const uint8_t *>(method_name.data()), method_name.size() + 1));
        compiled_method1.SetCodeInfo(Span(data).ToConst());
        aot_builder.AddMethod(compiled_method1, 0);
    }

    Method method2(klass, nullptr, File::EntityId(method2_id), File::EntityId(), 0, 1, nullptr);
    {
        CodeInfoBuilder code_builder(RUNTIME_ARCH, GetAllocator());
        ArenaVector<uint8_t> data(GetAllocator()->Adapter());
        code_builder.Encode(&data);
        CompiledMethod compiled_method2(RUNTIME_ARCH, &method2);
        compiled_method2.SetCode(Span(reinterpret_cast<const uint8_t *>(x86_add.data()), x86_add.size()));
        compiled_method2.SetCodeInfo(Span(data).ToConst());
        aot_builder.AddMethod(compiled_method2, 1);
    }

    aot_builder.EndClass();
    uint32_t hash = GetHash32String(reinterpret_cast<const uint8_t *>(class_name.data()));
    aot_builder.InsertEntityPairHeader(hash, 13);
    aot_builder.InsertClassHashTableSize(1);
    aot_builder.EndFile();

    aot_builder.Write(cmdline, tmpfile.c_str());
    {
        auto res = FileManager::LoadAnFile(tmpfile.c_str());
        ASSERT_TRUE(res) << "Fail to load an file";
    }

    auto aot_manager = Runtime::GetCurrent()->GetClassLinker()->GetAotManager();
    auto aot_file = aot_manager->GetFile(tmpfile.c_str());
    ASSERT_TRUE(aot_file);
    ASSERT_TRUE(strcmp(cmdline, aot_file->GetCommandLine()) == 0U);
    ASSERT_TRUE(strcmp(tmpfile.c_str(), aot_file->GetFileName()) == 0U);
    ASSERT_EQ(aot_file->GetFilesCount(), 1U);

    auto pfile = aot_manager->FindPandaFile(tmpfile_pf);
    ASSERT_NE(pfile, nullptr);
    auto cls = pfile->GetClass(13);
    ASSERT_TRUE(cls.IsValid());

    {
        auto code = cls.FindMethodCodeEntry(0);
        ASSERT_FALSE(code == nullptr);
        ASSERT_EQ(method_name, reinterpret_cast<const char *>(code));
    }

    {
        auto code = cls.FindMethodCodeEntry(1);
        ASSERT_FALSE(code == nullptr);
        ASSERT_EQ(std::memcmp(x86_add.data(), code, x86_add.size()), 0);
#ifdef PANDA_TARGET_AMD64
        auto func_add = (int (*)(int, int))code;
        ASSERT_EQ(func_add(2, 3), 5);
#endif
    }
}

TEST_F(AotTest, PaocClusters)
{
    // Test basic functionality only in host mode.
    if (RUNTIME_ARCH != Arch::X86_64) {
        return;
    }

    TmpFile paoc_clusters("clusters.json");
    std::ofstream(paoc_clusters.GetFileName()) <<
        R"(
    {
        "clusters_map" :
        {
            "A::count" : ["unroll_enable"],
            "B::count2" : ["unroll_disable"],
            "_GLOBAL::main" : ["inline_disable", 1]
        },

        "compiler_options" :
        {
            "unroll_disable" :
            {
                "compiler-loop-unroll" : "false"
            },

            "unroll_enable" :
            {
                "compiler-loop-unroll" : "true",
                "compiler-loop-unroll-factor" : 42,
                "compiler-loop-unroll-inst-limit" : 850
            },

            "inline_disable" :
            {
                "compiler-inlining" : "false"
            }
        }
    }
    )";

    TmpFile panda_fname("test.pf");
    auto source = R"(
        .record A {}
        .record B {}

        .function i32 A.count() <static> {
            movi v1, 5
            ldai 0
        main_loop:
            jeq v1, main_ret
            addi 1
            jmp main_loop
        main_ret:
            return
        }

        .function i32 B.count() <static> {
            movi v1, 5
            ldai 0
        main_loop:
            jeq v1, main_ret
            addi 1
            jmp main_loop
        main_ret:
            return
        }

        .function i32 B.count2() <static> {
            movi v1, 5
            ldai 0
        main_loop:
            jeq v1, main_ret
            addi 1
            jmp main_loop
        main_ret:
            return
        }

        .function i32 main() {
            call.short A.count
            sta v0
            call.short B.count
            add2 v0
            call.short B.count2
            add2 v0
            return
        }
    )";

    {
        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(panda_fname.GetFileName(), res.Value()));
    }

    {
        TmpFile compiler_events("events.csv");
        auto res =
            os::exec::Exec(paoc_path_.c_str(), "--paoc-panda-files", panda_fname.GetFileName(), "--paoc-clusters",
                           paoc_clusters.GetFileName(), "--compiler-loop-unroll-factor=7",
                           "--compiler-enable-events=true", "--compiler-events-path", compiler_events.GetFileName());
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0);

        bool first_found = false;
        bool second_found = false;
        std::ifstream events_file(compiler_events.GetFileName());

        std::regex rgx_unroll_applied_cluster("A::count,loop-unroll,.*,unroll_factor:42,.*");
        std::regex rgx_unroll_restored_default("B::count,loop-unroll,.*,unroll_factor:7,.*");

        for (std::string line; std::getline(events_file, line);) {
            if (line.rfind("loop-unroll") != std::string::npos) {
                if (!first_found) {
                    // Check that the cluster is applied:
                    ASSERT_TRUE(std::regex_match(line, rgx_unroll_applied_cluster));
                    first_found = true;
                    continue;
                }
                ASSERT_FALSE(second_found);
                // Check that the option is restored:
                ASSERT_TRUE(std::regex_match(line, rgx_unroll_restored_default));
                second_found = true;
            }
        }
        ASSERT_TRUE(first_found && second_found);
    }
}

TEST_F(AotTest, PandaFiles)
{
#ifndef PANDA_EVENTS_ENABLED
    GTEST_SKIP();
#endif

    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile aot_fname("./test.an");
    TmpFile panda_fname1("test1.pf");
    TmpFile panda_fname2("test2.pf");
    TmpFile paoc_output_name("events-out.csv");

    {
        auto source = R"(
            .record Z {}
            .function i32 Z.zoo() <static> {
                ldai 45
                return
            }
        )";

        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(panda_fname1.GetFileName(), res.Value()));
    }

    {
        auto source = R"(
            .record Z <external>
            .function i32 Z.zoo() <external, static>
            .record X {}
            .function i32 X.main() {
                call.short Z.zoo
                return
            }
        )";

        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(panda_fname2.GetFileName(), res.Value()));
    }

    {
        std::stringstream panda_files;
        panda_files << panda_fname1.GetFileName() << ',' << panda_fname2.GetFileName();
        auto res = os::exec::Exec(paoc_path_.c_str(), "--paoc-panda-files", panda_fname2.GetFileName(), "--panda-files",
                                  panda_fname1.GetFileName(), "--events-output=csv", "--events-file",
                                  paoc_output_name.GetFileName());
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0);

        std::ifstream infile(paoc_output_name.GetFileName());
        // Inlining attempt proofs that Z::zoo was available to inline
        std::regex rgx("Inline,.*Z::zoo.*");
        bool inline_attempt = false;
        for (std::string line; std::getline(infile, line);) {
            inline_attempt |= std::regex_match(line, rgx);
        }
        ASSERT_TRUE(inline_attempt);
    }
}

}  // namespace panda::compiler
