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

/*---
desc: >-
    Export default target effectively means that an anonymous constant variable is created with a value
    equal to the value of the expression evaluation result. The export can be imported only by providing
    a name for the constant variable that is exported by using this export directive.
    Otherwise, a compile-time error occurs.
files:
- ./default_export_target_custom_constructor.ets
---*/

import {default as a} from "./default_export_target_custom_constructor"

function main() {
  arktest.assertEQ(a.foo(), 7)
}
