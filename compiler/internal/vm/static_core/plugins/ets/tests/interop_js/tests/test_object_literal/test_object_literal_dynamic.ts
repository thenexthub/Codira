/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let etsVm = globalThis.gtest.etsVm;
let testFoo = etsVm.getFunction('Ltest_object_literal_static/ETSGLOBAL;', 'testFoo');
let testFooInter = etsVm.getFunction('Ltest_object_literal_static/ETSGLOBAL;', 'testFooInter');

function assertEq(a, b) {
    if (a !== b) {
        throw Error(`assertEq failed: '${a}' == '${b}'`);
    }
}

export class Foo {
    bar: number = 111;
    baz: string = "HelloWorld";
}

export interface FooInter {
    bar: number;
    baz: string
}

function main() {
    let foo = testFoo();
    assertEq(foo.bar, 111);
    assertEq(foo.baz, "object literal");
    let foo_inter = testFooInter();
    assertEq(foo_inter.bar, 222);
    assertEq(foo_inter.baz, "interface");
}

main();