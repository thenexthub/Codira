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

// This test check that JS objects of JS class
// that is derived from ETS class and transferred from ETS -> JS
// creating cyclic reference - can be collected by GC

import { interop } from './gc_test_common';

function main(): void {
    class JsDerivedClass extends interop.PandaBaseClass {
        derivedObj: Object;
        derivedStr: string;
        constructor() {
            super();
            this.derivedObj = new Object();
            this.derivedStr = this.str + 'JsDerivedClass';
        }
    }
    for (let i = 0; i < 100000; i++) {
        let customObj = new JsDerivedClass();
        if (i % 100 === 0) {
            print(i);
        }
        if (i % 10000 === 0) {
            interop.RunInteropGC();
            interop.RunPandaGC();
            globalThis.test.RunJsGC();
        }
    }
}

main();
