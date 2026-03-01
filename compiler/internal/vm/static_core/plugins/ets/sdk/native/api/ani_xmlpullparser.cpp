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

#include <securec.h>
#include <unordered_map>

#include "stdlib/native/core/stdlib_ani_helpers.h"
#include "tools/format_logger.h"
#include "Util.h"

#include "ani_xmlpullparser.h"

namespace ark::ets::sdk::util {
class TagText {
public:
    static TagText &GetInstance()
    {
        static TagText tagTextInstance;
        return tagTextInstance;
    };
    const std::string startCdata_ {"<![CDATA["};
    const std::string endCdata_ {"]]>"};
    const std::string startComment_ {"<!--"};
    const std::string endComment_ {"-->"};
    const std::string commentDoubleDash_ {"--"};
    const std::string endProcessingInstruction_ {"?>"};
    const std::string startDoctype_ {"<!DOCTYPE"};
    const std::string system_ {"SYSTEM"};
    const std::string public_ {"PUBLIC"};
    const std::string doubleQuote_ {"\""};
    const std::string singleQuote_ {"\\"};
    const std::string startElement_ {"<!ELEMENT"};
    const std::string empty_ {"EMPTY"};
    const std::string any_ {"ANY"};
    const std::string startAttlist_ {"<!ATTLIST"};
    const std::string notation_ {"NOTATION"};
    const std::string required_ {"REQUIRED"};
    const std::string implied_ {"IMPLIED"};
    const std::string fixed_ {"FIXED"};
    const std::string startEntity_ {"<!ENTITY"};
    const std::string ndata_ {"NDATA"};
    const std::string startNotation_ {"<!NOTATION"};
    const std::string illegalType_ {"Wrong event type"};
    const std::string startProcessingInstruction_ {"<?"};
    const std::string xml_ {"xml "};
};
constexpr size_t MAX_NAMESPACE_SIZE = 16UL;
constexpr int MAX_ASCII_CODE = 127;
constexpr int HEX_RADIX = 16;

std::map<std::string, std::string> &GetDefaultEntities()
{
    static std::map<std::string, std::string> defaultEntities = {
        {"lt;", "<"}, {"gt;", ">"}, {"amp;", "&"}, {"apos;", "'"}, {"quot;", "\""}};
    return defaultEntities;
}

XmlPullParser::ParseInfoClassCache::ParseInfoClassCache(ani_env *env) : env_(env)
{
    ani_class result {};
    ANI_FATAL_IF_ERROR(env->FindClass("@ohos.xml.xml.ParseInfoImpl", &result));
    ANI_FATAL_IF_ERROR(
        env->GlobalReference_Create(static_cast<ani_ref>(result), reinterpret_cast<ani_ref *>(&cachedClass_)));
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(cachedClass_, "<ctor>", nullptr, &constructor_));
    InitFieldCache();
}

XmlPullParser::ParseInfoClassCache::~ParseInfoClassCache() {}

ani_class XmlPullParser::ParseInfoClassCache::GetCachedClass_()
{
    return cachedClass_;
}

ani_object XmlPullParser::ParseInfoClassCache::New() const
{
    ani_object object {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_FATAL_IF_ERROR(env_->Object_New(cachedClass_, constructor_, &object));
    return object;
}

void XmlPullParser::ParseInfoClassCache::InitFieldCache()
{
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CACHE_LIST_TO_MAP(cache, _func, _item, _type, _typeName) \
    /* CC-OFFNXT(G.PRE.09) function defination */                \
    ANI_FATAL_IF_ERROR(env_->Class_FindField(cachedClass_, #cache, &cache##_));
    PARSE_INFO_FIELD_LIST(CACHE_LIST_TO_MAP)
#undef CACHE_LIST_TO_MAP
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PARSE_INFO_SETTER(cache, func, item, type, typeName)                                      \
    ani_status XmlPullParser::ParseInfoClassCache::Set##func(ani_object object, type value) const \
    {                                                                                             \
        /* CC-OFFNXT(G.PRE.05) function defination */                                             \
        return env_->Object_SetField_##typeName(object, cache##_, value);                         \
    }
PARSE_INFO_FIELD_LIST(PARSE_INFO_SETTER)
#undef PARSE_INFO_SETTER

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_NON_NULL_PARSER_AND_RETURN(pointer, returnVal)   \
    if ((pointer) == 0) {                                      \
        LOG_FATAL_SDK("%s:: object is nullptr", __FUNCTION__); \
        /* CC-OFFNXT(G.PRE.05) return if invalid argument */   \
        return (returnVal);                                    \
    }                                                          \
    /* CC-OFFNXT(G.PRE.02) assignment statement */             \
    XmlPullParser *that = reinterpret_cast<XmlPullParser *>(pointer)

ani_long XmlPullParser::BindNative(ani_env *env, [[maybe_unused]] ani_class self, ani_string content)
{
    ani_size strLen = 0;
    env->String_GetUTF8Size(content, &strLen);
    std::string strContent;
    strContent.resize(strLen + 1);
    env->String_GetUTF8(content, strContent.data(), strContent.size(), &strLen);
    auto *binding = new XmlPullParser(env, strContent.substr(0, strLen));
    if (binding == nullptr) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_FATAL_SDK("%s:: memory allocate failed, binding is nullptr", __FUNCTION__);
    }
    return reinterpret_cast<ani_long>(binding);
}

ani_boolean XmlPullParser::ReleaseNative([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_class self,
                                         ani_long pointer)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    CHECK_NON_NULL_PARSER_AND_RETURN(pointer, false);
    ANI_FATAL_IF_ERROR(env->GlobalReference_Delete(static_cast<ani_ref>(that->enumTypeClass_)));
    ANI_FATAL_IF_ERROR(env->GlobalReference_Delete(static_cast<ani_ref>(that->infoClass_.GetCachedClass_())));
    delete that;
    return static_cast<ani_boolean>(true);
}

XmlPullParser::XmlPullParser(ani_env *env, std::string strXml) : env_(env), infoClass_(env), strXml_(std::move(strXml))
{
    ani_enum eTypeCache {};
    ANI_FATAL_IF_ERROR(env->FindEnum("@ohos.xml.xml.EventType", &eTypeCache));
    ANI_FATAL_IF_ERROR(
        env->GlobalReference_Create(static_cast<ani_ref>(eTypeCache), reinterpret_cast<ani_ref *>(&enumTypeClass_)));
}

XmlPullParser::~XmlPullParser() {}

// CC-OFFNXT(G.FUN.01-CPP) options of parse
ani_boolean XmlPullParser::ParseXml(ani_env *env, [[maybe_unused]] ani_class self, ani_long pointer,
                                    ani_boolean ignoreNS, ani_boolean supportDCT, ani_fn_object attrValCb,
                                    ani_fn_object tagValCb, ani_fn_object tknCalCb)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    CHECK_NON_NULL_PARSER_AND_RETURN(pointer, false);
    that->bIgnoreNS_ = static_cast<bool>(ignoreNS);
    that->bDoctype_ = static_cast<bool>(supportDCT);

    that->attrFunc_ = attrValCb;
    that->tagFunc_ = tagValCb;
    that->tokenFunc_ = tknCalCb;
    that->Parse(env);
    return static_cast<ani_boolean>(that->xmlPullParserError_.empty());
}

ani_ref XmlPullParser::GetParseError(ani_env *env, [[maybe_unused]] ani_class self, ani_long pointer)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    CHECK_NON_NULL_PARSER_AND_RETURN(pointer, GetAniNull(env));
    ani_string result {};
    env->String_NewUTF8(that->xmlPullParserError_.c_str(), that->xmlPullParserError_.size(), &result);
    return result;
}

bool XmlPullParser::DealLength(size_t minimum)
{
    for (size_t i = 0; i < position_; i++) {
        if (strXml_[i] == '\n') {
            bufferStartLine_++;
            bufferStartColumn_ = 0;
        } else {
            bufferStartColumn_++;
        }
    }
    if (!keyInfo_.empty()) {
        keyInfo_.append(strXml_, 0, position_);
    }

    if (max_ > position_) {
        max_ -= position_;
        for (size_t j = 0; j < max_; ++j) {
            strXml_[j] = strXml_[position_ + j];
        }
    } else {
        max_ = 0;
    }
    if (position_ != strXml_.size()) {
        position_ = 0;
    }
    if (strXml_.size() - max_ > 0 && position_ == 0) {
        max_ += strXml_.size() - max_;
        if (max_ >= minimum) {
            return true;
        }
    }
    return false;
}

size_t XmlPullParser::GetNSCount(size_t iTemp)
{
    if (iTemp > depth_) {
        xmlPullParserError_ = " IndexOutOfBoundsException";
    }
    return nspCounts_[depth_];
}

std::string XmlPullParser::XmlPullParserError() const
{
    return xmlPullParserError_;
}

bool XmlPullParser::ParseTag(ani_env *env) const
{
    ani_string key = CreateStringUtf8(env, name_);
    ani_string value = CreateStringUtf8(env, text_);
    std::vector<ani_ref> argv {key, value};
    ani_ref returnVal {};
    env->FunctionalObject_Call(tagFunc_, argv.size(), argv.data(), &returnVal);
    return static_cast<bool>(ToBoolean(env, static_cast<ani_object>(returnVal)));
}

bool XmlPullParser::ParseToken(ani_env *env) const
{
    ASSERT(type_ >= TagEnum::START_DOCUMENT || type_ <= TagEnum::WHITESPACE);
    ani_object info = infoClass_.New();

    infoClass_.SetDepth(info, static_cast<ani_int>(depth_));
    infoClass_.SetColumnNumber(info, static_cast<ani_int>(GetColumnNumber()));
    infoClass_.SetLineNumber(info, static_cast<ani_int>(GetLineNumber()));
    infoClass_.SetAttributeCount(info, static_cast<ani_int>(attrCount_));

    infoClass_.SetName(info, CreateStringUtf8(env, name_));
    infoClass_.SetNameSpace(info, CreateStringUtf8(env, namespace_));
    infoClass_.SetPrefix(info, CreateStringUtf8(env, prefix_));
    infoClass_.SetText(info, CreateStringUtf8(env, GetText()));

    infoClass_.SetEmptyElementTag(info, static_cast<ani_boolean>(bEndFlag_));
    infoClass_.SetWhitespace(info, static_cast<ani_boolean>(bWhitespace_));

    ani_enum_item eTypeItem {};
    env->Enum_GetEnumItemByIndex(enumTypeClass_, static_cast<ani_size>(type_), &eTypeItem);
    std::vector<ani_ref> argv {eTypeItem, info};
    ani_ref returnVal {};
    env->FunctionalObject_Call(tokenFunc_, argv.size(), argv.data(), &returnVal);
    return static_cast<bool>(ToBoolean(env, static_cast<ani_object>(returnVal)));
}

bool XmlPullParser::ParseAttr(ani_env *env) const
{
    for (size_t i = 0; i < attrCount_; ++i) {
        ani_string key = CreateStringUtf8(env, attributes_[i * 4 + 2]);    // 4 and 2: number of args
        ani_string value = CreateStringUtf8(env, attributes_[i * 4 + 3]);  // 4 and 3: number of args
        std::vector<ani_ref> argv {key, value};
        ani_ref returnVal {};
        auto status = env->FunctionalObject_Call(attrFunc_, argv.size(), argv.data(), &returnVal);
        if (status != ANI_OK) {
            return false;
        }
        if (!static_cast<bool>(ToBoolean(env, static_cast<ani_object>(returnVal)))) {
            return false;
        }
    }
    return true;
}

bool XmlPullParser::HandleTagFunc(ani_env *env)
{
    if (IsNullishValue(env, static_cast<ani_ref>(tagFunc_))) {
        return true;
    }
    bool bRec = ParseTag(env);
    return bRec || type_ != TagEnum::START_TAG;
}

bool XmlPullParser::HandleTokenFunc(ani_env *env)
{
    if (IsNullishValue(env, static_cast<ani_ref>(tokenFunc_))) {
        return true;
    }
    return ParseToken(env);
}

/**
 * This function will be deprecated in the future due to a bug.
 * After parsing the data, since the variable attrCount_ was reset to 0,
 * The program cannot stop normally because it cannot enter the 'return false' branch.
 */
bool XmlPullParser::HandleAttrFunc(ani_env *env)
{
    if (IsNullishValue(env, static_cast<ani_ref>(attrFunc_)) || attrCount_ == 0U) {
        return true;
    }
    bool bRec = ParseAttr(env);
    attrCount_ = 0;
    return bRec;
}

void XmlPullParser::Parse(ani_env *env)
{
    bool tagFuncIsNull = IsNullishValue(env, static_cast<ani_ref>(tagFunc_));
    bool attrFuncIsNull = IsNullishValue(env, static_cast<ani_ref>(attrFunc_));
    bool tokenFuncIsNull = IsNullishValue(env, static_cast<ani_ref>(tokenFunc_));
    if (tagFuncIsNull && attrFuncIsNull && tokenFuncIsNull) {
        return;
    }
    while (type_ != TagEnum::END_DOCUMENT) {
        if (ParseOneTag() == TagEnum::ERROR) {
            break;
        }
        if (!tagFuncIsNull && !HandleTagFunc(env)) {
            break;
        }
        if (!tokenFuncIsNull && !HandleTokenFunc(env)) {
            break;
        }
        if (!attrFuncIsNull && !HandleAttrFunc(env)) {
            break;
        }
    }
}

TagEnum XmlPullParser::DealExclamationGroup()
{
    switch (strXml_[position_ + 2]) {  // 2:  number of args
        case 'D':
            return TagEnum::DOCDECL;
        case '[':
            return TagEnum::CDSECT;
        case '-':
            return TagEnum::COMMENT;
        case 'E':
            switch (strXml_[position_ + 3]) {  // 3:  number of args
                case 'L':
                    return TagEnum::ELEMENTDECL;
                case 'N':
                    return TagEnum::ENTITYDECL;
                default:
                    break;
            }
            xmlPullParserError_ = "Unexpected <!";
            break;
        case 'A':
            return TagEnum::ATTLISTDECL;
        case 'N':
            return TagEnum::NOTATIONDECL;
        default:
            break;
    }
    return TagEnum::ERROR;
}

TagEnum XmlPullParser::DealLtGroup()
{
    if (position_ + 3 >= max_ && !DealLength(4)) {  // 4: number of args 3: number of args
        xmlPullParserError_ = ("Dangling <");
    }
    char cTemp = strXml_[position_ + 1];
    if (cTemp == '/') {
        return TagEnum::END_TAG;
    }
    if (cTemp == '?') {
        std::string strXml = strXml_.substr(position_ + 2, 4);  // 2 and 4:position and length
        MakeStrUpper(strXml);
        if (max_ >= position_ + 5 && strXml == TagText::GetInstance().xml_) {  // 5: number of args
            return TagEnum::XML_DECLARATION;
        }
        return TagEnum::INSTRUCTION;
    }
    if (cTemp == '!') {
        return DealExclamationGroup();
    }
    return TagEnum::START_TAG;
}

TagEnum XmlPullParser::ParseTagType(bool inDeclaration)
{
    if (bStartDoc_) {
        bStartDoc_ = false;
        return TagEnum::START_DOCUMENT;
    }
    if (position_ >= max_ && !DealLength(1)) {
        return TagEnum::END_DOCUMENT;
    }
    switch (strXml_[position_]) {
        case '&':
            return TagEnum::ENTITY_REFERENCE;
        case '<':
            return DealLtGroup();
        case '%':
            return inDeclaration ? TagEnum::PARAMETER_ENTITY_REF : TagEnum::TEXT;
        default:
            return TagEnum::TEXT;
    }
}

void XmlPullParser::MakeStrUpper(std::string &src) const
{
    size_t i = 0;

    while (i < src.size()) {
        if (src[i] >= 'A' && src[i] <= 'Z') {
            src[i] += ' ';
        }
        ++i;
    }
}

void XmlPullParser::SkipText(const std::string &chars)
{
    if (position_ + chars.size() > max_ && !DealLength(chars.size())) {
        xmlPullParserError_ = "expected: '" + chars + "' but was EOF";
        return;
    }
    size_t len = chars.length();
    if (strXml_.substr(position_, len) != chars) {
        xmlPullParserError_ = "expected: \"" + chars + "\" but was \"" + strXml_.substr(position_, len) + "...\"";
    }
    position_ += len;
}

int XmlPullParser::PriorDealChar()
{
    if (position_ < max_ || DealLength(1)) {
        return strXml_[position_];
    }
    return -1;
}

void XmlPullParser::SkipChar(char expected)
{
    int c = PriorDealChar();
    if (c != expected) {
        xmlPullParserError_ = "expected:";
        if (c == -1) {
            return;
        }
    }
    position_++;
}

std::string XmlPullParser::ParseNameInner(size_t start)
{
    std::string result;
    char c = 0;
    while (true) {
        if (position_ >= max_) {
            result.append(strXml_, start, position_ - start);
            if (!DealLength(1)) {
                return result;
            }
            start = position_;
        }
        c = strXml_[position_];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' ||
            c == ':' || c == '.') {
            position_++;
            continue;
        }
        result.append(strXml_, start, position_ - start);
        return result;
    }
}

std::string XmlPullParser::ParseName()
{
    if (position_ >= max_ && !DealLength(1)) {
        xmlPullParserError_ = "name expected";
        return "";
    }
    size_t start = position_;
    char c = strXml_[position_];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == ':') {
        position_++;
    } else {
        xmlPullParserError_ = "The node name contains invalid characters: ";
        xmlPullParserError_ += c;
        return "";
    }
    return ParseNameInner(start);
}

void XmlPullParser::SkipInvalidChar()
{
    while (position_ < max_ || DealLength(1)) {
        unsigned char temp = strXml_[position_];
        if (temp > ' ') {
            break;
        }
        position_++;
    }
}

void XmlPullParser::ParseEntityFunc(size_t start, std::string &out, bool isEntityToken, TextEnum textEnum)
{
    std::string strEntity = out.substr(start + 1, out.length() - 1);
    if (isEntityToken) {
        name_ = strEntity;
    }
    if (!strEntity.empty() && strEntity[0] == '#') {
        int c = 0;
        if (strEntity.size() >= 2 && strEntity[1] == 'x') {          // 2: number of args
            c = std::stoi(strEntity.substr(2), nullptr, HEX_RADIX);  // 16: number of args 2: number of args
        } else {
            c = std::stoi(strEntity.substr(1), nullptr);
        }
        out = "";
        out += static_cast<char>(c);
        bUnresolved_ = false;
        return;
    }
    if (textEnum == TextEnum::ENTITY_DECL) {
        return;
    }
    if (GetDefaultEntities().count(strEntity) != 0) {
        out = out.substr(0, start);
        bUnresolved_ = false;
        out.append(GetDefaultEntities()[strEntity]);
        return;
    }
    std::string resolved = " ";
    if (!documentEntities_.empty() && !(resolved = strEntity).empty()) {
        out = "";
        bUnresolved_ = false;
        {
            out.append(resolved);
        }
        return;
    }
    if (!sysInfo_.empty()) {
        out = "";
        return;
    }
    bUnresolved_ = true;
}

void XmlPullParser::ParseEntity(std::string &out, bool isEntityToken, TextEnum textEnum)
{
    size_t start = out.length();
    if (strXml_[position_++] != '&') {
        xmlPullParserError_ = "Should not be reached";
    }
    out += '&';
    while (true) {
        int c = PriorDealChar();
        if (c == ';') {
            out += ';';
            position_++;
            break;
        }
        if (c > MAX_ASCII_CODE || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            c == '_' || c == '-' || c == '#') {
            position_++;
            out.push_back(static_cast<char>(c));
        } else {
            xmlPullParserError_ = "unterminated entity ref";
            break;
        }
    }
    ParseEntityFunc(start, out, isEntityToken, textEnum);
}

bool XmlPullParser::ParseTagValueFunc(char &c, TextEnum textEnum, size_t &start, std::string &result)
{
    if (c == '\r') {
        if ((position_ + 1 < max_ || DealLength(2)) && strXml_[position_ + 1] == '\n') {  // 2: number of args
            position_++;
        }
        c = (textEnum == TextEnum::ATTRI) ? ' ' : '\n';
    } else if (c == '\n') {
        c = ' ';
    } else if (c == '&') {
        bWhitespace_ = false;
        ParseEntity(result, false, textEnum);
        start = position_;
        return false;
    } else if (c == '<') {
        if (textEnum == TextEnum::ATTRI) {
            xmlPullParserError_ = "Illegal: \"<\" inside attribute value";
        }
        bWhitespace_ = false;
    } else if (c == ']') {
        if ((position_ + 2 < max_ || DealLength(3)) &&                         // 2: number of args 3: number of args
            strXml_[position_ + 1] == ']' && strXml_[position_ + 2] == '>') {  // 2: number of args
            xmlPullParserError_ = "Illegal: \"]]>\" outside CDATA section";
        }
        bWhitespace_ = false;
    } else if (c == '%') {
        xmlPullParserError_ = "This parser doesn't support parameter entities";
    } else {
        xmlPullParserError_ = "AssertionError";
    }
    return true;
}

void XmlPullParser::DealWhiteSpace(unsigned char c)
{
    bWhitespace_ = bWhitespace_ && (c <= ' ');
}

size_t XmlPullParser::ParseTagValueInner(size_t &start, std::string &result, char delimiter, TextEnum textEnum)
{
    if (position_ >= max_) {
        if (start < position_) {
            result.append(strXml_, start, position_ - start);
        }
        if (!DealLength(1)) {
            result = (!result.empty() ? result : "");
            return 0;
        }
        start = position_;
    }
    unsigned char c = strXml_[position_];
    if (c == delimiter || (delimiter == ' ' && (c <= ' ' || c == '>'))) {
        return 1;
    }
    if (c != '\r' && (c != '\n' || textEnum != TextEnum::ATTRI) && c != '&' && c != '<' &&
        (c != ']' || textEnum != TextEnum::TEXT) && (c != '%' || textEnum != TextEnum::ENTITY_DECL)) {
        DealWhiteSpace(c);
        position_++;
        return 2;  // 2: break flag
    }
    result.append(strXml_, start, position_ - start);
    return c;
}

std::string XmlPullParser::ParseTagValue(char delimiter, TextEnum textEnum)
{
    size_t start = position_;
    std::string result;
    if (textEnum == TextEnum::TEXT && !text_.empty()) {
        result.append(text_);
    }
    while (true) {
        auto cRecv = static_cast<char>(ParseTagValueInner(start, result, delimiter, textEnum));
        if (cRecv == 0) {
            return result;
        }
        if (cRecv == 1) {
            break;
        }
        if (cRecv == 2) {  // 2: break flag
            continue;
        }
        if (!ParseTagValueFunc(cRecv, textEnum, start, result)) {
            continue;
        }
        ++position_;
        result += static_cast<char>(cRecv);
        start = position_;
    }
    result.append(strXml_, start, position_ - start);
    return result;
}

std::string XmlPullParser::GetNamespace(const std::string &prefix)
{
    size_t temp = GetNSCount(depth_) << 1UL;
    if (temp != 0UL) {
        size_t i = temp - 2;  // 2: number of args
        while (true) {
            if (prefix.empty() && nspStack_[i].empty()) {
                return nspStack_[i + 1];
            }
            if (prefix == nspStack_[i]) {
                return nspStack_[i + 1];
            }
            if (i == 0) {
                break;
            }
            i -= 2;  // 2: number of args
        }
    }
    return "";
}

void XmlPullParser::Replace(std::string &strTemp, const std::string &strSrc, const std::string &strDes) const
{
    size_t iPos = 0;
    while ((iPos = strTemp.find(strSrc)) != std::string::npos) {
        strTemp.replace(iPos, strSrc.size(), strDes);
    }
}

void XmlPullParser::ParseNspFunc(size_t &i, const std::string &attrName)
{
    size_t j = (nspCounts_[depth_]++) << 1UL;
    size_t uiSize = nspStack_.size();
    if (uiSize < j + 2) {  // 2: number of args
        nspStack_.resize(j + MAX_NAMESPACE_SIZE);
    }
    nspStack_[j] = attrName;
    nspStack_[j + 1] = attributes_[i + 3];                  // 3: number of args
    if (!attrName.empty() && attributes_[i + 3].empty()) {  // 3: number of args
        xmlPullParserError_ = "illegal empty namespace";
    }

    for (size_t iCount = i; iCount < ((--attrCount_) << 2UL); iCount++) {  // 2: number of args
        attributes_[iCount] = attributes_[iCount + 4];                     // 4: number of args
    }
    i -= 4;  // 4:
}

void XmlPullParser::ParseNspFunction()
{
    int i = (attrCount_ << 2UL) - 4UL;              // 4: number of args 2: number of args
    for (; i >= 0; i -= 4) {                        // 4: number of args
        std::string attrName = attributes_[i + 2];  // 2: number of args
        size_t cut = attrName.find(':');
        if (cut == 0) {
            xmlPullParserError_ = "illegal attribute name: ";
        } else if (cut != std::string::npos) {
            std::string attrPrefix = attrName.substr(0, cut);
            attrName = attrName.substr(cut + 1);
            std::string attrNs = GetNamespace(attrPrefix);
            if (attrNs.empty()) {
                xmlPullParserError_ = ("Undefined Prefix: " + attrPrefix + " in ");
            }
            attributes_[i] = attrNs;
            attributes_[i + 1] = attrPrefix;
            attributes_[i + 2] = attrName;  // 2: number of args
        }
    }
}

bool XmlPullParser::ParseNsp()
{
    bool any = false;
    size_t cut = 0;
    for (size_t i = 0; i < (attrCount_ << 2UL); i += 4) {  // 2 and 4: number of args
        std::string attrName = attributes_[i + 2];         // 2: number of args
        cut = attrName.find(':');
        std::string prefix;
        if (cut != std::string::npos) {
            prefix = attrName.substr(0, cut);
            attrName = attrName.substr(cut + 1);
        } else if (attrName == ("xmlns")) {
            prefix = attrName;
            attrName = "";
        } else {
            continue;
        }
        if (!(prefix == "xmlns")) {
            any = true;
        } else {
            // CC-OFFNXT(G.EXP.41) element ereased
            ParseNspFunc(i, attrName);
        }
    }
    if (any) {
        ParseNspFunction();
    }
    cut = name_.find(':');
    if (cut == 0) {
        xmlPullParserError_ = "illegal tag name: " + name_;
    }
    if (cut != std::string::npos) {
        prefix_ = name_.substr(0, cut);
        name_ = name_.substr(cut + 1);
    }
    namespace_ = GetNamespace(prefix_);
    return any;
}

bool XmlPullParser::ParseStartTagFuncDeal()
{
    std::string attrName = ParseName();
    if (attrName.empty()) {
        return false;
    }
    int i = (attrCount_++) * 4;                  // 4: number of args
    attributes_.resize(attributes_.size() + 4);  // 4: number of args
    attributes_[i] = "";
    attributes_[i + 1] = "";
    attributes_[i + 2] = attrName;  // 2: number of args
    SkipInvalidChar();
    if (position_ >= max_ && !DealLength(1)) {
        xmlPullParserError_ = "UNEXPECTED_EOF";
        return false;
    }
    if (strXml_[position_] == '=') {
        position_++;
        SkipInvalidChar();
        if (position_ >= max_) {
            xmlPullParserError_ = "UNEXPECTED_EOF";
            return false;
        }
        char delimiter = strXml_[position_];
        if (delimiter == '\'' || delimiter == '"') {
            position_++;
        } else {
            xmlPullParserError_ = "attr value delimiter missing!";
            return false;
        }
        attributes_[i + 3] = ParseTagValue(delimiter, TextEnum::ATTRI);  // 3: number of args
        if (delimiter != ' ' && PriorDealChar() == delimiter) {
            position_++;
        }
    } else {
        attributes_[i + 3] = attrName;  // 3: number of args
    }
    return true;
}

TagEnum XmlPullParser::ParseStartTagFunc(bool xmldecl)
{
    while (true) {
        SkipInvalidChar();
        if (position_ >= max_ && DealLength(1)) {
            xmlPullParserError_ = "UNEXPECTED_EOF";
            return TagEnum::ERROR;
        }
        unsigned char temp = strXml_[position_];
        if (xmldecl) {
            if (temp == '?') {
                position_++;
                SkipChar('>');
                return TagEnum::XML_DECLARATION;
            }
        } else {
            if (temp == '/') {
                bEndFlag_ = true;
                position_++;
                SkipInvalidChar();
                SkipChar('>');
                break;
            }
            if (temp == '>') {
                position_++;
                break;
            }
        }
        bool bRecv = ParseStartTagFuncDeal();
        if (!bRecv) {
            return TagEnum::ERROR;
        }
    }
    return TagEnum::OK;
}

TagEnum XmlPullParser::ParseStartTag(bool xmldecl)
{
    if (!xmldecl) {
        SkipChar('<');
    }
    name_ = ParseName();
    attrCount_ = 0;
    TagEnum bRecv = ParseStartTagFunc(xmldecl);
    if (bRecv != TagEnum::OK) {
        return bRecv;
    }
    size_t sp = depth_++ * 4;       // 4: number of args
    elementStack_.resize(sp + 4);   // 4: number of args
    elementStack_[sp + 3] = name_;  // 3: number of args
    if (depth_ >= nspCounts_.size()) {
        nspCounts_.resize(depth_ + 4);  // 4: number of args
    }
    nspCounts_[depth_] = nspCounts_[depth_ - 1];
    if (!bIgnoreNS_) {
        ParseNsp();
    } else {
        namespace_ = "";
    }
    elementStack_[sp] = namespace_;
    elementStack_[sp + 1] = prefix_;
    elementStack_[sp + 2] = name_;  // 2: number of args
    return TagEnum::OK;
}

void XmlPullParser::ParseDeclaration()
{
    if (bufferStartLine_ != 0 || bufferStartColumn_ != 0 || position_ != 0) {
        xmlPullParserError_ = "processing instructions must not start with xml";
    }
    SkipText(TagText::GetInstance().startProcessingInstruction_);
    ParseStartTag(true);
    if (attrCount_ < 1 || attributes_[2] != "version") {  // 2: number of args
        xmlPullParserError_ = "version expected";
    }

    size_t pos = 1;
    if (pos < attrCount_ && (attributes_[2 + 4]) == "encoding") {  // 4: number of args 2: number of args
        pos++;
    }
    if (pos < attrCount_ && (attributes_[4 * pos + 2]) == "standalone") {  // 4: number of args 2: number of args
        std::string st = attributes_[3 + 4 * pos];                         // 3: number of args 4: number of args
        if (st != "yes" && st != "no") {
            xmlPullParserError_ = "illegal standalone value: " + st;
        }
        pos++;
    }
    if (pos != attrCount_) {
        xmlPullParserError_ = "unexpected attributes in XML declaration";
    }
    bWhitespace_ = true;
    text_ = "";
}

bool XmlPullParser::ParseEndTag()
{
    SkipChar('<');
    SkipChar('/');
    name_ = ParseName();
    if (name_.empty()) {
        return false;
    }
    SkipInvalidChar();
    SkipChar('>');
    if (depth_ == 0) {
        xmlPullParserError_ = "read end tag " + name_ + " with no tags open";
        type_ = TagEnum::COMMENT;
        return true;
    }
    size_t sp = (depth_ - 1) * 4;          // 4: number of args
    if (name_ == elementStack_[sp + 3]) {  // 3: number of args
        namespace_ = elementStack_[sp];
        prefix_ = elementStack_[sp + 1];
        name_ = elementStack_[sp + 2];  // 2: number of args
    } else {
        xmlPullParserError_ = "expected: /" + elementStack_[sp + 3] + " read: " + name_;  // 3: number of args
    }
    return true;
}

std::string XmlPullParser::ParseDelimiterInfo(std::string delimiter, bool returnText)
{
    size_t start = position_;
    std::string result;
    if (returnText && !text_.empty()) {
        result.append(text_);
    }
    bool bFlag = true;
    size_t delimLen = delimiter.length();
    while (bFlag) {
        if (position_ + delimLen > max_) {
            xmlPullParserError_ = "Cannot find the '" + delimiter + "' in xml string.";
            return "";
        }
        size_t i = 0;
        for (; i < delimLen; i++) {
            if (strXml_[position_ + i] != delimiter[i]) {
                position_++;
                break;
            }
        }
        if (i == delimLen) {
            bFlag = false;
        }
    }
    size_t end = position_;
    position_ += delimLen;
    if (!returnText) {
        return "";
    }
    result.append(strXml_, start, end - start);
    return result;
}

std::string XmlPullParser::ParseDelimiter(bool returnText)
{
    int quote = PriorDealChar();
    std::string delimiter;
    if (quote == '"') {
        delimiter = TagText::GetInstance().doubleQuote_;
    } else if (quote == '\'') {
        delimiter = TagText::GetInstance().singleQuote_;
    } else {
        xmlPullParserError_ = "Expected a quoted std::string ";
    }
    position_++;
    return ParseDelimiterInfo(delimiter, returnText);
}

bool XmlPullParser::ParserDoctInnerInfo(bool requireSystemName, bool assignFields)
{
    SkipInvalidChar();
    int c = PriorDealChar();
    if (c == 'S') {
        SkipText(TagText::GetInstance().system_);
    } else if (c == 'P') {
        SkipText(TagText::GetInstance().public_);
        SkipInvalidChar();
        if (assignFields) {
            pubInfo_ = ParseDelimiter(true);
        } else {
            ParseDelimiter(false);
        }
    } else {
        return false;
    }
    SkipInvalidChar();
    if (!requireSystemName) {
        int delimiter = PriorDealChar();
        if (delimiter != '"' && delimiter != '\'') {
            return true;  // no system name!
        }
    }
    if (assignFields) {
        sysInfo_ = ParseDelimiter(true);
    } else {
        ParseDelimiter(false);
    }
    return true;
}

void XmlPullParser::ParseComment(bool returnText)
{
    SkipText(TagText::GetInstance().startComment_);
    std::string commentText = ParseDelimiterInfo(TagText::GetInstance().commentDoubleDash_, returnText);
    if (PriorDealChar() != '>') {
        xmlPullParserError_ = "Comments may not contain -- ";
    }
    position_++;
    if (returnText) {
        text_ = commentText;
    }
}

void XmlPullParser::ParseSpecText()
{
    SkipInvalidChar();
    int c = PriorDealChar();
    if (c == '(') {
        int iDepth = 0;
        do {
            if (c == '(') {
                iDepth++;
            } else if (c == ')') {
                iDepth--;
            } else if (c == -1) {
                xmlPullParserError_ = "Unterminated element content spec ";
            }
            position_++;
            c = PriorDealChar();
        } while (iDepth > 0);
        if (c == '*' || c == '?' || c == '+') {
            position_++;
        }
    } else if (c == TagText::GetInstance().empty_[0]) {
        SkipText(TagText::GetInstance().empty_);
    } else if (c == TagText::GetInstance().any_[0]) {
        SkipText(TagText::GetInstance().any_);
    } else {
        xmlPullParserError_ = "Expected element content spec ";
    }
}

void XmlPullParser::ParseInnerEleDec()
{
    SkipText(TagText::GetInstance().startElement_);
    SkipInvalidChar();
    ParseName();
    ParseSpecText();
    SkipInvalidChar();
    SkipChar('>');
}

void XmlPullParser::ParseInnerAttriDeclFunc(int &c)
{
    if (c == '(') {
        position_++;
        while (true) {
            SkipInvalidChar();
            ParseName();
            SkipInvalidChar();
            c = PriorDealChar();
            if (c == ')') {
                position_++;
                break;
            }
            if (c == '|') {
                position_++;
            } else {
                xmlPullParserError_ = "Malformed attribute type ";
            }
        }
    } else {
        ParseName();
    }
}

void XmlPullParser::ParseInnerAttriDecl()
{
    SkipText(TagText::GetInstance().startAttlist_);
    SkipInvalidChar();
    std::string elementName = ParseName();
    while (true) {
        SkipInvalidChar();
        int c = PriorDealChar();
        if (c == '>') {
            position_++;
            return;
        }
        std::string attributeName = ParseName();
        SkipInvalidChar();
        if (position_ + 1 >= max_ && !DealLength(2)) {  // 2: lengths
            xmlPullParserError_ = "Malformed attribute list ";
        }
        if (strXml_[position_] == TagText::GetInstance().notation_[0] &&
            strXml_[position_ + 1] == TagText::GetInstance().notation_[1]) {
            SkipText(TagText::GetInstance().notation_);
            SkipInvalidChar();
        }
        c = PriorDealChar();
        ParseInnerAttriDeclFunc(c);
        SkipInvalidChar();
        c = PriorDealChar();
        if (c == '#') {
            position_++;
            c = PriorDealChar();
            if (c == 'R') {
                SkipText(TagText::GetInstance().required_);
            } else if (c == 'I') {
                SkipText(TagText::GetInstance().implied_);
            } else if (c == 'F') {
                SkipText(TagText::GetInstance().fixed_);
            } else {
                xmlPullParserError_ = "Malformed attribute type";
            }
            SkipInvalidChar();
            c = PriorDealChar();
        }
        if (c == '"' || c == '\'') {
            position_++;
            std::string value = ParseTagValue(static_cast<char>(c), TextEnum::ATTRI);
            if (PriorDealChar() == c) {
                position_++;
            }
        }
    }
}

void XmlPullParser::ParseEntityDecl()
{
    SkipText(TagText::GetInstance().startEntity_);
    SkipInvalidChar();
    if (PriorDealChar() == '%') {
        position_++;
        SkipInvalidChar();
    }
    std::string name = ParseName();
    SkipInvalidChar();
    int quote = PriorDealChar();
    std::string entityValue;
    if (quote == '"' || quote == '\'') {
        position_++;
        entityValue = ParseTagValue(static_cast<char>(quote), TextEnum::ENTITY_DECL);
        if (PriorDealChar() == quote) {
            position_++;
        }
    } else if (ParserDoctInnerInfo(true, false)) {
        entityValue = "";
        SkipInvalidChar();
        if (PriorDealChar() == TagText::GetInstance().ndata_[0]) {
            SkipText(TagText::GetInstance().ndata_);
            SkipInvalidChar();
            ParseName();
        }
    } else {
        xmlPullParserError_ = "Expected entity value or external ID";
    }
    SkipInvalidChar();
    SkipChar('>');
}

void XmlPullParser::ParseInneNotaDecl()
{
    SkipText(TagText::GetInstance().startNotation_);
    SkipInvalidChar();
    ParseName();
    if (!ParserDoctInnerInfo(false, false)) {
        xmlPullParserError_ = "Expected external ID or public ID for notation ";
    }
    SkipInvalidChar();
    SkipChar('>');
}

void XmlPullParser::ReadInternalSubset()
{
    SkipChar('[');
    while (true) {
        SkipInvalidChar();
        if (PriorDealChar() == ']') {
            position_++;
            return;
        }
        TagEnum declarationType = ParseTagType(true);
        switch (declarationType) {
            case TagEnum::ELEMENTDECL:
                ParseInnerEleDec();
                break;
            case TagEnum::ATTLISTDECL:
                ParseInnerAttriDecl();
                break;
            case TagEnum::ENTITYDECL:
                ParseEntityDecl();
                break;
            case TagEnum::NOTATIONDECL:
                ParseInneNotaDecl();
                break;
            case TagEnum::INSTRUCTION:
                SkipText(TagText::GetInstance().startProcessingInstruction_);
                ParseDelimiterInfo(TagText::GetInstance().endProcessingInstruction_, false);
                break;
            case TagEnum::COMMENT:
                ParseComment(false);
                break;
            case TagEnum::PARAMETER_ENTITY_REF:
                xmlPullParserError_ = "Parameter entity references are not supported ";
                break;
            default:
                xmlPullParserError_ = "Unexpected token";
                break;
        }
    }
}

void XmlPullParser::ParseDoctype(bool saveDtdText)
{
    SkipText(TagText::GetInstance().startDoctype_);
    size_t startPosition = 0;
    if (saveDtdText) {
        keyInfo_ = "";
        startPosition = position_;
    }
    SkipInvalidChar();
    ParseName();
    ParserDoctInnerInfo(true, true);
    SkipInvalidChar();
    if (PriorDealChar() == '[') {
        ReadInternalSubset();
    }
    SkipInvalidChar();
    if (saveDtdText) {
        keyInfo_.append(strXml_, 0, position_);
        keyInfo_ = keyInfo_.substr(startPosition);
        text_ = keyInfo_;
        keyInfo_ = "";
    }
    SkipChar('>');
}

void XmlPullParser::ParseText()
{
    text_ = ParseTagValue('<', TextEnum::TEXT);
    std::string strTemp = text_;
    Replace(strTemp, "\r", "");
    Replace(strTemp, "\n", "");
    Replace(strTemp, " ", "");
    if ((depth_ == 0 && bWhitespace_) || strTemp.empty()) {
        type_ = TagEnum::WHITESPACE;
    }
}

void XmlPullParser::ParseCdect()
{
    SkipText(TagText::GetInstance().startCdata_);
    text_ = ParseDelimiterInfo(TagText::GetInstance().endCdata_, true);
}

TagEnum XmlPullParser::ParseOneTagFunc()
{
    switch (type_) {
        case TagEnum::START_DOCUMENT:
        case TagEnum::END_DOCUMENT:
            return type_;
        case TagEnum::START_TAG: {
            if (ParseStartTag(false) == TagEnum::ERROR) {
                return TagEnum::ERROR;
            }
            return type_;
        }
        case TagEnum::END_TAG: {
            if (ParseEndTag()) {
                return type_;
            }
            return TagEnum::ERROR;
        }
        case TagEnum::ENTITY_REFERENCE: {
            std::string entityTextBuilder;
            ParseEntity(entityTextBuilder, true, TextEnum::TEXT);
            text_ = entityTextBuilder;
            return TagEnum::OK;
        }
        case TagEnum::TEXT:
            ParseText();
            return TagEnum::OK;
        case TagEnum::CDSECT:
            ParseCdect();
            return TagEnum::OK;
        case TagEnum::COMMENT:
            ParseComment(true);
            return TagEnum::OK;
        case TagEnum::INSTRUCTION:
            ParseInstruction();
            return TagEnum::OK;
        case TagEnum::DOCDECL:
            ParseDoctype(true);
            return TagEnum::OK;
        default:
            xmlPullParserError_ = "Unexpected token";
            return TagEnum::ERROR;
    }
}

TagEnum XmlPullParser::ParseOneTag()
{
    if (type_ == TagEnum::END_TAG) {
        depth_--;
    }
    if (bEndFlag_) {
        bEndFlag_ = false;
        type_ = TagEnum::END_TAG;
        return type_;
    }
    ParserPriorDeal();
    while (true) {
        TagEnum typeTem = ParseOneTagFunc();
        if (typeTem != TagEnum::OK) {
            return typeTem;
        }
        if (depth_ == 0 && (type_ == TagEnum::ENTITY_REFERENCE || type_ == TagEnum::TEXT || type_ == TagEnum::CDSECT)) {
            std::string errMsg;
            if (!text_.empty()) {
                errMsg = ": " + text_;
            }
            xmlPullParserError_ = "Unexpected token" + errMsg;
        }
        if (type_ == TagEnum::DOCDECL && (!bDoctype_)) {
            ParseOneTag();
        }
        return type_;
    }
}

void XmlPullParser::ParserPriorDeal()
{
    type_ = ParseTagType(false);
    if (type_ == TagEnum::XML_DECLARATION) {
        ParseDeclaration();
        type_ = ParseTagType(false);
    }
    text_ = "";
    bWhitespace_ = true;
    prefix_ = "";
    name_ = "";
    namespace_ = "";
    attrCount_ = 0;
}

void XmlPullParser::ParseInstruction()
{
    SkipText(TagText::GetInstance().startProcessingInstruction_);
    text_ = ParseDelimiterInfo(TagText::GetInstance().endProcessingInstruction_, true);
}

int XmlPullParser::GetColumnNumber() const
{
    size_t result = 0;
    for (size_t i = 0; i < position_; i++) {
        if (strXml_[i] == '\n') {
            result = 0;
        } else {
            result++;
        }
    }
    return result + 1;
}

int XmlPullParser::GetLineNumber() const
{
    int result = bufferStartLine_;
    for (size_t i = 0; i < position_; i++) {
        if (strXml_[i] == '\n') {
            result++;
        }
    }
    return result + 1;
}

std::string XmlPullParser::GetText() const
{
    if (type_ < TagEnum::TEXT || (type_ == TagEnum::ENTITY_REFERENCE && bUnresolved_)) {
        return "";
    }
    return text_;
}
}  // namespace ark::ets::sdk::util
