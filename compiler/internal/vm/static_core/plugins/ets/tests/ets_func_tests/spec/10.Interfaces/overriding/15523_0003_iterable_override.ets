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
 desc: 10.06 Interface Method Declarations
 name: 10.interfaces/10.06.interface_method _declarations/10.06.02_inheritance_and_overriding/interface_overriding_01
 tags:
 ---*/

class MyClass {
    public constructor(element: Iterable<String>) {
        let it = element.$_iterator();
        while(true) {
            const v = it.next();
            if (v.done) {
                break;
            }
        }
    }
}

function main() : void {
    let a = new Set<String>();
    a.add("hello");
    let myClass = new MyClass(a.values());
}
