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

#include <iostream>
#include <string>

#include <gtest/gtest.h>
#include "assembly-parser.h"
#include "disassembler.h"

static inline std::string ExtractFuncBody(const std::string &text, const std::string &header)
{
    auto beg = text.find(header);
    auto end = text.find('}', beg);

    ASSERT(beg != std::string::npos);
    ASSERT(end != std::string::npos);

    return text.substr(beg + header.length(), end - (beg + header.length()));
}

namespace ark::disasm::test {

TEST(FunctionsTest, EmptyFunction)
{
    auto program = ark::pandasm::Parser().Parse(R"(
        .function void A(i32 a0) {}
    )");
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    std::string line;
    std::getline(ss, line);
    ASSERT_EQ(line, ".language PandaAssembly");
    std::getline(ss, line);
    ASSERT_EQ(line, "");
    std::getline(ss, line);
    ASSERT_EQ(line, ".function void A(i32 a0) <static> {");
    std::getline(ss, line);
    ASSERT_EQ(line, "}");
}

TEST(FunctionsTest, OverloadingTest)
{
    auto program = ark::pandasm::Parser().Parse(R"(
        .function void f() {}

        .function void f(u1 a0) {}

        .function void f(u1 a0, i8 a1) {}

        .function void main() {
            call f:()
            call f:(u1), v1
            call f:(u1, i8), v1, v1
        }
    )");
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    std::string bodyMain = ExtractFuncBody(ss.str(), "main() <static> {\n");

    std::string line;
    std::stringstream main {bodyMain};
    std::getline(main, line);
    EXPECT_EQ("\tcall.short <static> f:()", line);
    std::getline(main, line);
    EXPECT_EQ("\tcall.short <static> f:(u1), v1", line);
    std::getline(main, line);
    EXPECT_EQ("\tcall.short <static> f:(u1,i8), v1, v1", line);

    EXPECT_TRUE(ss.str().find(".function void f() <static> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function void f(u1 a0) <static> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function void f(u1 a0, i8 a1) <static> {") != std::string::npos);
}

static std::pair<std::unique_ptr<const ark::panda_file::File>, ark::panda_file::File::EntityId> BuildPandaMethod()
{
    auto program = ark::pandasm::Parser().Parse(R"(
        .function void func() {

            nop

        label:

            nop

            jmp label

            return
        }
    )");
    ASSERT(program);

    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::panda_file::DebugInfoExtractor debugInfo(pf.get());
    auto methodIds = debugInfo.GetMethodIdList();
    ASSERT(methodIds.size() == 1);

    return std::make_pair(std::move(pf), methodIds[0]);
}

TEST(FunctionsTest, SerializeText)
{
    auto [pf, method_id] = BuildPandaMethod();

    ark::disasm::Disassembler d {};
    d.SetFile(*pf);

    ark::pandasm::Function method("", ark::SourceLanguage::PANDA_ASSEMBLY);
    d.GetMethod(&method, method_id);

    std::stringstream ss {};
    d.Serialize(method, ss);

    std::string line;
    std::getline(ss, line);
    ASSERT_EQ(line, ".function void func() <static> {");
    std::getline(ss, line);
    ASSERT_EQ(line, "\tnop");
    std::getline(ss, line);
    ASSERT_EQ(line, "jump_label_0:");
    std::getline(ss, line);
    ASSERT_EQ(line, "\tnop");
    std::getline(ss, line);
    ASSERT_EQ(line, "\tjmp jump_label_0");
    std::getline(ss, line);
    ASSERT_EQ(line, "\treturn");
    std::getline(ss, line);
    ASSERT_EQ(line, "}");
}

TEST(FunctionsTest, SerializeLineTable)
{
    auto [pf, method_id] = BuildPandaMethod();

    ark::disasm::Disassembler d {};
    d.SetFile(*pf);

    ark::pandasm::Function method("", ark::SourceLanguage::PANDA_ASSEMBLY);
    d.GetMethod(&method, method_id);

    std::stringstream ss {};
    ark::panda_file::LineNumberTable lineTable;
    d.Serialize(method, ss, false, &lineTable);

    ASSERT_EQ(lineTable.size(), 4);

    ASSERT_EQ(lineTable[0].offset, 0);
    ASSERT_EQ(lineTable[0].line, 2);

    ASSERT_EQ(lineTable[1].offset, 1);
    ASSERT_EQ(lineTable[1].line, 4);

    ASSERT_EQ(lineTable[2].offset, 2);
    ASSERT_EQ(lineTable[2].line, 5);

    ASSERT_EQ(lineTable[3].offset, 4);
    ASSERT_EQ(lineTable[3].line, 6);
}
static std::unique_ptr<const panda_file::File> GetPandaFile(std::vector<uint8_t> *data)
{
    os::mem::ConstBytePtr ptr(reinterpret_cast<std::byte *>(data->data()), data->size(),
                              [](std::byte *, size_t) noexcept {});
    return panda_file::File::OpenFromMemory(std::move(ptr));
}

TEST(DisassemblerTest, FullRecordNameEmptyNameDeathTest)
{
    // Write panda file to memory
    ark::panda_file::ItemContainer container;

    // set current class's superclass to self
    ark::panda_file::ClassItem *classItem = container.GetOrCreateClassItem("");
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
    auto fwriter = ark::panda_file::FileWriter("test.abc");
    ASSERT_TRUE(container.Write(&fwriter));
    panda_file::MemoryWriter writer;
    ASSERT_TRUE(container.Write(&writer));
    auto data = writer.GetData();

    auto pandaFile = GetPandaFile(&data);
    ASSERT_NE(pandaFile, nullptr);

    ark::disasm::Disassembler d {};
    ark::Logger::InitializeStdLogging(ark::Logger::Level::FATAL,
                                      ark::Logger::ComponentMask().set(ark::Logger::Component::DISASSEMBLER));
    EXPECT_DEATH({ d.Disassemble(*pandaFile); }, ".*Record name is empty.*|.*");
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid test logic
TEST(FunctionsTest, AnyTypeTest)
{
    auto program = ark::pandasm::Parser().Parse(R"(
        .record Y <external>
        .record N <external>

        .function i32 Any(Y a0) <static, access.function=public> {}
        .function i32 never(Y a0) <static, access.function=public> {}
        .function i32 foo(Y a0) <static, access.function=public> {}
        .function Y foo1() <static, access.function=public> {}
        .function Y foo2(Y a0) <static, access.function=public> {}
        .function Y foo7(Y a0, Y a1, Y a2, Y a3, Y a4) <static, access.function=public> {}
        .function Y foo8(Y a0, f32 a1, Y a2, Y a3, Y a4) <static, access.function=public> {}
        .function i32 foo6(Y a0, Y a1, Y a2, Y a3, Y a4) <static, access.function=public> {}
        .function i32 foo5(Y a0, f32 a1, i32 a2) <static, access.function=public> {}
        .function Y[] foo(Y a0, Y a1, Y a2, Y a3, Y a4) <static, access.function=public> {}
        .function Y[] foo(Y a0, f32 a1, Y a2, Y[] a3, Y a4) <static, access.function=public> {}

        .function i32 foo(N a0) <static, access.function=public> {}
        .function N foo9() <static, access.function=public> {}
        .function N foo2(N a0) <static, access.function=public> {}
        .function N foo7(N a0, N a1, N a2, N a3, N a4) <static, access.function=public> {}
        .function N foo8(N a0, f32 a1, N a2, N a3, N a4) <static, access.function=public> {}
        .function i32 foo6(N a0, N a1, N a2, N a3, N a4) <static, access.function=public> {}
        .function i32 foo5(N a0, f32 a1, i32 a2) <static, access.function=public> {}
    )");
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    EXPECT_TRUE(ss.str().find(".function i32 Any(Y a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function i32 never(Y a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function i32 foo(Y a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function Y foo1() <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function Y foo2(Y a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function Y foo7(Y a0, Y a1, Y a2, Y a3, Y a4) <static, access.function=public> {") !=
                std::string::npos);
    EXPECT_TRUE(ss.str().find(".function Y foo8(Y a0, f32 a1, Y a2, Y a3, Y a4) <static, access.function=public> {") !=
                std::string::npos);
    EXPECT_TRUE(ss.str().find(".function i32 foo6(Y a0, Y a1, Y a2, Y a3, Y a4) <static, access.function=public> {") !=
                std::string::npos);
    EXPECT_TRUE(
        ss.str().find(".function Y[] foo(Y a0, f32 a1, Y a2, Y[] a3, Y a4) <static, access.function=public> {") !=
        std::string::npos);
    EXPECT_TRUE(ss.str().find(".function Y[] foo(Y a0, Y a1, Y a2, Y a3, Y a4) <static, access.function=public> {") !=
                std::string::npos);
    EXPECT_TRUE(ss.str().find(".function i32 foo5(Y a0, f32 a1, i32 a2) <static, access.function=public> {") !=
                std::string::npos);

    EXPECT_TRUE(ss.str().find(".function i32 foo(N a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function N foo9() <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function N foo2(N a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function N foo7(N a0, N a1, N a2, N a3, N a4) <static, access.function=public> {") !=
                std::string::npos);
    EXPECT_TRUE(ss.str().find(".function N foo8(N a0, f32 a1, N a2, N a3, N a4) <static, access.function=public> {") !=
                std::string::npos);
    EXPECT_TRUE(ss.str().find(".function i32 foo6(N a0, N a1, N a2, N a3, N a4) <static, access.function=public> {") !=
                std::string::npos);
    EXPECT_TRUE(ss.str().find(".function i32 foo5(N a0, f32 a1, i32 a2) <static, access.function=public> {") !=
                std::string::npos);
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid test logic
TEST(FunctionsTest, AnyInInstructionsTest)
{
    auto program = ark::pandasm::Parser().Parse(R"(
        .record Y <external>
        .record N <external>

        .function void foo1(Y a0) <static, access.function=public> {
            lda.type Y
            return.void
        }

        .function void foo2(N a0) <static, access.function=public> {
            lda.type N
            return.void
        }

        .function void foo3(Y a0) <static, access.function=public> {
            lda.type Y[]
            return.void
        }

       .function void foo4(Y a0) <static, access.function=public> {
            lda.type Y[][]
            return.void
        }

        .function i32 foo5(N a0) <static, access.function=public> {
            lda.obj a0
            isinstance Y
            return
        }

        .function i32 foo6(N a0) <static, access.function=public> {
            lda.obj a0
            isinstance N
            return
        }

        .function i32 foo7(N a0) <static, access.function=public> {
            lda.obj a0
            isinstance Y[]
            return
        }

        .function i32 foo8(N a0) <static, access.function=public> {
            lda.obj a0
            isinstance Y[][]
            return
        }

        .function void foo9(Y a0) <static, access.function=public> {
            lda.obj a0
            checkcast Y
            return.void
        }

        .function void foo10(N a0) <static, access.function=public> {
            lda.obj a0
            checkcast N
            return.void
        }

        .function void foo11(Y a0) <static, access.function=public> {
            lda.obj a0
            checkcast Y[]
            return.void
        }

        .function void foo12(Y a0) <static, access.function=public> {
            lda.obj a0
            checkcast Y[][]
            return.void
        }
    )");
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    EXPECT_TRUE(ss.str().find(".function void foo1(Y a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function void foo2(N a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function void foo3(Y a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function void foo4(Y a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function i32 foo5(N a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function i32 foo6(N a0) <static, access.function=public> {") != std::string::npos);

    EXPECT_TRUE(ss.str().find(".function i32 foo7(N a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function i32 foo8(N a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function void foo9(Y a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function void foo10(N a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function void foo11(Y a0) <static, access.function=public> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function void foo12(Y a0) <static, access.function=public> {") != std::string::npos);
}
#undef DISASM_BIN_DIR

}  // namespace ark::disasm::test
