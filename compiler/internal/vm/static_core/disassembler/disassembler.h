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

#ifndef DISASM_LIB_H_INCLUDED
#define DISASM_LIB_H_INCLUDED

#include "libarkbase/utils/type_helpers.h"
#include "libarkbase/utils/span.h"

#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "libarkfile/debug_data_accessor-inl.h"
#include "libarkfile/debug_info_extractor.h"
#include "libarkfile/field_data_accessor-inl.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/literal_data_accessor-inl.h"
#include "libarkfile/param_annotations_data_accessor.h"
#include "libarkfile/annotation_data_accessor.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "libarkfile/file-inl.h"
#include "libarkfile/file.h"
#include "libarkbase/os/file.h"

#include "runtime/profiling/profiling-disasm-inl.h"

#include "assembly-program.h"
#include "assembly-ins.h"

#include "libarkfile/bytecode_instruction-inl.h"
#include "libarkbase/macros.h"

#include <map>
#include <memory>
#include <string>

#include "accumulators.h"

namespace ark::disasm {
class Disassembler {
public:
    NO_COPY_SEMANTIC(Disassembler);
    DEFAULT_MOVE_SEMANTIC(Disassembler);

    Disassembler() = default;
    ~Disassembler()
    {
        profiling::DestroyProfile(profile_, fileLanguage_);
    }

    PANDA_PUBLIC_API void Serialize(std::ostream &os, bool addSeparators = false, bool printInformation = false) const;
    PANDA_PUBLIC_API void Disassemble(std::string_view filenameIn, bool quiet = false, bool skipStrings = false);
    PANDA_PUBLIC_API void Disassemble(const panda_file::File &file, bool quiet = false, bool skipStrings = false);
    PANDA_PUBLIC_API void Disassemble(std::unique_ptr<const panda_file::File> &file, bool quiet = false,
                                      bool skipStrings = false);
    PANDA_PUBLIC_API void CollectInfo();

    PANDA_PUBLIC_API void SetProfile(std::string_view fname);

    PANDA_PUBLIC_API void SetFile(std::unique_ptr<const panda_file::File> &file);
    PANDA_PUBLIC_API void SetFile(const panda_file::File &file);

    PANDA_PUBLIC_API void Serialize(const pandasm::Function &method, std::ostream &os, bool printInformation = false,
                                    panda_file::LineNumberTable *lineTable = nullptr) const;
    PANDA_PUBLIC_API void GetMethod(pandasm::Function *method, const panda_file::File::EntityId &methodId);

    const ProgInfo &GetProgInfo() const
    {
        return progInfo_;
    }

private:
    void DisassembleImpl(const bool quiet = false, const bool skipStrings = false);
    static inline bool IsSystemType(const std::string &typeName);

    void GetRecord(pandasm::Record &record, const panda_file::File::EntityId &recordId);
    void AddMethodToTables(const panda_file::File::EntityId &methodId);
    void GetLiteralArrayByOffset(pandasm::LiteralArray *litArray, panda_file::File::EntityId offset) const;
    void GetLiteralArray(pandasm::LiteralArray *litArray, size_t index);
    template <typename T>
    void FillLiteralArrayData(pandasm::LiteralArray *litArray, const panda_file::LiteralTag &tag,
                              const panda_file::LiteralDataAccessor::LiteralValue &value) const;
    std::variant<bool, uint8_t, uint16_t, uint32_t, uint64_t, float, double, std::string> ParseLiteralValue(
        const panda_file::LiteralDataAccessor::LiteralValue &value, const panda_file::LiteralTag &tag) const;
    std::string ParseStringData(const panda_file::LiteralDataAccessor::LiteralValue &value) const;
    std::string ParseLiteralArrayData(const panda_file::LiteralDataAccessor::LiteralValue &value) const;
    void FillLiteralData(pandasm::LiteralArray *litArray, const panda_file::LiteralDataAccessor::LiteralValue &value,
                         const panda_file::LiteralTag &tag) const;

    static inline bool IsPandasmFriendly(char c);
    void GetLiteralArrays();
    void GetRecords();
    void GetFields(pandasm::Record &record, const panda_file::File::EntityId &recordId);

    void GetField(pandasm::Field &field, const panda_file::FieldDataAccessor &fieldAccessor);
    void AddExternalFieldsToRecords();
    void AddExternalFieldsInfoToRecords();

    void GetMethods(const panda_file::File::EntityId &recordId);
    void GetParams(pandasm::Function *method, const panda_file::File::EntityId &protoId) const;
    IdList GetInstructions(pandasm::Function *method, panda_file::File::EntityId methodId,
                           panda_file::File::EntityId codeId);
    LabelTable GetExceptions(pandasm::Function *method, panda_file::File::EntityId methodId,
                             panda_file::File::EntityId codeId) const;
    // CC-OFFNXT(G.FUN.01) solid logic
    bool LocateTryBlock(const BytecodeInstruction &bcIns, const BytecodeInstruction &bcInsLast,
                        const panda_file::CodeDataAccessor::TryBlock &tryBlock,
                        pandasm::Function::CatchBlock *catchBlockPa, LabelTable *labelTable, size_t tryIdx) const;
    // CC-OFFNXT(G.FUN.01) solid logic
    bool LocateCatchBlock(const BytecodeInstruction &bcIns, const BytecodeInstruction &bcInsLast,
                          const panda_file::CodeDataAccessor::CatchBlock &catchBlock,
                          pandasm::Function::CatchBlock *catchBlockPa, LabelTable *labelTable, size_t tryIdx,
                          size_t catchIdx) const;

    void GetMetaData(pandasm::Record *record, const panda_file::File::EntityId &recordId) const;
    void GetMetaData(pandasm::Function *method, const panda_file::File::EntityId &methodId) const;
    void GetMetaData(pandasm::Field *field, const panda_file::File::EntityId &fieldId) const;

    void GetLanguageSpecificMetadata();

    std::string AnnotationTagToString(const char tag) const;

    std::string ScalarValueToString(const panda_file::ScalarValue &value, const std::string &type);
    std::string ArrayValueToString(const panda_file::ArrayValue &value, const std::string &type, size_t idx);

    std::string GetFullMethodName(const panda_file::File::EntityId &methodId) const;
    std::string GetMethodSignature(const panda_file::File::EntityId &methodId) const;
    std::string GetFullRecordName(const panda_file::File::EntityId &classId) const;

    void GetRecordInfo(const panda_file::File::EntityId &recordId, RecordInfo *recordInfo) const;
    void GetMethodInfo(const panda_file::File::EntityId &methodId, MethodInfo *methodInfo) const;
    void GetInsInfo(panda_file::MethodDataAccessor &mda, const panda_file::File::EntityId &codeId,
                    MethodInfo *methodInfo) const;
    template <typename T>
    void SerializeValue(bool isSigned, const pandasm::LiteralArray::Literal &lit, std::ostream &os) const;
    void Serialize(const std::string &name, const pandasm::LiteralArray &litArray, std::ostream &os) const;
    void SerializeValues(const pandasm::LiteralArray &litArray, bool isConst, std::ostream &os) const;
    std::string LiteralTagToString(const panda_file::LiteralTag &tag) const;
    std::string LiteralValueToString(const pandasm::LiteralArray::Literal &lit) const;
    void Serialize(const pandasm::Record &record, std::ostream &os, bool printInformation = false) const;
    void SerializeFields(const pandasm::Record &record, std::ostream &os, bool printInformation) const;
    void Serialize(const pandasm::Function::CatchBlock &catchBlock, std::ostream &os) const;
    void Serialize(const pandasm::ItemMetadata &meta, const AnnotationList &annList, std::ostream &os) const;
    void SerializeLineNumberTable(const panda_file::LineNumberTable &lineNumberTable, std::ostream &os) const;
    void SerializeLocalVariableTable(const panda_file::LocalVariableTable &localVariableTable,
                                     const pandasm::Function &method, std::ostream &os) const;
    void SerializeLanguage(std::ostream &os) const;
    void SerializeFilename(std::ostream &os) const;
    void SerializeLitArrays(std::ostream &os, bool addSeparators = false) const;
    void SerializeRecords(std::ostream &os, bool addSeparators = false, bool printInformation = false) const;
    void SerializeMethods(std::ostream &os, bool addSeparators = false, bool printInformation = false) const;
    void SerializePrintStartInfo(const pandasm::Function &method, std::ostringstream &headerSs) const;
    void SerializeCheckEnd(const pandasm::Function &method, std::ostream &os, bool printMethodInfo,
                           const MethodInfo *&methodInfo) const;
    size_t SerializeIfPrintMethodInfo(
        const pandasm::Function &method, bool printMethodInfo, std::ostringstream &headerSs,
        const MethodInfo *&methodInfo,
        std::map<std::string, ark::disasm::MethodInfo>::const_iterator &methodInfoIt) const;

    void EnumerateAnnotations(panda_file::AnnotationDataAccessor &annotationAccessor, AnnotationList &annList,
                              const std::string &type, const panda_file::File::EntityId &annotationId);

    void HandleArrayAnnotation(panda_file::AnnotationDataAccessor &annotationAccessor, AnnotationList &annList,
                               size_t i);

    pandasm::Type PFTypeToPandasmType(const panda_file::Type &type, panda_file::ProtoDataAccessor &pda,
                                      size_t &refIdx) const;

    pandasm::Type FieldTypeToPandasmType(const uint32_t &type) const;

    static std::string StringDataToString(panda_file::File::StringData sd)
    {
        std::string str = std::string(utf::Mutf8AsCString(sd.data));
        size_t symPos = 0;
        while (symPos = str.find_first_of("\a\b\f\n\r\t\v\'\?\\", symPos), symPos != std::string::npos) {
            std::string sym;
            switch (str[symPos]) {
                case '\a':
                    sym = R"(\a)";
                    break;
                case '\b':
                    sym = R"(\b)";
                    break;
                case '\f':
                    sym = R"(\f)";
                    break;
                case '\n':
                    sym = R"(\n)";
                    break;
                case '\r':
                    sym = R"(\r)";
                    break;
                case '\t':
                    sym = R"(\t)";
                    break;
                case '\v':
                    sym = R"(\v)";
                    break;
                case '\'':
                    sym = R"(\')";
                    break;
                case '\?':
                    sym = R"(\?)";
                    break;
                case '\\':
                    sym = R"(\\)";
                    break;
                default:
                    UNREACHABLE();
            }
            str = str.replace(symPos, 1, sym);
            ASSERT(sym.size() == 2U);
            symPos += 2U;
        }
        return str;
    }

    template <typename T>
    std::string LiteralIntegralValueToString(const pandasm::LiteralArray::Literal &lit) const
    {
        std::stringstream res {};
        T value = std::get<T>(lit.value);
        if (lit.IsSigned()) {
            res << +static_cast<typename std::make_signed<T>::type>(value);
        } else {
            res << +value;
        }
        return res.str();
    }

    void DumpLiteralArray(const pandasm::LiteralArray &literalArray, std::stringstream &ss) const;
    template <typename T, pandasm::Value::Type VALUE_TYPE>
    void SetMetadata(panda_file::FieldDataAccessor &accessor, pandasm::Field *field) const;
    void SerializeFieldValue(const pandasm::Field &f, std::stringstream &ss) const;
    void GetMetadataFieldValue(panda_file::FieldDataAccessor &fieldAccessor, pandasm::Field *field) const;
    std::string SerializeLiterals(const pandasm::LiteralArray::Literal &literal) const;
    pandasm::Opcode BytecodeOpcodeToPandasmOpcode(BytecodeInstruction::Opcode o) const;
    pandasm::Opcode BytecodeOpcodeToPandasmOpcode(uint8_t o) const;
    void CollectExternalFields(const panda_file::FieldDataAccessor &fieldAccessor);

    pandasm::Ins BytecodeInstructionToPandasmInstruction(BytecodeInstruction bcIns,
                                                         panda_file::File::EntityId methodId) const;

    std::string IDToString(BytecodeInstruction bcIns, panda_file::File::EntityId methodId) const;

    ark::panda_file::SourceLang GetRecordLanguage(panda_file::File::EntityId classId) const;

    std::unique_ptr<const panda_file::File> fileHolder_ {nullptr};
    const panda_file::File *file_ {};
    pandasm::Program prog_ {};

    ark::panda_file::SourceLang fileLanguage_ = ark::panda_file::SourceLang::PANDA_ASSEMBLY;

    std::map<std::string, panda_file::File::EntityId> recordNameToId_ {};
    std::map<std::string, panda_file::File::EntityId> methodStaticNameToId_ {};
    std::map<std::string, panda_file::File::EntityId> methodInstanceNameToId_ {};

    std::map<std::string, std::vector<pandasm::Field>> externalFieldTable_ {};
    std::map<std::string, std::vector<std::string>> externalFieldsInfoTable_ {};
    std::map<std::pair<panda_file::File::EntityId, uint32_t>, uint32_t> bytecodeOffset_;

    ProgAnnotations progAnn_ {};

    ProgInfo progInfo_ {};

    profiling::ProfileContainer profile_ {profiling::INVALID_PROFILE};

    std::unique_ptr<panda_file::DebugInfoExtractor> debugInfoExtractor_ {nullptr};

    bool quiet_ {false};
    bool skipStrings_ {false};

#include "disasm_plugins.inc"
};
}  // namespace ark::disasm

#endif
