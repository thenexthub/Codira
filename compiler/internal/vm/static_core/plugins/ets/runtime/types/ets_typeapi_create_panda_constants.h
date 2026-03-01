/*
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

#ifndef PANDA_PLUGINS_ETS_TYPEAPI_CREATE_PANDA_CONSTANTS_H
#define PANDA_PLUGINS_ETS_TYPEAPI_CREATE_PANDA_CONSTANTS_H

#include <array>
#include <string_view>

namespace ark::ets::typeapi_create_consts {
inline constexpr std::string_view ATTR_ACCESS = "ets.access";
inline constexpr std::string_view ATTR_ACCESS_VAL_PUBLIC = "public";
inline constexpr std::string_view ATTR_ACCESS_VAL_PROTECTED = "protected";
inline constexpr std::string_view ATTR_ACCESS_VAL_PRIVATE = "private";

inline constexpr std::string_view ATTR_ACCESS_FUNCTION = "access.function";
inline constexpr std::string_view ATTR_ACCESS_FIELD = "access.field";

inline constexpr std::string_view ATTR_EXTENDS = "ets.extends";
inline constexpr std::string_view ATTR_IMPLEMENTS = "ets.implements";

inline constexpr std::string_view ATTR_ABSTRACT = "ets.abstract";
inline constexpr std::string_view ATTR_NOIMPL = "noimpl";
inline constexpr std::string_view ATTR_EXTERNAL = "external";
inline constexpr std::string_view ATTR_FINAL = "final";
inline constexpr std::string_view ATTR_STATIC = "static";
inline constexpr std::string_view ATTR_CTOR = "ctor";
inline constexpr std::string_view ATTR_CCTOR = "cctor";

inline constexpr std::array<std::string_view, 2> ATTR_INTERFACE = {"ets.interface", ATTR_ABSTRACT};

inline constexpr std::array<std::string_view, 2> ATTR_ABSTRACT_METHOD = {ATTR_ABSTRACT, ATTR_NOIMPL};

inline constexpr std::string_view TYPE_OBJECT = "std.core.Object";
inline constexpr std::string_view TYPE_BOXED_PREFIX = "std.core.";

inline constexpr std::string_view FUNCTION_GET_OBJECTS_FOR_CCTOR = "std.core.TypeCreatorCtx.getObjectsArrayForCCtor";
inline constexpr std::string_view TYPE_TYPE_CREATOR_CTX =
    FUNCTION_GET_OBJECTS_FOR_CCTOR.substr(0, FUNCTION_GET_OBJECTS_FOR_CCTOR.rfind('.'));
static_assert(TYPE_TYPE_CREATOR_CTX.back() != '.');

inline constexpr std::string_view CREATOR_CTX_DATA_PREFIX = "TypeAPI$CtxData$";
}  // namespace ark::ets::typeapi_create_consts

#endif  // PANDA_PLUGINS_ETS_TYPEAPI_CREATE_PANDA_CONSTANTS_H
