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
desc: Multiple modules stored in the package.
files:
- ./pkg2/compilation_units_in_host_system_export1.ets
- ./pkg2/compilation_units_in_host_system_export3.ets
---*/

import {sum} from "./pkg2/compilation_units_in_host_system_export1.ets"
import {sum as sum0} from "./pkg2/compilation_units_in_host_system_export3.ets"

function main(): void {
    arktest.assertEQ(sum(1, 2), 3);
    arktest.assertEQ(sum0(1, 2), 3);
}