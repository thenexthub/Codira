/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_EXCEPTIONS_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_EXCEPTIONS_H_

#include <string_view>
#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "runtime/include/mem/panda_string.h"

namespace ark::ets {

class EtsCoroutine;

class EtsObject;

PANDA_PUBLIC_API EtsObject *SetupEtsException(EtsCoroutine *coroutine, const char *classDescriptor, const char *msg);

// NOTE: Is used to throw all language exceptional objects (currently Errors and Exceptions)
PANDA_PUBLIC_API void ThrowEtsException(EtsCoroutine *coroutine, const char *classDescriptor, const char *msg);

// NOTE: Is used to throw all language exceptional objects (currently Errors and Exceptions)
inline void ThrowEtsException(EtsCoroutine *coroutine, std::string_view classDescriptor, const char *msg)
{
    ThrowEtsException(coroutine, classDescriptor.data(), msg);
}

// NOTE: Is used to throw all language exceptional objects (currently Errors and Exceptions)
inline void ThrowEtsException(EtsCoroutine *coroutine, std::string_view classDescriptor, std::string_view msg)
{
    ThrowEtsException(coroutine, classDescriptor.data(), msg.data());
}

inline void ThrowEtsFieldNotFoundException(EtsCoroutine *coroutine, const char *holderClassDescriptor,
                                           const char *fieldName)
{
    PandaString message = "Field " + PandaString(fieldName) + " not found in " + PandaString(holderClassDescriptor);
    ThrowEtsException(coroutine, panda_file_items::class_descriptors::TYPE_ERROR, message.c_str());
}

inline void ThrowEtsMethodNotFoundException(EtsCoroutine *coroutine, const char *holderClassDescriptor,
                                            const char *methodName, const char *methodSignature)
{
    PandaString message = "Method " + PandaString(methodName) + "(" + methodSignature + ") not found in " +
                          PandaString(holderClassDescriptor);
    ThrowEtsException(coroutine, panda_file_items::class_descriptors::TYPE_ERROR, message.c_str());
}

inline void ThrowEtsInvalidKey(EtsCoroutine *coroutine, const char *classSignature)
{
    PandaString message = "Invalid key type: " + PandaString(classSignature);
    ThrowEtsException(coroutine, panda_file_items::class_descriptors::TYPE_ERROR, message.c_str());
}

inline void ThrowEtsInvalidType(EtsCoroutine *coroutine, const char *classSignature)
{
    PandaString message = "Invalid operand type: " + PandaString(classSignature);
    ThrowEtsException(coroutine, panda_file_items::class_descriptors::TYPE_ERROR, message.c_str());
}

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_EXCEPTIONS_H_
