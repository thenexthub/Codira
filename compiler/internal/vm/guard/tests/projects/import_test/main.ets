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

import { ExportClass } from './common/export_file';
import DefaultExportedItemBindedName from './common/export_file'
import {default as DefaultExportedItemNewName} from './common/export_file'
import {Space2 as Space1} from './common/export_file'
import {foo, f1} from './common/export_file'
import { MyAnno } from './common/export_file'
import type { S1 } from './common/export_file'
import type { S2 as TypeString } from './common/export_file'
import type { UNI, ClassType1, MyClassType } from './common/export_file'
import * as m from './export_file'

function foo1() {
    let v1 = new DefaultExportedItemBindedName()
    let v2 = new DefaultExportedItemNewName()
    v1.foo();
    v2.foo();
}

foo1()
f1(5)
foo(6)
foo('a string')

Space1.foo(1234)

let b = new m.B // B instance is created
m.process_field(b.a) // exported field is passed to function as an argument

function importTest() {
    let inst = new ExportClass();
    inst.memC = 'memCStr';
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log(`ret ${inst.memA}, ${inst.memB()}, ${inst.memC}`);
}

importTest();

@MyAnno
class C {
    foo() {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('C');
    }
}
let var1 = new C();
var1.foo();

// import type
let str1: S1 = 'a String 1';
// CC-OFFNXT(G.AOD.03) for test purposes
console.log(str1);

let str2: TypeString = 'a String 2';
// CC-OFFNXT(G.AOD.03) for test purposes
console.log(str2);

let uni1: UNI = 'a String 3';
// CC-OFFNXT(G.AOD.03) for test purposes
console.log(uni1);

let uni2: UNI = true;
// CC-OFFNXT(G.AOD.03) for test purposes
console.log(uni2);

let uni3: UNI = [1, 2, 3];
// CC-OFFNXT(G.AOD.03) for test purposes
console.log(uni3);

let ct: ClassType1 = new ClassType1();
// CC-OFFNXT(G.AOD.03) for test purposes
console.log(ct.test());

let mct: MyClassType = new MyClassType();
// CC-OFFNXT(G.AOD.03) for test purposes
console.log(mct.test());
