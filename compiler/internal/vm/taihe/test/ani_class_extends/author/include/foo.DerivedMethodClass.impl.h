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
#ifndef TEST_ANI_CLASS_EXTENDS_AUTHOR_INCLUDE_FOO_DERIVEDMETHODCLASS_TEMPLATE_H_
#define TEST_ANI_CLASS_EXTENDS_AUTHOR_INCLUDE_FOO_DERIVEDMETHODCLASS_TEMPLATE_H_
// NOLINTBEGIN

#include "taihe/string.hpp"

class DerivedMethodClassImpl {
public:
    taihe::string Derived();
    taihe::string Base();
    taihe::string Foo();
    taihe::string Bar();
};
// NOLINTEND
#endif  // TEST_ANI_CLASS_EXTENDS_AUTHOR_INCLUDE_FOO_DERIVEDMETHODCLASS_TEMPLATE_H_