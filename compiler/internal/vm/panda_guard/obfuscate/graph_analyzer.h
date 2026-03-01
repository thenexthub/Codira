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

#ifndef PANDA_GUARD_OBFUSCATE_GRAPH_ANALYZER_H
#define PANDA_GUARD_OBFUSCATE_GRAPH_ANALYZER_H

#include "entity.h"

namespace panda::guard {

class GraphAnalyzer {
public:
    GraphAnalyzer() = delete;

    ~GraphAnalyzer() = delete;
    NO_COPY_SEMANTIC(GraphAnalyzer);

    /**
     * Get Related Lda.Str instruction
     * @param inIns origin inst
     * @param outIns related Lda.Str inst; if not found, outIns.ins_ will be nullptr
     * @param index reg index, -1 as using default value
     */
    static void GetLdaStr(const InstructionInfo &inIns, InstructionInfo &outIns, int index = -1);

    /**
     * Get Related instruction of DefineMethod
     * @param inIns input ins: definemethod
     * @param defineIns ins info of define method
     * @param nameIns ins info of name define
     */
    static void HandleDefineMethod(const InstructionInfo &inIns, InstructionInfo &defineIns, InstructionInfo &nameIns);

    /**
     * Get Related instruction of definepropertybyname
     * @param inIns input ins
     * @param defineIns related define ins
     */
    static void HandleDefineProperty(const InstructionInfo &inIns, InstructionInfo &defineIns);

    /**
     * Get class is component
     *
     * @Component
     * struct ClassName{}
     *
     * @ComponentV2
     * struct ClassName{}
     *
     * @param inIns defineclasswithbuffer instruction
     * @return is component class
     */
    static bool IsComponentClass(const InstructionInfo &inIns);

    /**
     * get stmodulevar bind define instruction
     * @param inIns input ins (stmodulevar or wide.stdmodulevar)
     * @param defineIns bind define instruction
     */
    static void GetStModuleVarDefineIns(const InstructionInfo &inIns, InstructionInfo &defineIns);

    /**
     * get stobjbyname bind define instruction
     * @param inIns input ins (stobjbyname)
     * @param defineIns bind define instruction
     */
    static void GetStObjByNameDefineIns(const InstructionInfo &inIns, InstructionInfo &defineIns);

    /**
     * Get the predecessor instruction associated with the stobjbyname instruction and check if it is one of the
     * following commands NEWOBTRANSAGE, CALLTHIS2, CALLTHIS3, If it meets the requirements, return the corresponding
     * pre-instruction information
     * @param inIns stobjbyname ins
     * @param out related input ins
     */
    static void GetStObjByNameInput(const InstructionInfo &inIns, InstructionInfo &out);

    /**
     * Get the object name followed by the tryldglobalbyname instruction associated with the newobjrange instruction,
     * as well as the ldastr parameter associated with the newobjrange instruction
     * @param inIns newobjrange ins
     * @param name newobjrange ins name
     * @param out related ins
     */
    static void GetNewObjRangeInfo(const InstructionInfo &inIns, std::string &name, InstructionInfo &out);

    /**
     * Get call instruction object name
     * @param inIns call ins
     */
    static std::string GetCallName(const InstructionInfo &inIns);

    /**
     * Get Related ldaStr instruction of call ins
     * @param inIns call ins
     * @param paramIndex call param index
     * @param out param ins
     */
    static void GetCallLdaStrParam(const InstructionInfo &inIns, uint32_t paramIndex, InstructionInfo &out);

    /**
     * Get Related tryLdGlobalByName instruction of call ins
     * @param inIns call ins
     * @param paramIndex call param index
     * @param out param ins
     */
    static void GetCallTryLdGlobalByNameParam(const InstructionInfo &inIns, uint32_t paramIndex, InstructionInfo &out);

    /**
     * Get Related ldobjbyname instruction of call ins
     * @param inIns call ins
     * @param paramIndex call param index
     * @param out param ins
     */
    static void GetCallLdObjByNameParam(const InstructionInfo &inIns, uint32_t paramIndex, InstructionInfo &out);

    /**
     * Get Related instruction of isin ins
     * @param inIns isin ins
     * @param out related ins array
     */
    static void GetIsInInfo(const InstructionInfo &inIns, std::vector<InstructionInfo> &out);

    /**
     * Get Related createobjectwithbuffer instruction of call ins
     * @param inIns call ins
     * @param paramIndex call param index
     * @param out object ins
     */
    static void GetCallCreateObjectWithBufferParam(const InstructionInfo &inIns, uint32_t paramIndex,
                                                   InstructionInfo &out);

    /**
     * Get Related definefunc instruction in acc of definepropertybyname ins
     * @param inIns call ins
     * @param out function ins
     */
    static void GetDefinePropertyByNameFunction(const InstructionInfo &inIns, InstructionInfo &out);
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_GRAPH_ANALYZER_H
