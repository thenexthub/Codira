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
#ifndef PANDA_RUNTIME_VTABLE_BUILDER_BASE_H
#define PANDA_RUNTIME_VTABLE_BUILDER_BASE_H

#include "libarkbase/macros.h"
#include "libarkbase/utils/hash.h"
#include "libarkbase/utils/utf.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/file-inl.h"
#include "libarkfile/file_items.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "runtime/class_linker_context.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/vtable_builder_interface.h"

namespace ark {

class ClassLinker;
class ClassLinkerContext;

class MethodInfo {
public:
    MethodInfo(const panda_file::MethodDataAccessor &mda, size_t index, ClassLinkerContext *ctx)
        : pf_(&mda.GetPandaFile()),
          name_(pf_->GetStringData(mda.GetNameId())),
          protoId_(mda.GetProtoId()),
          accessFlags_(mda.GetAccessFlags()),
          classId_(mda.GetClassId()),
          ctx_(ctx),
          vmethodIndex_(index)
    {
    }

    explicit MethodInfo(Method *method, size_t index = 0, bool isBase = false)
        : pf_(method->GetPandaFile()),
          name_(method->GetName()),
          protoId_(method->GetProtoId().GetEntityId()),
          accessFlags_(method->GetAccessFlags()),
          isBase_(isBase),
          classId_(method->GetClass()->GetFileId()),
          method_(method),
          ctx_(method->GetClass()->GetLoadContext()),
          vmethodIndex_(index)
    {
    }

    const panda_file::File::StringData &GetName() const
    {
        return name_;
    }

    const uint8_t *GetClassName() const
    {
        return method_ != nullptr ? method_->GetClass()->GetDescriptor() : pf_->GetStringData(classId_).data;
    }

    Method::ProtoId GetProtoId() const
    {
        return Method::ProtoId(*pf_, protoId_);
    }

    Method *GetMethod() const
    {
        return method_;
    }

    size_t GetVirtualMethodIndex() const
    {
        return vmethodIndex_;
    }

    bool IsAbstract() const
    {
        return (accessFlags_ & ACC_ABSTRACT) != 0;
    }

    bool IsFinal() const
    {
        return (accessFlags_ & ACC_FINAL) != 0;
    }

    bool IsPublic() const
    {
        return (accessFlags_ & ACC_PUBLIC) != 0;
    }

    bool IsProtected() const
    {
        return (accessFlags_ & ACC_PROTECTED) != 0;
    }

    bool IsPrivate() const
    {
        return (accessFlags_ & ACC_PRIVATE) != 0;
    }

    bool IsInterfaceMethod() const
    {
        if (method_ != nullptr) {
            return method_->GetClass()->IsInterface();
        }

        panda_file::ClassDataAccessor cda(*pf_, classId_);
        return cda.IsInterface();
    }

    bool IsBase() const
    {
        return isBase_;
    }

    ClassLinkerContext *GetLoadContext() const
    {
        return ctx_;
    }

    ~MethodInfo() = default;

    bool operator==(MethodInfo &other) = delete;

    DEFAULT_COPY_CTOR(MethodInfo);
    NO_COPY_OPERATOR(MethodInfo);
    NO_MOVE_SEMANTIC(MethodInfo);

private:
    const panda_file::File *pf_;
    panda_file::File::StringData name_;
    panda_file::File::EntityId protoId_;
    uint32_t accessFlags_;
    bool isBase_ {false};

    panda_file::File::EntityId classId_;
    Method *method_ {nullptr};
    ClassLinkerContext *ctx_ {nullptr};

    size_t vmethodIndex_ {0};
};

class VTableInfo {
public:
    explicit VTableInfo(ArenaAllocator *allocator)
        : vmethods_(allocator->Adapter()), copiedMethods_(allocator->Adapter())
    {
    }

    struct MethodEntry {
        explicit MethodEntry(size_t index) : index_(index) {}

        MethodInfo const *CandidateOr(MethodInfo const *orig) const
        {
            return candidate_ != nullptr ? candidate_ : orig;
        }

        size_t GetIndex() const
        {
            return index_;
        }

        MethodInfo const *GetCandidate() const
        {
            return candidate_;
        }

        void SetCandidate(MethodInfo const *candidate)
        {
            candidate_ = candidate;
        }

    private:
        size_t index_ {};
        MethodInfo const *candidate_ {};
    };

    struct CopiedMethodEntry {
        explicit CopiedMethodEntry(size_t index) : index_(index) {}

        size_t GetIndex() const
        {
            return index_;
        }

        CopiedMethod::Status GetStatus() const
        {
            return status_;
        }

        void SetStatus(CopiedMethod::Status status)
        {
            status_ = status;
        }

    private:
        size_t index_ {};
        CopiedMethod::Status status_ {CopiedMethod::Status::ORDINARY};
    };

    auto &Methods()
    {
        return vmethods_;
    }

    auto &Methods() const
    {
        return vmethods_;
    }

    auto &CopiedMethods()
    {
        return copiedMethods_;
    }

    auto &CopiedMethods() const
    {
        return copiedMethods_;
    }

    void AddEntry(const MethodInfo *info);

    void ReplaceEntryWith(const MethodInfo *prev, const MethodInfo *current);

    CopiedMethodEntry &AddCopiedEntry(const MethodInfo *info);

    CopiedMethodEntry &UpdateCopiedEntry(const MethodInfo *orig, const MethodInfo *repl);

    size_t GetVTableSize() const
    {
        return vmethods_.size() + copiedMethods_.size();
    }

    void UpdateClass(Class *klass) const;

    void DumpMappings();

    static void DumpVTable([[maybe_unused]] Class *klass);

private:
    struct MethodNameHash {
        uint32_t operator()(const MethodInfo *methodInfo) const
        {
            return GetHash32String(methodInfo->GetName().data);
        }
    };

    ArenaUnorderedMap<MethodInfo const *, MethodEntry, MethodNameHash> vmethods_;
    ArenaUnorderedMap<MethodInfo const *, CopiedMethodEntry, MethodNameHash> copiedMethods_;
};

template <typename Pred, typename UMap>
class FilterBucketIterator {
public:
    using LocalIter = typename UMap::local_iterator;

    FilterBucketIterator(Pred pred, UMap &umap, const typename UMap::key_type &key) : pred_(pred)
    {
        if (umap.bucket_count() == 0) {
            return;
        }
        valid_ = true;
        auto const bucket = umap.bucket(key);
        iter_ = umap.begin(bucket);
        endIter_ = umap.end(bucket);
        Advance();
    }

    bool IsEmpty() const
    {
        return !valid_ || iter_ == endIter_;
    }

    typename UMap::reference Value()
    {
        ASSERT(!IsEmpty());
        return *iter_;
    }

    void Next()
    {
        ASSERT(!IsEmpty());
        ++iter_;
        Advance();
    }

private:
    void Advance()
    {
        while (!IsEmpty() && !pred_(iter_)) {
            ++iter_;
        }
    }

    bool valid_ {};
    LocalIter iter_ {};
    LocalIter endIter_ {};
    Pred pred_;
};

template <typename UMap>
auto SameNameMethodInfoIterator(UMap &umap, MethodInfo const *info)
{
    auto pred = [info](auto other) { return other->first->GetName() == info->GetName(); };
    return FilterBucketIterator(pred, umap, info);
}

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
template <bool VISIT_SUPERITABLE>
class VTableBuilderBase : public VTableBuilder {
public:
    bool Build(panda_file::ClassDataAccessor *cda, Class *baseClass, ITable itable, ClassLinkerContext *ctx) override;

    bool Build(Span<Method> methods, Class *baseClass, ITable itable, bool isInterface) override;

    bool FilterProxyClassMethods(Span<Method *> input, PandaVector<Method *> *output, Class *baseClass) override;

    void UpdateClass(Class *klass) const override;

    size_t GetNumVirtualMethods() const override
    {
        return numVmethods_;
    }

    size_t GetVTableSize() const override
    {
        return vtable_.GetVTableSize();
    }

    Span<const CopiedMethod> GetCopiedMethods() const override
    {
        return {orderedCopiedMethods_.data(), orderedCopiedMethods_.size()};
    }

protected:
    explicit VTableBuilderBase(ClassLinkerErrorHandler *errHandler) : errorHandler_(errHandler) {}

    [[nodiscard]] virtual bool ProcessClassMethod(const MethodInfo *info) = 0;
    [[nodiscard]] virtual bool ProcessDefaultMethod(ITable itable, size_t itableIdx, MethodInfo *methodInfo) = 0;

    [[nodiscard]] virtual bool ProcessProxyClassMethod([[maybe_unused]] const MethodInfo *info)
    {
        UNREACHABLE();
    }

    [[nodiscard]] bool CollectProxyMethods(PandaVector<Method *> *output);

    ArenaAllocator allocator_ {SpaceType::SPACE_TYPE_INTERNAL};
    VTableInfo vtable_ {&allocator_};
    size_t numVmethods_ {0};
    ArenaVector<CopiedMethod> orderedCopiedMethods_ {allocator_.Adapter()};
    ClassLinkerErrorHandler *errorHandler_;

private:
    void BuildForInterface(panda_file::ClassDataAccessor *cda);

    void BuildForInterface(Span<Method> methods);

    void AddBaseMethods(Class *baseClass);

    [[nodiscard]] bool AddClassMethods(panda_file::ClassDataAccessor *cda, ClassLinkerContext *ctx);

    [[nodiscard]] bool AddClassMethods(Span<Method> methods);

    [[nodiscard]] bool AddProxyClassMethods(Span<Method *> methods);

    [[nodiscard]] bool AddDefaultInterfaceMethods(ITable itable, size_t superItableSize);

    bool hasDefaultMethods_ {false};
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark

#endif  // PANDA_RUNTIME_VTABLE_BUILDER_BASE_H
