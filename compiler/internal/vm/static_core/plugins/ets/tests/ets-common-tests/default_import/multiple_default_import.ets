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
desc: Default import binding allows importing a declaration exported from some module as default export.
files:
  - ./default_export/default_export_class.ets
  - ./default_export/default_export_function.ets
  - ./default_export/default_export_interface.ets
  - ./default_export/default_export_variable.ets
---*/


import default_class from './default_export/default_export_class'
import default_func from './default_export/default_export_function'
import default_interface from './default_export/default_export_interface'
import default_var from './default_export/default_export_variable'

class I implements default_interface {
    public item = 1;
}

function main() {
    arktest.assertEQ(default_func(2), 3);
    arktest.assertEQ(default_var, 10);
    let e = new I();
    arktest.assertEQ(e.item, 1);
    let b : default_class = new default_class();
    arktest.assertEQ(b.hello('hello'), 'hello_world')  
}
