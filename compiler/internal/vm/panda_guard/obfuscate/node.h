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

#ifndef PANDA_GUARD_OBFUSCATE_NODE_H
#define PANDA_GUARD_OBFUSCATE_NODE_H

#include "array.h"
#include "class.h"
#include "entity.h"
#include "file_path.h"
#include "function.h"
#include "module_record.h"
#include "object.h"
#include "ui_decorator.h"
#include "annotation.h"

namespace panda::guard {

enum class NodeType {
    NONE = 0,
    SOURCE_FILE = 1,  // common source file
    JSON_FILE = 2,    // json file,
    ANNOTATION = 3,   // annotation
};

class Node final : public Entity {
public:
    Node(Program *program, const std::string &name)
        : Entity(program, name), moduleRecord_(program, name), sourceName_(name), obfSourceName_(name)
    {
    }

    /**
     * Find PkgName field in pandasm::Record
     */
    static bool FindPkgName(const pandasm::Record &record, std::string &pkgName);

    /**
     * record is json file
     * @param record record
     * @return true, false
     */
    static bool IsJsonFile(const pandasm::Record &record);

    /**
     * init type pkgName with record
     * @param record IR record
     */
    void InitWithRecord(const pandasm::Record &record);

    void Build() override;

    /**
     *  get origin IR program record
     * @return pandasm::Record
     */
    [[nodiscard]] pandasm::Record &GetRecord() const;

    /**
     * For Each Function In Node
     * 1. Functions
     * 2. For Each Function In Classes
     */
    void EnumerateFunctions(const std::function<FunctionTraver> &callback);

    /**
     * Update file name references in this node
     */
    void UpdateFileNameReferences();

    /**
     * Update SourceFile with obf file name
     * SourceFile: file path in "abc record's source_file"
     */
    void UpdateSourceFile(const std::string &file);

    /**
     * NameCache write interface, used when the current file does not need to be obfuscated
     */
    void WriteNameCache() override;

protected:
    void RefreshNeedUpdate() override;

    void Update() override;

private:
    void EnumerateIns(const InstructionInfo &info, Scope scope);

    /**
     * create function
     * @param info instruction info
     * @param scope function scope
     */
    void CreateFunction(const InstructionInfo &info, Scope scope);

    /**
     * create property for function
     * @param info instruction info
     */
    void CreateProperty(const InstructionInfo &info) const;

    void CreateObjectDecoratorProperty(const InstructionInfo &info);

    void CreateClass(const InstructionInfo &info, Scope scope);

    /**
     * e.g. method in class
     * class A { get b() {return 1;}}
     * bytecode:
     * defineclasswithbuffer
     * definemethod
     *
     * e.g. method in object
     * let obj = { get b() {return 1;}}
     * bytecode:
     * createobjectwithbuffer
     * definemethod
     */
    void CreateOuterMethod(const InstructionInfo &info);

    void CreateObject(const InstructionInfo &info, Scope scope);

    /**
     * when obj field is array, outer ins definepropertybyname will add an property for object
     * let obj = { arr:[] }
     * bytecode:
     * createobjectwithbuffer
     * sta v0
     * definepropertybyname arr v0
     * @param info instruction info
     */
    void CreateObjectOuterProperty(const InstructionInfo &info);

    /**
     * add name for export object
     *
     * export const obj = {};
     * pa:
     * createobjectwithbuffer
     *
     * stmodulevar 0x00(index for export name)
     *
     * @param info input ins(stmodulevar)
     */
    void AddNameForExportObject(const InstructionInfo &info);

    /**
     * update namespace member export
     * @param info input instruction
     */
    void UpdateExportForNamespaceMember(const InstructionInfo &info) const;

    /**
     * create array
     * e.g.
     * const array = [1, 2, 3];
     * byteCode:
     *  main_573 { 6 [ tag_value: 2, i32:1, tag_value: 2, i32:2, tag_value: 2, i32:3, ] }
     *  createarraywithbuffer 0x0 main_573
     * @param info instruction info
     */
    void CreateArray(const InstructionInfo &info);

    /**
     * this instruction crosses functions and cannot be analyzed by graph, therefore it has been added to the whitelist
     * e.g.
     * class A { ['field'] = 1; }
     * bytecode:
     *  func1: stlexvar
     *  func2: defineclasswithbuffer
     */
    static void FindStLexVarName(const InstructionInfo &info);

    void CreateUiDecorator(const InstructionInfo &info, Scope scope);

    void CreateFilePath();

    void CreateFilePathForDefaultMode();

    void CreateFilePathForNormalizedMode();

    void ExtractNames();

    void WriteFileCache(const std::string &filePath) override;

    void UpdateRecordTable();

    void UpdateFileNameDefine();

    /**
     * update record scopeNames
     */
    void UpdateScopeNames() const;

    /**
     * update record fields literalArrayIdx to updated recordName
     * fields: scopeNames, moduleRecordIdx
     */
    void UpdateFieldsLiteralArrayIdx();

    static void GetMethodNameInfo(const InstructionInfo &info, InstructionInfo &nameInfo);

public:
    ModuleRecord moduleRecord_;
    FilePath filepath_;
    std::unordered_map<std::string, std::shared_ptr<Function>> functionTable_ {};  // key: Function idx
    std::unordered_map<std::string, std::shared_ptr<Class>> classTable_ {};        // key: class literalArray idx
    std::unordered_map<std::string, std::shared_ptr<Object>> objectTable_ {};      // key: object literalArray idx
    std::vector<std::shared_ptr<UiDecorator>> uiDecorator_ {};
    std::vector<std::shared_ptr<Annotation>> annotations_ {};
    std::vector<std::shared_ptr<Array>> arrays_ {};
    std::set<std::string> strings_ {};
    std::string pkgName_;
    bool fileNameNeedUpdate_ = true;
    bool contentNeedUpdate_ = true;
    bool isNormalizedOhmUrl_ = false;
    std::string sourceName_;  // file path in "nameCache key" format
    std::string obfSourceName_;
    std::string sourceFile_;  // file path in "abc record's source_file"
    std::string obfSourceFile_;
    bool sourceFileUpdated_ = false;  // is sourceFile updated
    NodeType type_ = NodeType::NONE;  // node type
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_NODE_H
