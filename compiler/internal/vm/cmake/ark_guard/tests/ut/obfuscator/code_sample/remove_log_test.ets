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
class ClassB {
    foo() {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log(1);
    }
}

function foo2() {
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log();
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('hello');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('hello', 1);
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('hello', 1, 2, 3, 4, 'hello', 'hello');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log(1);
}

namespace Ns1 {
    function bar(){
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('hello');
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('hello', 1);
    }
}

class ClassA {
    public method1(a: int, b: string, c: Array<int>) {
        let ret = `clsA: ${a}|${b}|${c.length}`;
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log(ret);
        return ret;
    }
}
function foo1(a: int, b: ClassA, c: string, d: Array<int>) {
    let ret = `foo1: ${b.method1(a, c, d)}`;
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log(ret);
}
let cls1 = new ClassA();
foo1(2, cls1, 'abc', [1, 2, 3]);

function foo3() {
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('log');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.debug('debug');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.info('info');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.warn('warn');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.error('error');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.assert('assert');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.count('conut');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.countReset('countReset');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.dir('dir');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.dirxml('dirxml');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.group('group');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.groupCollapsed('groupCollapsed');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.groupEnd();
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.table('table');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.time('time');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.timeEnd('timeEnd');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.timeLog('timeLog');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.trace('trace');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.toString();
}

foo3();
