
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

#ifndef INTERPRETER_TEST_UTILS_H_
#define INTERPRETER_TEST_UTILS_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "assembly-parser.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/utils/utf.h"
#include "libarkbase/utils/utils.h"
#include "libarkfile/bytecode_emitter.h"
#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "libarkfile/value.h"
#include "optimizer/ir/runtime_interface.h"
#include "runtime/bridge/bridge.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/interpreter/frame.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/internal_allocator.h"
#include "runtime/core/core_class_linker_extension.h"
#include "runtime/tests/class_linker_test_extension.h"
#include "runtime/tests/interpreter/test_interpreter.h"
#include "runtime/tests/interpreter/test_runtime_interface.h"
#include "runtime/include/coretypes/dyn_objects.h"
#include "runtime/include/hclass.h"
#include "runtime/handle_base-inl.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/native_pointer.h"
#include "runtime/tests/test_utils.h"
#include "libarkbase/test_utilities.h"

namespace ark::interpreter::test {

inline auto CreateFrame(uint32_t nregs, Method *method, Frame *prev)
{
    auto frameDeleter = [](Frame *frame) {
        interpreter::test::RuntimeInterface::FreeFrame(ManagedThread::GetCurrent(), frame);
    };
    std::unique_ptr<Frame, decltype(frameDeleter)> frame(
        interpreter::test::RuntimeInterface::template CreateFrame<false>(nregs, method, prev), frameDeleter);
    return frame;
}

inline Class *CreateClass(panda_file::SourceLang lang)
{
    const std::string className("Foo");
    auto runtime = Runtime::GetCurrent();
    auto etx = runtime->GetClassLinker()->GetExtension(runtime->GetLanguageContext(lang));
    auto klass = etx->CreateClass(reinterpret_cast<const uint8_t *>(className.data()), 0, 0,
                                  AlignUp(sizeof(Class), OBJECT_POINTER_SIZE));
    return klass;
}

static std::pair<PandaUniquePtr<Method>, std::unique_ptr<const panda_file::File>> CreateMethod(
    Class *klass, uint32_t accessFlags, uint32_t nargs, uint32_t nregs, uint16_t *shorty,
    const std::vector<uint8_t> &bytecode)
{
    // Create panda_file

    panda_file::ItemContainer container;
    panda_file::ClassItem *classItem = container.GetOrCreateGlobalClassItem();
    classItem->SetAccessFlags(ACC_PUBLIC);

    panda_file::StringItem *methodName = container.GetOrCreateStringItem("test");
    panda_file::PrimitiveTypeItem *retType = container.GetOrCreatePrimitiveTypeItem(panda_file::Type::TypeId::VOID);
    std::vector<panda_file::MethodParamItem> params;
    panda_file::ProtoItem *protoItem = container.GetOrCreateProtoItem(retType, params);
    panda_file::MethodItem *methodItem = classItem->AddMethod(methodName, protoItem, ACC_PUBLIC | ACC_STATIC, params);

    auto *codeItem = container.CreateItem<panda_file::CodeItem>(nregs, nargs, bytecode);
    methodItem->SetCode(codeItem);

    panda_file::MemoryWriter memWriter;
    container.Write(&memWriter);

    auto data = memWriter.GetData();

    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto buf = allocator->AllocArray<uint8_t>(data.size());
    MemcpyUnsafe(buf, data.data(), data.size());

    os::mem::ConstBytePtr ptr(reinterpret_cast<std::byte *>(buf), data.size(), [](std::byte *buffer, size_t) noexcept {
        auto a = Runtime::GetCurrent()->GetInternalAllocator();
        a->Free(buffer);
    });
    auto pf = panda_file::File::OpenFromMemory(std::move(ptr));

    // Create method

    auto method = MakePandaUnique<Method>(klass, pf.get(), methodItem->GetFileId(), codeItem->GetFileId(),
                                          accessFlags | ACC_PUBLIC | ACC_STATIC, nargs, shorty);
    method->SetInterpreterEntryPoint();
    return {std::move(method), std::move(pf)};
}

inline std::pair<PandaUniquePtr<Method>, std::unique_ptr<const panda_file::File>> CreateMethod(
    Class *klass, Frame *f, const std::vector<uint8_t> &bytecode)
{
    return CreateMethod(klass, 0, 0, f->GetSize(), nullptr, bytecode);
}

inline Frame *CreateFrameWithSize(uint32_t size, uint32_t nregs, Method *method, Frame *prev, ManagedThread *current)
{
    uint32_t extSz = CORE_EXT_FRAME_DATA_SIZE;
    size_t allocSz = Frame::GetAllocSize(size, extSz);
    size_t mirrorOffset = extSz + sizeof(Frame) + sizeof(interpreter::VRegister) * nregs;
    void *frame = current->GetStackFrameAllocator()->Alloc<false>(allocSz);
    auto mirrorPartBytes = reinterpret_cast<uint64_t *>(ToVoidPtr(ToUintPtr(frame) + mirrorOffset));
    for (size_t i = 0; i < nregs; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        mirrorPartBytes[i] = 0x00;
    }
    return new (Frame::FromExt(frame, extSz)) Frame(frame, method, prev, nregs);
}

}  // namespace ark::interpreter::test

#endif  // INTERPRETER_TEST_UTILS_H_
