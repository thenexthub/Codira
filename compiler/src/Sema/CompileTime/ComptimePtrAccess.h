// ==================================================================================================================
// COPYRIGHT (c) 2026 Omnira CJSC. ALL RIGHTS RESERVED.
// 
// PROJECT INFORMATION:
//   Project Name: Codira Programming Language 
//   Status: Open-Source under the Apache License 2.0
//   Date: June 30nd, 2026, 23:22:38
// 
// CONTRIBUTORS:
//   Omnira CJSC – Initial development and design
//      Tunjay Akbarli (tunjay.akbarli@theomnira.com)
// ==================================================================================================================*/

#pragma once

#include "vm/core/ADT/StringRef.h"
#include "vm/core/ADT/APSInt.h"
#include "vm/core/IR/Type.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/Endian.h"
#include <variant>
#include <vector>
#include <optional>
#include <memory>

// Forward declarations for placeholder compiler engine infrastructure
class Sema;
class Block;
class LazySrcLoc;
class Value;
class MutableValue;
class Type;
using ComptimeAllocIndex = uint32_t;

namespace InternPool {
    using NullTerminatedString = vm::StringRef;
    using Index = uint32_t;
}

// --- Comptime Load Result ---
struct ComptimeLoadResult {
    enum Kind {
        Success, RuntimeLoad, Undef, ErrPayload, NullPayload,
        InactiveUnionField, NeededWellDefined, OutOfBounds, ExceedsHostSize
    };

    Kind getKind() const { return static_cast<Kind>(data.index()); }

    std::variant<
        MutableValue*,                     // Success
        std::monostate,                    // RuntimeLoad
        std::monostate,                    // Undef
        InternPool::NullTerminatedString,  // ErrPayload
        std::monostate,                    // NullPayload
        std::monostate,                    // InactiveUnionField
        Type*,                             // NeededWellDefined
        Type*,                             // OutOfBounds
        std::monostate                     // ExceedsHostSize
    > data;
};

// --- Comptime Store Strategy ---
struct ComptimeStoreStrategy {
    enum Kind {
        Direct, Index, FlatIndex, Reinterpret, ComptimeField,
        RuntimeStore, Undef, ErrPayload, NullPayload,
        InactiveUnionField, NeededWellDefined, OutOfBounds
    };

    struct DirectPayload      { ComptimeAllocIndex alloc; MutableValue* val; };
    struct IndexPayload       { ComptimeAllocIndex alloc; MutableValue* val; uint64_t elem_index; };
    struct FlatIndexPayload   { ComptimeAllocIndex alloc; MutableValue* val; uint64_t flat_elem_index; };
    struct ReinterpretPayload { ComptimeAllocIndex alloc; MutableValue* val; uint64_t byte_offset; };

    Kind getKind() const { return static_cast<Kind>(data.index()); }
    ComptimeAllocIndex getAlloc() const;

    std::variant<
        DirectPayload, IndexPayload, FlatIndexPayload, ReinterpretPayload,
        std::monostate,                    // ComptimeField
        std::monostate,                    // RuntimeStore
        std::monostate,                    // Undef
        InternPool::NullTerminatedString,  // ErrPayload
        std::monostate,                    // NullPayload
        std::monostate,                    // InactiveUnionField
        Type*,                             // NeededWellDefined
        Type*                              // OutOfBounds
    > data;
};

// --- Comptime Store Result ---
struct ComptimeStoreResult {
    enum Kind {
        Success, RuntimeStore, ComptimeFieldMismatch, Undef, ErrPayload,
        NullPayload, InactiveUnionField, NeededWellDefined, OutOfBounds, ExceedsHostSize
    };

    Kind getKind() const { return static_cast<Kind>(data.index()); }

    std::variant<
        std::monostate,                    // Success
        std::monostate,                    // RuntimeStore
        Value*,                            // ComptimeFieldMismatch
        std::monostate,                    // Undef
        InternPool::NullTerminatedString,  // ErrPayload
        std::monostate,                    // NullPayload
        std::monostate,                    // InactiveUnionField
        Type*,                             // NeededWellDefined
        Type*,                             // OutOfBounds
        std::monostate                     // ExceedsHostSize
    > data;
};

// --- API Declarations ---
ComptimeLoadResult loadComptimePtr(Sema* sema, Block* block, const LazySrcLoc& src, Value* ptr);

ComptimeStoreResult storeComptimePtr(Sema* sema, Block* block, const LazySrcLoc& src, Value* ptr, Value* store_val);

ComptimeLoadResult loadComptimePtrInner(Sema* sema, Block* block, const LazySrcLoc& src, Value* ptr_val,
                                       uint64_t bit_offset, uint64_t host_bits, Type* load_ty, uint64_t array_offset);

ComptimeStoreStrategy prepareComptimePtrStore(Sema* sema, Block* block, const LazySrcLoc& src, Value* ptr_val, 
                                              Type* store_ty, uint64_t array_offset);

void flattenArray(Sema* sema, MutableValue* val, uint64_t& skip, uint64_t& next_idx, std::vector<InternPool::Index>& out);

Value* unflattenArray(Sema* sema, Type* ty, const std::vector<InternPool::Index>& elems, uint64_t& next_idx);

std::optional<std::pair<MutableValue*, uint64_t>> recursiveIndex(Sema* sema, MutableValue* mv, uint64_t& index);

void checkComptimeVarStore(Sema* sema, Block* block, const LazySrcLoc& src, ComptimeAllocIndex alloc_index);
