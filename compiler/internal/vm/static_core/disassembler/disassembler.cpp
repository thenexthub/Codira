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

#include "disassembler.h"
#include "libarkfile/class_data_accessor.h"
#include "libarkfile/field_data_accessor.h"
#include "libarkfile/type_helper.h"
#include "libarkfile/literal_data_accessor.h"
#include "mangling.h"
#include "libarkbase/utils/logger.h"

#include <cstdint>
#include <iomanip>
#include <cstdlib>

#include "get_language_specific_metadata.inc"

namespace ark::disasm {

void Disassembler::Disassemble(std::string_view filenameIn, const bool quiet, const bool skipStrings)
{
    auto file = panda_file::File::Open(filenameIn);
    if (file == nullptr) {
        LOG(FATAL, DISASSEMBLER) << "> unable to open specified pandafile: <" << filenameIn << ">";
    }

    Disassemble(file, quiet, skipStrings);
}

void Disassembler::Disassemble(const panda_file::File &file, const bool quiet, const bool skipStrings)
{
    SetFile(file);
    DisassembleImpl(quiet, skipStrings);
}

void Disassembler::Disassemble(std::unique_ptr<const panda_file::File> &file, const bool quiet, const bool skipStrings)
{
    SetFile(file);
    DisassembleImpl(quiet, skipStrings);
}

void Disassembler::DisassembleImpl(const bool quiet, const bool skipStrings)
{
    prog_ = pandasm::Program {};

    recordNameToId_.clear();
    methodStaticNameToId_.clear();
    methodInstanceNameToId_.clear();

    skipStrings_ = skipStrings;
    quiet_ = quiet;

    progInfo_ = ProgInfo {};

    progAnn_ = ProgAnnotations {};

    GetLiteralArrays();
    GetRecords();

    AddExternalFieldsToRecords();
    GetLanguageSpecificMetadata();
}

void Disassembler::SetFile(std::unique_ptr<const panda_file::File> &file)
{
    fileHolder_.swap(file);
    file_ = fileHolder_.get();
}

void Disassembler::SetFile(const panda_file::File &file)
{
    fileHolder_.reset();
    file_ = &file;
}

void Disassembler::SetProfile(std::string_view fname)
{
    std::ifstream stm(fname.data(), std::ios::binary);
    if (!stm.is_open()) {
        LOG(FATAL, DISASSEMBLER) << "Cannot open profile file";
    }

    auto res = profiling::ReadProfile(stm, fileLanguage_);
    if (!res) {
        LOG(FATAL, DISASSEMBLER) << "Failed to deserialize: " << res.Error();
    }
    profile_ = res.Value();
}

void Disassembler::GetInsInfo(panda_file::MethodDataAccessor &mda, const panda_file::File::EntityId &codeId,
                              MethodInfo *methodInfo /* out */) const
{
    const static size_t FORMAT_WIDTH = 20;
    const static size_t INSTRUCTION_WIDTH = 2;

    panda_file::CodeDataAccessor codeAccessor(*file_, codeId);

    std::string methodName = mda.GetFullName();
    auto prof = profiling::INVALID_PROFILE;
    if (profile_ != profiling::INVALID_PROFILE) {
        prof = profiling::FindMethodInProfile(profile_, fileLanguage_, methodName);
    }

    auto insSz = codeAccessor.GetCodeSize();
    auto insArr = codeAccessor.GetInstructions();

    auto bcIns = BytecodeInstruction(insArr);
    auto bcInsLast = bcIns.JumpTo(insSz);

    while (bcIns.GetAddress() != bcInsLast.GetAddress()) {
        std::stringstream ss;

        uintptr_t bc = bcIns.GetAddress() - BytecodeInstruction(insArr).GetAddress();
        ss << "offset: 0x" << std::setfill('0') << std::setw(4U) << std::hex << bc;
        ss << ", " << std::setfill('.');

        BytecodeInstruction::Format format = bcIns.GetFormat();

        auto formatStr = std::string("[") + BytecodeInstruction::GetFormatString(format) + ']';
        ss << std::setw(FORMAT_WIDTH) << std::left << formatStr;

        ss << "[";

        const uint8_t *pc = bcIns.GetAddress();
        const size_t sz = bcIns.GetSize();

        for (size_t i = 0; i < sz; i++) {
            ss << "0x" << std::setw(INSTRUCTION_WIDTH) << std::setfill('0') << std::right << std::hex
               << static_cast<int>(pc[i]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

            if (i != sz - 1) {
                ss << " ";
            }
        }

        ss << "]";

        if (profile_ != profiling::INVALID_PROFILE && prof != profiling::INVALID_PROFILE) {
            auto profId = bcIns.GetProfileId();
            if (profId != -1) {
                ss << ", Profile: ";
                profiling::DumpProfile(prof, fileLanguage_, &bcIns, ss);
            }
        }

        methodInfo->instructionsInfo.push_back(ss.str());

        bcIns = bcIns.GetNext();
    }
}

void Disassembler::CollectInfo()
{
    LOG(DEBUG, DISASSEMBLER) << "\n[getting program info]\n";

    debugInfoExtractor_ = std::make_unique<panda_file::DebugInfoExtractor>(file_);

    for (const auto &pair : recordNameToId_) {
        GetRecordInfo(pair.second, &progInfo_.recordsInfo[pair.first]);
    }

    for (const auto &pair : methodStaticNameToId_) {
        GetMethodInfo(pair.second, &progInfo_.methodsStaticInfo[pair.first]);
    }
    for (const auto &pair : methodInstanceNameToId_) {
        GetMethodInfo(pair.second, &progInfo_.methodsInstanceInfo[pair.first]);
    }

    AddExternalFieldsInfoToRecords();
}

void Disassembler::Serialize(std::ostream &os, bool addSeparators, bool printInformation) const
{
    if (os.bad()) {
        LOG(DEBUG, DISASSEMBLER) << "> serialization failed. os bad\n";

        return;
    }

    SerializeFilename(os);
    SerializeLanguage(os);
    SerializeLitArrays(os, addSeparators);
    SerializeRecords(os, addSeparators, printInformation);
    SerializeMethods(os, addSeparators, printInformation);
}

void Disassembler::SerializePrintStartInfo(const pandasm::Function &method, std::ostringstream &headerSs) const
{
    headerSs << ".function " << method.returnType.GetPandasmName() << " " << method.name << "(";

    if (!method.params.empty()) {
        headerSs << method.params[0].type.GetPandasmName() << " a0";

        for (size_t i = 1; i < method.params.size(); i++) {
            headerSs << ", " << method.params[i].type.GetPandasmName() << " a" << (size_t)i;
        }
    }
    headerSs << ")";
}

void Disassembler::SerializeCheckEnd(const pandasm::Function &method, std::ostream &os, bool printMethodInfo,
                                     const MethodInfo *&methodInfo) const
{
    if (!method.catchBlocks.empty()) {
        os << "\n";

        for (const auto &catchBlock : method.catchBlocks) {
            Serialize(catchBlock, os);
            os << "\n";
        }
    }

    if (printMethodInfo) {
        ASSERT(methodInfo != nullptr);
        SerializeLineNumberTable(methodInfo->lineNumberTable, os);
        SerializeLocalVariableTable(methodInfo->localVariableTable, method, os);
    }

    os << "}\n\n";
}

size_t Disassembler::SerializeIfPrintMethodInfo(
    const pandasm::Function &method, bool printMethodInfo, std::ostringstream &headerSs, const MethodInfo *&methodInfo,
    std::map<std::string, ark::disasm::MethodInfo>::const_iterator &methodInfoIt) const
{
    size_t width = 0;
    if (printMethodInfo) {
        methodInfo = &methodInfoIt->second;

        for (const auto &i : method.ins) {
            if (i.ToString().size() > width) {
                width = i.ToString().size();
            }
        }

        headerSs << " # " << methodInfo->methodInfo << "\n#   CODE:";
    }

    headerSs << "\n";
    return width;
}

// CC-OFFNXT(huge_method) solid logic
void Disassembler::Serialize(const pandasm::Function &method, std::ostream &os, bool printInformation,
                             panda_file::LineNumberTable *lineTable) const
{
    std::ostringstream headerSs;
    SerializePrintStartInfo(method, headerSs);
    const std::string signature = pandasm::GetFunctionSignatureFromName(method.name, method.params);
    const auto methodIter = progAnn_.methodAnnotations.find(signature);
    if (methodIter != progAnn_.methodAnnotations.end()) {
        Serialize(*method.metadata, methodIter->second, headerSs);
    } else {
        Serialize(*method.metadata, {}, headerSs);
    }

    if (!method.HasImplementation()) {
        headerSs << "\n\n";
        os << headerSs.str();
        return;
    }

    headerSs << " {";

    const MethodInfo *methodInfo = nullptr;
    auto &methodsInfo = method.IsStatic() ? progInfo_.methodsStaticInfo : progInfo_.methodsInstanceInfo;
    auto methodInfoIt = methodsInfo.find(signature);
    bool printMethodInfo = printInformation && methodInfoIt != methodsInfo.end();
    size_t width = SerializeIfPrintMethodInfo(method, printMethodInfo, headerSs, methodInfo, methodInfoIt);

    auto headerSsStr = headerSs.str();
    size_t lineNumber = static_cast<size_t>(std::count(headerSsStr.begin(), headerSsStr.end(), '\n')) + 1;

    os << headerSsStr;

    for (size_t i = 0; i < method.ins.size(); i++) {
        std::ostringstream insSs;

        std::string ins = method.ins[i].ToString("", method.GetParamsNum() != 0, method.regsNum);
        if (method.ins[i].HasLabel()) {
            insSs << ins.substr(0, ins.find(": ")) << ":\n";
            ins.erase(0, ins.find(": ") + std::string(": ").length());
        }

        insSs << "\t";
        if (printMethodInfo) {
            insSs << std::setw(width) << std::left;
        }
        insSs << ins;
        if (printMethodInfo) {
            ASSERT(methodInfo != nullptr);
            insSs << " # " << methodInfo->instructionsInfo[i];
        }
        insSs << "\n";

        auto insSsStr = insSs.str();
        lineNumber += static_cast<size_t>(std::count(insSsStr.begin(), insSsStr.end(), '\n'));

        if (lineTable != nullptr) {
            auto &table = method.IsStatic() ? methodStaticNameToId_ : methodInstanceNameToId_;
            ASSERT(table.find(signature) != table.end());
            lineTable->emplace_back(
                panda_file::LineTableEntry {bytecodeOffset_.at({table.at(signature), i}), lineNumber - 1});
        }

        os << insSsStr;
    }

    SerializeCheckEnd(method, os, printMethodInfo, methodInfo);
}

inline bool Disassembler::IsSystemType(const std::string &typeName)
{
    bool isArrayType = typeName.back() == ']';
    bool isSyntheticType = typeName.front() == '{';
    bool isGlobal = typeName == "_GLOBAL";

    return isArrayType || isGlobal || isSyntheticType;
}

void Disassembler::GetRecord(pandasm::Record &record, const panda_file::File::EntityId &recordId)
{
    LOG(DEBUG, DISASSEMBLER) << "\n[getting record]\nid: " << recordId << " (0x" << std::hex << recordId << ")";

    record.name = GetFullRecordName(recordId);

    LOG(DEBUG, DISASSEMBLER) << "name: " << record.name;

    GetMetaData(&record, recordId);

    if (!file_->IsExternal(recordId)) {
        GetMethods(recordId);
        GetFields(record, recordId);
    }
}

void Disassembler::AddMethodToTables(const panda_file::File::EntityId &methodId)
{
    panda_file::MethodDataAccessor methodAccessor(*file_, methodId);
    pandasm::Function newMethod("", fileLanguage_);
    GetMethod(&newMethod, methodId);

    const auto signature = pandasm::GetFunctionSignatureFromName(newMethod.name, newMethod.params);
    auto isStatic = methodAccessor.IsStatic();
    auto &functionTable = isStatic ? prog_.functionStaticTable : prog_.functionInstanceTable;
    if (functionTable.find(signature) != functionTable.end()) {
        return;
    }

    prog_.functionSynonyms[newMethod.name].push_back(signature);
    functionTable.emplace(signature, std::move(newMethod));
}

void Disassembler::GetMethod(pandasm::Function *method, const panda_file::File::EntityId &methodId)
{
    LOG(DEBUG, DISASSEMBLER) << "\n[getting method]\nid: " << methodId << " (0x" << std::hex << methodId << ")";

    if (method == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved, but method ptr expected!";

        return;
    }

    panda_file::MethodDataAccessor methodAccessor(*file_, methodId);

    method->name = GetFullMethodName(methodId);

    LOG(DEBUG, DISASSEMBLER) << "name: " << method->name;

    GetParams(method, methodAccessor.GetProtoId());
    GetMetaData(method, methodId);

    const auto signature = pandasm::GetFunctionSignatureFromName(method->name, method->params);
    method->IsStatic() ? methodStaticNameToId_.emplace(signature, methodId)
                       : methodInstanceNameToId_.emplace(signature, methodId);

    if (!method->HasImplementation()) {
        return;
    }

    if (methodAccessor.GetCodeId().has_value()) {
        const IdList idList = GetInstructions(method, methodId, methodAccessor.GetCodeId().value());

        for (const auto &id : idList) {
            AddMethodToTables(id);
        }
    } else {
        LOG(ERROR, DISASSEMBLER) << "> error encountered at " << methodId << " (0x" << std::hex << methodId
                                 << "). implementation of method expected, but no \'CODE\' tag was found!";

        return;
    }
}

template <typename T>
void Disassembler::FillLiteralArrayData(pandasm::LiteralArray *litArray, const panda_file::LiteralTag &tag,
                                        const panda_file::LiteralDataAccessor::LiteralValue &value) const
{
    panda_file::File::EntityId id(std::get<uint32_t>(value));
    auto sp = file_->GetSpanFromId(id);
    auto len = panda_file::helpers::Read<sizeof(uint32_t)>(&sp);
    if (tag != panda_file::LiteralTag::ARRAY_STRING) {
        for (size_t i = 0; i < len; i++) {
            pandasm::LiteralArray::Literal lit;
            lit.tag = tag;
            lit.value = bit_cast<T>(panda_file::helpers::Read<sizeof(T)>(&sp));
            litArray->literals.push_back(lit);
        }
    } else {
        for (size_t i = 0; i < len; i++) {
            auto strId = panda_file::helpers::Read<sizeof(T)>(&sp);
            pandasm::LiteralArray::Literal lit;
            lit.tag = tag;
            lit.value = StringDataToString(file_->GetStringData(panda_file::File::EntityId(strId)));
            litArray->literals.push_back(lit);
        }
    }
}

void Disassembler::FillLiteralData(pandasm::LiteralArray *litArray,
                                   const panda_file::LiteralDataAccessor::LiteralValue &value,
                                   const panda_file::LiteralTag &tag) const
{
    pandasm::LiteralArray::Literal lit;
    if (tag == panda_file::LiteralTag::TAGVALUE) {
        return;
    }
    lit.tag = tag;
    lit.value = ParseLiteralValue(value, tag);
    litArray->literals.push_back(lit);
}

std::variant<bool, uint8_t, uint16_t, uint32_t, uint64_t, float, double, std::string> Disassembler::ParseLiteralValue(
    const panda_file::LiteralDataAccessor::LiteralValue &value, const panda_file::LiteralTag &tag) const
{
    switch (tag) {
        case panda_file::LiteralTag::BOOL:
            return std::get<bool>(value);
        case panda_file::LiteralTag::ACCESSOR:
        case panda_file::LiteralTag::NULLVALUE:
            return std::get<uint8_t>(value);
        case panda_file::LiteralTag::METHODAFFILIATE:
            return std::get<uint16_t>(value);
        case panda_file::LiteralTag::INTEGER:
            return std::get<uint32_t>(value);
        case panda_file::LiteralTag::BIGINT:
            return std::get<uint64_t>(value);
        case panda_file::LiteralTag::FLOAT:
            return std::get<float>(value);
        case panda_file::LiteralTag::DOUBLE:
            return std::get<double>(value);
        case panda_file::LiteralTag::STRING:
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GENERATORMETHOD:
            return ParseStringData(value);
        case panda_file::LiteralTag::LITERALARRAY:
            return ParseLiteralArrayData(value);
        default:
            LOG(ERROR, DISASSEMBLER) << "Unsupported literal with tag 0x" << std::hex << static_cast<uint32_t>(tag);
            UNREACHABLE();
    }
}

std::string Disassembler::ParseStringData(const panda_file::LiteralDataAccessor::LiteralValue &value) const
{
    auto strData = file_->GetStringData(panda_file::File::EntityId(std::get<uint32_t>(value)));
    return StringDataToString(strData);
}

std::string Disassembler::ParseLiteralArrayData(const panda_file::LiteralDataAccessor::LiteralValue &value) const
{
    std::stringstream ss;
    ss << "0x" << std::hex << std::get<uint32_t>(value);
    return ss.str();
}

void Disassembler::GetLiteralArrayByOffset(pandasm::LiteralArray *litArray, panda_file::File::EntityId offset) const
{
    panda_file::LiteralDataAccessor litArrayAccessor(*file_, file_->GetLiteralArraysId());
    auto processLiteralValue = [this, litArray](const panda_file::LiteralDataAccessor::LiteralValue &value,
                                                const panda_file::LiteralTag &tag) {
        switch (tag) {
            case panda_file::LiteralTag::ARRAY_U1: {
                FillLiteralArrayData<bool>(litArray, tag, value);
                break;
            }
            case panda_file::LiteralTag::ARRAY_I8:
            case panda_file::LiteralTag::ARRAY_U8: {
                FillLiteralArrayData<uint8_t>(litArray, tag, value);
                break;
            }
            case panda_file::LiteralTag::ARRAY_I16:
            case panda_file::LiteralTag::ARRAY_U16: {
                FillLiteralArrayData<uint16_t>(litArray, tag, value);
                break;
            }
            case panda_file::LiteralTag::ARRAY_I32:
            case panda_file::LiteralTag::ARRAY_U32: {
                FillLiteralArrayData<uint32_t>(litArray, tag, value);
                break;
            }
            case panda_file::LiteralTag::ARRAY_I64:
            case panda_file::LiteralTag::ARRAY_U64: {
                FillLiteralArrayData<uint64_t>(litArray, tag, value);
                break;
            }
            case panda_file::LiteralTag::ARRAY_F32: {
                FillLiteralArrayData<float>(litArray, tag, value);
                break;
            }
            case panda_file::LiteralTag::ARRAY_F64: {
                FillLiteralArrayData<double>(litArray, tag, value);
                break;
            }
            case panda_file::LiteralTag::ARRAY_STRING: {
                FillLiteralArrayData<uint32_t>(litArray, tag, value);
                break;
            }
            default: {
                FillLiteralData(litArray, value, tag);
                break;
            }
        }
    };

    litArrayAccessor.EnumerateLiteralVals(offset, processLiteralValue);
}

void Disassembler::GetLiteralArray(pandasm::LiteralArray *litArray, const size_t index)
{
    LOG(DEBUG, DISASSEMBLER) << "\n[getting literal array]\nindex: " << index;

    panda_file::LiteralDataAccessor litArrayAccessor(*file_, file_->GetLiteralArraysId());
    GetLiteralArrayByOffset(litArray, litArrayAccessor.GetLiteralArrayId(index));
}

void Disassembler::GetLiteralArrays()
{
    const auto litArraysId = file_->GetLiteralArraysId();

    LOG(DEBUG, DISASSEMBLER) << "\n[getting literal arrays]\nid: " << litArraysId << " (0x" << std::hex << litArraysId
                             << ")";

    panda_file::LiteralDataAccessor litArrayAccessor(*file_, litArraysId);
    size_t numLitarrays = litArrayAccessor.GetLiteralNum();
    for (size_t index = 0; index < numLitarrays; index++) {
        ark::pandasm::LiteralArray litAr;
        GetLiteralArray(&litAr, index);
        prog_.literalarrayTable.emplace(std::to_string(index), litAr);
    }
}

void Disassembler::GetRecords()
{
    LOG(DEBUG, DISASSEMBLER) << "\n[getting records]\n";

    const auto classIdx = file_->GetClasses();

    for (size_t i = 0; i < classIdx.size(); i++) {
        uint32_t classId = classIdx[i];
        auto classOff = file_->GetHeader()->classIdxOff + sizeof(uint32_t) * i;

        if (classId > file_->GetHeader()->fileSize) {
            LOG(ERROR, DISASSEMBLER) << "> error encountered in record at " << classOff << " (0x" << std::hex
                                     << classOff << "). binary file corrupted. record offset (0x" << classId
                                     << ") out of bounds (0x" << file_->GetHeader()->fileSize << ")!";
            break;
        }

        const panda_file::File::EntityId recordId {classId};
        auto language = GetRecordLanguage(recordId);
        if (language != fileLanguage_) {
            if (fileLanguage_ == panda_file::SourceLang::PANDA_ASSEMBLY) {
                fileLanguage_ = language;
            } else if (language != panda_file::SourceLang::PANDA_ASSEMBLY) {
                LOG(ERROR, DISASSEMBLER) << "> possible error encountered in record at" << classOff << " (0x"
                                         << std::hex << classOff << "). record's language  ("
                                         << panda_file::LanguageToString(language)
                                         << ")  differs from file's language ("
                                         << panda_file::LanguageToString(fileLanguage_) << ")!";
            }
        }

        pandasm::Record record("", fileLanguage_);
        GetRecord(record, recordId);

        if (prog_.recordTable.find(record.name) == prog_.recordTable.end()) {
            recordNameToId_.emplace(record.name, recordId);
            prog_.recordTable.emplace(record.name, std::move(record));
        }
    }
}

void Disassembler::GetField(pandasm::Field &field, const panda_file::FieldDataAccessor &fieldAccessor)
{
    panda_file::File::EntityId fieldNameId = fieldAccessor.GetNameId();
    field.name = StringDataToString(file_->GetStringData(fieldNameId));

    uint32_t fieldType = fieldAccessor.GetType();
    field.type = FieldTypeToPandasmType(fieldType);

    GetMetaData(&field, fieldAccessor.GetFieldId());
}

void Disassembler::GetFields(pandasm::Record &record, const panda_file::File::EntityId &recordId)
{
    panda_file::ClassDataAccessor classAccessor {*file_, recordId};

    classAccessor.EnumerateFields([&](panda_file::FieldDataAccessor &fieldAccessor) -> void {
        pandasm::Field field(fileLanguage_);

        GetField(field, fieldAccessor);

        record.fieldList.push_back(std::move(field));
    });
}

void Disassembler::AddExternalFieldsToRecords()
{
    for (auto &[recordName, record] : prog_.recordTable) {
        auto iter = externalFieldTable_.find(recordName);
        if (iter == externalFieldTable_.end() || iter->second.empty()) {
            continue;
        }
        for (auto &fieldIter : iter->second) {
            record.fieldList.push_back(std::move(fieldIter));
        }
        externalFieldTable_.erase(recordName);
    }
}

void Disassembler::AddExternalFieldsInfoToRecords()
{
    for (auto &[recordName, recordInfo] : progInfo_.recordsInfo) {
        auto iter = externalFieldsInfoTable_.find(recordName);
        if (iter == externalFieldsInfoTable_.end() || iter->second.empty()) {
            continue;
        }
        for (auto &info : iter->second) {
            recordInfo.fieldsInfo.push_back(std::move(info));
        }
        externalFieldsInfoTable_.erase(recordName);
    }
}

void Disassembler::GetMethods(const panda_file::File::EntityId &recordId)
{
    panda_file::ClassDataAccessor classAccessor {*file_, recordId};

    classAccessor.EnumerateMethods([&](panda_file::MethodDataAccessor &methodAccessor) -> void {
        AddMethodToTables(methodAccessor.GetMethodId());
    });
}

void Disassembler::GetParams(pandasm::Function *method, const panda_file::File::EntityId &protoId) const
{
    /// frame size - 2^16 - 1
    static const uint32_t MAX_ARG_NUM = 0xFFFF;

    LOG(DEBUG, DISASSEMBLER) << "[getting params]\nproto id: " << protoId << " (0x" << std::hex << protoId << ")";

    if (method == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved, but method ptr expected!";

        return;
    }

    panda_file::ProtoDataAccessor protoAccessor(*file_, protoId);

    if (protoAccessor.GetNumArgs() > MAX_ARG_NUM) {
        LOG(ERROR, DISASSEMBLER) << "> error encountered at " << protoId << " (0x" << std::hex << protoId
                                 << "). number of function's arguments (" << std::dec << protoAccessor.GetNumArgs()
                                 << ") exceeds MAX_ARG_NUM (" << MAX_ARG_NUM << ") !";

        return;
    }

    size_t refIdx = 0;
    method->returnType = PFTypeToPandasmType(protoAccessor.GetReturnType(), protoAccessor, refIdx);

    for (size_t i = 0; i < protoAccessor.GetNumArgs(); i++) {
        auto argType = PFTypeToPandasmType(protoAccessor.GetArgType(i), protoAccessor, refIdx);
        method->params.emplace_back(argType, fileLanguage_);
    }
}

LabelTable Disassembler::GetExceptions(pandasm::Function *method, panda_file::File::EntityId methodId,
                                       panda_file::File::EntityId codeId) const
{
    LOG(DEBUG, DISASSEMBLER) << "[getting exceptions]\ncode id: " << codeId << " (0x" << std::hex << codeId << ")";

    if (method == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved, but method ptr expected!\n";
        return LabelTable {};
    }

    panda_file::CodeDataAccessor codeAccessor(*file_, codeId);

    const auto bcIns = BytecodeInstruction(codeAccessor.GetInstructions());
    const auto bcInsLast = bcIns.JumpTo(codeAccessor.GetCodeSize());

    size_t tryIdx = 0;
    LabelTable labelTable {};
    codeAccessor.EnumerateTryBlocks([&](panda_file::CodeDataAccessor::TryBlock &tryBlock) {
        pandasm::Function::CatchBlock catchBlockPa {};
        if (!LocateTryBlock(bcIns, bcInsLast, tryBlock, &catchBlockPa, &labelTable, tryIdx)) {
            return false;
        }
        size_t catchIdx = 0;
        tryBlock.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catchBlock) {
            auto classIdx = catchBlock.GetTypeIdx();
            if (classIdx == panda_file::INVALID_INDEX) {
                catchBlockPa.exceptionRecord = "";
            } else {
                const auto classId = file_->ResolveClassIndex(methodId, classIdx);
                catchBlockPa.exceptionRecord = GetFullRecordName(classId);
            }
            if (!LocateCatchBlock(bcIns, bcInsLast, catchBlock, &catchBlockPa, &labelTable, tryIdx, catchIdx)) {
                return false;
            }

            method->catchBlocks.emplace_back(catchBlockPa);
            catchBlockPa.catchBeginLabel.clear();
            catchBlockPa.catchEndLabel.clear();
            catchIdx++;

            return true;
        });
        tryIdx++;

        return true;
    });

    return labelTable;
}

static size_t GetBytecodeInstructionNumber(BytecodeInstruction bcInsFirst, BytecodeInstruction bcInsCur)
{
    size_t count = 0;

    while (bcInsFirst.GetAddress() != bcInsCur.GetAddress()) {
        count++;
        bcInsFirst = bcInsFirst.GetNext();
        if (bcInsFirst.GetAddress() > bcInsCur.GetAddress()) {
            return std::numeric_limits<size_t>::max();
        }
    }

    return count;
}

// CC-OFFNXT(G.FUN.01) solid logic
bool Disassembler::LocateTryBlock(const BytecodeInstruction &bcIns, const BytecodeInstruction &bcInsLast,
                                  const panda_file::CodeDataAccessor::TryBlock &tryBlock,
                                  pandasm::Function::CatchBlock *catchBlockPa, LabelTable *labelTable,
                                  size_t tryIdx) const
{
    const auto tryBeginBcIns = bcIns.JumpTo(tryBlock.GetStartPc());
    const auto tryEndBcIns = bcIns.JumpTo(tryBlock.GetStartPc() + tryBlock.GetLength());

    const size_t tryBeginIdx = GetBytecodeInstructionNumber(bcIns, tryBeginBcIns);
    const size_t tryEndIdx = GetBytecodeInstructionNumber(bcIns, tryEndBcIns);

    const bool tryBeginOffsetInRange = bcInsLast.GetAddress() > tryBeginBcIns.GetAddress();
    const bool tryEndOffsetInRange = bcInsLast.GetAddress() >= tryEndBcIns.GetAddress();
    const bool tryBeginOffsetValid = tryBeginIdx != std::numeric_limits<size_t>::max();
    const bool tryEndOffsetValid = tryEndIdx != std::numeric_limits<size_t>::max();

    if (!tryBeginOffsetInRange || !tryBeginOffsetValid) {
        LOG(ERROR, DISASSEMBLER) << "> invalid try block begin offset! address is: 0x" << std::hex
                                 << tryBeginBcIns.GetAddress();
        return false;
    }

    auto itBegin = labelTable->find(tryBeginIdx);
    if (itBegin == labelTable->end()) {
        std::stringstream ss {};
        ss << "try_begin_label_" << tryIdx;
        catchBlockPa->tryBeginLabel = ss.str();
        labelTable->insert(std::pair<size_t, std::string>(tryBeginIdx, ss.str()));
    } else {
        catchBlockPa->tryBeginLabel = itBegin->second;
    }

    if (!tryEndOffsetInRange || !tryEndOffsetValid) {
        LOG(ERROR, DISASSEMBLER) << "> invalid try block end offset! address is: 0x" << std::hex
                                 << tryEndBcIns.GetAddress();
        return false;
    }

    auto itEnd = labelTable->find(tryEndIdx);
    if (itEnd == labelTable->end()) {
        std::stringstream ss {};
        ss << "try_end_label_" << tryIdx;
        catchBlockPa->tryEndLabel = ss.str();
        labelTable->insert(std::pair<size_t, std::string>(tryEndIdx, ss.str()));
    } else {
        catchBlockPa->tryEndLabel = itEnd->second;
    }

    return true;
}

bool Disassembler::LocateCatchBlock(const BytecodeInstruction &bcIns, const BytecodeInstruction &bcInsLast,
                                    const panda_file::CodeDataAccessor::CatchBlock &catchBlock,
                                    pandasm::Function::CatchBlock *catchBlockPa, LabelTable *labelTable, size_t tryIdx,
                                    size_t catchIdx) const
{
    const auto handlerBeginOffset = catchBlock.GetHandlerPc();
    const auto handlerEndOffset = handlerBeginOffset + catchBlock.GetCodeSize();

    const auto handlerBeginBcIns = bcIns.JumpTo(handlerBeginOffset);
    const auto handlerEndBcIns = bcIns.JumpTo(handlerEndOffset);

    const size_t handlerBeginIdx = GetBytecodeInstructionNumber(bcIns, handlerBeginBcIns);
    const size_t handlerEndIdx = GetBytecodeInstructionNumber(bcIns, handlerEndBcIns);

    const bool handlerBeginOffsetInRange = bcInsLast.GetAddress() > handlerBeginBcIns.GetAddress();
    const bool handlerEndOffsetInRange = bcInsLast.GetAddress() > handlerEndBcIns.GetAddress();
    const bool handlerEndPresent = catchBlock.GetCodeSize() != 0;
    const bool handlerBeginOffsetValid = handlerBeginIdx != std::numeric_limits<size_t>::max();
    const bool handlerEndOffsetValid = handlerEndIdx != std::numeric_limits<size_t>::max();

    if (!handlerBeginOffsetInRange || !handlerBeginOffsetValid) {
        LOG(ERROR, DISASSEMBLER) << "> invalid catch block begin offset! address is: 0x" << std::hex
                                 << handlerBeginBcIns.GetAddress();
        return false;
    }

    auto itBegin = labelTable->find(handlerBeginIdx);
    if (itBegin == labelTable->end()) {
        std::stringstream ss {};
        ss << "handler_begin_label_" << tryIdx << "_" << catchIdx;
        catchBlockPa->catchBeginLabel = ss.str();
        labelTable->insert(std::pair<size_t, std::string>(handlerBeginIdx, ss.str()));
    } else {
        catchBlockPa->catchBeginLabel = itBegin->second;
    }

    if (!handlerEndOffsetInRange || !handlerEndOffsetValid) {
        LOG(ERROR, DISASSEMBLER) << "> invalid catch block end offset! address is: 0x" << std::hex
                                 << handlerEndBcIns.GetAddress();
        return false;
    }

    if (handlerEndPresent) {
        auto itEnd = labelTable->find(handlerEndIdx);
        if (itEnd == labelTable->end()) {
            std::stringstream ss {};
            ss << "handler_end_label_" << tryIdx << "_" << catchIdx;
            catchBlockPa->catchEndLabel = ss.str();
            labelTable->insert(std::pair<size_t, std::string>(handlerEndIdx, ss.str()));
        } else {
            catchBlockPa->catchEndLabel = itEnd->second;
        }
    }

    return true;
}

template <typename T>
static void SetEntityAttribute(T *entity, const std::function<bool()> &shouldSet, std::string_view attribute)
{
    if (shouldSet()) {
        auto err = entity->metadata->SetAttribute(attribute);
        if (err.has_value()) {
            LOG(ERROR, DISASSEMBLER) << err.value().GetMessage();
        }
    }
}

template <typename T>
static void SetEntityAttributeValue(T *entity, const std::function<bool()> &shouldSet, std::string_view attribute,
                                    const char *value)
{
    if (shouldSet()) {
        auto err = entity->metadata->SetAttributeValue(attribute, value);
        if (err.has_value()) {
            LOG(ERROR, DISASSEMBLER) << err.value().GetMessage();
        }
    }
}

void Disassembler::GetMetaData(pandasm::Function *method, const panda_file::File::EntityId &methodId) const
{
    LOG(DEBUG, DISASSEMBLER) << "[getting metadata]\nmethod id: " << methodId << " (0x" << std::hex << methodId << ")";

    if (method == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved, but method ptr expected!";

        return;
    }

    panda_file::MethodDataAccessor methodAccessor(*file_, methodId);

    const auto methodNameRaw = StringDataToString(file_->GetStringData(methodAccessor.GetNameId()));

    if (!methodAccessor.IsStatic()) {
        const auto className = StringDataToString(file_->GetStringData(methodAccessor.GetClassId()));
        auto thisType = pandasm::Type::FromDescriptor(className);

        LOG(DEBUG, DISASSEMBLER) << "method (raw: \'" << methodNameRaw
                                 << "\') is not static. emplacing self-argument of type " << thisType.GetName();

        method->params.insert(method->params.begin(), pandasm::Function::Parameter(thisType, fileLanguage_));
    }
    SetEntityAttribute(
        method, [&methodAccessor]() { return methodAccessor.IsStatic(); }, "static");

    SetEntityAttribute(
        method, [this, &methodAccessor]() { return file_->IsExternal(methodAccessor.GetMethodId()); }, "external");

    SetEntityAttribute(
        method, [&methodAccessor]() { return methodAccessor.IsNative(); }, "native");

    SetEntityAttribute(
        method, [&methodAccessor]() { return methodAccessor.IsAbstract(); }, "noimpl");

    SetEntityAttribute(
        method, [&methodAccessor]() { return methodAccessor.IsVarArgs(); }, "varargs");

    SetEntityAttributeValue(
        method, [&methodAccessor]() { return methodAccessor.IsPublic(); }, "access.function", "public");

    SetEntityAttributeValue(
        method, [&methodAccessor]() { return methodAccessor.IsProtected(); }, "access.function", "protected");

    SetEntityAttributeValue(
        method, [&methodAccessor]() { return methodAccessor.IsPrivate(); }, "access.function", "private");

    SetEntityAttribute(
        method, [&methodAccessor]() { return methodAccessor.IsFinal(); }, "final");

    std::string ctorName = ark::panda_file::GetCtorName(fileLanguage_);
    std::string cctorName = ark::panda_file::GetCctorName(fileLanguage_);

    const bool isCtor = (methodNameRaw == ctorName);
    const bool isCctor = (methodNameRaw == cctorName);

    if (isCtor) {
        method->metadata->SetAttribute("ctor");
        method->name.replace(method->name.find(ctorName), ctorName.length(), "_ctor_");
    } else if (isCctor) {
        method->metadata->SetAttribute("cctor");
        method->name.replace(method->name.find(cctorName), cctorName.length(), "_cctor_");
    }
}

void Disassembler::GetMetaData(pandasm::Record *record, const panda_file::File::EntityId &recordId) const
{
    LOG(DEBUG, DISASSEMBLER) << "[getting metadata]\nrecord id: " << recordId << " (0x" << std::hex << recordId << ")";

    if (record == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved, but record ptr expected!";

        return;
    }

    SetEntityAttribute(
        record, [this, recordId]() { return file_->IsExternal(recordId); }, "external");

    auto external = file_->IsExternal(recordId);
    if (!external) {
        auto cda = panda_file::ClassDataAccessor {*file_, recordId};
        SetEntityAttributeValue(
            record, [&cda]() { return cda.IsPublic(); }, "access.record", "public");

        SetEntityAttributeValue(
            record, [&cda]() { return cda.IsProtected(); }, "access.record", "protected");

        SetEntityAttributeValue(
            record, [&cda]() { return cda.IsPrivate(); }, "access.record", "private");

        SetEntityAttribute(
            record, [&cda]() { return cda.IsFinal(); }, "final");
    }
}

template <typename T, pandasm::Value::Type VALUE_TYPE>
void Disassembler::SetMetadata(panda_file::FieldDataAccessor &accessor, pandasm::Field *field) const
{
    std::optional<T> val = accessor.GetValue<T>();
    if (val.has_value()) {
        field->metadata->SetValue(pandasm::ScalarValue::Create<VALUE_TYPE>(val.value()));
    }
}

void Disassembler::GetMetadataFieldValue(panda_file::FieldDataAccessor &fieldAccessor, pandasm::Field *field) const
{
    static const std::unordered_map<panda_file::Type::TypeId,
                                    std::function<void(panda_file::FieldDataAccessor &, pandasm::Field *)>>
        HANDLERS = {
            {panda_file::Type::TypeId::U1,
             [this](auto &accessor, auto *f) { SetMetadata<bool, pandasm::Value::Type::U1>(accessor, f); }},
            {panda_file::Type::TypeId::U8,
             [this](auto &accessor, auto *f) { SetMetadata<uint8_t, pandasm::Value::Type::U8>(accessor, f); }},
            {panda_file::Type::TypeId::U16,
             [this](auto &accessor, auto *f) { SetMetadata<uint16_t, pandasm::Value::Type::U16>(accessor, f); }},
            {panda_file::Type::TypeId::U32,
             [this](auto &accessor, auto *f) { SetMetadata<uint32_t, pandasm::Value::Type::U32>(accessor, f); }},
            {panda_file::Type::TypeId::F64,
             [this](auto &accessor, auto *f) { SetMetadata<double, pandasm::Value::Type::F64>(accessor, f); }},
            {panda_file::Type::TypeId::I8,
             [this](auto &accessor, auto *f) { SetMetadata<int8_t, pandasm::Value::Type::I8>(accessor, f); }},
            {panda_file::Type::TypeId::I16,
             [this](auto &accessor, auto *f) { SetMetadata<int16_t, pandasm::Value::Type::I16>(accessor, f); }},
            {panda_file::Type::TypeId::I32,
             [this](auto &accessor, auto *f) { SetMetadata<int32_t, pandasm::Value::Type::I32>(accessor, f); }},
            {panda_file::Type::TypeId::I64,
             [this](auto &accessor, auto *f) { SetMetadata<int64_t, pandasm::Value::Type::I64>(accessor, f); }},
        };

    auto it = HANDLERS.find(field->type.GetId());
    if (it != HANDLERS.end()) {
        it->second(fieldAccessor, field);
    } else if (field->type.GetId() == panda_file::Type::TypeId::REFERENCE &&
               field->type.GetName() == "std/core/String") {
        std::optional<uint32_t> stringOffsetVal = fieldAccessor.GetValue<uint32_t>();
        if (stringOffsetVal.has_value()) {
            std::string_view val {reinterpret_cast<const char *>(
                file_->GetStringData(panda_file::File::EntityId(stringOffsetVal.value())).data)};
            field->metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::STRING>(val));
        }
    } else if (field->type.GetRank() > 0) {
        std::optional<uint32_t> litarrayOffsetVal = fieldAccessor.GetValue<uint32_t>();
        if (litarrayOffsetVal.has_value()) {
            field->metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::LITERALARRAY>(
                std::string_view {std::to_string(litarrayOffsetVal.value())}));
        }
    }
}

void Disassembler::GetMetaData(pandasm::Field *field, const panda_file::File::EntityId &fieldId) const
{
    LOG(DEBUG, DISASSEMBLER) << "[getting metadata]\nfield id: " << fieldId << " (0x" << std::hex << fieldId << ")";

    if (field == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved, but method ptr expected!";

        return;
    }

    panda_file::FieldDataAccessor fieldAccessor(*file_, fieldId);

    SetEntityAttribute(
        field, [&fieldAccessor]() { return fieldAccessor.IsExternal(); }, "external");

    SetEntityAttribute(
        field, [&fieldAccessor]() { return fieldAccessor.IsStatic(); }, "static");

    SetEntityAttributeValue(
        field, [&fieldAccessor]() { return fieldAccessor.IsPublic(); }, "access.field", "public");

    SetEntityAttributeValue(
        field, [&fieldAccessor]() { return fieldAccessor.IsProtected(); }, "access.field", "protected");

    SetEntityAttributeValue(
        field, [&fieldAccessor]() { return fieldAccessor.IsPrivate(); }, "access.field", "private");

    SetEntityAttribute(
        field, [&fieldAccessor]() { return fieldAccessor.IsFinal(); }, "final");
    GetMetadataFieldValue(fieldAccessor, field);
}

std::string Disassembler::AnnotationTagToString(const char tag) const
{
    static const std::unordered_map<char, std::string> TAG_TO_STRING = {{'1', "u1"},
                                                                        {'2', "i8"},
                                                                        {'3', "u8"},
                                                                        {'4', "i16"},
                                                                        {'5', "u16"},
                                                                        {'6', "i32"},
                                                                        {'7', "u32"},
                                                                        {'8', "i64"},
                                                                        {'9', "u64"},
                                                                        {'A', "f32"},
                                                                        {'B', "f64"},
                                                                        {'C', "string"},
                                                                        {'D', "record"},
                                                                        {'E', "method"},
                                                                        {'F', "enum"},
                                                                        {'G', "annotation"},
                                                                        {'J', "method_handle"},
                                                                        {'H', "array"},
                                                                        {'K', "u1[]"},
                                                                        {'L', "i8[]"},
                                                                        {'M', "u8[]"},
                                                                        {'N', "i16[]"},
                                                                        {'O', "u16[]"},
                                                                        {'P', "i32[]"},
                                                                        {'Q', "u32[]"},
                                                                        {'R', "i64[]"},
                                                                        {'S', "u64[]"},
                                                                        {'T', "f32[]"},
                                                                        {'U', "f64[]"},
                                                                        {'V', "string[]"},
                                                                        {'W', "record[]"},
                                                                        {'X', "method[]"},
                                                                        {'Y', "enum[]"},
                                                                        {'Z', "annotation[]"},
                                                                        {'@', "method_handle[]"},
                                                                        {'*', "nullptr_string"}};

    return TAG_TO_STRING.at(tag);
}

std::string Disassembler::ScalarValueToString(const panda_file::ScalarValue &value, const std::string &type)
{
    std::stringstream ss;

    if (type == "i8") {
        auto res = value.Get<int8_t>();
        ss << static_cast<int>(res);
    } else if (type == "u1" || type == "u8") {
        auto res = value.Get<uint8_t>();
        ss << static_cast<unsigned int>(res);
    } else if (type == "i16") {
        ss << value.Get<int16_t>();
    } else if (type == "u16") {
        ss << value.Get<uint16_t>();
    } else if (type == "i32") {
        ss << value.Get<int32_t>();
    } else if (type == "u32") {
        ss << value.Get<uint32_t>();
    } else if (type == "i64") {
        ss << value.Get<int64_t>();
    } else if (type == "u64") {
        ss << value.Get<uint64_t>();
    } else if (type == "f32") {
        ss << value.Get<float>();
    } else if (type == "f64") {
        ss << value.Get<double>();
    } else if (type == "string") {
        const auto id = value.Get<panda_file::File::EntityId>();
        ss << "\"" << StringDataToString(file_->GetStringData(id)) << "\"";
    } else if (type == "record") {
        const auto id = value.Get<panda_file::File::EntityId>();
        ss << GetFullRecordName(id);
    } else if (type == "method") {
        const auto id = value.Get<panda_file::File::EntityId>();
        AddMethodToTables(id);
        ss << GetMethodSignature(id);
    } else if (type == "enum") {
        const auto id = value.Get<panda_file::File::EntityId>();
        panda_file::FieldDataAccessor fieldAccessor(*file_, id);
        ss << GetFullRecordName(fieldAccessor.GetClassId()) << "."
           << StringDataToString(file_->GetStringData(fieldAccessor.GetNameId()));
    } else if (type == "annotation") {
        const auto id = value.Get<panda_file::File::EntityId>();
        ss << "id_" << id;
    } else if (type == "void") {
        return std::string();
    } else if (type == "method_handle") {
    } else if (type == "nullptr_string") {
        ss << static_cast<uint32_t>(0);
    }

    return ss.str();
}

std::string Disassembler::ArrayValueToString(const panda_file::ArrayValue &value, const std::string &type,
                                             const size_t idx)
{
    std::stringstream ss;

    if (type == "i8") {
        auto res = value.Get<int8_t>(idx);
        ss << static_cast<int>(res);
    } else if (type == "u1" || type == "u8") {
        auto res = value.Get<uint8_t>(idx);
        ss << static_cast<unsigned int>(res);
    } else if (type == "i16") {
        ss << (value.Get<int16_t>(idx));
    } else if (type == "u16") {
        ss << (value.Get<uint16_t>(idx));
    } else if (type == "i32") {
        ss << (value.Get<int32_t>(idx));
    } else if (type == "u32") {
        ss << (value.Get<uint32_t>(idx));
    } else if (type == "i64") {
        ss << (value.Get<int64_t>(idx));
    } else if (type == "u64") {
        ss << (value.Get<uint64_t>(idx));
    } else if (type == "f32") {
        ss << value.Get<float>(idx);
    } else if (type == "f64") {
        ss << value.Get<double>(idx);
    } else if (type == "string") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        ss << '\"' << StringDataToString(file_->GetStringData(id)) << '\"';
    } else if (type == "record") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        ss << GetFullRecordName(id);
    } else if (type == "method") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        AddMethodToTables(id);
        ss << GetMethodSignature(id);
    } else if (type == "enum") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        panda_file::FieldDataAccessor fieldAccessor(*file_, id);
        ss << GetFullRecordName(fieldAccessor.GetClassId()) << "."
           << StringDataToString(file_->GetStringData(fieldAccessor.GetNameId()));
    } else if (type == "annotation") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        ss << "id_" << id;
    } else if (type == "method_handle") {
    } else if (type == "nullptr_string") {
    }

    return ss.str();
}

std::string Disassembler::GetFullMethodName(const panda_file::File::EntityId &methodId) const
{
    ark::panda_file::MethodDataAccessor methodAccessor(*file_, methodId);

    const auto methodNameRaw = StringDataToString(file_->GetStringData(methodAccessor.GetNameId()));

    std::string className = GetFullRecordName(methodAccessor.GetClassId());
    if (IsSystemType(className)) {
        className = "";
    } else {
        className += ".";
    }

    return className + methodNameRaw;
}

std::string Disassembler::GetMethodSignature(const panda_file::File::EntityId &methodId) const
{
    ark::panda_file::MethodDataAccessor methodAccessor(*file_, methodId);

    pandasm::Function method(GetFullMethodName(methodId), fileLanguage_);
    GetParams(&method, methodAccessor.GetProtoId());
    GetMetaData(&method, methodId);

    auto res = pandasm::GetFunctionSignatureFromName(method.name, method.params);
    return method.IsStatic() ? "<static> " + res : res;
}

std::string Disassembler::GetFullRecordName(const panda_file::File::EntityId &classId) const
{
    std::string name = StringDataToString(file_->GetStringData(classId));
    if (name.empty()) {
        LOG(FATAL, DISASSEMBLER) << "Record name is empty";
    }
    auto type = pandasm::Type::FromDescriptor(name);
    if (type.GetName().empty()) {
        LOG(FATAL, DISASSEMBLER) << "Type name is empty";
    }
    type = pandasm::Type(type.GetNameWithoutRank(), type.GetRank());

    return type.GetPandasmName();
}

static constexpr size_t DEFAULT_OFFSET_WIDTH = 4;

static void GetFieldInfo(const panda_file::FieldDataAccessor &fieldAccessor, std::stringstream &ss)
{
    ss << "offset: 0x" << std::setfill('0') << std::setw(DEFAULT_OFFSET_WIDTH) << std::hex << fieldAccessor.GetFieldId()
       << ", type: 0x" << fieldAccessor.GetType();
}

static std::string GetFieldInfo(const panda_file::FieldDataAccessor &fieldAccessor)
{
    std::stringstream ss;
    ss << "offset: 0x" << std::setfill('0') << std::setw(DEFAULT_OFFSET_WIDTH) << std::hex << fieldAccessor.GetFieldId()
       << ", type: 0x" << fieldAccessor.GetType();
    return ss.str();
}

void Disassembler::GetRecordInfo(const panda_file::File::EntityId &recordId, RecordInfo *recordInfo) const
{
    if (file_->IsExternal(recordId)) {
        return;
    }

    panda_file::ClassDataAccessor classAccessor {*file_, recordId};
    std::stringstream ss;

    ss << "offset: 0x" << std::setfill('0') << std::setw(DEFAULT_OFFSET_WIDTH) << std::hex << classAccessor.GetClassId()
       << ", size: 0x" << std::setfill('0') << std::setw(DEFAULT_OFFSET_WIDTH) << classAccessor.GetSize() << " ("
       << std::dec << classAccessor.GetSize() << ")";

    recordInfo->recordInfo = ss.str();
    ss.str(std::string());

    classAccessor.EnumerateFields([&](panda_file::FieldDataAccessor &fieldAccessor) -> void {
        GetFieldInfo(fieldAccessor, ss);

        recordInfo->fieldsInfo.push_back(ss.str());

        ss.str(std::string());
    });
}

void Disassembler::GetMethodInfo(const panda_file::File::EntityId &methodId, MethodInfo *methodInfo) const
{
    panda_file::MethodDataAccessor methodAccessor {*file_, methodId};
    std::stringstream ss;

    ss << "offset: 0x" << std::setfill('0') << std::setw(DEFAULT_OFFSET_WIDTH) << std::hex
       << methodAccessor.GetMethodId();

    if (methodAccessor.GetCodeId().has_value()) {
        ss << ", code offset: 0x" << std::setfill('0') << std::setw(DEFAULT_OFFSET_WIDTH) << std::hex
           << methodAccessor.GetCodeId().value();

        GetInsInfo(methodAccessor, methodAccessor.GetCodeId().value(), methodInfo);
    } else {
        ss << ", <no code>";
    }

    auto profileSize = methodAccessor.GetProfileSize();
    if (profileSize) {
        ss << ", profile size: " << profileSize.value();
    }

    methodInfo->methodInfo = ss.str();

    if (methodAccessor.GetCodeId()) {
        ASSERT(debugInfoExtractor_ != nullptr);
        methodInfo->lineNumberTable = debugInfoExtractor_->GetLineNumberTable(methodId);
        methodInfo->localVariableTable = debugInfoExtractor_->GetLocalVariableTable(methodId);

        // Add information about parameters into the table
        panda_file::CodeDataAccessor codeda(*file_, methodAccessor.GetCodeId().value());
        auto argIdx = static_cast<int32_t>(codeda.GetNumVregs());
        uint32_t codeSize = codeda.GetCodeSize();
        for (const auto &info : debugInfoExtractor_->GetParameterInfo(methodId)) {
            panda_file::LocalVariableInfo argInfo {info.name, info.signature, "", argIdx++, 0, codeSize};
            methodInfo->localVariableTable.emplace_back(argInfo);
        }
    }
}

void Disassembler::Serialize(const std::string &name, const pandasm::LiteralArray &litArray, std::ostream &os) const
{
    if (litArray.literals.empty()) {
        return;
    }

    bool isConst = litArray.literals[0].IsArray();

    std::stringstream specifiers {};

    if (isConst) {
        specifiers << LiteralTagToString(litArray.literals[0].tag) << " " << litArray.literals.size() << " ";
    }

    os << ".array array_" << name << " " << specifiers.str() << "{";

    SerializeValues(litArray, isConst, os);

    os << "}\n";
}

std::string Disassembler::LiteralTagToString(const panda_file::LiteralTag &tag) const
{
    switch (tag) {
        case panda_file::LiteralTag::BOOL:
        case panda_file::LiteralTag::ARRAY_U1:
            return "u1";
        case panda_file::LiteralTag::ARRAY_U8:
            return "u8";
        case panda_file::LiteralTag::ARRAY_I8:
            return "i8";
        case panda_file::LiteralTag::ARRAY_U16:
            return "u16";
        case panda_file::LiteralTag::ARRAY_I16:
            return "i16";
        case panda_file::LiteralTag::ARRAY_U32:
            return "u32";
        case panda_file::LiteralTag::INTEGER:
        case panda_file::LiteralTag::ARRAY_I32:
            return "i32";
        case panda_file::LiteralTag::ARRAY_U64:
            return "u64";
        case panda_file::LiteralTag::BIGINT:
        case panda_file::LiteralTag::ARRAY_I64:
            return "i64";
        case panda_file::LiteralTag::FLOAT:
        case panda_file::LiteralTag::ARRAY_F32:
            return "f32";
        case panda_file::LiteralTag::DOUBLE:
        case panda_file::LiteralTag::ARRAY_F64:
            return "f64";
        case panda_file::LiteralTag::STRING:
        case panda_file::LiteralTag::ARRAY_STRING:
            return pandasm::Type::FromDescriptor(panda_file::GetStringClassDescriptor(fileLanguage_)).GetPandasmName();
        case panda_file::LiteralTag::ACCESSOR:
            return "accessor";
        case panda_file::LiteralTag::NULLVALUE:
            return "nullvalue";
        case panda_file::LiteralTag::METHODAFFILIATE:
            return "method_affiliate";
        case panda_file::LiteralTag::METHOD:
            return "method";
        case panda_file::LiteralTag::GENERATORMETHOD:
            return "generator_method";
        case panda_file::LiteralTag::LITERALARRAY:
            return "lit_offset";
        default:
            LOG(ERROR, DISASSEMBLER) << "Unsupported literal with tag 0x" << std::hex << static_cast<uint32_t>(tag);
            UNREACHABLE();
    }
}

std::string Disassembler::SerializeLiterals(const pandasm::LiteralArray::Literal &lit) const
{
    std::stringstream res {};
    const auto &val = lit.value;
    switch (lit.tag) {
        case panda_file::LiteralTag::BOOL: {
            res << (std::get<bool>(val));
            break;
        }
        case panda_file::LiteralTag::INTEGER: {
            res << (bit_cast<int32_t>(std::get<uint32_t>(val)));
            break;
        }
        case panda_file::LiteralTag::DOUBLE: {
            res << (std::get<double>(val));
            break;
        }
        case panda_file::LiteralTag::STRING: {
            res << "\"" << (std::get<std::string>(val)) << "\"";
            break;
        }
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GENERATORMETHOD: {
            res << (std::get<std::string>(val));
            break;
        }
        case panda_file::LiteralTag::NULLVALUE:
        case panda_file::LiteralTag::ACCESSOR: {
            res << (static_cast<int16_t>(bit_cast<int8_t>(std::get<uint8_t>(val))));
            break;
        }
        case panda_file::LiteralTag::METHODAFFILIATE: {
            res << (std::get<uint16_t>(val));
            break;
        }
        case panda_file::LiteralTag::LITERALARRAY: {
            res << (std::get<std::string>(val));
            break;
        }
        default:
            UNREACHABLE();
    }
    res << ", ";
    return res.str();
}

std::string Disassembler::LiteralValueToString(const pandasm::LiteralArray::Literal &lit) const
{
    if (lit.IsBoolValue()) {
        std::stringstream res {};
        res << (std::get<bool>(lit.value));
        return res.str();
    }

    if (lit.IsByteValue()) {
        return LiteralIntegralValueToString<uint8_t>(lit);
    }

    if (lit.IsShortValue()) {
        return LiteralIntegralValueToString<uint16_t>(lit);
    }

    if (lit.IsIntegerValue()) {
        return LiteralIntegralValueToString<uint32_t>(lit);
    }

    if (lit.IsLongValue()) {
        return LiteralIntegralValueToString<uint64_t>(lit);
    }

    if (lit.IsDoubleValue()) {
        std::stringstream res {};
        res << std::get<double>(lit.value);
        return res.str();
    }

    if (lit.IsFloatValue()) {
        std::stringstream res {};
        res << std::get<float>(lit.value);
        return res.str();
    }

    if (lit.IsStringValue()) {
        std::stringstream res {};
        res << "\"" << std::get<std::string>(lit.value) << "\"";
        return res.str();
    }

    if (lit.IsLiteralArrayValue()) {
        return SerializeLiterals(lit);
    }

    UNREACHABLE();
}

void Disassembler::SerializeValues(const pandasm::LiteralArray &litArray, const bool isConst, std::ostream &os) const
{
    std::string separator = (isConst) ? (" ") : ("\n");

    os << separator;

    if (isConst) {
        for (const auto &l : litArray.literals) {
            os << LiteralValueToString(l) << separator;
        }
    } else {
        for (const auto &l : litArray.literals) {
            os << "\t" << LiteralTagToString(l.tag) << " " << LiteralValueToString(l) << separator;
        }
    }
}

void Disassembler::Serialize(const pandasm::Record &record, std::ostream &os, bool printInformation) const
{
    if (IsSystemType(record.name)) {
        return;
    }

    os << ".record " << record.name;

    const auto recordIter = progAnn_.recordAnnotations.find(record.name);
    const bool recordInTable = recordIter != progAnn_.recordAnnotations.end();
    if (recordInTable) {
        Serialize(*record.metadata, recordIter->second.annList, os);
    } else {
        Serialize(*record.metadata, {}, os);
    }

    if (record.metadata->IsForeign() && record.fieldList.empty()) {
        os << "\n\n";
        return;
    }

    os << " {";

    if (printInformation && progInfo_.recordsInfo.find(record.name) != progInfo_.recordsInfo.end()) {
        os << " # " << progInfo_.recordsInfo.at(record.name).recordInfo << "\n";
        SerializeFields(record, os, true);
    } else {
        os << "\n";
        SerializeFields(record, os, false);
    }

    os << "}\n\n";
}

void Disassembler::DumpLiteralArray(const pandasm::LiteralArray &literalArray, std::stringstream &ss) const
{
    ss << "[";
    bool firstItem = true;
    for (const auto &item : literalArray.literals) {
        if (!firstItem) {
            ss << ", ";
        } else {
            firstItem = false;
        }

        switch (item.tag) {
            case panda_file::LiteralTag::INTEGER: {
                ss << std::get<uint32_t>(item.value);  // CC-OFF(G.EXP.30-CPP) false positive
                break;
            }
            case panda_file::LiteralTag::DOUBLE: {
                ss << std::get<double>(item.value);
                break;
            }
            case panda_file::LiteralTag::BOOL: {
                ss << std::get<bool>(item.value);
                break;
            }
            case panda_file::LiteralTag::STRING: {
                ss << "\"" << std::get<std::string>(item.value) << "\"";
                break;
            }
            case panda_file::LiteralTag::LITERALARRAY: {
                std::string offsetStr = std::get<std::string>(item.value);
                const int hexBase = 16;
                const char *begin = offsetStr.data();
                uint32_t litArrayOffset = 0;
                litArrayOffset = strtoul(begin, nullptr, hexBase);
                pandasm::LiteralArray litArray;
                GetLiteralArrayByOffset(&litArray, panda_file::File::EntityId(litArrayOffset));
                DumpLiteralArray(litArray, ss);
                break;
            }
            default: {
                UNREACHABLE();
                break;
            }
        }
    }
    ss << "]";
}

void Disassembler::SerializeFieldValue(const pandasm::Field &f, std::stringstream &ss) const
{
    if (f.type.GetId() == panda_file::Type::TypeId::U32) {
        ss << " = 0x" << std::hex << f.metadata->GetValue().value().GetValue<uint32_t>();
    } else if (f.type.GetId() == panda_file::Type::TypeId::U8) {
        ss << " = 0x" << std::hex << static_cast<uint32_t>(f.metadata->GetValue().value().GetValue<uint8_t>());
    } else if (f.type.GetId() == panda_file::Type::TypeId::I8) {
        ss << " = 0x" << std::hex << static_cast<int32_t>(f.metadata->GetValue().value().GetValue<int8_t>());
    } else if (f.type.GetId() == panda_file::Type::TypeId::F64) {
        ss << " = " << static_cast<double>(f.metadata->GetValue().value().GetValue<double>());
    } else if (f.type.GetId() == panda_file::Type::TypeId::U1) {
        ss << " = " << static_cast<bool>(f.metadata->GetValue().value().GetValue<bool>());
    } else if (f.type.GetId() == panda_file::Type::TypeId::I32) {
        ss << " = " << f.metadata->GetValue().value().GetValue<int>();
    } else if (f.type.GetId() == panda_file::Type::TypeId::REFERENCE && f.type.GetName() == "std/core/String") {
        ss << " = \"" << static_cast<std::string>(f.metadata->GetValue().value().GetValue<std::string>()) << "\"";
    } else if (f.type.GetRank() > 0) {
        uint32_t litArrayOffset = 0;
        auto value = f.metadata->GetValue().value().GetValue<std::string>();
        litArrayOffset = strtoul(value.data(), nullptr, 0);
        pandasm::LiteralArray litArray;
        GetLiteralArrayByOffset(&litArray, panda_file::File::EntityId(litArrayOffset));
        ss << " = ";
        DumpLiteralArray(litArray, ss);
    }
}

void Disassembler::SerializeFields(const pandasm::Record &record, std::ostream &os, bool printInformation) const
{
    constexpr size_t INFO_OFFSET = 80;

    const auto recordIter = progAnn_.recordAnnotations.find(record.name);
    const bool recordInTable = recordIter != progAnn_.recordAnnotations.end();

    const auto recInf = (printInformation) ? (progInfo_.recordsInfo.at(record.name)) : (RecordInfo {});

    size_t fieldIdx = 0;

    std::stringstream ss;
    for (const auto &f : record.fieldList) {
        ss << "\t" << f.type.GetPandasmName() << " " << f.name;
        if (f.metadata->GetValue().has_value()) {
            SerializeFieldValue(f, ss);
        }
        if (recordInTable) {
            const auto fieldIter = recordIter->second.fieldAnnotations.find(f.name);
            if (fieldIter != recordIter->second.fieldAnnotations.end()) {
                Serialize(*f.metadata, fieldIter->second, ss);
            } else {
                Serialize(*f.metadata, {}, ss);
            }
        } else {
            Serialize(*f.metadata, {}, ss);
        }

        if (printInformation) {
            os << std::setw(INFO_OFFSET) << std::left << ss.str() << " # " << recInf.fieldsInfo.at(fieldIdx) << "\n";
        } else {
            os << ss.str() << "\n";
        }

        ss.str(std::string());
        ss.clear();

        fieldIdx++;
    }
}

void Disassembler::Serialize(const pandasm::Function::CatchBlock &catchBlock, std::ostream &os) const
{
    if (catchBlock.exceptionRecord.empty()) {
        os << ".catchall ";
    } else {
        os << ".catch " << catchBlock.exceptionRecord << ", ";
    }

    os << catchBlock.tryBeginLabel << ", " << catchBlock.tryEndLabel << ", " << catchBlock.catchBeginLabel;

    if (!catchBlock.catchEndLabel.empty()) {
        os << ", " << catchBlock.catchEndLabel;
    }
}

void Disassembler::Serialize(const pandasm::ItemMetadata &meta, const AnnotationList &annList, std::ostream &os) const
{
    auto boolAttributes = meta.GetBoolAttributes();
    auto attributes = meta.GetAttributes();
    if (boolAttributes.empty() && attributes.empty() && annList.empty()) {
        return;
    }

    os << " <";

    size_t size = boolAttributes.size();
    size_t idx = 0;
    for (const auto &attr : boolAttributes) {
        os << attr;
        ++idx;

        if (!attributes.empty() || !annList.empty() || idx < size) {
            os << ", ";
        }
    }

    size = attributes.size();
    idx = 0;
    for (const auto &[key, values] : attributes) {
        for (size_t i = 0; i < values.size(); i++) {
            os << key << "=" << values[i];

            if (i < values.size() - 1) {
                os << ", ";
            }
        }

        ++idx;

        if (!annList.empty() || idx < size) {
            os << ", ";
        }
    }

    size = annList.size();
    idx = 0;
    for (const auto &[key, value] : annList) {
        os << key << "=" << value;

        ++idx;

        if (idx < size) {
            os << ", ";
        }
    }

    os << ">";
}

void Disassembler::SerializeLineNumberTable(const panda_file::LineNumberTable &lineNumberTable, std::ostream &os) const
{
    if (lineNumberTable.empty()) {
        return;
    }

    os << "\n#   LINE_NUMBER_TABLE:\n";
    for (const auto &lineInfo : lineNumberTable) {
        os << "#\tline " << lineInfo.line << ": " << lineInfo.offset << "\n";
    }
}

void Disassembler::SerializeLocalVariableTable(const panda_file::LocalVariableTable &localVariableTable,
                                               const pandasm::Function &method, std::ostream &os) const
{
    if (localVariableTable.empty()) {
        return;
    }

    os << "\n#   LOCAL_VARIABLE_TABLE:\n";
    os << "#\t Start   End  Register           Name   Signature\n";
    const int startWidth = 5;
    const int endWidth = 4;
    const int regWidth = 8;
    const int nameWidth = 14;
    for (const auto &variableInfo : localVariableTable) {
        std::ostringstream regStream;
        regStream << variableInfo.regNumber << '(';
        if (variableInfo.regNumber < 0) {
            regStream << "acc";
        } else {
            uint32_t vreg = variableInfo.regNumber;
            uint32_t firstArgReg = method.GetTotalRegs();
            if (vreg < firstArgReg) {
                regStream << 'v' << vreg;
            } else {
                regStream << 'a' << vreg - firstArgReg;
            }
        }
        regStream << ')';

        os << "#\t " << std::setw(startWidth) << std::right << variableInfo.startOffset << "  ";
        os << std::setw(endWidth) << std::right << variableInfo.endOffset << "  ";
        os << std::setw(regWidth) << std::right << regStream.str() << " ";
        os << std::setw(nameWidth) << std::right << variableInfo.name << "   " << variableInfo.type;
        if (!variableInfo.typeSignature.empty() && variableInfo.typeSignature != variableInfo.type) {
            os << " (" << variableInfo.typeSignature << ")";
        }
        os << "\n";
    }
}

void Disassembler::SerializeLanguage(std::ostream &os) const
{
    os << ".language " << ark::panda_file::LanguageToString(fileLanguage_) << "\n\n";
}

void Disassembler::SerializeFilename(std::ostream &os) const
{
    if (file_ == nullptr || file_->GetFilename().empty()) {
        return;
    }

    os << "# source binary: " << file_->GetFilename() << "\n\n";
}

void Disassembler::SerializeLitArrays(std::ostream &os, bool addSeparators) const
{
    LOG(DEBUG, DISASSEMBLER) << "[serializing literals]";

    if (prog_.literalarrayTable.empty()) {
        return;
    }

    if (addSeparators) {
        os << "# ====================\n"
              "# LITERALS\n\n";
    }

    for (const auto &pair : prog_.literalarrayTable) {
        Serialize(pair.first, pair.second, os);
    }

    os << "\n";
}

void Disassembler::SerializeRecords(std::ostream &os, bool addSeparators, bool printInformation) const
{
    LOG(DEBUG, DISASSEMBLER) << "[serializing records]";

    if (prog_.recordTable.empty()) {
        return;
    }

    if (addSeparators) {
        os << "# ====================\n"
              "# RECORDS\n\n";
    }

    for (const auto &r : prog_.recordTable) {
        Serialize(r.second, os, printInformation);
    }
}

void Disassembler::SerializeMethods(std::ostream &os, bool addSeparators, bool printInformation) const
{
    LOG(DEBUG, DISASSEMBLER) << "[serializing methods]";

    if (prog_.functionInstanceTable.empty() && prog_.functionStaticTable.empty()) {
        return;
    }

    if (addSeparators) {
        os << "# ====================\n"
              "# METHODS\n\n";
    }

    for (const auto &m : prog_.functionStaticTable) {
        Serialize(m.second, os, printInformation);
    }
    for (const auto &m : prog_.functionInstanceTable) {
        Serialize(m.second, os, printInformation);
    }
}

pandasm::Opcode Disassembler::BytecodeOpcodeToPandasmOpcode(uint8_t o) const
{
    return BytecodeOpcodeToPandasmOpcode(BytecodeInstruction::Opcode(o));
}

std::string Disassembler::IDToString(BytecodeInstruction bcIns, panda_file::File::EntityId methodId) const
{
    std::stringstream name;

    if (bcIns.HasFlag(BytecodeInstruction::Flags::TYPE_ID)) {
        auto idx = bcIns.GetId().AsIndex();
        auto id = file_->ResolveClassIndex(methodId, idx);
        auto type = pandasm::Type::FromDescriptor(StringDataToString(file_->GetStringData(id)));

        name.str("");
        name << type.GetPandasmName();
    } else if (bcIns.HasFlag(BytecodeInstruction::Flags::METHOD_ID) ||
               bcIns.HasFlag(BytecodeInstruction::Flags::STATIC_METHOD_ID)) {
        auto idx = bcIns.GetId().AsIndex();
        auto id = file_->ResolveMethodIndex(methodId, idx);

        name << GetMethodSignature(id);
    } else if (bcIns.HasFlag(BytecodeInstruction::Flags::STRING_ID)) {
        name << '\"';

        if (skipStrings_ || quiet_) {
            name << std::hex << "0x" << bcIns.GetId().AsFileId();
        } else {
            name << StringDataToString(file_->GetStringData(bcIns.GetId().AsFileId()));
        }

        name << '\"';
    } else if (bcIns.HasFlag(BytecodeInstruction::Flags::FIELD_ID) ||
               bcIns.HasFlag(BytecodeInstruction::Flags::STATIC_FIELD_ID)) {
        auto idx = bcIns.GetId().AsIndex();
        auto id = file_->ResolveFieldIndex(methodId, idx);
        panda_file::FieldDataAccessor fieldAccessor(*file_, id);

        auto recordName = GetFullRecordName(fieldAccessor.GetClassId());
        name << recordName << '.';
        name << StringDataToString(file_->GetStringData(fieldAccessor.GetNameId()));
    } else if (bcIns.HasFlag(BytecodeInstruction::Flags::LITERALARRAY_ID)) {
        auto index = bcIns.GetId().AsIndex();
        name << "array_" << index;
    }

    return name.str();
}

ark::panda_file::SourceLang Disassembler::GetRecordLanguage(panda_file::File::EntityId classId) const
{
    if (file_->IsExternal(classId)) {
        return ark::panda_file::SourceLang::PANDA_ASSEMBLY;
    }

    panda_file::ClassDataAccessor cda(*file_, classId);
    return cda.GetSourceLang().value_or(panda_file::SourceLang::PANDA_ASSEMBLY);
}

// CC-OFFNXT(G.FUN.01) solid logic
static void TranslateImmToLabel(pandasm::Ins *paIns, LabelTable *labelTable, const uint8_t *insArr,
                                BytecodeInstruction bcIns, BytecodeInstruction bcInsLast,
                                panda_file::File::EntityId codeId)
{
    const int32_t jmpOffset = paIns->GetImm<int64_t>(0U);
    const auto bcInsDest = bcIns.JumpTo(jmpOffset);
    if (bcInsLast.GetAddress() > bcInsDest.GetAddress()) {
        size_t idx = GetBytecodeInstructionNumber(BytecodeInstruction(insArr), bcInsDest);
        if (idx != std::numeric_limits<size_t>::max()) {
            if (labelTable->find(idx) == labelTable->end()) {
                std::stringstream ss;
                ss << "jump_label_" << labelTable->size();
                (*labelTable)[idx] = ss.str();
            }

            paIns->ClearImm();
            paIns->EmplaceID(labelTable->at(idx));
        } else {
            LOG(ERROR, DISASSEMBLER) << "> error encountered at " << codeId << " (0x" << std::hex << codeId
                                     << "). incorrect instruction at offset: 0x" << (bcIns.GetAddress() - insArr)
                                     << ": invalid jump offset 0x" << jmpOffset
                                     << " - jumping in the middle of another instruction!";
        }
    } else {
        LOG(ERROR, DISASSEMBLER) << "> error encountered at " << codeId << " (0x" << std::hex << codeId
                                 << "). incorrect instruction at offset: 0x" << (bcIns.GetAddress() - insArr)
                                 << ": invalid jump offset 0x" << jmpOffset << " - jumping out of bounds!";
    }
}

void Disassembler::CollectExternalFields(const panda_file::FieldDataAccessor &fieldAccessor)
{
    auto recordName = GetFullRecordName(fieldAccessor.GetClassId());

    pandasm::Field field(fileLanguage_);
    GetField(field, fieldAccessor);
    if (field.name.empty()) {
        return;
    }

    auto &fieldList = externalFieldTable_[recordName];
    auto retField = std::find_if(fieldList.begin(), fieldList.end(), [&field](pandasm::Field &fieldFromList) {
        return field.name == fieldFromList.name && field.IsStatic() == fieldFromList.IsStatic();
    });
    if (retField == fieldList.end()) {
        fieldList.emplace_back(std::move(field));

        externalFieldsInfoTable_[recordName].emplace_back(GetFieldInfo(fieldAccessor));
    }
}

// CC-OFFNXT(huge_method) solid logic
IdList Disassembler::GetInstructions(pandasm::Function *method, panda_file::File::EntityId methodId,
                                     panda_file::File::EntityId codeId)
{
    panda_file::CodeDataAccessor codeAccessor(*file_, codeId);

    const auto insArr = codeAccessor.GetInstructions();

    method->regsNum = codeAccessor.GetNumVregs();

    auto bcIns = BytecodeInstruction(insArr);
    auto from = bcIns.GetAddress();
    const auto bcInsLast = bcIns.JumpTo(codeAccessor.GetCodeSize());

    LabelTable labelTable = GetExceptions(method, methodId, codeId);

    IdList unknownExternalMethods {};

    while (bcIns.GetAddress() != bcInsLast.GetAddress()) {
        if (bcIns.GetAddress() > bcInsLast.GetAddress()) {
            LOG(ERROR, DISASSEMBLER) << "> error encountered at " << codeId << " (0x" << std::hex << codeId
                                     << "). bytecode instructions sequence corrupted for method " << method->name
                                     << "! went out of bounds";

            break;
        }

        if (bcIns.HasFlag(BytecodeInstruction::Flags::FIELD_ID) ||
            bcIns.HasFlag(BytecodeInstruction::Flags::STATIC_FIELD_ID)) {
            auto idx = bcIns.GetId().AsIndex();
            auto id = file_->ResolveFieldIndex(methodId, idx);
            panda_file::FieldDataAccessor fieldAccessor(*file_, id);

            if (fieldAccessor.IsExternal()) {
                CollectExternalFields(fieldAccessor);
            }
        }

        auto paIns = BytecodeInstructionToPandasmInstruction(bcIns, methodId);
        bytecodeOffset_.emplace(std::pair {methodId, method->ins.size()}, bcIns.GetAddress() - from);
        if (paIns.IsJump()) {
            TranslateImmToLabel(&paIns, &labelTable, insArr, bcIns, bcInsLast, codeId);
        }

        // check if method id is unknown external method. if so, emplace it in table
        if (bcIns.HasFlag(BytecodeInstruction::Flags::METHOD_ID) ||
            bcIns.HasFlag(BytecodeInstruction::Flags::STATIC_METHOD_ID)) {
            const auto argMethodIdx = bcIns.GetId().AsIndex();
            const auto argMethodId = file_->ResolveMethodIndex(methodId, argMethodIdx);

            const auto argMethodSignature = GetMethodSignature(argMethodId);
            panda_file::MethodDataAccessor methodAccessor(*file_, argMethodId);
            const auto &functionTable =
                methodAccessor.IsStatic() ? prog_.functionStaticTable : prog_.functionInstanceTable;
            const bool isPresent = functionTable.find(argMethodSignature) != functionTable.cend();
            const bool isExternal = file_->IsExternal(argMethodId);
            if (isExternal && !isPresent) {
                unknownExternalMethods.push_back(argMethodId);
            }
        }

        method->ins.emplace_back(std::move(paIns));
        bcIns = bcIns.GetNext();
    }

    for (const auto &pair : labelTable) {
        method->ins[pair.first].SetLabel(pair.second);
    }

    return unknownExternalMethods;
}

}  // namespace ark::disasm
