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
#ifndef RUNTIME_INCLUDE_TAIHE_STRING_ABI_H_
#define RUNTIME_INCLUDE_TAIHE_STRING_ABI_H_

#include <taihe/common.h>

#include <stddef.h>
#include <stdint.h>

// Private ABI: Don't use in your code //

enum TStringFlags {
    TSTRING_REF = 1,
};

struct TString {
    uint32_t flags;
    uint32_t length;
    char const *ptr;  // always valid and non-null
};

struct TStringData {
    TRefCount count;
    char buffer[];
};

// Public C API //

// Returns the buffer of the TString.
TH_INLINE const char *tstr_buf(struct TString tstr)
{
    return tstr.ptr;
}

// Returns the length of the TString.
TH_INLINE size_t tstr_len(struct TString tstr)
{
    return tstr.length;
}

// Allocates memory and initializes a TString with a given capacity.
// # Arguments
// - `tstr_ptr`: Pointer to an uninitialized TString structure.
// - `capacity`: The desired capacity of the string buffer.
// # Returns
// - Pointer to the allocated buffer.
// # Notes
// - The caller is responsible for setting the string length.
// - Reference count is set to 1 after called.
TH_EXPORT char *tstr_initialize(struct TString *tstr_ptr, uint32_t capacity);

// Creates a new heap-allocated TString by copying an existing string.
// # Arguments
// - `buf`: A null-terminated string (must not be null).
// - `len`: The length of the string.
// # Returns
// - A new TString containing a copy of `buf`.
// # Notes
// - The returned TString must be freed using `tstr_drop`.
TH_EXPORT struct TString tstr_new(char const *buf TH_NONNULL, size_t len);

// Creates a TString from an existing string.
// # Arguments
// - `buf`: a null-terminated string. Null pointer is invalid.
// - `len`: the length of the string.
// - `tstr`: pointer to an uninitialized TString. Do not pass an
//    already-initialized TString here.
// # Returns
// - `tstr`, if the string is created successfully. The caller must ensure the
//    string buffer and the returned TString remain unchanged during the whole
//    lifetime of the TString.
// - `NULL`, if the string is not null-terminated, or the length is too large.
//    In this case, the original `tstr` is still uninitialized and should not be
//    used.
TH_EXPORT struct TString tstr_new_ref(char const *buf TH_NONNULL, size_t len);

// Frees a TString, releasing allocated memory if applicable.
// # Arguments
// - `tstr`: The TString to be freed.
// # Notes
// - The TString should not be accessed after calling this function.
TH_EXPORT void tstr_drop(struct TString tstr);

// Creates a duplicate of a TString.
// # Arguments
// - `tstr`: The TString to be copied.
// # Returns
// - A new TString that is either a reference or a deep copy.
// # Notes
// - If `tstr` is heap-allocated, its reference count is incremented.
// - If `tstr` is a reference, a new heap-allocated copy is created.
// - Use `tstr_drop` to free the duplicate when done.
TH_EXPORT struct TString tstr_dup(struct TString tstr);

// Concatenates two TString objects.
// # Parameters
// - `count`: The number of strings to concatenate.
// - `tstr_list`: An array of TString objects to concatenate.
// # Returns
// - A new TString object containing the concatenated result.
// # Notes
// - The returned TString must be freed using `tstr_drop`.
TH_EXPORT struct TString tstr_concat(size_t count, struct TString const *tstr_list);

// Extracts a substring from a TString object.
// # Parameters
// - `tstr`: The source TString object to extract the substring from.
// - `pos`: The starting position of the substring within the source TString
//   object.
// - `len`: The length of the substring to extract.
// # Returns
// - A TString reference of the extracted substring.
// # Notes
// - The returned TString is just a view of the original string and does not own
//   the memory, so it should not be freed.
TH_EXPORT struct TString tstr_substr(struct TString tstr, size_t pos, size_t len);
#endif  // RUNTIME_INCLUDE_TAIHE_STRING_ABI_H_