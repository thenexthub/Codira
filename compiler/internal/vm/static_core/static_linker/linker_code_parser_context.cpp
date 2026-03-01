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

#include "libarkfile/bytecode_instruction.h"
#include "libarkfile/bytecode_instruction-inl.h"
#include "libarkfile/file_items.h"
#include "libarkfile/debug_info_updater-inl.h"

#include "linker_context.h"

namespace ark::static_linker {

namespace {

class LinkerDebugInfoUpdater : public panda_file::DebugInfoUpdater<LinkerDebugInfoUpdater> {
public:
    using Super = panda_file::DebugInfoUpdater<LinkerDebugInfoUpdater>;

    LinkerDebugInfoUpdater(const panda_file::File *file, panda_file::ItemContainer *cont) : Super(file), cont_(cont) {}

    panda_file::StringItem *GetOrCreateStringItem(const std::string &s)
    {
        return cont_->GetOrCreateStringItem(s);
    }

    panda_file::BaseClassItem *GetType([[maybe_unused]] panda_file::File::EntityId typeId, const std::string &typeName)
    {
        auto *cm = cont_->GetClassMap();
        auto iter = cm->find(typeName);
        if (iter != cm->end()) {
            return iter->second;
        }
        return nullptr;
    }

private:
    panda_file::ItemContainer *cont_;
};

class LinkerDebugInfoScrapper : public panda_file::DebugInfoUpdater<LinkerDebugInfoScrapper> {
public:
    using Super = panda_file::DebugInfoUpdater<LinkerDebugInfoScrapper>;

    LinkerDebugInfoScrapper(const panda_file::File *file, CodePatcher *patcher, panda_file::ItemContainer *cont)
        : Super(file), patcher_(patcher), cont_(cont)
    {
    }

    panda_file::StringItem *GetOrCreateStringItem(const std::string &s)
    {
        patcher_->Add(s);
        return nullptr;
    }

    panda_file::BaseClassItem *GetType([[maybe_unused]] panda_file::File::EntityId typeId, const std::string &typeName)
    {
        auto *cm = cont_->GetClassMap();
        if (cm->find(typeName) == cm->end()) {
            // This action must create a string for primitive types.
            GetOrCreateStringItem(typeName);
        }
        return nullptr;
    }

private:
    CodePatcher *patcher_;
    panda_file::ItemContainer *cont_;
};
}  // namespace

void CodePatcher::Add(Change c)
{
    changes_.emplace_back(std::move(c));
}

void CodePatcher::Devour(CodePatcher &&p)
{
    const auto oldSize = changes_.size();
    changes_.insert(changes_.end(), std::move_iterator(p.changes_.begin()), std::move_iterator(p.changes_.end()));
    const auto newSize = changes_.size();
    p.changes_.clear();

    ranges_.emplace_back(oldSize, newSize);
}

void CodePatcher::AddRange(std::pair<size_t, size_t> range)
{
    ranges_.push_back(range);
}

void CodePatcher::ApplyLiteralArrayChange(LiteralArrayChange &lc, Context *ctx)
{
    auto id = ctx->literalArrayId_++;
    lc.it = ctx->GetContainer().GetOrCreateLiteralArrayItem(std::to_string(id));

    auto &oldIts = lc.old->GetItems();
    auto newIts = std::vector<panda_file::LiteralItem>();
    newIts.reserve(oldIts.size());

    for (const auto &i : oldIts) {
        using LIT = panda_file::LiteralItem::Type;
        switch (i.GetType()) {
            case LIT::B1:
            case LIT::B2:
            case LIT::B4:
            case LIT::B8:
                newIts.emplace_back(i);
                break;
            case LIT::STRING: {
                auto str = ctx->StringFromOld(i.GetValue<panda_file::StringItem *>());
                newIts.emplace_back(str);
                break;
            }
            case LIT::METHOD: {
                auto meth = i.GetValue<panda_file::MethodItem *>();
                auto iter = ctx->knownItems_.find(meth);
                ASSERT(iter != ctx->knownItems_.end());
                ASSERT(iter->second->GetItemType() == panda_file::ItemTypes::METHOD_ITEM);
                newIts.emplace_back(static_cast<panda_file::MethodItem *>(iter->second));
                break;
            }
            default:
                UNREACHABLE();
        }
    }

    lc.it->AddItems(newIts);
}

void CodePatcher::ApplyDeps(Context *ctx)
{
    for (auto &v : changes_) {
        std::visit(
            [this, ctx](auto &a) {
                using T = std::remove_cv_t<std::remove_reference_t<decltype(a)>>;
                // IndexedChange, StringChange, LiteralArrayChange, std::string, std::function<bool(bool)>>
                if constexpr (std::is_same_v<T, IndexedChange>) {
                    a.mi->AddIndexDependency(a.it);
                } else if constexpr (std::is_same_v<T, StringChange>) {
                    a.it = ctx->GetContainer().GetOrCreateStringItem(a.str);
                } else if constexpr (std::is_same_v<T, LiteralArrayChange>) {
                    ApplyLiteralArrayChange(a, ctx);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    // unreferenced string item should be mark
                    auto stringItem = ctx->GetContainer().GetOrCreateStringItem(a);
                    stringItem->SetDependencyMark();
                } else if constexpr (std::is_same_v<T, std::function<bool(bool)>>) {
                    // nothing
                } else {
                    UNREACHABLE();
                }
            },
            v);
    }
}

void CodePatcher::TryDeletePatch()
{
    auto markDependency = [](auto &a, bool &shouldDelete) {
        using T = std::remove_cv_t<std::remove_reference_t<decltype(a)>>;
        if constexpr (std::is_same_v<T, IndexedChange>) {
            if (!a.mi->GetDependencyMark()) {
                shouldDelete = true;
            }
        } else if constexpr (std::is_same_v<T, StringChange>) {
            if (!a.mi->GetDependencyMark()) {
                shouldDelete = true;
            }
        } else if constexpr (std::is_same_v<T, std::function<bool(bool)>>) {
            if (a(true)) {
                shouldDelete = true;
            }
        }
    };

    for (auto it = changes_.begin(); it != changes_.end();) {
        auto &v = *it;
        bool shouldDelete = false;
        std::visit([&](auto &a) { markDependency(a, shouldDelete); }, v);
        if (shouldDelete) {
            it = changes_.erase(it);
        } else {
            ++it;
        }
    }
}

void CodePatcher::Patch(const std::pair<size_t, size_t> range)
{
    for (size_t i = range.first; i < range.second; i++) {
        auto &v = changes_[i];
        std::visit(
            [](auto &a) {
                using T = std::remove_cv_t<std::remove_reference_t<decltype(a)>>;
                if constexpr (std::is_same_v<T, IndexedChange>) {
                    auto idx = a.it->GetIndex(a.mi);
                    a.inst.UpdateId(BytecodeId(idx));
                } else if constexpr (std::is_same_v<T, StringChange>) {
                    auto off = a.it->GetOffset();
                    a.inst.UpdateId(BytecodeId(off));
                } else if constexpr (std::is_same_v<T, LiteralArrayChange>) {
                    auto off = a.it->GetIndex();
                    a.inst.UpdateId(BytecodeId(off));
                } else if constexpr (std::is_same_v<T, std::string>) {
                    // nothing
                } else if constexpr (std::is_same_v<T, std::function<bool(bool)>>) {
                    a(false);
                } else {
                    UNREACHABLE();
                }
            },
            v);
    }
}

void CodePatcher::AddStringDependency()
{
    auto markStringDependency = [](auto &a) {
        using T = std::remove_cv_t<std::remove_reference_t<decltype(a)>>;
        if constexpr (std::is_same_v<T, StringChange>) {
            if (a.mi->GetDependencyMark()) {
                a.it->SetDependencyMark();
            }
        }
    };

    for (auto it = changes_.begin(); it != changes_.end();) {
        auto &v = *it;
        std::visit(markStringDependency, v);
        ++it;
    }
}

void Context::HandleStringId(CodePatcher &p, const BytecodeInstruction &inst, const panda_file::File *filePtr,
                             CodeData *data)
{
    BytecodeId bId = inst.GetId();
    auto oldId = bId.AsFileId();
    auto sData = filePtr->GetStringData(oldId);
    auto itemStr = std::string(utf::Mutf8AsCString(sData.data));
    p.Add(CodePatcher::StringChange {inst, std::move(itemStr), data->nmi});
}

void Context::HandleLiteralArrayId(CodePatcher &p, const BytecodeInstruction &inst, const panda_file::File *filePtr,
                                   const std::map<panda_file::File::EntityId, panda_file::BaseItem *> *items)
{
    BytecodeId bId = inst.GetId();
    auto oldIdx = bId.AsRawValue();
    auto arrs = filePtr->GetLiteralArrays();
    ASSERT(oldIdx < arrs.size());
    auto off = arrs[oldIdx];
    auto iter = items->find(panda_file::File::EntityId(off));
    ASSERT(iter != items->end());
    ASSERT(iter->second->GetItemType() == panda_file::ItemTypes::LITERAL_ARRAY_ITEM);
    p.Add(CodePatcher::LiteralArrayChange {inst, static_cast<panda_file::LiteralArrayItem *>(iter->second)});
}

void Context::MakeChangeWithId(CodePatcher &p, CodeData *data)
{
    using Flags = ark::BytecodeInst<ark::BytecodeInstMode::FAST>::Flags;
    using EntityId = panda_file::File::EntityId;

    const auto myId = EntityId(data->omi->GetOffset());
    auto *items = data->fileReader->GetItems();
    auto inst = BytecodeInstruction(data->code->data());
    size_t offset = 0;
    const auto limit = data->code->size();

    auto filePtr = data->fileReader->GetFilePtr();

    using Resolver = EntityId (panda_file::File::*)(EntityId id, panda_file::File::Index idx) const;

    auto makeWithId = [&p, &inst, &filePtr, &items, &myId, &data, this](Resolver resolve) {
        auto idx = inst.GetId().AsIndex();
        auto oldId = (filePtr->*resolve)(myId, idx);
        auto iter = items->find(oldId);
        ASSERT(iter != items->end());
        auto asIndexed = static_cast<panda_file::IndexedItem *>(iter->second);
        auto found = knownItems_.find(asIndexed);
        ASSERT(found != knownItems_.end());
        p.Add(CodePatcher::IndexedChange {inst, data->nmi, static_cast<panda_file::IndexedItem *>(found->second)});
    };

    while (offset < limit) {
        if (offset + inst.GetSize() > limit) {
            LOG(FATAL, STATIC_LINKER) << "Invalid code size: " << limit << ", offset: " << offset
                                      << ", instruction size: " << inst.GetSize();
            break;
        }
        if (inst.HasFlag(Flags::TYPE_ID)) {
            makeWithId(&panda_file::File::ResolveClassIndex);
            // inst.UpdateId()
        } else if (inst.HasFlag(Flags::METHOD_ID) || inst.HasFlag(Flags::STATIC_METHOD_ID)) {
            makeWithId(&panda_file::File::ResolveMethodIndex);
        } else if (inst.HasFlag(Flags::FIELD_ID) || inst.HasFlag(Flags::STATIC_FIELD_ID)) {
            makeWithId(&panda_file::File::ResolveFieldIndex);
        } else if (inst.HasFlag(Flags::STRING_ID)) {
            HandleStringId(p, inst, filePtr, data);
        } else if (inst.HasFlag(Flags::LITERALARRAY_ID)) {
            HandleLiteralArrayId(p, inst, filePtr, items);
        }

        offset += inst.GetSize();
        inst = inst.GetNext();
    }
    if (offset != limit) {
        LOG(FATAL, STATIC_LINKER) << "Code size mismatch: expected " << limit << ", got " << offset;
    }
}

void Context::ProcessCodeData(CodePatcher &p, CodeData *data)
{
    if (data->code != nullptr) {
        MakeChangeWithId(p, data);
    }

    auto *dbg = data->omi->GetDebugInfo();
    if (dbg == nullptr || conf_.stripDebugInfo) {
        return;
    }

    // Collect string items for each method with debug information.
    auto file = data->fileReader->GetFilePtr();
    auto scrapper = LinkerDebugInfoScrapper(file, &p, &cont_);
    auto off = dbg->GetOffset();
    ASSERT(off != 0);
    auto eId = panda_file::File::EntityId(off);
    scrapper.Scrap(eId);

    auto newDbg = data->nmi->GetDebugInfo();
    p.Add([file, this, newDbg, eId, patchLnp = data->patchLnp, nmi = data->nmi](bool peek) -> bool {
        if (peek) {
            // peek won't patch lnp, only return method mark for delete judge
            return !nmi->GetDependencyMark();
        }
        auto updater = LinkerDebugInfoUpdater(file, &cont_);

        auto *constantPool = newDbg->GetConstantPool();
        if (patchLnp) {
            // Original `LineNumberProgram` - must emit both instructions and their arguments.
            auto *lnpItem = newDbg->GetLineNumberProgram();
            updater.Emit(lnpItem, constantPool, eId);
        } else {
            auto *lnpItemold = newDbg->GetLineNumberProgram();
            if (lnpItemold->GetData().empty()) {
                // emit if lnp size zero after delete item
                auto *lnpItem = newDbg->GetLineNumberProgram();
                updater.Emit(lnpItem, constantPool, eId);
            } else {
                // `LineNumberProgram` is reused and its instructions will be emitted by owner-method.
                // Still need to emit instructions' arguments, which are unique for each method.
                panda_file::LineNumberProgramItemBase lnpItem;
                updater.Emit(&lnpItem, constantPool, eId);
            }
        }
        return false;
    });
}

}  // namespace ark::static_linker
