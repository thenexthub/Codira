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

#ifndef PANDA_LINKER_CONTEXT_H
#define PANDA_LINKER_CONTEXT_H

#include <forward_list>
#include <functional>

#include "libarkfile/bytecode_instruction.h"
#include "libarkfile/file_items.h"
#include "libarkfile/file_reader.h"
#include "libarkfile/file_item_container.h"

#include "linker.h"
#include "libarkbase/macros.h"

namespace ark::static_linker {

class Context;

class CodePatcher {
public:
    struct IndexedChange {
        BytecodeInstruction inst;
        panda_file::MethodItem *mi;
        panda_file::IndexedItem *it;
    };

    struct StringChange {
        BytecodeInstruction inst;
        std::string str;
        panda_file::MethodItem *mi;
        panda_file::StringItem *it {};
    };

    struct LiteralArrayChange {
        BytecodeInstruction inst;
        panda_file::LiteralArrayItem *old;

        panda_file::LiteralArrayItem *it {};
    };

    using Change =
        std::variant<IndexedChange, StringChange, LiteralArrayChange, std::string, std::function<bool(bool peek)>>;

    void Add(Change c);

    void ApplyDeps(Context *ctx);

    void TryDeletePatch();
    void Patch(const std::pair<size_t, size_t> range);
    void AddStringDependency();

    void Devour(CodePatcher &&p);

    void AddRange(std::pair<size_t, size_t> range);

    const std::vector<std::pair<size_t, size_t>> &GetRanges() const
    {
        return ranges_;
    }

    void Clear()
    {
        changes_.clear();
        ranges_.clear();
    }

    size_t GetSize() const
    {
        return changes_.size();
    }

private:
    std::vector<Change> changes_;
    std::vector<std::pair<size_t, size_t>> ranges_;

    void ApplyLiteralArrayChange(LiteralArrayChange &lc, Context *ctx);
};

struct CodeData {
    std::vector<uint8_t> *code;
    panda_file::MethodItem *omi;
    panda_file::MethodItem *nmi;
    const panda_file::FileReader *fileReader;

    bool patchLnp;
};

class Helpers {
public:
    static std::vector<panda_file::Type> BreakProto(panda_file::ProtoItem *p);
};

class Context {
public:
    explicit Context(Config conf);

    ~Context();

    NO_COPY_SEMANTIC(Context);
    NO_MOVE_SEMANTIC(Context);

    void Write(const std::string &out);

    void Read(const std::vector<std::string> &input);

    void Merge();

    void Parse();

    void ComputeLayout();

    void Patch();

    void TryDelete();

    const std::unordered_map<panda_file::BaseItem *, panda_file::BaseItem *> &GetKnownItems() const
    {
        return knownItems_;
    }

    panda_file::ItemContainer &GetContainer()
    {
        return cont_;
    }

    const panda_file::ItemContainer &GetContainer() const
    {
        return cont_;
    }

    const Result &GetResult() const
    {
        return result_;
    }

    bool HasErrors() const
    {
        return !result_.errors.empty();
    }

private:
    friend class CodePatcher;

    Config conf_;
    Result result_;
    panda_file::ItemContainer cont_;

    std::vector<std::function<void()>> deferredFailedAnnotations_;

    std::vector<CodeData> codeDatas_;
    CodePatcher patcher_;

    std::forward_list<panda_file::FileReader> readers_;
    std::unordered_map<panda_file::BaseItem *, panda_file::BaseItem *> knownItems_;
    std::multimap<const panda_file::BaseItem *, const panda_file::FileReader *> cameFrom_;
    size_t literalArrayId_ {};
    std::map<std::tuple<panda_file::BaseClassItem *, panda_file::StringItem *, panda_file::TypeItem *>,
             panda_file::ForeignFieldItem *>
        foreignFields_;
    std::map<std::tuple<panda_file::BaseClassItem *, panda_file::StringItem *, panda_file::ProtoItem *, uint32_t>,
             panda_file::ForeignMethodItem *>
        foreignMethods_;

    panda_file::BaseClassItem *ClassFromOld(panda_file::BaseClassItem *old);

    panda_file::TypeItem *TypeFromOld(panda_file::TypeItem *old);

    panda_file::StringItem *StringFromOld(const panda_file::StringItem *s);

    static std::string GetStr(const panda_file::StringItem *si);

    void MergeClass(const panda_file::FileReader *reader, panda_file::ClassItem *ni, panda_file::ClassItem *oi);

    void AddRegularClasses();

    void CheckClassRedifinition(const std::string &name, panda_file::FileReader *reader);

    void FillRegularClasses();

    void MergeMethod(const panda_file::FileReader *reader, panda_file::ClassItem *clz, panda_file::MethodItem *oi);

    void MergeField(const panda_file::FileReader *reader, panda_file::ClassItem *clz, panda_file::FieldItem *oi);

    void MergeForeignMethod(const panda_file::FileReader *reader, panda_file::ForeignMethodItem *fm);

    void MergeForeignMethodCreate(const panda_file::FileReader *reader, panda_file::BaseClassItem *clz,
                                  panda_file::ForeignMethodItem *fm);

    void MergeForeignField(const panda_file::FileReader *reader, panda_file::ForeignFieldItem *ff);

    void MergeForeignFieldCreate(const panda_file::FileReader *reader, panda_file::BaseClassItem *clz,
                                 panda_file::ForeignFieldItem *ff);

    std::pair<bool, bool> UpdateDebugInfo(panda_file::MethodItem *ni, panda_file::MethodItem *oi);

    void CreateTryBlocks(panda_file::MethodItem *ni, panda_file::CodeItem *nci, panda_file::MethodItem *oi,
                         panda_file::CodeItem *oci);

    bool IsSameProto(panda_file::ProtoItem *op1, panda_file::ProtoItem *op2);

    template <typename T>
    struct AddAnnotationImplData {
        const panda_file::FileReader *reader;
        T *ni;
        T *oi;
        size_t from;
        size_t retriesLeft;
    };

    template <typename T, typename Getter, typename Adder>
    void AddAnnotationImpl(AddAnnotationImplData<T> ad, Getter getter, Adder adder);

    template <typename T>
    void TransferAnnotations(const panda_file::FileReader *reader, T *ni, T *oi);

    std::pair<panda_file::ProtoItem *, std::vector<panda_file::MethodParamItem>> GetProto(panda_file::ProtoItem *p);

    bool IsSameType(ark::panda_file::TypeItem *nevv, ark::panda_file::TypeItem *old);

    void ProcessCodeData(CodePatcher &p, CodeData *data);

    void MakeChangeWithId(CodePatcher &p, CodeData *data);

    void HandleStringId(CodePatcher &p, const BytecodeInstruction &inst, const panda_file::File *filePtr,
                        CodeData *data);

    void HandleLiteralArrayId(CodePatcher &p, const BytecodeInstruction &inst, const panda_file::File *filePtr,
                              const std::map<panda_file::File::EntityId, panda_file::BaseItem *> *items);

    void AddItemToKnown(panda_file::BaseItem *item, const std::map<std::string, panda_file::BaseClassItem *> &cm,
                        const panda_file::FileReader &reader);

    void MergeItem(panda_file::BaseItem *item, const panda_file::FileReader &reader);

    void HandleCandidates(const panda_file::FileReader *reader, const std::vector<panda_file::FieldItem *> &candidates,
                          panda_file::ForeignFieldItem *ff);

    bool MethodFind(const std::string &className, const std::string &methodName,
                    std::map<std::string, panda_file::BaseClassItem *> &classesMap);

    bool FileFind(const std::string &fileName, std::map<std::string, panda_file::BaseClassItem *> &classesMap);

    bool HandleEntryDependencies();

    class ErrorDetail {
    public:
        using InfoType = std::variant<const panda_file::BaseItem *, std::string>;

        ErrorDetail(std::string name, const panda_file::BaseItem *item1) : name_(std::move(name)), info_(item1)
        {
            ASSERT(item1 != nullptr);
        }

        explicit ErrorDetail(std::string name, std::string data = "") : name_(std::move(name)), info_(std::move(data))
        {
        }

        const std::string &GetName() const
        {
            return name_;
        }

        const InfoType &GetInfo() const
        {
            return info_;
        }

    private:
        std::string name_;
        InfoType info_;
    };

    class ErrorToStringWrapper {
    public:
        ErrorToStringWrapper(Context *ctx, ErrorDetail error, size_t indent)
            : error_(std::move(error)), indent_(indent), ctx_(ctx)
        {
        }

        DEFAULT_COPY_SEMANTIC(ErrorToStringWrapper);
        DEFAULT_MOVE_SEMANTIC(ErrorToStringWrapper);

        ~ErrorToStringWrapper() = default;

        friend std::ostream &operator<<(std::ostream &o, const ErrorToStringWrapper &self);

    private:
        ErrorDetail error_;
        size_t indent_;
        Context *ctx_;
    };

    ErrorToStringWrapper ErrorToString(ErrorDetail error, size_t indent = 0)
    {
        return ErrorToStringWrapper {this, std::move(error), indent};
    }

    friend std::ostream &operator<<(std::ostream &o, const ErrorToStringWrapper &self);

    void Error(const std::string &msg, const std::vector<ErrorDetail> &details,
               const panda_file::FileReader *reader = nullptr);

    std::variant<std::monostate, panda_file::FieldItem *, panda_file::ForeignClassItem *> TryFindField(
        panda_file::BaseClassItem *klass, const std::string &name, panda_file::TypeItem *expectedType,
        std::vector<panda_file::FieldItem *> *badCandidates);

    std::variant<bool, panda_file::MethodItem *> TryFindMethod(panda_file::BaseClassItem *klass,
                                                               panda_file::ForeignMethodItem *fm,
                                                               std::vector<ErrorDetail> *relatedItems);

    std::variant<panda_file::AnnotationItem *, ErrorDetail> AnnotFromOld(panda_file::AnnotationItem *oa);

    std::variant<panda_file::ValueItem *, ErrorDetail> ArrayValueFromOld(panda_file::ValueItem *oi);

    std::variant<panda_file::ValueItem *, ErrorDetail> ValueFromOld(panda_file::ValueItem *oi);

    std::variant<panda_file::BaseItem *, ErrorDetail> ScalarValueIdFromOld(panda_file::BaseItem *oi);
};

std::ostream &operator<<(std::ostream &o, const static_linker::Context::ErrorToStringWrapper &self);

}  // namespace ark::static_linker

#endif
