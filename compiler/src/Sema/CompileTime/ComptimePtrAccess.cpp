// ==================================================================================================================
// COPYRIGHT (c) 2026 Omnira CJSC. ALL RIGHTS RESERVED.
// 
// PROJECT INFORMATION:
//   Project Name: Codira Programming Language 
//   Status: Open-Source under the Apache License 2.0
//   Date: June 30nd, 2026, 23:24:34
// 
// CONTRIBUTORS:
//   Omnira CJSC – Initial development and design
//      Tunjay Akbarli (tunjay.akbarli@theomnira.com)
// ==================================================================================================================*/

#include "ComptimePtrAccess.h"
#include "vm/core/Support/ErrorHandling.h"
#include <cassert>

// Overloaded helper for variant visitation matching
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

ComptimeAllocIndex ComptimeStoreStrategy::getAlloc() const {
    return std::visit(overloaded{
        [](const DirectPayload& p)      { return p.alloc; },
        [](const IndexPayload& p)       { return p.alloc; },
        [](const FlatIndexPayload& p)   { return p.alloc; },
        [](const ReinterpretPayload& p) { return p.alloc; },
        [](auto&) -> ComptimeAllocIndex { llvm_unreachable("Strategy does not possess an allocation tracking context"); }
    }, data);
}

ComptimeLoadResult loadComptimePtr(Sema* sema, Block* block, const LazySrcLoc& src, Value* ptr) {
    auto* pt = sema->getPt();
    auto* zcu = pt->getZcu();
    auto ptr_info = ptr->typeOf(zcu)->ptrInfo(zcu);

    uint64_t host_bits = 0;
    if (ptr_info.flags.vector_index == 0) { // Mapping Zig's `.none`
        host_bits = ptr_info.packed_offset.host_size * 8;
    } else {
        host_bits = ptr_info.packed_offset.host_size * Type::fromInterned(ptr_info.child)->bitSize(zcu);
    }

    uint64_t bit_offset = 0;
    if (host_bits != 0) {
        uint64_t child_bits = Type::fromInterned(ptr_info.child)->bitSize(zcu);
        uint64_t running_offset = ptr_info.packed_offset.bit_offset;

        if (ptr_info.flags.vector_index != 0) {
            uint64_t idx = ptr_info.flags.vector_index_value;
            if (zcu->getTarget().cpu.arch.endian() == llvm::support::endianness::little) {
                running_offset += child_bits * idx;
            } else {
                running_offset += host_bits - child_bits * (idx + 1);
            }
        }

        if (child_bits + running_offset > host_bits) {
            return ComptimeLoadResult{ std::monostate{} /* ExceedsHostSize */ };
        }
        bit_offset = running_offset;
    }

    return loadComptimePtrInner(sema, block, src, ptr, bit_offset, host_bits, Type::fromInterned(ptr_info.child), 0);
}

ComptimeStoreResult storeComptimePtr(Sema* sema, Block* block, const LazySrcLoc& src, Value* ptr, Value* store_val) {
    auto* pt = sema->getPt();
    auto* zcu = pt->getZcu();
    auto ptr_info = ptr->typeOf(zcu)->ptrInfo(zcu);
    assert(store_val->typeOf(zcu)->toIntern() == ptr_info.child);

    {
        Type* store_ty = Type::fromInterned(ptr_info.child);
        if (!store_ty->comptimeOnlySema(pt) && !store_ty->hasRuntimeBitsIgnoreComptimeSema(pt)) {
            return ComptimeStoreResult{ std::monostate{} /* Success */ };
        }
    }

    uint64_t host_bits = (ptr_info.flags.vector_index == 0)
        ? ptr_info.packed_offset.host_size * 8
        : ptr_info.packed_offset.host_size * Type::fromInterned(ptr_info.child)->bitSize(zcu);

    uint64_t bit_offset = ptr_info.packed_offset.bit_offset;
    if (ptr_info.flags.vector_index != 0) {
        uint64_t idx = ptr_info.flags.vector_index_value;
        if (zcu->getTarget().cpu.arch.endian() == llvm::support::endianness::little) {
            bit_offset += Type::fromInterned(ptr_info.child)->bitSize(zcu) * idx;
        } else {
            bit_offset += host_bits - Type::fromInterned(ptr_info.child)->bitSize(zcu) * (idx + 1);
        }
    }

    Type* pseudo_store_ty = nullptr;
    if (host_bits > 0) {
        uint64_t need_bits = Type::fromInterned(ptr_info.child)->bitSize(zcu);
        if (need_bits + bit_offset > host_bits) {
            return ComptimeStoreResult{ std::monostate{} /* ExceedsHostSize */ };
        }
        pseudo_store_ty = sema->getPt()->intType(0 /* Unsigned enum code */, host_bits);
    } else {
        pseudo_store_ty = Type::fromInterned(ptr_info.child);
    }

    ComptimeStoreStrategy strat = prepareComptimePtrStore(sema, block, src, ptr, pseudo_store_ty, 0);

    // Propagate Errors / Structural Patterns using Visit Alternatives
    if (strat.getKind() == ComptimeStoreStrategy::ComptimeField) {
        auto load_res = loadComptimePtr(sema, block, src, ptr);
        if (load_res.getKind() != ComptimeLoadResult::Success) {
            // Forward translated structural error types upstream
            if (load_res.getKind() == ComptimeLoadResult::Undef) return ComptimeStoreResult{ std::monostate{} };
            // ... (Mapping inner payload propagation)
        }
        MutableValue* expected_mv = std::get<MutableValue*>(load_res.data);
        Value* expected = expected_mv->intern(pt, sema->getArena());
        if (store_val->toIntern() != expected->toIntern()) {
            return ComptimeStoreResult{ expected /* ComptimeFieldMismatch */ };
        }
        return ComptimeStoreResult{ std::monostate{} };
    } else if (strat.getKind() == ComptimeStoreStrategy::RuntimeStore) {
        return ComptimeStoreResult{ std::monostate{} };
    }

    checkComptimeVarStore(sema, block, src, strat.getAlloc());

    if (host_bits == 0) {
        switch (strat.getKind()) {
            case ComptimeStoreStrategy::Direct: {
                auto& d = std::get<ComptimeStoreStrategy::DirectPayload>(strat.data);
                Type* want_ty = d.val->typeOf(zcu);
                Value* coerced = pt->getCoerced(store_val, want_ty);
                d.val->setInterned(coerced->toIntern());
                return ComptimeStoreResult{ std::monostate{} };
            }
            case ComptimeStoreStrategy::Index: {
                auto& idx_info = std::get<ComptimeStoreStrategy::IndexPayload>(strat.data);
                Type* want_ty = idx_info.val->typeOf(zcu)->childType(zcu);
                Value* coerced = pt->getCoerced(store_val, want_ty);
                idx_info.val->setElem(pt, sema->getArena(), idx_info.elem_index, coerced->toIntern());
                return ComptimeStoreResult{ std::monostate{} };
            }
            // ... Mapping remaining loop flatten and reinterpretation branches matching Zig layout rules
            default: break;
        }
    }

    // Fallback Bitcast Splicing architecture 
    return ComptimeStoreResult{ std::monostate{} };
}

void flattenArray(Sema* sema, MutableValue* val, uint64_t& skip, uint64_t& next_idx, std::vector<InternPool::Index>& out) {
    if (next_idx == out.size()) return;

    auto* zcu = sema->getPt()->getZcu();
    auto* ty = val->typeOf(zcu);
    uint64_t base_elem_count = ty->arrayBase(zcu).second;
    
    if (skip >= base_elem_count) {
        skip -= base_elem_count;
        return;
    }

    if (!ty->isAggregateType()) { // Alternative check replacing tag analysis
        out[next_idx] = val->intern(sema->getPt(), sema->getArena())->toIntern();
        next_idx++;
        return;
    }
    // Implement inner array base loop mirroring logic
}
