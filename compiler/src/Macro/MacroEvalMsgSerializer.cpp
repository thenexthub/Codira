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

/**
 * @file
 *
 * This file implements the Macro evaluation messages serializer for macro evaluation.
 */
#include "Codira/Macro/MacroEvalMsgSerializer.h"
#include "Codira/Macro/TokenSerialization.h"

using namespace MacroMsgFormat;
using namespace flatbuffers;
using namespace Codira;

namespace {
static const int MACRO_SETITEM_INFO_NUM = 3;

static auto CreateTokenVec(FlatBufferBuilder& builder, const std::vector<Codira::Token>& tks)
{
    std::vector<Offset<MacroMsgFormat::Token>> offsetVec;
    for (auto& tk : tks) {
        auto tbegin = MacroMsgFormat::Position(tk.Begin().fileID, tk.Begin().line, tk.Begin().column);
        auto tend = MacroMsgFormat::Position(tk.End().fileID, tk.End().line, tk.End().column);
        offsetVec.push_back(CreateToken(
            builder, static_cast<uint8_t>(tk.kind), builder.CreateString(tk.Value()), &tbegin, &tend, tk.delimiterNum));
    }
    return offsetVec;
}

static void GetChildMessages(std::vector<ChildMessage>& childMessages, const Codira::MacroCall& macCall)
{
    for (auto& child : macCall.children) {
        if (!child->items.empty()) {
            childMessages.emplace_back(ChildMessage{child->GetFullName(), child->items});
        }
        GetChildMessages(childMessages, *child);
    }
}

static auto CreatItmeInfoVec(FlatBufferBuilder& builder, const std::vector<Codira::ItemInfo>& items)
{
    std::vector<Offset<MacroMsgFormat::ItemInfo>> itemsVec;
    for (auto& it : items) {
        auto key = builder.CreateString(it.key);
        switch (it.kind) {
            case ItemKind::STRING: {
                auto str = builder.CreateString(it.sValue);
                itemsVec.push_back(CreateItemInfo(builder, key, OptionValue_sValue, str.Union()));
                break;
            }
            case ItemKind::INT: {
                auto i = builder.CreateStruct(IntValue(it.iValue));
                itemsVec.push_back(CreateItemInfo(builder, key, OptionValue_iValue, i.Union()));
                break;
            }
            case ItemKind::BOOL: {
                auto b = builder.CreateStruct(BoolValue(it.bValue));
                itemsVec.push_back(CreateItemInfo(builder, key, OptionValue_bValue, b.Union()));
                break;
            }
            case ItemKind::TKS: {
                auto tksVec = builder.CreateVector(CreateTokenVec(builder, it.tValue));
                auto t = CreateTokensValue(builder, tksVec);
                itemsVec.push_back(CreateItemInfo(builder, key, OptionValue_tValue, t.Union()));
                break;
            }
        }
    }
    return itemsVec;
}

static auto CreateDiagVec(FlatBufferBuilder& builder, const std::vector<Codira::Diagnostic>& diags)
{
    std::vector<Offset<MacroMsgFormat::Diagnostic>> offsetVec;
    for (auto& diag : diags) {
        auto& beginPos = diag.mainHint.range.begin;
        auto& endPos = diag.mainHint.range.end;
        auto begin = MacroMsgFormat::Position(beginPos.fileID, beginPos.line, beginPos.column);
        auto end = MacroMsgFormat::Position(endPos.fileID, endPos.line, endPos.column);

        auto errorMessage = builder.CreateString(diag.errorMessage);
        auto mainHint = builder.CreateString(diag.mainHint.str);

        offsetVec.push_back(
            CreateDiagnostic(builder, static_cast<int>(diag.diagSeverity), &begin, &end, errorMessage, mainHint));
    }
    return offsetVec;
}

static auto CreateMacroCall(FlatBufferBuilder& builder, const Codira::MacroCall& macCall)
{
    auto& inv = *(macCall.GetInvocation());
    auto& idPos = inv.identifierPos;
    auto pos = MacroMsgFormat::Position(idPos.fileID, idPos.line, idPos.column);
    auto id = CreateIdInfo(builder, builder.CreateString(inv.identifier), &pos);

    auto args = builder.CreateVector(CreateTokenVec(builder, inv.args));
    auto attrs = builder.CreateVector(CreateTokenVec(builder, inv.attrs));

    auto parentNames = builder.CreateVectorOfStrings(macCall.parentNames);

    std::vector<Offset<ChildMsg>> childMsgVec;
    std::vector<ChildMessage> childMessages;
    GetChildMessages(childMessages, macCall);
    for (auto& msg : childMessages) {
        auto childName = builder.CreateString(msg.childName);
        childMsgVec.push_back(
            CreateChildMsg(builder, childName, builder.CreateVector(CreatItmeInfoVec(builder, msg.items))));
    }

    auto childMsges = builder.CreateVector(childMsgVec);

    auto methodName = builder.CreateString(macCall.methodName);
    auto packageName = builder.CreateString(macCall.packageName);
    auto libPath = builder.CreateString(macCall.libPath);

    auto beginPos = macCall.GetBeginPos();
    auto endPos = macCall.GetEndPos();
    auto bPos = MacroMsgFormat::Position(beginPos.fileID, beginPos.line, beginPos.column);
    auto ePos = MacroMsgFormat::Position(endPos.fileID, endPos.line, endPos.column);

    return CreateMacroCall(builder, id, inv.hasAttr, args, attrs, parentNames, childMsges,
                           methodName, packageName, libPath, &bPos, &ePos);
}

static auto CreateItemsVecFromRecordMacroInfo(
    FlatBufferBuilder& builder, const std::vector<InvokeRuntime::HANDLE>& recordMacroInfo)
{
    std::vector<Offset<MacroMsgFormat::ItemInfo>> itemsVec;
    auto itemNum = recordMacroInfo.size() / MACRO_SETITEM_INFO_NUM;
    for (size_t i = 0; i < itemNum; i++) {
        auto itemid = i * MACRO_SETITEM_INFO_NUM;
        // Item: key.
        Offset<String> key;
        auto pKey = recordMacroInfo[itemid]; // 0: key malloc from ffi, need free in child process.
        if (pKey) {
            key = builder.CreateString(std::string(static_cast<char*>(pKey)));
            free(pKey);
        } else {
            continue;
        }

        auto pValue = recordMacroInfo[itemid + 1];
        auto pType = recordMacroInfo[itemid + 2];
        auto type = static_cast<ItemKind>(*static_cast<uint8_t*>(pType));
        free(pType);

        if (type == ItemKind::STRING) {
            auto val = builder.CreateString(std::string(static_cast<char*>(pValue)));
            itemsVec.push_back(CreateItemInfo(builder, key, OptionValue_sValue, val.Union()));
        } else if (type == ItemKind::INT) {
            auto val = builder.CreateStruct(IntValue(*static_cast<int64_t*>(pValue)));
            itemsVec.push_back(CreateItemInfo(builder, key, OptionValue_iValue, val.Union()));
        } else if (type == ItemKind::BOOL) {
            auto val = builder.CreateStruct(BoolValue(*static_cast<bool*>(pValue)));
            itemsVec.push_back(CreateItemInfo(builder, key, OptionValue_bValue, val.Union()));
        } else if (type == ItemKind::TKS) {
            if (pValue == nullptr) {
                auto tksVec = builder.CreateVector(CreateTokenVec(builder, std::vector<Codira::Token>()));
                auto val = CreateTokensValue(builder, tksVec);
                itemsVec.push_back(CreateItemInfo(builder, key, OptionValue_tValue, val.Union()));
            } else {
                auto tokens = TokenSerialization::GetTokensFromBytes(static_cast<uint8_t*>(pValue) + sizeof(uint32_t));
                auto tksVec = builder.CreateVector(CreateTokenVec(builder, tokens));
                auto val = CreateTokensValue(builder, tksVec);
                itemsVec.push_back(CreateItemInfo(builder, key, OptionValue_tValue, val.Union()));
            }
        }

        if (pValue) {
            free(pValue);
        }
    }
    return builder.CreateVector(itemsVec);
}

static void DeserializePositionFromPositionBuf(Codira::Position& pos, const MacroMsgFormat::Position& posBuf)
{
    pos.fileID = posBuf.file_id();
    pos.line = posBuf.line();
    pos.column = posBuf.column();
}
static void DeserializeTokenFromTokenBuf(Codira::Token& tk, const MacroMsgFormat::Token& tkBuffer)
{
    tk.kind = static_cast<TokenKind>(tkBuffer.kind());
    tk.delimiterNum = tkBuffer.delimiterNum();
    Codira::Position begin;
    Codira::Position end;
    DeserializePositionFromPositionBuf(begin, *tkBuffer.begin());
    DeserializePositionFromPositionBuf(end, *tkBuffer.end());
    tk.SetValuePos(tkBuffer.value()->str(), begin, end);
}

static void DeserializeTokensFromOffsetTokenVec(
    std::vector<Codira::Token>& tks, const Vector<Offset<MacroMsgFormat::Token>>& offsetVec)
{
    tks.clear();
    uoffset_t num = offsetVec.size();
    Codira::Token tmp{TokenKind::ILLEGAL};
    for (uoffset_t i = 0; i < num; i++) {
        DeserializeTokenFromTokenBuf(tmp, *offsetVec.Get(i));
        tks.push_back(tmp);
    }
}

static void DeserializeDiagFromDiagBuf(Codira::Diagnostic& diag, const MacroMsgFormat::Diagnostic& diagBuffer)
{
    DeserializePositionFromPositionBuf(diag.start, *diagBuffer.begin());
    DeserializePositionFromPositionBuf(diag.end, *diagBuffer.end());
    diag.errorMessage = diagBuffer.errorMessage()->str();
    diag.mainHint.str = diagBuffer.mainHint()->str();
    diag.diagSeverity = static_cast<Codira::DiagSeverity>(diagBuffer.diagSeverity());
}

static void DeserializeDiagsFromOffsetDiagVec(
    std::vector<Codira::Diagnostic>& diags, const Vector<Offset<MacroMsgFormat::Diagnostic>>& offsetVec)
{
    diags.clear();
    uoffset_t num = offsetVec.size();
    Codira::Diagnostic tmp{};
    for (uoffset_t i = 0; i < num; i++) {
        DeserializeDiagFromDiagBuf(tmp, *offsetVec.Get(i));
        diags.push_back(tmp);
    }
}

static void DeserializeItemsFromItemsBuf(
    std::vector<Codira::ItemInfo>& items, const Vector<Offset<MacroMsgFormat::ItemInfo>>& itemsBuf)
{
    uoffset_t num = itemsBuf.size();
    items.resize(num);
    for (uoffset_t i = 0; i < num; i++) {
        items[i].key = itemsBuf.Get(i)->key()->str();
        switch (itemsBuf.Get(i)->value_type()) {
            case OptionValue_sValue:
                items[i].kind = ItemKind::STRING;
                items[i].sValue = itemsBuf.Get(i)->value_as_sValue()->str();
                break;
            case OptionValue_iValue:
                items[i].kind = ItemKind::INT;
                items[i].iValue = itemsBuf.Get(i)->value_as_iValue()->val();
                break;
            case OptionValue_bValue:
                items[i].kind = ItemKind::BOOL;
                items[i].bValue = itemsBuf.Get(i)->value_as_bValue()->val();
                break;
            case OptionValue_tValue:
                items[i].kind = ItemKind::TKS;
                DeserializeTokensFromOffsetTokenVec(items[i].tValue, *(itemsBuf.Get(i)->value_as_tValue()->val()));
                break;
            case OptionValue_NONE:
                [[fallthrough]]; // error
            default:
                Errorln("DeserializeItemsFromItemsBuf value type error, key: ", items[i].key);
                break;
        }
    }
}
} // namespace

void MacroEvalMsgSerializer::SerializeDeflibMsg(
    const std::unordered_set<std::string>& macrolibs, std::vector<uint8_t>& bufferData)
{
    builder.Clear();
    std::vector<Offset<String>> pathsVec;
    for (auto& p : macrolibs) {
        pathsVec.push_back(builder.CreateString(p));
    }
    auto paths = builder.CreateVector(pathsVec);
    auto content = CreateDefLib(builder, paths);
    auto macroMsg = CreateMacroMsg(builder, MsgContent_defLib, content.Union());
    builder.Finish(macroMsg);
    bufferData.assign(builder.GetBufferPointer(), builder.GetBufferPointer() + builder.GetSize());
}

void MacroEvalMsgSerializer::SerializeMacroCallMsg(const Codira::MacroCall& macCall, std::vector<uint8_t>& bufferData)
{
    builder.Clear();
    std::vector<Offset<MacroMsgFormat::MacroCall>> callsVec;
    if (macCall.GetInvocation() == nullptr) {
        Errorln("SerializeMacroCallMsg nullptr");
        return;
    }
    callsVec.push_back(CreateMacroCall(builder, macCall));
    auto calls = builder.CreateVector(callsVec);
    auto cont = CreateMultiMacroCalls(builder, calls);
    auto macroMsg = CreateMacroMsg(builder, MsgContent_multiCalls, cont.Union());
    builder.Finish(macroMsg);
    bufferData.assign(builder.GetBufferPointer(), builder.GetBufferPointer() + builder.GetSize());
}

void MacroEvalMsgSerializer::SerializeMultiCallsMsg(
    const std::list<Codira::MacroCall*>& macCalls, std::vector<uint8_t>& bufferData)
{
    builder.Clear();
    std::vector<Offset<MacroMsgFormat::MacroCall>> callsVec;
    for (auto callPtr : macCalls) {
        if (callPtr->GetInvocation() == nullptr) {
            Errorln("SerializeMultiCallsMsg nullptr");
            return;
        }
        callsVec.push_back(CreateMacroCall(builder, *callPtr));
    }
    auto calls = builder.CreateVector(callsVec);
    auto cont = CreateMultiMacroCalls(builder, calls);
    auto macroMsg = CreateMacroMsg(builder, MsgContent_multiCalls, cont.Union());
    builder.Finish(macroMsg);
    bufferData.assign(builder.GetBufferPointer(), builder.GetBufferPointer() + builder.GetSize());
}

bool MacroEvalMsgSerializer::SerializeMacroCallResultMsg(const MacroCall& macCall, std::vector<uint8_t>& bufferData)
{
    if (macCall.GetInvocation() == nullptr) {
        return false;
    }
    builder.Clear();
    auto& inv = *(macCall.GetInvocation());
    auto& idPos = inv.identifierPos;
    auto pos = MacroMsgFormat::Position(idPos.fileID, idPos.line, idPos.column);
    auto id = CreateIdInfo(builder, builder.CreateString(inv.identifier), &pos);

    auto tks = builder.CreateVector(CreateTokenVec(builder, inv.newTokens));

    Offset<Vector<Offset<MacroMsgFormat::ItemInfo>>> items;
    if (!macCall.items.empty()) {
        items = builder.CreateVector(CreatItmeInfoVec(builder, macCall.items));
    } else if (macCall.recordMacroInfo.size() / MACRO_SETITEM_INFO_NUM > 0) {
        // serialize and free ffi malloc.
        items = CreateItemsVecFromRecordMacroInfo(builder, macCall.recordMacroInfo);
    } else {
        std::vector<Offset<MacroMsgFormat::ItemInfo>> itemsVec;
        items = builder.CreateVector(itemsVec);
    }

    auto assertParents = builder.CreateVectorOfStrings(macCall.assertParents);
    auto diags = macCall.ci->diag.GetCategoryDiagnostic(DiagCategory::PARSE);
    auto diagnostics = builder.CreateVector(CreateDiagVec(builder, diags));

    auto macroResultCont =
        CreateMacroResult(builder, id, static_cast<uint8_t>(macCall.status), tks, items, assertParents, diagnostics);
    auto macroMsg = CreateMacroMsg(builder, MsgContent_macroResult, macroResultCont.Union());
    builder.Finish(macroMsg);
    bufferData.assign(builder.GetBufferPointer(), builder.GetBufferPointer() + builder.GetSize());
    return true;
}

void MacroEvalMsgSerializer::SerializeExitMsg(std::vector<uint8_t>& bufferData, bool flag)
{
    builder.Clear();
    auto cont = builder.CreateStruct(MacroMsgFormat::ExitTask(flag));
    auto macroMsg = CreateMacroMsg(builder, MsgContent_exitTask, cont.Union());
    builder.Finish(macroMsg);
    bufferData.assign(builder.GetBufferPointer(), builder.GetBufferPointer() + builder.GetSize());
}

MacroMsgFormat::MsgContent MacroEvalMsgSerializer::GetMacroMsgContenType(const std::vector<uint8_t>& bufferData)
{
    return GetMacroMsg(bufferData.data())->content_type();
}

void MacroEvalMsgSerializer::DeSerializeDeflibMsg(
    std::list<std::string>& macroLibs, const std::vector<uint8_t>& bufferData)
{
    auto defLib = GetMacroMsg(bufferData.data())->content_as_defLib();
    uoffset_t pathNum = defLib->paths()->size();
    macroLibs.clear();
    for (uoffset_t i = 0; i < pathNum; i++) {
        macroLibs.push_back(defLib->paths()->Get(i)->str());
    }
}

void MacroEvalMsgSerializer::DeSerializeRangeFromCall(
    Position& begin, Position& end, const MacroMsgFormat::MacroCall& callFmt)
{
    DeserializePositionFromPositionBuf(begin, *callFmt.begin());
    DeserializePositionFromPositionBuf(end, *callFmt.end());
}

void MacroEvalMsgSerializer::DeSerializeIdInfoFromCall(
    std::string& id, Position& pos, const MacroMsgFormat::MacroCall& callFmt)
{
    id = callFmt.id()->name()->str();
    DeserializePositionFromPositionBuf(pos, *callFmt.id()->pos());
}


void MacroEvalMsgSerializer::DeSerializeArgsFromCall(std::vector<Token>& args, const MacroMsgFormat::MacroCall& callFmt)
{
    DeserializeTokensFromOffsetTokenVec(args, *callFmt.args());
}

void MacroEvalMsgSerializer::DeSerializeAttrsFromCall(
    std::vector<Token>& attrs, const MacroMsgFormat::MacroCall& callFmt)
{
    DeserializeTokensFromOffsetTokenVec(attrs, *callFmt.attrs());
}

void MacroEvalMsgSerializer::DeSerializeParentNamesFromCall(
    std::vector<std::string>& parentNames, const MacroMsgFormat::MacroCall& callFmt)
{
    uoffset_t num = callFmt.parentNames()->size();
    parentNames.resize(num);
    for (uoffset_t i = 0; i < num; i++) {
        parentNames[i] = callFmt.parentNames()->Get(i)->str();
    }
}

void MacroEvalMsgSerializer::DeSerializeChildMsgesFromCall(
    std::vector<ChildMessage>& childMsges, const MacroMsgFormat::MacroCall& callFmt)
{
    uoffset_t msgNum = callFmt.childMsges()->size();
    childMsges.resize(msgNum);
    for (uoffset_t i = 0; i < msgNum; i++) {
        childMsges[i].childName = callFmt.childMsges()->Get(i)->childName()->str();
        DeserializeItemsFromItemsBuf(childMsges[i].items, *callFmt.childMsges()->Get(i)->items());
    }
}

void MacroEvalMsgSerializer::DeSerializeIdInfoFromResult(
    std::string& id, Position& pos, const std::vector<uint8_t>& bufferData)
{
    auto result = GetMacroMsg(bufferData.data())->content_as_macroResult();
    if (result != nullptr) {
        id = result->id()->name()->str();
        DeserializePositionFromPositionBuf(pos, *result->id()->pos());
    }
}

void MacroEvalMsgSerializer::DeSerializeTksFromResult(std::vector<Token>& tks, const std::vector<uint8_t>& bufferData)
{
    auto result = GetMacroMsg(bufferData.data())->content_as_macroResult();
    if (result != nullptr) {
        DeserializeTokensFromOffsetTokenVec(tks, *result->tks());
    }
}

void MacroEvalMsgSerializer::DeSerializeDiagsFromResult(std::vector<Diagnostic>& diags,
                                                        const std::vector<uint8_t>& bufferData)
{
    auto result = GetMacroMsg(bufferData.data())->content_as_macroResult();
    if (result != nullptr) {
        DeserializeDiagsFromOffsetDiagVec(diags, *result->diags());
    }
}

MacroEvalStatus MacroEvalMsgSerializer::DeSerializeStatusFromResult(const std::vector<uint8_t>& bufferData)
{
    auto result = GetMacroMsg(bufferData.data())->content_as_macroResult();
    if (result != nullptr) {
        return static_cast<MacroEvalStatus>(result->status());
    }
    return MacroEvalStatus::REEVALFAILED;
}

void MacroEvalMsgSerializer::DeSerializeItemsFromResult(
    std::vector<ItemInfo>& items, const std::vector<uint8_t>& bufferData)
{
    auto result = GetMacroMsg(bufferData.data())->content_as_macroResult();
    if (result != nullptr) {
        DeserializeItemsFromItemsBuf(items, *result->items());
    }
}

void MacroEvalMsgSerializer::DeSerializeAssertParentsFromResult(
    std::vector<std::string>& assertParents, const std::vector<uint8_t>& bufferData)
{
    auto result = GetMacroMsg(bufferData.data())->content_as_macroResult();
    if (result != nullptr) {
        uoffset_t num = result->assertParents()->size();
        assertParents.resize(num);
        for (uoffset_t i = 0; i < num; i++) {
            assertParents[i] = result->assertParents()->Get(i)->str();
        }
    }
}
