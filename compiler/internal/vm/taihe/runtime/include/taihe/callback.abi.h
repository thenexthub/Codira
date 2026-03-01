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
#ifndef RUNTIME_INCLUDE_TAIHE_CALLBACK_ABI_H_
#define RUNTIME_INCLUDE_TAIHE_CALLBACK_ABI_H_

#include <taihe/object.abi.h>

// TCallback
// Represents a callback structure containing a pointer to the data block
// and a function pointer.
// # Members
// - `vtbl_ptr`: A pointer to the function associated with the callback.
// - `data_ptr`: A pointer to the data block.
struct TCallback {
    void *vtbl_ptr;
    struct DataBlockHead *data_ptr;
};
#endif  // RUNTIME_INCLUDE_TAIHE_CALLBACK_ABI_H_