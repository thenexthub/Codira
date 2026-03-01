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

#ifndef LIBPANDAFILE_FILE_ITEMS_H_
#define LIBPANDAFILE_FILE_ITEMS_H_

#include "libarkfile/file.h"
#include "libarkfile/file_writer.h"
#include "libarkbase/macros.h"
#include "libarkfile/modifiers.h"
#include <libarkfile/include/type.h>
#include <libarkfile/include/file_format_version.h>
#include <libarkfile/include/source_lang_enum.h>

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>
#include <list>
#include <set>

namespace ark::panda_file {

enum class ClassTag : uint8_t {
    NOTHING = 0x00,
    INTERFACES = 0x01,
    SOURCE_LANG = 0x02,
    RUNTIME_ANNOTATION = 0x03,
    ANNOTATION = 0x04,
    RUNTIME_TYPE_ANNOTATION = 0x05,
    TYPE_ANNOTATION = 0x06,
    SOURCE_FILE = 0x07
};

enum class MethodTag : uint8_t {
    NOTHING = 0x00,
    CODE = 0x01,
    SOURCE_LANG = 0x02,
    RUNTIME_ANNOTATION = 0x03,
    RUNTIME_PARAM_ANNOTATION = 0x04,
    DEBUG_INFO = 0x05,
    ANNOTATION = 0x06,
    PARAM_ANNOTATION = 0x07,
    TYPE_ANNOTATION = 0x08,
    RUNTIME_TYPE_ANNOTATION = 0x09,
    PROFILE_INFO = 0x0a
};

enum class FieldTag : uint8_t {
    NOTHING = 0x00,
    INT_VALUE = 0x01,
    VALUE = 0x02,
    RUNTIME_ANNOTATION = 0x03,
    ANNOTATION = 0x04,
    RUNTIME_TYPE_ANNOTATION = 0x05,
    TYPE_ANNOTATION = 0x06
};

PANDA_PUBLIC_API bool IsDynamicLanguage(ark::panda_file::SourceLang lang);
PANDA_PUBLIC_API std::optional<ark::panda_file::SourceLang> LanguageFromString(std::string_view lang);
PANDA_PUBLIC_API const char *LanguageToString(ark::panda_file::SourceLang lang);
PANDA_PUBLIC_API const char *GetCtorName(ark::panda_file::SourceLang lang);
PANDA_PUBLIC_API const char *GetCctorName(ark::panda_file::SourceLang lang);
PANDA_PUBLIC_API const char *GetStringClassDescriptor(ark::panda_file::SourceLang lang);

constexpr size_t ID_SIZE = File::EntityId::GetSize();
constexpr size_t IDX_SIZE = sizeof(uint16_t);
constexpr size_t TAG_SIZE = 1;
constexpr uint32_t INVALID_OFFSET = std::numeric_limits<uint32_t>::max();
constexpr uint32_t INVALID_INDEX = std::numeric_limits<uint32_t>::max();
constexpr uint32_t MAX_INDEX_16 = std::numeric_limits<uint16_t>::max();
constexpr uint32_t MAX_INDEX_32 = std::numeric_limits<uint32_t>::max();

constexpr uint32_t PGO_STRING_DEFAULT_COUNT = 5;
constexpr uint32_t PGO_CLASS_DEFAULT_COUNT = 3;
constexpr uint32_t PGO_CODE_DEFAULT_COUNT = 1;

enum class ItemTypes {
    ANNOTATION_ITEM,
    CATCH_BLOCK_ITEM,
    CLASS_INDEX_ITEM,
    CLASS_ITEM,
    CODE_ITEM,
    DEBUG_INFO_ITEM,
    END_ITEM,
    FIELD_INDEX_ITEM,
    FIELD_ITEM,
    FOREIGN_CLASS_ITEM,
    FOREIGN_FIELD_ITEM,
    FOREIGN_METHOD_ITEM,
    LINE_NUMBER_PROGRAM_INDEX_ITEM,
    LINE_NUMBER_PROGRAM_ITEM,
    LITERAL_ARRAY_ITEM,
    LITERAL_ITEM,
    METHOD_HANDLE_ITEM,
    METHOD_INDEX_ITEM,
    METHOD_ITEM,
    PARAM_ANNOTATIONS_ITEM,
    PRIMITIVE_TYPE_ITEM,
    PROTO_INDEX_ITEM,
    PROTO_ITEM,
    REGION_HEADER,
    REGION_SECTION,
    STRING_ITEM,
    TRY_BLOCK_ITEM,
    VALUE_ITEM
};

std::string ItemTypeToString(ItemTypes type);
constexpr std::string_view STRING_ITEM = "string_item";
constexpr std::string_view CLASS_ITEM = "class_item";
constexpr std::string_view CODE_ITEM = "code_item";

enum class IndexType {
    // 16-bit indexes
    CLASS = 0x0,
    METHOD = 0x1,
    FIELD = 0x2,
    PROTO = 0x3,
    LAST_16 = PROTO,

    // 32-bit indexes
    LINE_NUMBER_PROG = 0x04,
    LAST_32 = LINE_NUMBER_PROG,

    NONE
};

constexpr size_t INDEX_COUNT_16 = static_cast<size_t>(IndexType::LAST_16) + 1;

class IndexedItem;

class BaseItem {
public:
    using VisitorCallBack = std::function<bool(BaseItem *)>;

    BaseItem() = default;
    virtual ~BaseItem() = default;

    DEFAULT_COPY_SEMANTIC(BaseItem);
    DEFAULT_MOVE_SEMANTIC(BaseItem);

    size_t GetSize() const
    {
        return CalculateSize();
    }

    virtual size_t CalculateSize() const = 0;

    virtual void ComputeLayout() {}

    virtual size_t Alignment()
    {
        return 1;
    }

    virtual bool IsForeign() const
    {
        return false;
    }

    uint32_t GetOffset() const
    {
        return offset_;
    }

    panda_file::File::EntityId GetFileId() const
    {
        return panda_file::File::EntityId(offset_);
    }

    void SetOffset(uint32_t offset)
    {
        offset_ = offset;
    }

    bool NeedsEmit() const
    {
        return needsEmit_;
    }

    void SetNeedsEmit(bool needsEmit)
    {
        needsEmit_ = needsEmit;
    }

    const std::list<IndexedItem *> &GetIndexDependencies() const
    {
        return indexDeps_;
    }

    void AddIndexDependency(IndexedItem *item)
    {
        ASSERT(item != nullptr);
        indexDeps_.push_back(item);
    }

    void SetOrderIndex(uint32_t order)
    {
        order_ = order;
    }

    uint32_t GetOrderIndex() const
    {
        return order_;
    }

    bool HasOrderIndex() const
    {
        return order_ != INVALID_INDEX;
    }

    virtual bool Write(Writer *writer) = 0;

    std::string GetName() const;

    virtual ItemTypes GetItemType() const = 0;

    virtual void Dump([[maybe_unused]] std::ostream &os) const {}

    virtual void Visit([[maybe_unused]] const VisitorCallBack &cb) {}

    void SetPGORank(uint32_t rank)
    {
        pgoRank_ = rank;
    }

    uint32_t GetPGORank() const
    {
        return pgoRank_;
    }

    void SetOriginalRank(uint32_t rank)
    {
        originalRank_ = rank;
    }

    uint32_t GetOriginalRank() const
    {
        return originalRank_;
    }

    virtual void SetDependencyMark()
    {
        dependencyMarked_ = true;
    }

    bool GetDependencyMark() const
    {
        return dependencyMarked_;
    }

    ItemTypes GetBaseItemType() const
    {
        return type_;
    }

    void SetBaseItemType(ItemTypes type)
    {
        type_ = type;
    }

private:
    bool needsEmit_ {true};
    bool dependencyMarked_ {false};
    uint32_t offset_ {0};
    uint32_t order_ {INVALID_INDEX};
    std::list<IndexedItem *> indexDeps_;
    uint32_t pgoRank_ {0};
    uint32_t originalRank_ {0};
    ItemTypes type_ = ItemTypes::ANNOTATION_ITEM;
};

class IndexedItem : public BaseItem {
public:
    IndexedItem()
    {
        itemAllocId_ = itemAllocIdNext_++;
    }

    uint32_t GetIndex(const BaseItem *item) const
    {
        auto *idx = FindIndex(item);
        ASSERT(idx != nullptr);
        return idx->index;
    }

    bool HasIndex(const BaseItem *item) const
    {
        return FindIndex(item) != nullptr;
    }

    void SetIndex(const BaseItem *start, const BaseItem *end, uint32_t index)
    {
        ASSERT(FindIndex(start, end) == nullptr);
        indexes_.push_back({start, end, index});
    }

    void ClearIndexes()
    {
        indexes_.clear();
    }

    void IncRefCount()
    {
        ++refCount_;
    }

    void DecRefCount()
    {
        ASSERT(refCount_ != 0);
        --refCount_;
    }

    size_t GetRefCount() const
    {
        return refCount_;
    }

    virtual IndexType GetIndexType() const
    {
        return IndexType::NONE;
    }

    size_t GetItemAllocId() const
    {
        return itemAllocId_;
    }

private:
    struct Index {
        const BaseItem *start;
        const BaseItem *end;
        uint32_t index;
    };

    const Index *FindIndex(const BaseItem *start, const BaseItem *end) const
    {
        auto it = std::find_if(indexes_.cbegin(), indexes_.cend(),
                               [start, end](const Index &idx) { return idx.start == start && idx.end == end; });

        return it != indexes_.cend() ? &*it : nullptr;
    }

    const Index *FindIndex(const BaseItem *item) const
    {
        ASSERT(item);
        ASSERT(item->HasOrderIndex());
        auto orderIdx = item->GetOrderIndex();

        auto it = std::find_if(indexes_.cbegin(), indexes_.cend(), [orderIdx](const Index &idx) {
            if (idx.start == nullptr && idx.end == nullptr) {
                return true;
            }

            if (idx.start == nullptr || idx.end == nullptr) {
                return false;
            }

            ASSERT(idx.start->HasOrderIndex());
            ASSERT(idx.end->HasOrderIndex());
            return idx.start->GetOrderIndex() <= orderIdx && orderIdx < idx.end->GetOrderIndex();
        });

        return it != indexes_.cend() ? &*it : nullptr;
    }

    std::vector<Index> indexes_;
    size_t refCount_ {1};
    size_t itemAllocId_ {0};

    // needed for keeping same layout of panda file after rebuilding it,
    // even if same `IndexedItem` was allocated at different addresses
    static size_t itemAllocIdNext_;
};

class TypeItem : public IndexedItem {
public:
    explicit TypeItem(Type type) : type_(type) {}

    explicit TypeItem(Type::TypeId typeId) : type_(typeId) {}

    ~TypeItem() override = default;

    Type GetType() const
    {
        return type_;
    }

    IndexType GetIndexType() const override
    {
        return IndexType::CLASS;
    }

    DEFAULT_MOVE_SEMANTIC(TypeItem);
    DEFAULT_COPY_SEMANTIC(TypeItem);

private:
    Type type_;
};

class PrimitiveTypeItem : public TypeItem {
public:
    explicit PrimitiveTypeItem(Type type) : PrimitiveTypeItem(type.GetId()) {}

    explicit PrimitiveTypeItem(Type::TypeId typeId) : TypeItem(typeId)
    {
        ASSERT(GetType().IsPrimitive());
        SetNeedsEmit(false);
        SetOffset(GetType().GetFieldEncoding());
    }

    ~PrimitiveTypeItem() override = default;

    size_t CalculateSize() const override
    {
        return 0;
    }

    bool Write([[maybe_unused]] Writer *writer) override
    {
        return true;
    }

    ItemTypes GetItemType() const override
    {
        return ItemTypes::PRIMITIVE_TYPE_ITEM;
    }

    DEFAULT_MOVE_SEMANTIC(PrimitiveTypeItem);
    DEFAULT_COPY_SEMANTIC(PrimitiveTypeItem);
};

class StringItem : public BaseItem {
public:
    explicit StringItem(std::string str);

    explicit StringItem(File::StringData data);

    ~StringItem() override = default;

    size_t CalculateSize() const override;

    bool Write(Writer *writer) override;

    ItemTypes GetItemType() const override
    {
        return ItemTypes::STRING_ITEM;
    }

    const std::string &GetData() const
    {
        return str_;
    }

    size_t GetUtf16Len() const
    {
        return utf16Length_;
    }

    DEFAULT_MOVE_SEMANTIC(StringItem);
    DEFAULT_COPY_SEMANTIC(StringItem);

private:
    std::string str_;
    size_t utf16Length_;
    size_t isAscii_ = 0;
};

class AnnotationItem;
class BaseClassItem;
class ClassItem;
class ForeignClassItem;
class ValueItem;

class BaseFieldItem : public IndexedItem {
public:
    IndexType GetIndexType() const override
    {
        return IndexType::FIELD;
    }

    StringItem *GetNameItem() const
    {
        return name_;
    }

    TypeItem *GetTypeItem() const
    {
        return type_;
    }

    BaseClassItem *GetClassItem() const
    {
        return class_;
    }

    uint32_t GetAccessFlags() const
    {
        return accessFlags_;
    }

    bool IsStatic() const
    {
        return (accessFlags_ & ACC_STATIC) != 0;
    }

    ~BaseFieldItem() override = default;

    DEFAULT_MOVE_SEMANTIC(BaseFieldItem);
    DEFAULT_COPY_SEMANTIC(BaseFieldItem);

protected:
    PANDA_PUBLIC_API BaseFieldItem(BaseClassItem *cls, StringItem *name, TypeItem *type, uint32_t accessFlags);

    PANDA_PUBLIC_API size_t CalculateSize() const override;

    PANDA_PUBLIC_API bool Write(Writer *writer) override;

    PANDA_PUBLIC_API void SetDependencyMark() override;

private:
    BaseClassItem *class_;
    StringItem *name_;
    TypeItem *type_;
    uint32_t accessFlags_;
};

class FieldItem : public BaseFieldItem {
public:
    PANDA_PUBLIC_API FieldItem(ClassItem *cls, StringItem *name, TypeItem *type, uint32_t accessFlags);

    ~FieldItem() override = default;

    PANDA_PUBLIC_API void SetValue(ValueItem *value);

    ValueItem *GetValue() const
    {
        return value_;
    }

    void AddRuntimeAnnotation(AnnotationItem *runtimeAnnotation)
    {
        runtimeAnnotations_.push_back(runtimeAnnotation);
    }

    void AddAnnotation(AnnotationItem *annotation)
    {
        annotations_.push_back(annotation);
    }

    void AddRuntimeTypeAnnotation(AnnotationItem *runtimeTypeAnnotation)
    {
        runtimeTypeAnnotations_.push_back(runtimeTypeAnnotation);
    }

    void AddTypeAnnotation(AnnotationItem *typeAnnotation)
    {
        typeAnnotations_.push_back(typeAnnotation);
    }

    size_t CalculateSize() const override;

    bool Write(Writer *writer) override;

    ItemTypes GetItemType() const override
    {
        return ItemTypes::FIELD_ITEM;
    }

    std::vector<AnnotationItem *> *GetRuntimeAnnotations()
    {
        return &runtimeAnnotations_;
    }

    std::vector<AnnotationItem *> *GetAnnotations()
    {
        return &annotations_;
    }

    std::vector<AnnotationItem *> *GetTypeAnnotations()
    {
        return &typeAnnotations_;
    }

    std::vector<AnnotationItem *> *GetRuntimeTypeAnnotations()
    {
        return &runtimeTypeAnnotations_;
    }

    void SetDependencyMark() override;

    DEFAULT_MOVE_SEMANTIC(FieldItem);
    DEFAULT_COPY_SEMANTIC(FieldItem);

private:
    bool WriteValue(Writer *writer);

    bool WriteAnnotations(Writer *writer);

    bool WriteTaggedData(Writer *writer);

    ValueItem *value_ {nullptr};
    std::vector<AnnotationItem *> runtimeAnnotations_;
    std::vector<AnnotationItem *> annotations_;
    std::vector<AnnotationItem *> typeAnnotations_;
    std::vector<AnnotationItem *> runtimeTypeAnnotations_;
};

class ProtoItem;
class CodeItem;

/**
 * @brief Base class of `LineNumberProgramItem`.
 *
 * Instances of this class might be used in order to fill constant pool of a shared `LineNumberProgram`.
 * Implementations must override `Empty`, `EmitOpcode` and `EmitRegister`, which are left no-op.
 */
class LineNumberProgramItemBase {
public:
    enum class Opcode : uint8_t {
        END_SEQUENCE = 0x00,
        ADVANCE_PC = 0x01,
        ADVANCE_LINE = 0x02,
        START_LOCAL = 0x03,
        START_LOCAL_EXTENDED = 0x04,
        END_LOCAL = 0x05,
        RESTART_LOCAL = 0x06,
        SET_PROLOGUE_END = 0x07,
        SET_EPILOGUE_BEGIN = 0x08,
        SET_FILE = 0x09,
        SET_SOURCE_CODE = 0x0a,
        SET_COLUMN = 0X0b,
        LAST
    };

    static constexpr uint8_t OPCODE_BASE = static_cast<uint8_t>(Opcode::LAST);
    static constexpr int32_t LINE_RANGE = 15;
    static constexpr int32_t LINE_BASE = -4;

    PANDA_PUBLIC_API void EmitEnd();

    PANDA_PUBLIC_API void EmitAdvancePc(std::vector<uint8_t> *constantPool, uint32_t value);

    PANDA_PUBLIC_API void EmitAdvanceLine(std::vector<uint8_t> *constantPool, int32_t value);

    PANDA_PUBLIC_API void EmitColumn(std::vector<uint8_t> *constantPool, uint32_t pcInc, uint32_t column);

    PANDA_PUBLIC_API void EmitStartLocal(std::vector<uint8_t> *constantPool, int32_t registerNumber, StringItem *name,
                                         StringItem *type);

    PANDA_PUBLIC_API void EmitStartLocalExtended(std::vector<uint8_t> *constantPool, int32_t registerNumber,
                                                 StringItem *name, StringItem *type, StringItem *typeSignature);

    PANDA_PUBLIC_API void EmitEndLocal(int32_t registerNumber);

    void EmitRestartLocal(int32_t registerNumber);

    PANDA_PUBLIC_API bool EmitSpecialOpcode(uint32_t pcInc, int32_t lineInc);

    void EmitPrologueEnd();

    void EmitEpilogueBegin();

    PANDA_PUBLIC_API void EmitSetFile(std::vector<uint8_t> *constantPool, StringItem *sourceFile);

    PANDA_PUBLIC_API void EmitSetSourceCode(std::vector<uint8_t> *constantPool, StringItem *sourceCode);

    void EmitOpcode(Opcode opcode);

    virtual bool Empty() const
    {
        return true;
    }

protected:
    virtual void EmitOpcode([[maybe_unused]] uint8_t opcode) {}
    virtual void EmitRegister([[maybe_unused]] int32_t registerNumber) {}

    static void EmitUleb128(std::vector<uint8_t> *data, uint32_t value);

    static void EmitSleb128(std::vector<uint8_t> *data, int32_t value);
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LineNumberProgramItem final : public IndexedItem, public LineNumberProgramItemBase {
public:
    bool Write(Writer *writer) override;

    size_t CalculateSize() const override;

    ItemTypes GetItemType() const override
    {
        return ItemTypes::LINE_NUMBER_PROGRAM_ITEM;
    }

    bool Empty() const override
    {
        return GetData().empty();
    }

    const std::vector<uint8_t> &GetData() const
    {
        return data_;
    }

    IndexType GetIndexType() const override
    {
        return IndexType::LINE_NUMBER_PROG;
    }

    void SetData(std::vector<uint8_t> &&data);

private:
    void EmitOpcode(uint8_t opcode) override;
    void EmitRegister(int32_t registerNumber) override;

    std::vector<uint8_t> data_;
};

class PANDA_PUBLIC_API DebugInfoItem : public BaseItem {
public:
    explicit DebugInfoItem(LineNumberProgramItem *item) : program_(item) {}

    ~DebugInfoItem() override = default;

    DEFAULT_MOVE_SEMANTIC(DebugInfoItem);
    DEFAULT_COPY_SEMANTIC(DebugInfoItem);

    size_t GetLineNumber() const
    {
        return lineNum_;
    }

    void SetLineNumber(size_t lineNum)
    {
        lineNum_ = lineNum;
    }

    LineNumberProgramItem *GetLineNumberProgram() const
    {
        return program_;
    }

    void SetLineNumberProgram(LineNumberProgramItem *program)
    {
        ASSERT(program->GetOffset() != 0);
        program_ = program;
    }

    void AddParameter(StringItem *name)
    {
        parameters_.push_back(name);
    }

    const std::vector<StringItem *> *GetParameters() const
    {
        return &parameters_;
    }

    std::vector<uint8_t> *GetConstantPool()
    {
        return &constantPool_;
    }

    size_t CalculateSize() const override;

    bool Write(Writer *writer) override;

    ItemTypes GetItemType() const override
    {
        return ItemTypes::DEBUG_INFO_ITEM;
    }

    void Dump(std::ostream &os) const override;

    void SetDependencyMark() override
    {
        BaseItem::SetDependencyMark();
        if (program_ != nullptr) {
            program_->SetDependencyMark();
        }
        for (auto &item : parameters_) {
            item->SetDependencyMark();
        }
    }

private:
    size_t lineNum_ {0};
    LineNumberProgramItem *program_;
    std::vector<uint8_t> constantPool_;
    std::vector<StringItem *> parameters_;
};

class BaseMethodItem : public IndexedItem {
public:
    ProtoItem *GetProto() const
    {
        return proto_;
    }

    bool IsStatic() const
    {
        return (accessFlags_ & ACC_STATIC) != 0;
    }

    IndexType GetIndexType() const override
    {
        return IndexType::METHOD;
    }

    StringItem *GetNameItem() const
    {
        return name_;
    }

    BaseClassItem *GetClassItem() const
    {
        return class_;
    }

    uint32_t GetAccessFlags() const
    {
        return accessFlags_;
    }

    ~BaseMethodItem() override = default;

    DEFAULT_MOVE_SEMANTIC(BaseMethodItem);
    DEFAULT_COPY_SEMANTIC(BaseMethodItem);

protected:
    PANDA_PUBLIC_API BaseMethodItem(BaseClassItem *cls, StringItem *name, ProtoItem *proto, uint32_t accessFlags);

    PANDA_PUBLIC_API size_t CalculateSize() const override;

    PANDA_PUBLIC_API bool Write(Writer *writer) override;

    PANDA_PUBLIC_API void SetDependencyMark() override;

private:
    BaseClassItem *class_;
    StringItem *name_;
    ProtoItem *proto_;
    uint32_t accessFlags_;
};

class MethodParamItem {
public:
    explicit MethodParamItem(TypeItem *type) : type_(type) {}

    ~MethodParamItem() = default;

    DEFAULT_MOVE_SEMANTIC(MethodParamItem);
    DEFAULT_COPY_SEMANTIC(MethodParamItem);

    void AddRuntimeAnnotation(AnnotationItem *runtimeAnnotation)
    {
        runtimeAnnotations_.push_back(runtimeAnnotation);
    }

    void AddAnnotation(AnnotationItem *annotation)
    {
        annotations_.push_back(annotation);
    }

    void AddRuntimeTypeAnnotation(AnnotationItem *runtimeTypeAnnotation)
    {
        runtimeTypeAnnotations_.push_back(runtimeTypeAnnotation);
    }

    void AddTypeAnnotation(AnnotationItem *typeAnnotation)
    {
        typeAnnotations_.push_back(typeAnnotation);
    }

    TypeItem *GetType() const
    {
        return type_;
    }

    const std::vector<AnnotationItem *> &GetRuntimeAnnotations() const
    {
        return runtimeAnnotations_;
    }

    const std::vector<AnnotationItem *> &GetAnnotations() const
    {
        return annotations_;
    }

    const std::vector<AnnotationItem *> &GetRuntimeTypeAnnotations() const
    {
        return runtimeTypeAnnotations_;
    }

    const std::vector<AnnotationItem *> &GetTypeAnnotations() const
    {
        return typeAnnotations_;
    }

    bool HasAnnotations() const
    {
        return !annotations_.empty();
    }

    bool HasRuntimeAnnotations() const
    {
        return !runtimeAnnotations_.empty();
    }

private:
    TypeItem *type_;
    std::vector<AnnotationItem *> runtimeAnnotations_;
    std::vector<AnnotationItem *> annotations_;
    std::vector<AnnotationItem *> typeAnnotations_;
    std::vector<AnnotationItem *> runtimeTypeAnnotations_;
};

class ParamAnnotationsItem;
class BaseClassItem;

class MethodItem : public BaseMethodItem {
public:
    PANDA_PUBLIC_API MethodItem(ClassItem *cls, StringItem *name, ProtoItem *proto, uint32_t accessFlags,
                                std::vector<MethodParamItem> params);

    ~MethodItem() override = default;

    DEFAULT_MOVE_SEMANTIC(MethodItem);
    DEFAULT_COPY_SEMANTIC(MethodItem);

    void SetSourceLang(SourceLang lang)
    {
        sourceLang_ = lang;
    }

    void SetCode(CodeItem *code)
    {
        code_ = code;
    }

    void SetDebugInfo(DebugInfoItem *debugInfo)
    {
        debugInfo_ = debugInfo;
    }

    DebugInfoItem *GetDebugInfo()
    {
        return debugInfo_;
    }

    void AddRuntimeAnnotation(AnnotationItem *runtimeAnnotation)
    {
        runtimeAnnotations_.push_back(runtimeAnnotation);
    }

    void AddAnnotation(AnnotationItem *annotation)
    {
        annotations_.push_back(annotation);
    }

    void AddRuntimeTypeAnnotation(AnnotationItem *runtimeTypeAnnotation)
    {
        runtimeTypeAnnotations_.push_back(runtimeTypeAnnotation);
    }

    void AddTypeAnnotation(AnnotationItem *typeAnnotation)
    {
        typeAnnotations_.push_back(typeAnnotation);
    }

    void SetRuntimeParamAnnotationItem(ParamAnnotationsItem *annotations)
    {
        runtimeParamAnnotations_ = annotations;
    }

    void SetParamAnnotationItem(ParamAnnotationsItem *annotations)
    {
        paramAnnotations_ = annotations;
    }

    bool HasRuntimeParamAnnotations() const
    {
        return std::any_of(params_.cbegin(), params_.cend(),
                           [](const MethodParamItem &item) { return item.HasRuntimeAnnotations(); });
    }

    bool HasParamAnnotations() const
    {
        return std::any_of(params_.cbegin(), params_.cend(),
                           [](const MethodParamItem &item) { return item.HasAnnotations(); });
    }

    CodeItem *GetCode() const
    {
        return code_;
    }

    DebugInfoItem *GetDebugInfo() const
    {
        return debugInfo_;
    }

    size_t CalculateSize() const override;

    bool Write(Writer *writer) override;

    ItemTypes GetItemType() const override
    {
        return ItemTypes::METHOD_ITEM;
    }

    std::vector<MethodParamItem> &GetParams()
    {
        return params_;
    }

    std::vector<AnnotationItem *> *GetRuntimeAnnotations()
    {
        return &runtimeAnnotations_;
    }

    std::vector<AnnotationItem *> *GetAnnotations()
    {
        return &annotations_;
    }

    std::vector<AnnotationItem *> *GetTypeAnnotations()
    {
        return &typeAnnotations_;
    }

    std::vector<AnnotationItem *> *GetRuntimeTypeAnnotations()
    {
        return &runtimeTypeAnnotations_;
    }

    void SetProfileSize(size_t size)
    {
        ASSERT(size <= MAX_PROFILE_SIZE);
        profileSize_ = size;
    }

    size_t GetProfileSize() const
    {
        return profileSize_;
    }

    void SetDependencyMark() override;

private:
    bool WriteRuntimeAnnotations(Writer *writer);

    bool WriteTypeAnnotations(Writer *writer);

    bool WriteTaggedData(Writer *writer);

    std::vector<MethodParamItem> params_;

    SourceLang sourceLang_ {SourceLang::PANDA_ASSEMBLY};
    CodeItem *code_ {nullptr};
    DebugInfoItem *debugInfo_ {nullptr};
    std::vector<AnnotationItem *> runtimeAnnotations_;
    std::vector<AnnotationItem *> annotations_;
    std::vector<AnnotationItem *> typeAnnotations_;
    std::vector<AnnotationItem *> runtimeTypeAnnotations_;
    ParamAnnotationsItem *runtimeParamAnnotations_ {nullptr};
    ParamAnnotationsItem *paramAnnotations_ {nullptr};
    uint16_t profileSize_ {0};

public:
    constexpr static auto MAX_PROFILE_SIZE = std::numeric_limits<decltype(profileSize_)>::max();
};

class BaseClassItem : public TypeItem {
public:
    StringItem *GetNameItem()
    {
        return &name_;
    }

    const std::string &GetNameItemData() const
    {
        return name_.GetData();
    }

protected:
    explicit BaseClassItem(const std::string &name) : TypeItem(Type::TypeId::REFERENCE), name_(name) {}

    ~BaseClassItem() override = default;

    size_t CalculateSize() const override;

    void ComputeLayout() override;

    bool Write(Writer *writer) override;

    DEFAULT_MOVE_SEMANTIC(BaseClassItem);
    DEFAULT_COPY_SEMANTIC(BaseClassItem);

private:
    StringItem name_;
};

class ClassItem : public BaseClassItem {
public:
    explicit ClassItem(const std::string &name) : BaseClassItem(name) {}

    ~ClassItem() override = default;

    void SetAccessFlags(uint32_t accessFlags)
    {
        accessFlags_ = accessFlags;
    }

    void SetSourceLang(SourceLang lang)
    {
        sourceLang_ = lang;
    }

    void SetSuperClass(BaseClassItem *superClass)
    {
        superClass_ = superClass;
    }

    void AddInterface(BaseClassItem *iface)
    {
        AddIndexDependency(iface);
        ifaces_.push_back(iface);
    }

    void AddRuntimeAnnotation(AnnotationItem *runtimeAnnotation)
    {
        runtimeAnnotations_.push_back(runtimeAnnotation);
    }

    void AddAnnotation(AnnotationItem *annotation)
    {
        annotations_.push_back(annotation);
    }

    void AddRuntimeTypeAnnotation(AnnotationItem *runtimeTypeAnnotation)
    {
        runtimeTypeAnnotations_.push_back(runtimeTypeAnnotation);
    }

    void AddTypeAnnotation(AnnotationItem *typeAnnotation)
    {
        typeAnnotations_.push_back(typeAnnotation);
    }

    template <class... Args>
    FieldItem *AddField(Args... args)
    {
        fields_.emplace_back(std::make_unique<FieldItem>(this, std::forward<Args>(args)...));
        return fields_.back().get();
    }

    template <class... Args>
    MethodItem *AddMethod(Args... args)
    {
        // insert new method to set ordered by method name
        return methods_.insert(std::make_unique<MethodItem>(this, std::forward<Args>(args)...))->get();
    }

    StringItem *GetSourceFile() const
    {
        return sourceFile_;
    }

    void SetSourceFile(StringItem *item)
    {
        sourceFile_ = item;
    }

    size_t CalculateSizeWithoutFieldsAndMethods() const;

    size_t CalculateSize() const override;

    void ComputeLayout() override;

    bool Write(Writer *writer) override;

    ItemTypes GetItemType() const override
    {
        return ItemTypes::CLASS_ITEM;
    }

    void VisitFields(const VisitorCallBack &cb)
    {
        for (auto &field : fields_) {
            if (!cb(field.get())) {
                break;
            }
        }
    }

    void VisitMethods(const VisitorCallBack &cb)
    {
        for (auto &method : methods_) {
            if (!cb(method.get())) {
                break;
            }
        }
    }

    void Visit(const VisitorCallBack &cb) override
    {
        VisitFields(cb);
        VisitMethods(cb);
    }

    std::vector<AnnotationItem *> *GetRuntimeAnnotations()
    {
        return &runtimeAnnotations_;
    }

    std::vector<AnnotationItem *> *GetAnnotations()
    {
        return &annotations_;
    }

    std::vector<AnnotationItem *> *GetTypeAnnotations()
    {
        return &typeAnnotations_;
    }

    std::vector<AnnotationItem *> *GetRuntimeTypeAnnotations()
    {
        return &runtimeTypeAnnotations_;
    }

    SourceLang GetSourceLang() const
    {
        return sourceLang_;
    }

    uint32_t GetAccessFlags() const
    {
        return accessFlags_;
    }

    BaseClassItem *GetSuperClass() const
    {
        return superClass_;
    }

    const std::vector<BaseClassItem *> &GetInterfaces() const
    {
        return ifaces_;
    }
    void SetDependencyMark() override;

    DEFAULT_MOVE_SEMANTIC(ClassItem);
    DEFAULT_COPY_SEMANTIC(ClassItem);

private:
    struct MethodCompByName {
        using is_transparent = std::true_type;  // NOLINT(readability-identifier-naming)

        bool operator()(const StringItem *str1, const StringItem *str2) const
        {
            if (str1->GetUtf16Len() == str2->GetUtf16Len()) {
                return str1->GetData() < str2->GetData();
            }
            return str1->GetUtf16Len() < str2->GetUtf16Len();
        }

        bool operator()(const StringItem *m1, const std::unique_ptr<MethodItem> &m2) const
        {
            return (*this)(m1, m2->GetNameItem());
        }

        bool operator()(const std::unique_ptr<MethodItem> &m1, const StringItem *str2) const
        {
            return (*this)(m1->GetNameItem(), str2);
        }

        bool operator()(const std::unique_ptr<MethodItem> &m1, const std::unique_ptr<MethodItem> &m2) const
        {
            return (*this)(m1->GetNameItem(), m2->GetNameItem());
        }
    };

    bool WriteIfaces(Writer *writer);

    bool WriteAnnotations(Writer *writer);

    bool WriteTaggedData(Writer *writer);

    BaseClassItem *superClass_ {nullptr};
    uint32_t accessFlags_ {0};
    SourceLang sourceLang_ {SourceLang::PANDA_ASSEMBLY};
    std::vector<BaseClassItem *> ifaces_;
    std::vector<AnnotationItem *> runtimeAnnotations_;
    std::vector<AnnotationItem *> annotations_;
    std::vector<AnnotationItem *> typeAnnotations_;
    std::vector<AnnotationItem *> runtimeTypeAnnotations_;
    StringItem *sourceFile_ {nullptr};
    std::vector<std::unique_ptr<FieldItem>> fields_;
    std::multiset<std::unique_ptr<MethodItem>, MethodCompByName> methods_;

public:
    using FindMethodIterator = typename std::multiset<std::unique_ptr<MethodItem>, MethodCompByName>::const_iterator;

    std::pair<FindMethodIterator, FindMethodIterator> FindMethod(StringItem *name) const
    {
        return methods_.equal_range(name);
    }
};

class ForeignClassItem : public BaseClassItem {
public:
    explicit ForeignClassItem(const std::string &name) : BaseClassItem(name) {}

    ~ForeignClassItem() override = default;

    bool IsForeign() const override
    {
        return true;
    }

    ItemTypes GetItemType() const override
    {
        return ItemTypes::FOREIGN_CLASS_ITEM;
    }

    DEFAULT_MOVE_SEMANTIC(ForeignClassItem);
    DEFAULT_COPY_SEMANTIC(ForeignClassItem);
};

class ForeignFieldItem : public BaseFieldItem {
public:
    ForeignFieldItem(BaseClassItem *cls, StringItem *name, TypeItem *type, uint32_t accessFlags)
        : BaseFieldItem(cls, name, type, accessFlags)
    {
    }

    ~ForeignFieldItem() override = default;

    bool IsForeign() const override
    {
        return true;
    }

    ItemTypes GetItemType() const override
    {
        return ItemTypes::FOREIGN_FIELD_ITEM;
    }

    DEFAULT_MOVE_SEMANTIC(ForeignFieldItem);
    DEFAULT_COPY_SEMANTIC(ForeignFieldItem);
};

class ForeignMethodItem : public BaseMethodItem {
public:
    ForeignMethodItem(BaseClassItem *cls, StringItem *name, ProtoItem *proto, uint32_t accessFlags)
        : BaseMethodItem(cls, name, proto, accessFlags)
    {
    }

    ~ForeignMethodItem() override = default;

    bool IsForeign() const override
    {
        return true;
    }

    ItemTypes GetItemType() const override
    {
        return ItemTypes::FOREIGN_METHOD_ITEM;
    }

    DEFAULT_MOVE_SEMANTIC(ForeignMethodItem);
    DEFAULT_COPY_SEMANTIC(ForeignMethodItem);
};

class ProtoItem;

class ParamAnnotationsItem : public BaseItem {
public:
    PANDA_PUBLIC_API ParamAnnotationsItem(MethodItem *method, bool isRuntimeAnnotations);

    ~ParamAnnotationsItem() override = default;

    ItemTypes GetItemType() const override
    {
        return ItemTypes::PARAM_ANNOTATIONS_ITEM;
    }

    size_t CalculateSize() const override;

    bool Write(Writer *writer) override;

    DEFAULT_MOVE_SEMANTIC(ParamAnnotationsItem);
    DEFAULT_COPY_SEMANTIC(ParamAnnotationsItem);

private:
    std::vector<std::vector<AnnotationItem *>> annotations_;
};

class ProtoItem : public IndexedItem {
public:
    ProtoItem(TypeItem *retType, const std::vector<MethodParamItem> &params);

    ~ProtoItem() override = default;

    DEFAULT_MOVE_SEMANTIC(ProtoItem);
    DEFAULT_COPY_SEMANTIC(ProtoItem);

    size_t CalculateSize() const override
    {
        size_t size = shorty_.size() * sizeof(uint16_t);
        size += referenceTypes_.size() * IDX_SIZE;
        return size;
    }

    bool Write(Writer *writer) override;

    ItemTypes GetItemType() const override
    {
        return ItemTypes::PROTO_ITEM;
    }

    IndexType GetIndexType() const override
    {
        return IndexType::PROTO;
    }

    size_t Alignment() override
    {
        return sizeof(uint16_t);
    }

    const std::vector<uint16_t> &GetShorty() const
    {
        return shorty_;
    }

    const std::vector<TypeItem *> &GetRefTypes() const
    {
        return referenceTypes_;
    }

    void SetDependencyMark() override;

private:
    static constexpr size_t SHORTY_ELEM_SIZE = 4;

    void AddType(TypeItem *type, size_t *n);

    std::vector<uint16_t> shorty_;
    std::vector<TypeItem *> referenceTypes_;
};

class PANDA_PUBLIC_API CodeItem : public BaseItem {
public:
    class PANDA_PUBLIC_API CatchBlock : public BaseItem {
    public:
        CatchBlock(MethodItem *method, BaseClassItem *type, size_t handlerPc, size_t codeSize = 0)
            : method_(method), type_(type), handlerPc_(handlerPc), codeSize_(codeSize)
        {
        }

        ~CatchBlock() override = default;

        DEFAULT_MOVE_SEMANTIC(CatchBlock);
        DEFAULT_COPY_SEMANTIC(CatchBlock);

        MethodItem *GetMethod() const
        {
            return method_;
        }

        BaseClassItem *GetType() const
        {
            return type_;
        }

        size_t GetHandlerPc() const
        {
            return handlerPc_;
        }

        size_t GetCodeSize() const
        {
            return codeSize_;
        }

        size_t CalculateSize() const override;

        bool Write(Writer *writer) override;

        ItemTypes GetItemType() const override
        {
            return ItemTypes::CATCH_BLOCK_ITEM;
        }

    private:
        MethodItem *method_;
        BaseClassItem *type_;
        size_t handlerPc_;
        size_t codeSize_;
    };

    class PANDA_PUBLIC_API TryBlock : public BaseItem {
    public:
        TryBlock(size_t startPc, size_t length, std::vector<CatchBlock> catchBlocks)
            : startPc_(startPc), length_(length), catchBlocks_(std::move(catchBlocks))
        {
        }

        ~TryBlock() override = default;

        DEFAULT_MOVE_SEMANTIC(TryBlock);
        DEFAULT_COPY_SEMANTIC(TryBlock);

        size_t GetStartPc() const
        {
            return startPc_;
        }

        size_t GetLength() const
        {
            return length_;
        }

        std::vector<CatchBlock> GetCatchBlocks() const
        {
            return catchBlocks_;
        }

        size_t CalculateSizeWithoutCatchBlocks() const;

        void ComputeLayout() override;

        size_t CalculateSize() const override;

        bool Write(Writer *writer) override;

        ItemTypes GetItemType() const override
        {
            return ItemTypes::TRY_BLOCK_ITEM;
        }

    private:
        size_t startPc_;
        size_t length_;
        std::vector<CatchBlock> catchBlocks_;
    };

    CodeItem(size_t numVregs, size_t numArgs, std::vector<uint8_t> instructions)
        : numVregs_(numVregs), numArgs_(numArgs), instructions_(std::move(instructions))
    {
    }

    CodeItem() = default;

    ~CodeItem() override = default;

    void SetNumVregs(size_t numVregs)
    {
        numVregs_ = numVregs;
    }

    void SetNumArgs(size_t numArgs)
    {
        numArgs_ = numArgs;
    }

    std::vector<uint8_t> *GetInstructions()
    {
        return &instructions_;
    }

    void SetNumInstructions(size_t numIns)
    {
        numIns_ = numIns;
    }

    size_t GetNumInstructions() const
    {
        return numIns_;
    }

    std::vector<TryBlock> GetTryBlocks()
    {
        return tryBlocks_;
    }

    void AddTryBlock(const TryBlock &tryBlock)
    {
        tryBlocks_.push_back(tryBlock);
    }

    size_t CalculateSizeWithoutTryBlocks() const;

    void ComputeLayout() override;

    size_t CalculateSize() const override;

    size_t GetCodeSize() const;

    bool Write(Writer *writer) override;

    ItemTypes GetItemType() const override
    {
        return ItemTypes::CODE_ITEM;
    }

    void AddMethod(BaseMethodItem *method)
    {
        methods_.emplace_back(method);
    }

    std::vector<std::string> GetMethodNames() const
    {
        std::vector<std::string> names;
        for (const auto *method : methods_) {
            if (method == nullptr) {
                continue;
            }
            std::string className;
            if (method->GetClassItem() != nullptr) {
                className = method->GetClassItem()->GetNameItem()->GetData();
                className.pop_back();          // remove '\0'
                ASSERT(className.size() > 2);  // 2 - L and ;
                className.erase(0, 1);
                className.pop_back();
                className.append("::");
            }
            className.append(method->GetNameItem()->GetData());
            className.pop_back();  // remove '\0'
            names.emplace_back(className);
        }
        return names;
    }

    size_t GetNumVregs()
    {
        return numVregs_;
    }

    size_t GetNumArgs()
    {
        return numArgs_;
    }

    DEFAULT_MOVE_SEMANTIC(CodeItem);
    DEFAULT_COPY_SEMANTIC(CodeItem);

private:
    size_t numVregs_ {0};
    size_t numArgs_ {0};
    size_t numIns_ {0};
    std::vector<uint8_t> instructions_;
    std::vector<TryBlock> tryBlocks_;
    std::vector<BaseMethodItem *> methods_;
};

class ScalarValueItem;
class ArrayValueItem;

class ValueItem : public BaseItem {
public:
    enum class Type { INTEGER, LONG, FLOAT, DOUBLE, ID, ARRAY };

    explicit ValueItem(Type type) : type_(type) {}

    ~ValueItem() override = default;

    DEFAULT_MOVE_SEMANTIC(ValueItem);
    DEFAULT_COPY_SEMANTIC(ValueItem);

    Type GetType() const
    {
        return type_;
    }

    bool IsArray() const
    {
        return type_ == Type::ARRAY;
    }

    bool Is32bit() const
    {
        return type_ == Type::INTEGER || type_ == Type::FLOAT || type_ == Type::ID;
    }

    ItemTypes GetItemType() const override
    {
        return ItemTypes::VALUE_ITEM;
    }

    ScalarValueItem *GetAsScalar();

    ArrayValueItem *GetAsArray();

private:
    Type type_;
};

class PANDA_PUBLIC_API ScalarValueItem : public ValueItem {
public:
    explicit ScalarValueItem(uint32_t v) : ValueItem(Type::INTEGER), value_(v) {}

    explicit ScalarValueItem(uint64_t v) : ValueItem(Type::LONG), value_(v) {}

    explicit ScalarValueItem(float v) : ValueItem(Type::FLOAT), value_(v) {}

    explicit ScalarValueItem(double v) : ValueItem(Type::DOUBLE), value_(v) {}

    explicit ScalarValueItem(BaseItem *v) : ValueItem(Type::ID), value_(v) {}

    ~ScalarValueItem() override = default;

    DEFAULT_MOVE_SEMANTIC(ScalarValueItem);
    DEFAULT_COPY_SEMANTIC(ScalarValueItem);

    template <class T>
    T GetValue() const
    {
        return std::get<T>(value_);
    }

    template <class T>
    bool HasValue() const
    {
        return std::holds_alternative<T>(value_);
    }

    File::EntityId GetId() const
    {
        return File::EntityId(GetValue<BaseItem *>()->GetOffset());
    }

    BaseItem *GetIdItem() const
    {
        return GetValue<BaseItem *>();
    }

    size_t GetULeb128EncodedSize();

    size_t GetSLeb128EncodedSize();

    size_t CalculateSize() const override;

    size_t Alignment() override;

    bool Write(Writer *writer) override;

    bool WriteAsUleb128(Writer *writer);

    void SetDependencyMark() override
    {
        if (GetType() == Type::ID) {
            if (!GetIdItem()->GetDependencyMark()) {
                GetIdItem()->SetDependencyMark();
            }
        } else {
            ValueItem::SetDependencyMark();
        }
    }

private:
    std::variant<uint32_t, uint64_t, float, double, BaseItem *> value_;
};

class PANDA_PUBLIC_API ArrayValueItem : public ValueItem {
public:
    ArrayValueItem(panda_file::Type componentType, std::vector<ScalarValueItem> items)
        : ValueItem(Type::ARRAY), componentType_(componentType), items_(std::move(items))
    {
    }

    ~ArrayValueItem() override = default;

    DEFAULT_MOVE_SEMANTIC(ArrayValueItem);
    DEFAULT_COPY_SEMANTIC(ArrayValueItem);

    size_t CalculateSize() const override;

    void ComputeLayout() override;

    bool Write(Writer *writer) override;

    panda_file::Type GetComponentType() const
    {
        return componentType_;
    }

    const std::vector<ScalarValueItem> &GetItems() const
    {
        return items_;
    }

    std::vector<ScalarValueItem> *GetMutableItems()
    {
        return &items_;
    }

    void SetDependencyMark() override;

private:
    size_t GetComponentSize() const;

    panda_file::Type componentType_;
    std::vector<ScalarValueItem> items_;
};

class LiteralItem;
class LiteralArrayItem;

class PANDA_PUBLIC_API LiteralItem : public BaseItem {
public:
    enum class Type { B1, B2, B4, B8, STRING, METHOD, LITERALARRAY };

    explicit LiteralItem(uint8_t v) : type_(Type::B1), value_(v) {}

    explicit LiteralItem(uint16_t v) : type_(Type::B2), value_(v) {}

    explicit LiteralItem(uint32_t v) : type_(Type::B4), value_(v) {}

    explicit LiteralItem(uint64_t v) : type_(Type::B8), value_(v) {}

    explicit LiteralItem(StringItem *v) : type_(Type::STRING), value_(v) {}

    explicit LiteralItem(MethodItem *v) : type_(Type::METHOD), value_(v) {}

    explicit LiteralItem(LiteralArrayItem *v) : type_(Type::LITERALARRAY), value_(v) {}

    ~LiteralItem() override = default;

    DEFAULT_MOVE_SEMANTIC(LiteralItem);
    DEFAULT_COPY_SEMANTIC(LiteralItem);

    Type GetType() const
    {
        return type_;
    }

    ItemTypes GetItemType() const override
    {
        return ItemTypes::LITERAL_ITEM;
    }

    template <class T>
    T GetValue() const
    {
        return std::get<T>(value_);
    }

    size_t CalculateSize() const override;

    size_t Alignment() override;

    File::EntityId GetId() const
    {
        return File::EntityId(GetValue<StringItem *>()->GetOffset());
    }

    File::EntityId GetLiteralArrayFileId() const;

    File::EntityId GetMethodId() const
    {
        return File::EntityId(GetValue<MethodItem *>()->GetFileId());
    }

    bool Write(Writer *writer) override;

    void SetDependencyMark() override;

private:
    Type type_;
    std::variant<uint8_t, uint16_t, uint32_t, uint64_t, StringItem *, MethodItem *, LiteralArrayItem *> value_;
};

class LiteralArrayItem : public ValueItem {
public:
    explicit LiteralArrayItem() : ValueItem(Type::ARRAY) {}

    ~LiteralArrayItem() override = default;

    DEFAULT_MOVE_SEMANTIC(LiteralArrayItem);
    DEFAULT_COPY_SEMANTIC(LiteralArrayItem);

    PANDA_PUBLIC_API void AddItems(const std::vector<LiteralItem> &item);

    const std::vector<LiteralItem> &GetItems() const
    {
        return items_;
    }

    size_t CalculateSize() const override;

    void ComputeLayout() override;

    bool Write(Writer *writer) override;

    ItemTypes GetItemType() const override
    {
        return ItemTypes::LITERAL_ARRAY_ITEM;
    }

    void SetIndex(uint32_t index)
    {
        index_ = index;
    }

    uint32_t GetIndex() const
    {
        return index_;
    }

    void SetDependencyMark() override;

private:
    std::vector<LiteralItem> items_;
    uint32_t index_ {0};
};

class PANDA_PUBLIC_API AnnotationItem : public BaseItem {
public:
    class Elem {
    public:
        Elem(StringItem *name, ValueItem *value) : name_(name), value_(value)
        {
            value_->SetNeedsEmit(!value_->Is32bit());
        }

        ~Elem() = default;

        DEFAULT_MOVE_SEMANTIC(Elem);
        DEFAULT_COPY_SEMANTIC(Elem);

        const StringItem *GetName() const
        {
            return name_;
        }

        ValueItem *GetValue() const
        {
            return value_;
        }

        void SetValue(ValueItem *item)
        {
            value_ = item;
        }

        void SetDependencyMark()
        {
            if (name_ != nullptr) {
                name_->SetDependencyMark();
            }
            if (value_ != nullptr) {
                if (value_->IsArray()) {
                    value_->GetAsArray()->SetDependencyMark();
                } else {
                    value_->SetDependencyMark();
                }
            }
        }

    private:
        StringItem *name_;
        ValueItem *value_;
    };

    class Tag {
    public:
        explicit Tag(char item) : item_(item) {}

        ~Tag() = default;

        DEFAULT_MOVE_SEMANTIC(Tag);
        DEFAULT_COPY_SEMANTIC(Tag);

        uint8_t GetItem() const
        {
            return item_;
        }

    private:
        uint8_t item_;
    };

    AnnotationItem(BaseClassItem *cls, std::vector<Elem> elements, std::vector<Tag> tags)
        : class_(cls), elements_(std::move(elements)), tags_(std::move(tags))
    {
        AddIndexDependency(cls);
    }

    ~AnnotationItem() override = default;

    DEFAULT_MOVE_SEMANTIC(AnnotationItem);
    DEFAULT_COPY_SEMANTIC(AnnotationItem);

    size_t CalculateSize() const override;

    bool Write(Writer *writer) override;

    ItemTypes GetItemType() const override
    {
        return ItemTypes::ANNOTATION_ITEM;
    }

    BaseClassItem *GetClassItem() const
    {
        return class_;
    }

    std::vector<Elem> *GetElements()
    {
        return &elements_;
    }

    const std::vector<Elem> *GetElements() const
    {
        return &elements_;
    }

    void SetElements(std::vector<Elem> &&elements)
    {
        elements_ = std::move(elements);
    }

    const std::vector<Tag> &GetTags() const
    {
        return tags_;
    }

    void SetTags(std::vector<Tag> &&tags)
    {
        tags_ = std::move(tags);
    }
    void SetDependencyMark() override;

private:
    BaseClassItem *class_;
    std::vector<Elem> elements_;
    std::vector<Tag> tags_;
};

enum class MethodHandleType : uint8_t {
    PUT_STATIC = 0x00,
    GET_STATIC = 0x01,
    PUT_INSTANCE = 0x02,
    GET_INSTANCE = 0x03,
    INVOKE_STATIC = 0x04,
    INVOKE_INSTANCE = 0x05,
    INVOKE_CONSTRUCTOR = 0x06,
    INVOKE_DIRECT = 0x07,
    INVOKE_INTERFACE = 0x08
};

class PANDA_PUBLIC_API MethodHandleItem : public BaseItem {
public:
    MethodHandleItem(MethodHandleType type, BaseItem *entity) : type_(type), entity_(entity) {}

    ~MethodHandleItem() override = default;

    DEFAULT_MOVE_SEMANTIC(MethodHandleItem);
    DEFAULT_COPY_SEMANTIC(MethodHandleItem);

    size_t CalculateSize() const override
    {
        return sizeof(uint8_t) + leb128::UnsignedEncodingSize(entity_->GetOffset());
    }

    bool Write(Writer *writer) override;

    ItemTypes GetItemType() const override
    {
        return ItemTypes::METHOD_HANDLE_ITEM;
    }

    MethodHandleType GetType() const
    {
        return type_;
    }

private:
    MethodHandleType type_;
    BaseItem *entity_;
};

enum class ArgumentType : uint8_t {
    INTEGER = 0x00,
    LONG = 0x01,
    FLOAT = 0x02,
    DOUBLE = 0x03,
    STRING = 0x04,
    CLASS = 0x05,
    METHOD_HANDLE = 0x06,
    METHOD_TYPE = 0x07
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_FILE_ITEMS_H_
