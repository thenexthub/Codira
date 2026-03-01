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
#ifndef RUNTIME_INCLUDE_TAIHE_OBJECT_ABI_H_
#define RUNTIME_INCLUDE_TAIHE_OBJECT_ABI_H_

#include <taihe/common.h>

#include <stdint.h>
#include <stdlib.h>

struct DataBlockHead;

// IdMapItem
// Represents an ID mapping item containing an ID and a vtable pointer.
// # Members
// - `id`: A constant pointer representing the interface ID.
// - `vtbl_ptr`: A pointer to the virtual table associated with the ID.
struct IdMapItem {
    void const *id;
    void const *vtbl_ptr;
};

typedef void free_func_t(struct DataBlockHead *);
typedef size_t hash_func_t(struct DataBlockHead *);
typedef bool same_func_t(struct DataBlockHead *, struct DataBlockHead *);
typedef void const *qiid_func_t(void const *id);

// TypeInfo
// Represents metadata information for a type, including version, length, and
// function pointers.
// # Members
// - `free_fptr`: Pointer to function that frees the data block.
// - `hash_fptr`: Pointer to function that computes the hash of a data block.
// - `same_fptr`: Pointer to function that compares equality of two data blocks.
// - `qiid_fptr`: Pointer to function that queries the interface id;
// - `len`: A 64-bit unsigned integer representing the length of idmap.
// - `idmap`: An array of IdMapItem structures representing the ID to vtable
//   mapping.
struct TypeInfo {
    free_func_t *free_fptr;
    hash_func_t *hash_fptr;
    same_func_t *same_fptr;
    qiid_func_t *qiid_fptr;
    uint64_t len;
    struct IdMapItem idmap[];
};

// DataBlockHead
// Represents the head of a data block, containing a pointer to the runtime
// type information structure and a reference count.
// # Members
// - `rtti_ptr`: A pointer to the runtime type information structure.
// - `m_count`: A reference count for the data block.
struct DataBlockHead {
    struct TypeInfo const *rtti_ptr;
    TRefCount m_count;
};

// Initializes the TObject with the given runtime typeinfo.
// # Arguments
// - `data_ptr`: The data pointer.
// - `rtti_ptr`: The runtime typeinfo pointer.
TH_EXPORT void tobj_init(struct DataBlockHead *data_ptr, struct TypeInfo const *rtti_ptr);

// Increments the reference count of the given TObject.
// # Arguments
// - `data_ptr`: The data pointer.
// # Returns
// - The new data pointer.
TH_EXPORT struct DataBlockHead *tobj_dup(struct DataBlockHead *data_ptr);

// Decrements the reference count of the given TObject. If the reference count
// reaches zero, the object is destroyed.
// # Arguments
// - `tobj`: The data pointer.
TH_EXPORT void tobj_drop(struct DataBlockHead *data_ptr);
#endif  // RUNTIME_INCLUDE_TAIHE_OBJECT_ABI_H_