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

#ifndef GUARD_CONFIGURATION_CONFIGURATION_CONSTANTS_H
#define GUARD_CONFIGURATION_CONFIGURATION_CONSTANTS_H

#include <string_view>

namespace ark::guard {

namespace AccessFlagsConstants {
constexpr std::string_view PUBLIC = "public";
constexpr std::string_view PROTECTED = "protected";
constexpr std::string_view PRIVATE = "private";
constexpr std::string_view STATIC = "static";
constexpr std::string_view ASYNC = "async";
constexpr std::string_view ABSTRACT = "abstract";
constexpr std::string_view NATIVE = "native";
constexpr std::string_view FINAL = "final";
constexpr std::string_view ANNOTATION = "@";
constexpr std::string_view DECLARE = "declare";
}  // namespace AccessFlagsConstants

namespace TypeDeclarationsConstants {
constexpr std::string_view INTERFACE = "interface";
constexpr std::string_view CLASS = "class";
constexpr std::string_view ENUM = "enum";
constexpr std::string_view TYPE = "type";
constexpr std::string_view NAMESPACE = "namespace";
constexpr std::string_view PACKAGE = "package";
}  // namespace TypeDeclarationsConstants

namespace ExtensionTypesConstants {
constexpr std::string_view EXTENDS = "extends";
constexpr std::string_view IMPLEMENTS = "implements";
}  // namespace ExtensionTypesConstants

namespace KeepOptionsConstants {
constexpr std::string_view KEEP_OPTION = "-keep";
constexpr std::string_view KEEP_CLASS_MEMBERS = "-keepclassmembers";
constexpr std::string_view KEEP_CLASS_WITH_MEMBERS = "-keepclasswithmembers";
}  // namespace KeepOptionsConstants

namespace ConfigurationConstants {
constexpr std::string_view KEEP_OPTION_PREFIX = "-";
constexpr std::string_view AT_DIRECTIVE = "@";

constexpr std::string_view NEGATOR_KEYWORD = "!";
constexpr std::string_view OPEN_KEYWORD = "{";
constexpr std::string_view CLOSE_KEYWORD = "}";

constexpr std::string_view ANNOTATION_KEYWORD = "@";

constexpr std::string_view ANY_CLASS_KEYWORD = "*";
constexpr std::string_view ANY_CLASS_MEMBER_KEYWORD = "*";
constexpr std::string_view ANY_FIELD_KEYWORD = "<fields>";
constexpr std::string_view ANY_METHOD_KEYWORD = "<methods>";

constexpr std::string_view OPEN_ARGUMENTS_KEYWORD = "(";
constexpr std::string_view CLOSE_ARGUMENTS_KEYWORD = ")";

constexpr std::string_view EQUAL_KEYWORD = "=";
constexpr std::string_view RETURN_KEYWORD = "return";

constexpr std::string_view COMMA_KEYWORD = ",";

constexpr std::string_view COLON_KEYWORD = ":";

constexpr std::string_view SEMICOLON_KEYWORD = ";";

}  // namespace ConfigurationConstants

}  // namespace ark::guard

#endif  // GUARD_CONFIGURATION_CONFIGURATION_CONSTANTS_H
