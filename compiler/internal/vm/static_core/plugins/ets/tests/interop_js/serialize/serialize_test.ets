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

// CC-OFFNXT(G.FMT.09) project code style
export function testFunction() {
    return 'testFunction';
}

export let testArray = ['testArrayElement'];
export let testInstance = new TestClass();

class TestClass {
    // CC-OFFNXT(G.FMT.09) project code style
    static testStaticMethod() {
        return 'testStaticMethod';
    }
    public static staticProp: String = 'staticProp';

    public prop: String = 'prop';
    // CC-OFFNXT(G.FMT.09) project code style
    testMethod() {
        return 'testMethod';
    }
}

// CC-OFFNXT(G.FMT.09) project code style
export function checkAsync(ptr: bigint) {
    let w = new EAWorker(true);
    w.start();
    let cb = (): int => {
        let ohmurlPath = './serialize_interop_tests_helpers_js';
        let module = ESValue.load(ohmurlPath);
        let method = module.getProperty('checkResult');
        try {
            method.invoke(ESValue.wrap(ptr));
        } catch (e) {
            return 1;
        }
        return 0;
    }
    let p = w.run<int>(cb);
    w.join();
    let result = p.Await();
    arktest.assertEQ(result, 0);
}
