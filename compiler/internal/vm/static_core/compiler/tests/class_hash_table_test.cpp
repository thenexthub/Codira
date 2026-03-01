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

#include "aot/aot_builder/aot_builder.h"
#include "aot/aot_manager.h"
#include "assembly-parser.h"
#include "unit_test.h"
#include "libarkbase/os/exec.h"
#include "libarkbase/os/filesystem.h"
#include "runtime/include/file_manager.h"

namespace ark::compiler {
class ClassHashTableTest : public AsmTest {
public:
    ClassHashTableTest()
    {
        std::string exePath = GetExecPath();
        auto pos = exePath.rfind('/');
        paocPath_ = exePath.substr(0U, pos) + "/../bin/ark_aot";
        aotdumpPath_ = exePath.substr(0U, pos) + "/../bin/ark_aotdump";
        pandastdlibPath_ = GetPaocDirectory() + "/../pandastdlib/arkstdlib.abc";
    }

    const char *GetPaocFile() const
    {
        return paocPath_.c_str();
    }

    const char *GetAotdumpFile() const
    {
        return aotdumpPath_.c_str();
    }

    const char *GetPandaStdLibFile() const
    {
        return pandastdlibPath_.c_str();
    }

    std::string GetPaocDirectory() const
    {
        auto pos = paocPath_.rfind('/');
        return paocPath_.substr(0U, pos);
    }

    std::string GetSourceCode() const
    {
        return source_;
    }

private:
    std::string paocPath_;
    std::string aotdumpPath_;
    std::string pandastdlibPath_;
    const std::string source_ = R"(
        .record A {}
        .record B {}
        .record C {}
        .record D {}
        .record E {}

        .function i32 A.f() {
            ldai 1
            return
        }

        .function i32 B.f() {
            ldai 2
            return
        }

        .function i32 C.f() {
            ldai 3
            return
        }

        .function i32 D.f() {
            ldai 4
            return
        }

        .function i32 main() {
            ldai 0
            return
        }
    )";
};

TEST_F(ClassHashTableTest, AddClassHashTable)
{
    TmpFile pandaFname("AddClassHashTableTest.abc");

    {
        pandasm::Parser parser;
        auto res = parser.Parse(GetSourceCode());
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
    }

    auto pandaFilePtr = panda_file::OpenPandaFile(pandaFname.GetFileName());
    ASSERT(pandaFilePtr != nullptr);
    auto &pfileRef = *pandaFilePtr;

    AotBuilder aotBuilder;
    aotBuilder.AddClassHashTable(pfileRef);

    auto entityPairHeaders = aotBuilder.GetEntityPairHeaders();
    auto classHashTablesSize = aotBuilder.GetClassHashTableSize()->back();

    ASSERT(!entityPairHeaders->empty());
    ASSERT_EQ(classHashTablesSize, entityPairHeaders->size());
}

void GetClassHashTableABC1(const std::string &fn)
{
    auto source = R"(
            .record Animal {}
            .function i32 Animal.fun() <static> {
                ldai 1
                return
            }
        )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    ASSERT_TRUE(pandasm::AsmEmitter::Emit(fn, res.Value()));
}

void GetClassHashTableABC2(const std::string &fn)
{
    auto source = R"(
            .record Animal <external>
            .record Cat {}
            .record Dog {}
            .record Pig {}
            .function i32 Animal.fun() <external, static>
            .function i32 Cat.main() <static> {
                call.short Animal.fun
                return
            }
        )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    ASSERT_TRUE(pandasm::AsmEmitter::Emit(fn, res.Value()));
}

void GetClassHashTableAN(const std::string &fn1, const std::string &fn2, const std::string &fnAot,
                         const std::string &fnPaoc, const std::string &stdlib)
{
    auto res = os::exec::Exec(fnPaoc.c_str(), "--paoc-panda-files", fn2.c_str(), "--panda-files", fn1.c_str(),
                              "--paoc-output", fnAot.c_str(), "--boot-panda-files", stdlib.c_str());
    ASSERT_TRUE(res);
    ASSERT_EQ(res.Value(), 0U);
}

TEST_F(ClassHashTableTest, GetClassHashTable)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile aotFname("TestTwo.an");
    TmpFile pandaFname1("Animal.abc");
    TmpFile pandaFname2("Cat.abc");

    GetClassHashTableABC1(pandaFname1.GetFileName());
    GetClassHashTableABC2(pandaFname2.GetFileName());
    GetClassHashTableAN(pandaFname1.GetFileName(), pandaFname2.GetFileName(), aotFname.GetFileName(), GetPaocFile(),
                        GetPandaStdLibFile());

    std::string filename = os::GetAbsolutePath(aotFname.GetFileName());
    auto aotFileRet = AotFile::Open(filename, 0U, true);
    ASSERT(aotFileRet.Value() != nullptr);
    auto aotFile = std::move(aotFileRet.Value());

    for (size_t i = 0; i < aotFile->GetFilesCount(); i++) {
        auto fileHeader = aotFile->FileHeaders()[i];
        auto table = aotFile->GetClassHashTable(fileHeader);

        ASSERT(!table.empty());
        ASSERT_EQ(table.size(), fileHeader.classHashTableSize);
    }

    for (size_t i = 0; i < aotFile->GetFilesCount(); i++) {
        auto fileHeader = aotFile->FileHeaders()[i];
        AotPandaFile aotPandaFile(aotFile.get(), &fileHeader);
        auto table = aotPandaFile.GetClassHashTable();

        ASSERT(!table.empty());
        ASSERT_EQ(table.size(), fileHeader.classHashTableSize);
    }
}

TEST_F(ClassHashTableTest, DumpClassHashTable)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile pandaFname("DumpClassHashTableTest.abc");
    TmpFile aotFname("DumpClassHashTableTest.an");

    {
        pandasm::Parser parser;
        auto res = parser.Parse(GetSourceCode());
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
    }

    {
        auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                                  aotFname.GetFileName(), "--boot-panda-files", GetPandaStdLibFile());
        ASSERT_TRUE(res);
        ASSERT_EQ(res.Value(), 0U);
    }

    {
        std::string filename = os::GetAbsolutePath(aotFname.GetFileName());
        auto res = os::exec::Exec(GetAotdumpFile(), "--show-code=disasm", filename.c_str());
        ASSERT_TRUE(res);
        ASSERT_EQ(res.Value(), 0U);
    }
}

TEST_F(ClassHashTableTest, LoadClassHashTableFromAnFileToAbcFile)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile pandaFname("LoadClassHashTableFromAnFileToAbcFileTest.abc");
    TmpFile aotFname("LoadClassHashTableFromAnFileToAbcFileTest.an");

    {
        pandasm::Parser parser;
        auto res = parser.Parse(GetSourceCode());
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
    }

    auto pfile = panda_file::OpenPandaFile(pandaFname.GetFileName());

    {
        auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                                  aotFname.GetFileName(), "--boot-panda-files", GetPandaStdLibFile());
        ASSERT_TRUE(res);
        ASSERT_EQ(res.Value(), 0U);
    }

    std::string filename = os::GetAbsolutePath(aotFname.GetFileName());
    auto aotFileRet = AotFile::Open(filename, 0U, true);
    ASSERT(aotFileRet.Value() != nullptr);
    auto aotFile = std::move(aotFileRet.Value());

    auto fileHeader = aotFile->FileHeaders()[0U];
    AotPandaFile aotPandaFile(aotFile.get(), &fileHeader);
    pfile->SetClassHashTable(aotPandaFile.GetClassHashTable());

    ASSERT(!pfile->GetClassHashTable().empty());
    ASSERT_EQ(pfile->GetClassHashTable().size(), aotPandaFile.GetClassHashTable().size());
}

TEST_F(ClassHashTableTest, LoadAbcFileCanLoadClassHashTable)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile pandaFname("LoadAbcFileTest.abc");
    TmpFile aotFname("LoadAbcFileTest.an");

    {
        pandasm::Parser parser;
        auto res = parser.Parse(GetSourceCode());
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
    }

    auto pfile = panda_file::OpenPandaFile(pandaFname.GetFileName());

    {
        auto res = os::exec::Exec(GetPaocFile(), "--gc-type=epsilon", "--paoc-panda-files", pandaFname.GetFileName(),
                                  "--paoc-output", aotFname.GetFileName(), "--boot-panda-files", GetPandaStdLibFile());
        ASSERT_TRUE(res);
        ASSERT_EQ(res.Value(), 0U);
    }

    std::string filename = os::GetAbsolutePath(pandaFname.GetFileName());
    auto res = FileManager::LoadAbcFile(filename, panda_file::File::READ_ONLY);
    ASSERT_TRUE(res);

    const panda_file::File *pfPtr = nullptr;
    auto checkFilename = [&pfPtr, filename](const panda_file::File &pf) {
        if (pf.GetFilename() == filename) {
            pfPtr = &pf;
            return false;
        }
        return true;
    };
    Runtime::GetCurrent()->GetClassLinker()->EnumeratePandaFiles(checkFilename);

    ASSERT(!pfPtr->GetClassHashTable().empty());
}

void GetClassIdFromClassHashTableABC(const std::string &source, const std::string &fnOut)
{
    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    ASSERT_TRUE(pandasm::AsmEmitter::Emit(fnOut, res.Value()));
}

void GetClassIdFromClassHashTableAN(const std::string &fn, const std::string &fnAot, const std::string &fnPaoc,
                                    const std::string &stdlib)
{
    auto res = os::exec::Exec(fnPaoc.c_str(), "--gc-type=epsilon", "--paoc-panda-files", fn.c_str(), "--paoc-output",
                              fnAot.c_str(), "--boot-panda-files", stdlib.c_str());
    ASSERT_TRUE(res);
    ASSERT_EQ(res.Value(), 0U);
}

TEST_F(ClassHashTableTest, GetClassIdFromClassHashTable)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile pandaFname("GetClassIdFromClassHashTableTest.abc");
    TmpFile aotFname("GetClassIdFromClassHashTableTest.an");

    GetClassIdFromClassHashTableABC(GetSourceCode(), pandaFname.GetFileName());

    auto pfile = panda_file::OpenPandaFile(pandaFname.GetFileName());

    auto *descriptorA = reinterpret_cast<const uint8_t *>("A");
    auto *descriptorB = reinterpret_cast<const uint8_t *>("B");
    auto *descriptorC = reinterpret_cast<const uint8_t *>("C");
    auto *descriptorD = reinterpret_cast<const uint8_t *>("D");
    auto *descriptorE = reinterpret_cast<const uint8_t *>("E");

    auto classId1A = pfile->GetClassId(descriptorA);
    auto classId1B = pfile->GetClassId(descriptorB);
    auto classId1C = pfile->GetClassId(descriptorC);
    auto classId1D = pfile->GetClassId(descriptorD);
    auto classId1E = pfile->GetClassId(descriptorE);

    GetClassIdFromClassHashTableAN(pandaFname.GetFileName(), aotFname.GetFileName(), GetPaocFile(),
                                   GetPandaStdLibFile());

    std::string filename = os::GetAbsolutePath(pandaFname.GetFileName());
    auto res = FileManager::LoadAbcFile(filename, panda_file::File::READ_ONLY);
    ASSERT_TRUE(res);

    const panda_file::File *pfPtr = nullptr;
    Runtime::GetCurrent()->GetClassLinker()->EnumeratePandaFiles([&pfPtr, filename](const panda_file::File &pf) {
        if (pf.GetFilename() == filename) {
            pfPtr = &pf;
            return false;
        }
        return true;
    });

    auto classId2A = pfPtr->GetClassIdFromClassHashTable(descriptorA);
    auto classId2B = pfPtr->GetClassIdFromClassHashTable(descriptorB);
    auto classId2C = pfPtr->GetClassIdFromClassHashTable(descriptorC);
    auto classId2D = pfPtr->GetClassIdFromClassHashTable(descriptorD);
    auto classId2E = pfPtr->GetClassIdFromClassHashTable(descriptorE);

    ASSERT_EQ(classId1A, classId2A);
    ASSERT_EQ(classId1B, classId2B);
    ASSERT_EQ(classId1C, classId2C);
    ASSERT_EQ(classId1D, classId2D);
    ASSERT_EQ(classId1E, classId2E);
}

}  // namespace ark::compiler
