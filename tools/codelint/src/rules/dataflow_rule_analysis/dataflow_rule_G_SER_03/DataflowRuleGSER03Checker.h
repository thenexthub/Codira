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

#ifndef DATAFLOW_RULE_G_SER_03_CHECKER_H
#define DATAFLOW_RULE_G_SER_03_CHECKER_H

#include "../DataflowRule.h"
#include "common/CommonFunc.h"
#include "common/DiagnosticEngine.h"

namespace Codira::CodeCheck {
class DataflowRuleGSER03Checker : public DataflowRule {
public:
    explicit DataflowRuleGSER03Checker(CodeCheckDiagnosticEngine *diagEngine);
    ~DataflowRuleGSER03Checker() override = default;

protected:
    void CheckBasedOnCHIR(CHIR::Package &package) override;

private:
    using PositionPair = std::pair<Codira::Position, Codira::Position>;
    using TypeWithPosition = std::pair<std::string, PositionPair>;
    // Retrieve whether it is a serialization type from the function call,
    // and if so, record the serialization code address.
    TypeWithPosition TypeInSer(const CHIR::Apply *apply, bool inDataStruct = false);
    // Retrieve whether it is a deserialization type from the function call,
    // and if so, record the deserialization code address.
    TypeWithPosition TypeInDeser(const CHIR::Apply *apply);
    // Serialization Types and Corresponding DataModel Types
    std::map<std::string, std::string> typeToDataModel = {{"Bool", "DataModelBool"}, {"Int8", "DataModelInt"},
        {"Int16", "DataModelInt"}, {"Int32", "DataModelInt"}, {"Int64", "DataModelInt"}, {"UInt8", "DataModelInt"},
        {"UInt16", "DataModelInt"}, {"UInt32", "DataModelInt"}, {"UInt64", "DataModelInt"},
        {"Float16", "DataModelFloat"}, {"Float32", "DataModelFloat"}, {"Float64", "DataModelFloat"},
        {"String", "DataModelString"}, {"Rune", "DataModelString"}};
    // Records the serialization and deserialization functions of a class or struct.
    using SerialisePair = std::pair<CHIR::Func *, CHIR::Func *>;
    using SerialiseMap = std::map<std::string, std::vector<TypeWithPosition>>;
    // Build a map of serialization and deserialization function pairs
    // key: ClassDef.identifier, value: {serial func, deserial func}
    std::map<CHIR::CustomTypeDef *, SerialisePair> allSerialiseClass;
    void SetSerInRetToFieldSerMapHelper(CHIR::Load *load, TypeWithPosition &typeStr, SerialiseMap &serTypeMap);
    void SetSerInRetToFieldSerMap(CHIR::Apply *apply, TypeWithPosition &typeStr, SerialiseMap &serTypeMap);
    void CollectClassDefWithSer(CHIR::Package &package);

    void CollectSerRelatedFuncsInDef(CHIR::CustomTypeDef *classDef);
    std::string GetValueIdentifier(const CHIR::Value *value);
    // Collect all serialization types
    void CollectSerType(const CHIR::Expression &expr, std::vector<TypeWithPosition> &serTypes, SerialiseMap &serMap,
        SerialiseMap &fieldSerMap, CHIR::LocalVar *ret);
    // Check all deSerialization types
    void CheckSerType(const CHIR::Expression &expr, std::vector<TypeWithPosition> &serTypes, SerialiseMap &serMap,
        SerialiseMap &fieldSerMap);
    std::string JointSerializeTypeAndLine(std::vector<TypeWithPosition> &serTypes, bool dataModel = false);
    std::pair<Codira::Position, Codira::Position> deSerFuncLocation;
    bool CollectSerTypeInStore(
        const CHIR::Store *store, std::vector<TypeWithPosition> &serTypes, SerialiseMap &serMap, CHIR::LocalVar *ret);
    bool CollectSerTypeInInitApply(const CHIR::Apply *apply, CHIR::ImportedValue *imported,
        std::vector<TypeWithPosition> &serTypes, SerialiseMap &fieldSerMap);
    bool CollectSerTypeInAddApply(const CHIR::Apply *apply, CHIR::ImportedValue *imported,
        std::vector<TypeWithPosition> &serTypes, SerialiseMap &fieldSerMap);
    void CheckSerTypeInCallee(const CHIR::Apply *apply, std::vector<TypeWithPosition> &serTypes, SerialiseMap &serMap,
        SerialiseMap &fieldSerMap);
    void CheckSerTypeInArgs(const CHIR::Apply *apply, DataflowRuleGSER03Checker::TypeWithPosition &deserTypeStr,
        std::vector<TypeWithPosition> &serTypes, SerialiseMap &serMap, SerialiseMap &fieldSerMap);
};
} // namespace Codira::CodeCheck

#endif
