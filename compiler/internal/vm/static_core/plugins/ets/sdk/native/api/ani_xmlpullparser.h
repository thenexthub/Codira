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

#ifndef ANI_XMLPULLPARSER_H
#define ANI_XMLPULLPARSER_H

#include <ani.h>
#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace ark::ets::sdk::util {

enum class TagEnum {
    XML_DECLARATION = -1,
    START_DOCUMENT,
    END_DOCUMENT,
    START_TAG,
    END_TAG,
    TEXT,
    CDSECT,
    COMMENT,
    DOCDECL,
    INSTRUCTION,
    ENTITY_REFERENCE,
    WHITESPACE,
    ELEMENTDECL,
    ENTITYDECL,
    ATTLISTDECL,
    NOTATIONDECL,
    PARAMETER_ENTITY_REF,
    OK,
    ERROR
};

enum class TextEnum { ATTRI, TEXT, ENTITY_DECL };
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class XmlPullParser {
public:
    static ani_long BindNative(ani_env *env, ani_class self, ani_string content);
    static ani_boolean ReleaseNative(ani_env *env, ani_class self, ani_long pointer);
    // CC-OFFNXT(G.FUN.01-CPP) options of parse
    static ani_boolean ParseXml(ani_env *env, ani_class self, ani_long pointer, ani_boolean ignoreNS,
                                ani_boolean supportDCT, ani_fn_object attrValCb, ani_fn_object tagValCb,
                                ani_fn_object tknCalCb);
    static ani_ref GetParseError(ani_env *env, ani_class self, ani_long pointer);

private:
    // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
    class ParseInfoClassCache {
    public:
// CC-OFFNXT(G.PRE.06) option lists
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PARSE_INFO_FIELD_LIST(V)                                     \
    V(depth, Depth, DEPTH, ani_int, Int)                             \
    V(columnNumber, ColumnNumber, COLUMN_NUMBER, ani_int, Int)       \
    V(lineNumber, LineNumber, LINE_NUMBER, ani_int, Int)             \
    V(attributeCount, AttributeCount, ATTRIBUTE_COUNT, ani_int, Int) \
    V(name, Name, NAME, ani_ref, Ref)                                \
    V(namespace, NameSpace, NAMESPACE, ani_ref, Ref)                 \
    V(prefix, Prefix, PREFIX, ani_ref, Ref)                          \
    V(text, Text, TEXT, ani_ref, Ref)                                \
    V(whitespace, Whitespace, WHITESPACE, ani_boolean, Boolean)      \
    V(emptyElementTag, EmptyElementTag, EMPTY_ELEMENT_TAG, ani_boolean, Boolean)

        enum class FieldIndex : uint16_t {
// CC-OFFNXT(G.PRE.02) enum item defination
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FIELD_CLASS_ENUM(_cache, _func, item, _type, _typeName) item,
            PARSE_INFO_FIELD_LIST(FIELD_CLASS_ENUM)
#undef FIELD_CLASS_ENUM
        };

        explicit ParseInfoClassCache(ani_env *env);
        ~ParseInfoClassCache();
        ani_object New() const;
        void InitFieldCache();
        ani_class GetCachedClass_();

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FIELD_SETTER(_cache, func, _item, type, _typeName) \
    /* CC-OFFNXT(G.PRE.09) function defination */          \
    ani_status Set##func(ani_object object, type value) const;
        PARSE_INFO_FIELD_LIST(FIELD_SETTER)
#undef FIELD_SETTER
    private:
        ani_env *env_ {};
        ani_class cachedClass_ {};
        ani_method constructor_ {};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CACHED_FIELD(cache, _func, _item, _type, _typeName) \
    /* CC-OFFNXT(G.PRE.09) member defination */             \
    ani_field cache##_ {};
        PARSE_INFO_FIELD_LIST(CACHED_FIELD)
#undef CACHED_FIELD
    };

    XmlPullParser(ani_env *env, std::string strXml);
    ~XmlPullParser();

    std::string XmlPullParserError() const;
    void Parse(ani_env *env);

    int GetColumnNumber() const;
    int GetLineNumber() const;
    std::string GetText() const;

    bool DealLength(size_t minimum);
    void Replace(std::string &strTemp, const std::string &strSrc, const std::string &strDes) const;
    size_t GetNSCount(size_t iTemp);
    std::string GetNamespace(const std::string &prefix);
    TagEnum ParseTagType(bool inDeclaration);
    void SkipText(const std::string &chars);
    int PriorDealChar();
    void SkipChar(char expected);
    std::string ParseNameInner(size_t start);
    std::string ParseName();
    void SkipInvalidChar();
    void ParseEntity(std::string &out, bool isEntityToken, TextEnum textEnum);
    std::string ParseTagValue(char delimiter, TextEnum textEnum);
    bool ParseNsp();
    TagEnum ParseStartTag(bool xmldecl);
    void ParseDeclaration();
    bool ParseEndTag();
    std::string ParseDelimiterInfo(std::string delimiter, bool returnText);
    std::string ParseDelimiter(bool returnText);
    bool ParserDoctInnerInfo(bool requireSystemName, bool assignFields);
    void ParseComment(bool returnText);
    void ParseSpecText();
    void ParseInnerEleDec();
    void ParseInnerAttriDecl();
    void ParseEntityDecl();
    void ParseInneNotaDecl();
    void ReadInternalSubset();
    void ParseDoctype(bool saveDtdText);
    TagEnum ParseOneTag();
    void ParserPriorDeal();
    void ParseInstruction();
    void ParseText();
    void ParseCdect();

    bool ParseTag(ani_env *env) const;
    bool ParseAttr(ani_env *env) const;
    bool ParseToken(ani_env *env) const;
    bool HandleTagFunc(ani_env *env);
    bool HandleTokenFunc(ani_env *env);
    bool HandleAttrFunc(ani_env *env);

    void ParseNspFunction();
    void ParseNspFunc(size_t &i, const std::string &attrName);
    void ParseInnerAttriDeclFunc(int &c);
    TagEnum DealExclamationGroup();
    void ParseEntityFunc(size_t start, std::string &out, bool isEntityToken, TextEnum textEnum);
    bool ParseStartTagFuncDeal();
    TagEnum ParseStartTagFunc(bool xmldecl);
    TagEnum ParseOneTagFunc();
    size_t ParseTagValueInner(size_t &start, std::string &result, char delimiter, TextEnum textEnum);
    bool ParseTagValueFunc(char &c, TextEnum textEnum, size_t &start, std::string &result);
    void MakeStrUpper(std::string &src) const;
    TagEnum DealLtGroup();
    void DealWhiteSpace(unsigned char c);

    [[maybe_unused]] ani_env *env_ {};
    ParseInfoClassCache infoClass_;
    ani_enum enumTypeClass_ {};
    bool bDoctype_ {};
    bool bIgnoreNS_ {};
    bool bStartDoc_ {true};
    ani_fn_object tagFunc_ {nullptr};
    ani_fn_object attrFunc_ {nullptr};
    ani_fn_object tokenFunc_ {nullptr};

    std::string strXml_ {};
    std::string prefix_ {};
    std::string namespace_ {};
    std::string name_ {};
    std::string text_ {};
    std::string sysInfo_ {};
    std::string pubInfo_ {};
    std::string keyInfo_ {};
    std::string xmlPullParserError_ {};
    std::vector<size_t> nspCounts_;
    std::vector<std::string> nspStack_;
    std::vector<std::string> elementStack_;
    std::vector<std::string> attributes_;
    std::map<std::string, std::string> documentEntities_;
    size_t position_ {};
    size_t depth_ {};
    size_t max_ {};
    size_t bufferStartLine_ {};
    size_t bufferStartColumn_ {};
    size_t attrCount_ {};
    TagEnum type_ = TagEnum::START_DOCUMENT;
    bool bWhitespace_ {};
    bool bEndFlag_ {};
    bool bUnresolved_ {};
};
}  // namespace ark::ets::sdk::util
#endif  // ANI_XMLPULLPARSER_H
