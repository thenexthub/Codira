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

#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsInteropJsClassExtension : public EtsInteropTest {};

// NOTE Disabled until #17693 is resolved
TEST_F(EtsInteropJsClassExtension, DISABLED_eTS_can_extend_TS_user_class)
{
    // Please uncomment extendUserClass fn in index.ets before running
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "extendUserClass");
    ASSERT_EQ(ret, true);
}
// NOTE Disabled until #17693 is resolved
TEST_F(EtsInteropJsClassExtension, DISABLED_eTS_can_extend_TS_native_class)
{
    // Please uncomment extendNativeClass fn in index.ets before running
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "extendNativeClass");
    ASSERT_EQ(ret, true);
}
// NOTE Disabled until #17693 is resolved
TEST_F(EtsInteropJsClassExtension, DISABLED_eTS_extended_class_is_instanceOf_JS_user_class)
{
    // Please uncomment extendedUserClassIsInstanceOf fn in index.ets before running
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "extendedUserClassIsInstanceOf");
    ASSERT_EQ(ret, true);
}
// NOTE Disabled until #17693 is resolved
TEST_F(EtsInteropJsClassExtension, DISABLED_eTS_extended_class_is_instanceOf_JS_native_class)
{
    // Please uncomment extendedNativeClassIsInstanceOf fn in index.ets before running
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "extendedNativeClassIsInstanceOf");
    ASSERT_EQ(ret, true);
}
// NOTE Disabled until #17693 is resolved
TEST_F(EtsInteropJsClassExtension, DISABLED_eTS_extended_class_can_access_super_method)
{
    // Please uncomment canAccessSuperMethod fn in index.ets before running
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "canAccessSuperMethod");
    ASSERT_EQ(ret, true);
}
// NOTE Disabled until #17693 is resolved
TEST_F(EtsInteropJsClassExtension, DISABLED_eTS_extended_class_can_add_getter_method)
{
    // Please uncomment canAddGetter fn in index.ets before running
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "canAddGetter");
    ASSERT_EQ(ret, true);
}
// NOTE Disabled until #17693 is resolved
TEST_F(EtsInteropJsClassExtension, DISABLED_eTS_extended_class_can_set_protected_property)
{
    // Please uncomment canSetProtectedValue fn in index.ets before running
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "canSetProtectedValue");
    ASSERT_EQ(ret, true);
}
// NOTE Disabled until #17693 is resolved
TEST_F(EtsInteropJsClassExtension, DISABLED_JS_can_extend_eTS_user_class)
{
    // Please uncomment extendUserClass fn in index.ets before running
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallJsMethod<bool>("extendUserClass", "index.js");
    ASSERT_EQ(ret, true);
}
// NOTE Disabled until #17693 is resolved
TEST_F(EtsInteropJsClassExtension, DISABLED_JS_can_extend_eTS_native_class)
{
    // Please uncomment extendNativeClass fn in index.ets before running
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallJsMethod<bool>("extendNativeClass", "index.js");
    ASSERT_EQ(ret, true);
}
// NOTE Disabled until #17693 is resolved
TEST_F(EtsInteropJsClassExtension, DISABLED_JS_respects_eTS_protected_modifier)
{
    // Please uncomment jsRespectsProtectedModifier fn in index.ets before running
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallJsMethod<bool>("jsRespectsProtectedModifier", "index.js");
    ASSERT_EQ(ret, true);
}
// NOTE Disabled until #17693 is resolved
TEST_F(EtsInteropJsClassExtension, DISABLED_JS_respects_eTS_static_modifier)
{
    // Please uncomment jsRespectsStaticModifier fn in index.ets before running
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallJsMethod<bool>("jsRespectsStaticModifier", "index.js");
    ASSERT_EQ(ret, true);
}
}  // namespace ark::ets::interop::js::testing
