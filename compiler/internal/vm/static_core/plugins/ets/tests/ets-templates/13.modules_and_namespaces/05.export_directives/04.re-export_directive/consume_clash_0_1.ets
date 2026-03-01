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
desc: Import reexported entity with new name and define local with original
files: ['./modules/export_clash.ets', './modules/re_export_clash_0_1.ets']
---*/

import { Clash as Original } from './modules/re_export_clash_0_1.ets'

class Clash {
    clash(): string { return 'I am local'}
}

function main() {
    arktest.assertEQ('I am clash', new Original().clash())
    arktest.assertEQ('I am local', new Clash().clash())
}
