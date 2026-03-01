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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_STORAGE_VERIFIER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_STORAGE_VERIFIER_H_

#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/mem/items_pool.h"

namespace ark::ets::interop::js::ets_proxy {
namespace testing {
class SharedReferenceStorage1GTest;
}  // namespace testing
enum class XgcStatus {
    XGC_START,
    XGC_FINISHED,
};

class SharedReferenceStorageVerifier {
public:
    static size_t TraverseAllItems(const SharedReferenceStorage *const storage, XgcStatus status);

private:
    friend class testing::SharedReferenceStorage1GTest;
    static size_t VerifyOneItem(const SharedReferenceStorage *const storage, size_t index, XgcStatus status);
    static size_t CheckObjectAlive(const SharedReference *item, size_t index);
    static size_t CheckObjectAddressValid(const SharedReference *item, size_t index);
    static size_t CheckJsObjectType(const SharedReference *item, size_t index);
    static size_t CheckObjectReindex(const SharedReferenceStorage *const storage, const SharedReference *item,
                                     size_t index);
    static size_t IsAllItemsMarked(const SharedReference *item, size_t index);
    static size_t CheckJsObjectAlive(const SharedReference *item, size_t index);
    static size_t CheckJEtsObjectAlive(const SharedReference *item, size_t index);

    static size_t CheckJsObjectAddress(const SharedReference *item, size_t index);
    static size_t CheckEtsObjectAddress(const SharedReference *item, size_t index);

    static bool CheckJsObjectTypeIsJsXref(const SharedReference *item);

    static size_t CheckJsObjectReindex(const SharedReferenceStorage *const storage, const SharedReference *item,
                                       size_t index);
    static size_t CheckEtsObjectReindex(const SharedReference *item, size_t index);
};
}  // namespace ark::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_STORAGE_VERIFIER_H_