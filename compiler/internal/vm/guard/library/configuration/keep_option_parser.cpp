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

#include "keep_option_parser.h"

#include <iostream>
#include <regex>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "abckit_wrapper/modifiers.h"

#include "configuration_constants.h"
#include "error.h"
#include "logger.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace {
constexpr uint32_t VALID_FLAGS_FIELD = abckit_wrapper::AccessFlags::PUBLIC | abckit_wrapper::AccessFlags::PROTECTED |
                                       abckit_wrapper::AccessFlags::PRIVATE | abckit_wrapper::AccessFlags::STATIC |
                                       abckit_wrapper::AccessFlags::FINAL;

constexpr uint32_t VALID_FLAGS_METHOD = abckit_wrapper::AccessFlags::PUBLIC | abckit_wrapper::AccessFlags::PROTECTED |
                                        abckit_wrapper::AccessFlags::PRIVATE | abckit_wrapper::AccessFlags::STATIC |
                                        abckit_wrapper::AccessFlags::NATIVE | abckit_wrapper::AccessFlags::FINAL |
                                        abckit_wrapper::AccessFlags::ABSTRACT | abckit_wrapper::AccessFlags::ASYNC;

constexpr uint32_t INVALID_MODIFIERS = 0;

uint32_t GetClassAccessFlags(const std::string &word)
{
    static const std::unordered_map<std::string_view, abckit_wrapper::AccessFlags> table = {
        {ark::guard::AccessFlagsConstants::ANNOTATION, abckit_wrapper::AccessFlags::ANNOTATION},
        {ark::guard::AccessFlagsConstants::FINAL, abckit_wrapper::AccessFlags::FINAL},
        {ark::guard::AccessFlagsConstants::ABSTRACT, abckit_wrapper::AccessFlags::ABSTRACT},
        {ark::guard::AccessFlagsConstants::DECLARE, abckit_wrapper::AccessFlags::DECLARE},
    };

    const auto it = table.find(word);
    return it != table.end() ? static_cast<uint32_t>(it->second) : INVALID_MODIFIERS;
}

uint32_t GetMemberAccessFlags(const std::string &word)
{
    static const std::unordered_map<std::string_view, abckit_wrapper::AccessFlags> table = {
        {ark::guard::AccessFlagsConstants::PUBLIC, abckit_wrapper::AccessFlags::PUBLIC},
        {ark::guard::AccessFlagsConstants::PROTECTED, abckit_wrapper::AccessFlags::PROTECTED},
        {ark::guard::AccessFlagsConstants::PRIVATE, abckit_wrapper::AccessFlags::PRIVATE},
        {ark::guard::AccessFlagsConstants::STATIC, abckit_wrapper::AccessFlags::STATIC},
        {ark::guard::AccessFlagsConstants::NATIVE, abckit_wrapper::AccessFlags::NATIVE},
        {ark::guard::AccessFlagsConstants::FINAL, abckit_wrapper::AccessFlags::FINAL},
        {ark::guard::AccessFlagsConstants::ABSTRACT, abckit_wrapper::AccessFlags::ABSTRACT},
        {ark::guard::AccessFlagsConstants::ASYNC, abckit_wrapper::AccessFlags::ASYNC},
    };

    const auto it = table.find(word);
    return it != table.end() ? static_cast<uint32_t>(it->second) : INVALID_MODIFIERS;
}

uint32_t GetTypeDeclarations(const std::string &word)
{
    static const std::unordered_map<std::string_view, abckit_wrapper::TypeDeclarations> table = {
        {ark::guard::TypeDeclarationsConstants::INTERFACE, abckit_wrapper::TypeDeclarations::INTERFACE},
        {ark::guard::TypeDeclarationsConstants::CLASS, abckit_wrapper::TypeDeclarations::CLASS},
        {ark::guard::TypeDeclarationsConstants::ENUM, abckit_wrapper::TypeDeclarations::ENUM},
        {ark::guard::TypeDeclarationsConstants::TYPE, abckit_wrapper::TypeDeclarations::TYPE},
        {ark::guard::TypeDeclarationsConstants::NAMESPACE, abckit_wrapper::TypeDeclarations::NAMESPACE},
        {ark::guard::TypeDeclarationsConstants::PACKAGE, abckit_wrapper::TypeDeclarations::PACKAGE},
    };

    const auto it = table.find(word);
    return it != table.end() ? static_cast<uint32_t>(it->second) : INVALID_MODIFIERS;
}

uint32_t GetExtensionType(const std::string &word)
{
    static const std::unordered_map<std::string_view, abckit_wrapper::ExtensionType> table = {
        {ark::guard::ExtensionTypesConstants::EXTENDS, abckit_wrapper::ExtensionType::EXTENDS},
        {ark::guard::ExtensionTypesConstants::IMPLEMENTS, abckit_wrapper::ExtensionType::IMPLEMENTS},
    };

    const auto it = table.find(word);
    return it != table.end() ? static_cast<uint32_t>(it->second) : INVALID_MODIFIERS;
}

bool IsTypeDeclarations(const std::string &word)
{
    static const std::unordered_set<std::string_view> typeDeclarationsList = {
        {ark::guard::TypeDeclarationsConstants::INTERFACE}, {ark::guard::TypeDeclarationsConstants::CLASS},
        {ark::guard::TypeDeclarationsConstants::ENUM},      {ark::guard::TypeDeclarationsConstants::TYPE},
        {ark::guard::TypeDeclarationsConstants::NAMESPACE}, {ark::guard::TypeDeclarationsConstants::PACKAGE},
    };

    bool negated =
        ark::guard::StringUtil::IsSubStrMatched(word, ark::guard::ConfigurationConstants::NEGATOR_KEYWORD.data());
    std::string strippedWord =
        negated ? word.substr(ark::guard::ConfigurationConstants::NEGATOR_KEYWORD.length()) : word;
    const auto it = typeDeclarationsList.find(strippedWord);
    return it != typeDeclarationsList.end();
}

bool ConfigurationEnd(const std::string &word, bool expectingAtCharacter)
{
    return word.empty() ||
           ark::guard::StringUtil::IsSubStrMatched(word,
                                                   ark::guard::ConfigurationConstants::KEEP_OPTION_PREFIX.data()) ||
           (!expectingAtCharacter && word == ark::guard::ConfigurationConstants::AT_DIRECTIVE);
}

bool ConfigurationEnd(const std::string &word)
{
    return ConfigurationEnd(word, false);
}

void CheckFieldAccessFlags(uint32_t setAccessFlags, uint32_t unSetAccessFlags, const std::string &word)
{
    GUARD_ASSERT(((setAccessFlags | unSetAccessFlags) & ~VALID_FLAGS_FIELD) != 0,
                     ark::guard::ErrorCode::CLASS_SPECIFICATION_FORMAT_ERROR,
                     "ClassSpecification parsing failed: invalid access flags for field:" + word);
}

void CheckMethodAccessFlags(uint32_t setAccessFlags, uint32_t unSetAccessFlags, const std::string &word)
{
    GUARD_ASSERT(((setAccessFlags | unSetAccessFlags) & ~VALID_FLAGS_METHOD) != 0,
                     ark::guard::ErrorCode::CLASS_SPECIFICATION_FORMAT_ERROR,
                     "ClassSpecification parsing failed: invalid access flags for method:" + word);
}

void CheckAccessFlags(uint32_t setAccessFlags, uint32_t unSetAccessFlags, const std::string &word)
{
    GUARD_ASSERT((setAccessFlags & unSetAccessFlags) != 0, ark::guard::ErrorCode::CLASS_SPECIFICATION_FORMAT_ERROR,
                     "ClassSpecification parsing failed: conflicting access flags for:" + word);
}

void CheckTypeDeclarations(uint32_t setTypeDeclarations, uint32_t unSetTypeDeclarations, const std::string &word)
{
    GUARD_ASSERT((setTypeDeclarations & unSetTypeDeclarations) != 0,
                     ark::guard::ErrorCode::CLASS_SPECIFICATION_FORMAT_ERROR,
                     "ClassSpecification parsing failed: conflicting type declarations for:" + word);
}

void CheckSemicolonKeyword(const std::string &actualWord)
{
    GUARD_ASSERT(actualWord != ark::guard::ConfigurationConstants::SEMICOLON_KEYWORD,
                     ark::guard::ErrorCode::CLASS_SPECIFICATION_FORMAT_ERROR,
                     "ClassSpecification parsing failed: the expect character is " +
                         std::string(ark::guard::ConfigurationConstants::SEMICOLON_KEYWORD) +
                         " but actual character is " + actualWord);
}
}  // namespace

ark::guard::KeepOptionParser::KeepOptionParser(const std::string &keepOptionStr) : wordReader_(keepOptionStr)
{
    LOG_I << "keep option :" << keepOptionStr;
}

std::optional<ark::guard::ClassSpecification> ark::guard::KeepOptionParser::Parse()
{
    ClassSpecification specification;
    nextWord_ = wordReader_.NextWord();
    if (nextWord_.empty()) {
        return std::nullopt;
    }

    if (nextWord_ == KeepOptionsConstants::KEEP_OPTION || nextWord_ == KeepOptionsConstants::KEEP_CLASS_WITH_MEMBERS) {
        specification = ParseClassSpecificationArguments(true);
    } else if (nextWord_ == KeepOptionsConstants::KEEP_CLASS_MEMBERS) {
        specification = ParseClassSpecificationArguments(false);
    } else {
        GUARD_ABORT(ErrorCode::UNKNOWN_KEEP_OPTION, "KeepOption parsing failed: unknown keep option:" + nextWord_);
    }
    return specification;
}

/*
 * Parse keep option string and return the object ClassSpecification
 * class_specification format:
 * [@annotationName] [[!]final|abstract|declare...] [!]interface|class|enum|type|namespace|package className
 * [extends|implements] [@annotationName] className { member_specification }
 */
ark::guard::ClassSpecification ark::guard::KeepOptionParser::ParseClassSpecificationArguments(bool keepClassName)
{
    nextWord_ = wordReader_.NextWord();  // skip keep option word

    std::string annotationName;
    uint32_t setAccessFlags = 0;
    uint32_t unSetAccessFlags = 0;
    GetClassAnnotationNameAndAccessFlags(annotationName, setAccessFlags, unSetAccessFlags);

    uint32_t setTypeDeclarations = 0;
    uint32_t unSetTypeDeclarations = 0;
    GetClassTypeDeclarations(setTypeDeclarations, unSetTypeDeclarations);

    // parse className part
    std::string className = ParseCommaSeparatedList(true);
    if (className == ConfigurationConstants::ANNOTATION_KEYWORD) {
        className = std::string(ConfigurationConstants::ANNOTATION_KEYWORD) + ParseCommaSeparatedList(false);
    }
    className = StringUtil::ConvertWildcardToRegex(StringUtil::ReplaceSlashWithDot(className));

    // parse extension part
    uint32_t extensionType = 0;
    std::string extensionAnnotationName;
    std::string extensionClassName;
    GetExtensionInfo(extensionType, extensionAnnotationName, extensionClassName);

    ClassSpecification specification(annotationName, setAccessFlags, unSetAccessFlags);
    specification.SetClassInfo(setTypeDeclarations, unSetTypeDeclarations, className);
    specification.SetKeepClassName(keepClassName);
    specification.SetExtensionInfo(extensionType, extensionAnnotationName, extensionClassName);

    // parse member specification part
    if (!ConfigurationEnd(nextWord_)) {
        GUARD_ASSERT(nextWord_ != ConfigurationConstants::OPEN_KEYWORD, ErrorCode::CLASS_SPECIFICATION_FORMAT_ERROR,
                         "ClassSpecification parsing failed: the expect character is " +
                             std::string(ConfigurationConstants::OPEN_KEYWORD) + " but actual character is " +
                             nextWord_);

        while (!nextWord_.empty()) {
            nextWord_ = wordReader_.NextWord();
            if (nextWord_ == ConfigurationConstants::CLOSE_KEYWORD) {
                break;
            }
            ParseMemberSpecificationArguments(specification);
        }
    }
    return specification;
}

/*
 * Parse class_specification and return annotationName and accessFlags.
 * Format:[@annotationName] [[!]final|abstract|declare...] [!]interface|class|enum|type|namespace|package
 */
void ark::guard::KeepOptionParser::GetClassAnnotationNameAndAccessFlags(std::string &annotationName,
                                                                        uint32_t &setAccessFlags,
                                                                        uint32_t &unSetAccessFlags)
{
    while (!IsTypeDeclarations(nextWord_) && !ConfigurationEnd(nextWord_, true)) {
        bool negated = StringUtil::IsSubStrMatched(nextWord_, ConfigurationConstants::NEGATOR_KEYWORD.data());
        std::string strippedWord =
            negated ? nextWord_.substr(ConfigurationConstants::NEGATOR_KEYWORD.length()) : nextWord_;

        uint32_t accessFlag = GetClassAccessFlags(strippedWord);
        GUARD_ASSERT(accessFlag == INVALID_MODIFIERS, ErrorCode::CLASS_SPECIFICATION_FORMAT_ERROR,
                         "ClassSpecification parsing failed: unknown access flag:" + strippedWord);

        if (accessFlag == abckit_wrapper::AccessFlags::ANNOTATION) {
            nextWord_ = wordReader_.NextWord();
            if (!IsTypeDeclarations(nextWord_)) {
                annotationName = StringUtil::ConvertWildcardToRegex(ParseCommaSeparatedList(false));
                continue;
            }
        }

        if (!negated) {
            setAccessFlags |= accessFlag;
        } else {
            unSetAccessFlags |= accessFlag;
        }
        CheckAccessFlags(setAccessFlags, unSetAccessFlags, strippedWord);
        if (IsTypeDeclarations(strippedWord)) {
            break;
        }
        if (accessFlag != abckit_wrapper::AccessFlags::ANNOTATION) {
            nextWord_ = wordReader_.NextWord();
        }
    }
}

/*
 * Parse class_specification and return class type declarations.
 * Format:[!]interface|class|enum|type|namespace|package
 */
void ark::guard::KeepOptionParser::GetClassTypeDeclarations(uint32_t &setTypeDeclarations,
                                                            uint32_t &unSetTypeDeclarations) const
{
    bool negated = StringUtil::IsSubStrMatched(nextWord_, ConfigurationConstants::NEGATOR_KEYWORD.data());
    std::string strippedWord = negated ? nextWord_.substr(ConfigurationConstants::NEGATOR_KEYWORD.length()) : nextWord_;

    uint32_t typeDeclaration = GetTypeDeclarations(strippedWord);
    GUARD_ASSERT(typeDeclaration == INVALID_MODIFIERS, ErrorCode::CLASS_SPECIFICATION_FORMAT_ERROR,
                     "ClassSpecification parsing failed: unknown type declarations:" + strippedWord);

    if (!negated) {
        setTypeDeclarations |= typeDeclaration;
    } else {
        unSetTypeDeclarations |= typeDeclaration;
    }
    CheckTypeDeclarations(setTypeDeclarations, unSetTypeDeclarations, strippedWord);
}

/*
 * Parse class_specification and return extension info
 * Format:[extends|implements] [@annotationName] className
 */
void ark::guard::KeepOptionParser::GetExtensionInfo(uint32_t &extensionType, std::string &extensionAnnotationName,
                                                    std::string &extensionClassName)
{
    if (ConfigurationEnd(nextWord_)) {
        return;
    }

    extensionType = GetExtensionType(nextWord_);
    if (extensionType == INVALID_MODIFIERS) {
        return;
    }

    nextWord_ = wordReader_.NextWord();
    if (nextWord_ == ConfigurationConstants::ANNOTATION_KEYWORD) {
        extensionAnnotationName = ParseCommaSeparatedList(true);
    }
    std::string className = ParseCommaSeparatedList(false);
    if (className == ConfigurationConstants::OPEN_KEYWORD) {
        // scene: [extends|implements] @className {
        className = std::string(ConfigurationConstants::ANNOTATION_KEYWORD) + extensionAnnotationName;
        extensionAnnotationName = "";
    } else if (className == ConfigurationConstants::ANNOTATION_KEYWORD) {
        // scene: [extends|implements] @annotationName @className {
        std::string name = ParseCommaSeparatedList(false);
        className = std::string(ConfigurationConstants::ANNOTATION_KEYWORD) + name;
    }
    extensionClassName = StringUtil::ConvertWildcardToRegex(StringUtil::ReplaceSlashWithDot(className));
}

/*
 * parse class_specification member section
 * {
 *      [@annotationName]
 *      [[!]public|private|protected|internal|static|final ...]
 *      <fields> | fieldName [:fieldType] [=fieldValue];
 *
 *      [@annotationName]
 *      [[!]public|private|protected|internal|static|native|final|abstract|async ...]
 *      <methods> | methodName(argumentType ...):returnType;
 * }
 */
void ark::guard::KeepOptionParser::ParseMemberSpecificationArguments(ClassSpecification &specification)
{
    std::string annotationName;
    uint32_t setAccessFlags = 0;
    uint32_t unSetAccessFlags = 0;
    GetMemberAnnotationNameAndAccessFlags(annotationName, setAccessFlags, unSetAccessFlags);

    // parse memberName part
    std::string memberName = StringUtil::ConvertWildcardToRegex(nextWord_);
    if (HandleWildcardMemberSpecification(annotationName, setAccessFlags, unSetAccessFlags, specification)) {
        return;
    }

    if (HandleFieldSpecification(annotationName, setAccessFlags, unSetAccessFlags, memberName, specification)) {
        return;
    }

    if (HandleMethodSpecification(annotationName, setAccessFlags, unSetAccessFlags, memberName, specification)) {
        return;
    }

    GUARD_ABORT(ErrorCode::CLASS_SPECIFICATION_FORMAT_ERROR,
                    "ClassSpecification parsing failed: the format of member part of class_specification is incorrect");
}

/*
 * parse class_specification member with wildcard
 * Format: [@annotationName] [[!]public|private|protected|internal|static|native|final|abstract|async|override ...]
 */
void ark::guard::KeepOptionParser::GetMemberAnnotationNameAndAccessFlags(std::string &annotationName,
                                                                         uint32_t &setAccessFlags,
                                                                         uint32_t &unSetAccessFlags)
{
    while (!ConfigurationEnd(nextWord_, true)) {
        if (nextWord_ == ConfigurationConstants::ANNOTATION_KEYWORD) {
            annotationName = StringUtil::ConvertWildcardToRegex(ParseCommaSeparatedList(true));
            continue;
        }

        bool negated = StringUtil::IsSubStrMatched(nextWord_, ConfigurationConstants::NEGATOR_KEYWORD.data());
        std::string strippedWord =
            negated ? nextWord_.substr(ConfigurationConstants::NEGATOR_KEYWORD.length()) : nextWord_;

        uint32_t accessFlag = GetMemberAccessFlags(strippedWord);
        if (accessFlag == INVALID_MODIFIERS) {
            break;
        }

        if (!negated) {
            setAccessFlags |= accessFlag;
        } else {
            unSetAccessFlags |= accessFlag;
        }
        CheckAccessFlags(setAccessFlags, unSetAccessFlags, strippedWord);

        nextWord_ = wordReader_.NextWord();
    }
}

/*
 * parse class_specification member with wildcard
 * Format: * | <field> | <method>
 */
bool ark::guard::KeepOptionParser::HandleWildcardMemberSpecification(const std::string &annotationName,
                                                                     uint32_t setAccessFlags, uint32_t unSetAccessFlags,
                                                                     ClassSpecification &specification)
{
    bool isStar = nextWord_ == ConfigurationConstants::ANY_CLASS_MEMBER_KEYWORD;
    bool isFields = nextWord_ == ConfigurationConstants::ANY_FIELD_KEYWORD;
    bool isMethods = nextWord_ == ConfigurationConstants::ANY_METHOD_KEYWORD;
    bool isFieldsOrMethods = isFields || isMethods;

    // read the member name
    nextWord_ = wordReader_.NextWord();
    // check is a wildcard start(for all members) or is a type wildcard
    bool isReallyStar = isStar && nextWord_ == ConfigurationConstants::SEMICOLON_KEYWORD;
    if (!isFieldsOrMethods && !isReallyStar) {
        return false;
    }

    if (isStar) {
        CheckFieldAccessFlags(setAccessFlags, unSetAccessFlags, nextWord_);
        auto fieldSpecification = MemberSpecification(annotationName, setAccessFlags, unSetAccessFlags);
        fieldSpecification.SetKeepMember(true);
        specification.AddField(std::move(fieldSpecification));

        CheckMethodAccessFlags(setAccessFlags, unSetAccessFlags, nextWord_);
        auto methodSpecification = MemberSpecification(annotationName, setAccessFlags, unSetAccessFlags);
        methodSpecification.SetKeepMember(true);
        specification.AddMethod(std::move(methodSpecification));
    } else if (isFields) {
        CheckFieldAccessFlags(setAccessFlags, unSetAccessFlags, nextWord_);
        auto fieldSpecification = MemberSpecification(annotationName, setAccessFlags, unSetAccessFlags);
        fieldSpecification.SetKeepMember(true);
        specification.AddField(std::move(fieldSpecification));
    } else if (isMethods) {
        CheckMethodAccessFlags(setAccessFlags, unSetAccessFlags, nextWord_);
        auto methodSpecification = MemberSpecification(annotationName, setAccessFlags, unSetAccessFlags);
        methodSpecification.SetKeepMember(true);
        specification.AddMethod(std::move(methodSpecification));
    }

    // Ensure the separator keyword is present
    CheckSemicolonKeyword(nextWord_);
    return true;
}

/*
 * parse class_specification member field section
 * Format: fieldName [:fieldType] [=fieldValue];
 */
bool ark::guard::KeepOptionParser::HandleFieldSpecification(const std::string &annotationName, uint32_t setAccessFlags,
                                                            uint32_t unSetAccessFlags, const std::string &memberName,
                                                            ClassSpecification &specification)
{
    if (nextWord_ == ConfigurationConstants::SEMICOLON_KEYWORD) {
        CheckFieldAccessFlags(setAccessFlags, unSetAccessFlags, memberName);
        auto fieldSpecification = MemberSpecification(annotationName, setAccessFlags, unSetAccessFlags);
        fieldSpecification.SetMemberInfo(memberName, "", "");
        specification.AddField(std::move(fieldSpecification));
        return true;
    }

    if (nextWord_ != ConfigurationConstants::COLON_KEYWORD && nextWord_ != ConfigurationConstants::EQUAL_KEYWORD) {
        return false;
    }

    std::string memberType;
    if (nextWord_ == ConfigurationConstants::COLON_KEYWORD) {
        nextWord_ = wordReader_.NextWord();
        std::unordered_set<std::string_view> delimiters = {ConfigurationConstants::SEMICOLON_KEYWORD,
                                                           ConfigurationConstants::EQUAL_KEYWORD};
        memberType = GetMemberType(delimiters);
    }

    std::string memberValue;
    if (nextWord_ == ConfigurationConstants::EQUAL_KEYWORD) {
        nextWord_ = wordReader_.NextWord();
        memberValue = nextWord_;
        nextWord_ = wordReader_.NextWord();
    }

    CheckSemicolonKeyword(nextWord_);

    CheckFieldAccessFlags(setAccessFlags, unSetAccessFlags, memberName);
    auto fieldSpecification = MemberSpecification(annotationName, setAccessFlags, unSetAccessFlags);
    fieldSpecification.SetMemberInfo(memberName, memberType, memberValue);
    specification.AddField(std::move(fieldSpecification));
    return true;
}

/*
 * parse class_specification member method section
 * Format: methodName(argumentType ...):returnType;
 */
bool ark::guard::KeepOptionParser::HandleMethodSpecification(const std::string &annotationName, uint32_t setAccessFlags,
                                                             uint32_t unSetAccessFlags, const std::string &memberName,
                                                             ClassSpecification &specification)
{
    if (nextWord_ != ConfigurationConstants::OPEN_ARGUMENTS_KEYWORD) {
        return false;
    }

    std::string arguments = GetMethodArguments();

    nextWord_ = wordReader_.NextWord();
    GUARD_ASSERT(nextWord_ != ConfigurationConstants::COLON_KEYWORD, ErrorCode::CLASS_SPECIFICATION_FORMAT_ERROR,
                     "ClassSpecification parsing failed: the expect character is " +
                         std::string(ConfigurationConstants::COLON_KEYWORD) + " but actual character is " + nextWord_);

    nextWord_ = wordReader_.NextWord();  // skip COLON_KEYWORD
    std::unordered_set<std::string_view> delimiters = {ConfigurationConstants::SEMICOLON_KEYWORD};
    std::string returnType = GetMemberType(delimiters);

    CheckSemicolonKeyword(nextWord_);

    CheckMethodAccessFlags(setAccessFlags, unSetAccessFlags, memberName);
    auto methodSpecification = MemberSpecification(annotationName, setAccessFlags, unSetAccessFlags);
    methodSpecification.SetMemberInfo(memberName, arguments, returnType);
    specification.AddMethod(std::move(methodSpecification));
    return true;
}

/*
 * parse a comma separated list of strings
 */
std::string ark::guard::KeepOptionParser::ParseCommaSeparatedList(bool skipCurrentWord)
{
    if (skipCurrentWord) {
        nextWord_ = wordReader_.NextWord();
    }

    std::string result;
    while (!nextWord_.empty()) {
        if (!result.empty()) {
            result.append(ConfigurationConstants::COMMA_KEYWORD);
        }
        result.append(nextWord_);
        if (nextWord_ == ConfigurationConstants::OPEN_KEYWORD) {
            break;
        }
        nextWord_ = wordReader_.NextWord();
        if (nextWord_ != ConfigurationConstants::COMMA_KEYWORD) {
            return result;
        }
        nextWord_ = wordReader_.NextWord();
    }
    return result;
}

std::string ark::guard::KeepOptionParser::GetMethodArguments()
{
    size_t branchCnt = 0;
    if (nextWord_ == ConfigurationConstants::OPEN_ARGUMENTS_KEYWORD) {
        branchCnt++;
        nextWord_ = wordReader_.NextWord();
    }
    std::string arguments;
    while (!nextWord_.empty()) {
        if (nextWord_ == ConfigurationConstants::OPEN_ARGUMENTS_KEYWORD) {
            branchCnt++;
        } else if (nextWord_ == ConfigurationConstants::CLOSE_ARGUMENTS_KEYWORD) {
            branchCnt--;
        }
        if (branchCnt == 0) {
            break;
        }
        arguments += nextWord_;
        nextWord_ = wordReader_.NextWord();
    }
    auto noSpaceArguments = StringUtil::RemoveAllSpaces(arguments);
    return StringUtil::ConvertWildcardToRegexEx(noSpaceArguments);
}

std::string ark::guard::KeepOptionParser::GetMemberType(const std::unordered_set<std::string_view> &delimiters)
{
    std::string types;
    while (!nextWord_.empty()) {
        if (delimiters.count(nextWord_)) {
            break;
        }
        types += nextWord_;
        nextWord_ = wordReader_.NextWord();
    }
    auto noSpaceTypes = StringUtil::RemoveAllSpaces(types);
    return StringUtil::ConvertWildcardToRegexEx(noSpaceTypes);
}