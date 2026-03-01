/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "runtime/tests/interpreter/test_runtime_interface.h"

namespace ark::interpreter::test {

RuntimeInterface::NullPointerExceptionData RuntimeInterface::npeData_;

RuntimeInterface::ArrayIndexOutOfBoundsExceptionData RuntimeInterface::arrayOobExceptionData_;

RuntimeInterface::NegativeArraySizeExceptionData RuntimeInterface::arrayNegSizeExceptionData_;

RuntimeInterface::ArithmeticException RuntimeInterface::arithmeticExceptionData_;

RuntimeInterface::ClassCastExceptionData RuntimeInterface::classCastExceptionData_;

RuntimeInterface::AbstractMethodError RuntimeInterface::abstractMethodErrorData_;

RuntimeInterface::ArrayStoreExceptionData RuntimeInterface::arrayStoreExceptionData_;

coretypes::Array *RuntimeInterface::arrayObject_;

Class *RuntimeInterface::arrayClass_;

uint32_t RuntimeInterface::arrayLength_;

Class *RuntimeInterface::resolvedClass_;

ObjectHeader *RuntimeInterface::object_;

Class *RuntimeInterface::objectClass_;

uint32_t RuntimeInterface::catchBlockPcOffset_;

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
RuntimeInterface::InvokeMethodHandler RuntimeInterface::invokeHandler_;

DummyGC::DummyGC(ark::mem::ObjectAllocatorBase *objectAllocator, const ark::mem::GCSettings &settings)
    : GC(objectAllocator, settings)
{
}

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
DummyGC RuntimeInterface::dummyGc_(nullptr, ark::mem::GCSettings());

Method *RuntimeInterface::resolvedMethod_;

Field *RuntimeInterface::resolvedField_;

const void *RuntimeInterface::entryPoint_;

uint32_t RuntimeInterface::jitThreshold_;

}  // namespace ark::interpreter::test
