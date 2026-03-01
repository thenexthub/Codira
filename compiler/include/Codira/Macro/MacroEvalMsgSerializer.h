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

/*
 * @file
 *
 * This file declares the Macro evaluation messages serializer for MacroEvaluation.
 */

#ifndef CODIRA_MACROEVALMSGSLZER_H
#define CODIRA_MACROEVALMSGSLZER_H

#include <list>
#include <string>
#include <unordered_set>
#include <vector>
#include "Codira/Macro/MacroCommon.h"
#include "flatbuffers/MacroMsgFormat_generated.h"
namespace Codira {
class MacroEvalMsgSerializer {
public:
    void SerializeDeflibMsg(const std::unordered_set<std::string>& macrolibs, std::vector<uint8_t>& bufferData);
    void SerializeMacroCallMsg(const Codira::MacroCall& macCall, std::vector<uint8_t>& bufferData);
    bool SerializeMacroCallResultMsg(const MacroCall& macCall, std::vector<uint8_t>& bufferData);
    void SerializeMultiCallsMsg(const std::list<MacroCall*>& macCalls, std::vector<uint8_t>& bufferData);

    void SerializeExitMsg(std::vector<uint8_t>& bufferData, bool flag = true);

    static MacroMsgFormat::MsgContent GetMacroMsgContenType(const std::vector<uint8_t>& bufferData);

    static void DeSerializeDeflibMsg(std::list<std::string>& macroLibs, const std::vector<uint8_t>& bufferData);

    static void DeSerializeRangeFromCall(Position& begin, Position& end, const MacroMsgFormat::MacroCall& callFmt);

    static void DeSerializeIdInfoFromCall(std::string& id, Position& pos, const MacroMsgFormat::MacroCall& callFmt);
    static void DeSerializeArgsFromCall(std::vector<Token>& args, const MacroMsgFormat::MacroCall& callFmt);

    static void DeSerializeAttrsFromCall(std::vector<Token>& attrs, const MacroMsgFormat::MacroCall& callFmt);
    static void DeSerializeParentNamesFromCall(
        std::vector<std::string>& parentNames, const MacroMsgFormat::MacroCall& callFmt);
    static void DeSerializeChildMsgesFromCall(
        std::vector<ChildMessage>& childMsges, const MacroMsgFormat::MacroCall& callFmt);

    static void DeSerializeIdInfoFromResult(std::string& id, Position& pos, const std::vector<uint8_t>& bufferData);
    static void DeSerializeTksFromResult(std::vector<Token>& tks, const std::vector<uint8_t>& bufferData);
    static MacroEvalStatus DeSerializeStatusFromResult(const std::vector<uint8_t>& bufferData);
    static void DeSerializeItemsFromResult(std::vector<ItemInfo>& items, const std::vector<uint8_t>& bufferData);
    static void DeSerializeAssertParentsFromResult(
        std::vector<std::string>& assertParents, const std::vector<uint8_t>& bufferData);
    static void DeSerializeDiagsFromResult(std::vector<Diagnostic>& diags, const std::vector<uint8_t>& bufferData);
    MacroEvalMsgSerializer() noexcept {};

private:
    flatbuffers::FlatBufferBuilder builder{1024};
};
} // namespace Codira

#endif
