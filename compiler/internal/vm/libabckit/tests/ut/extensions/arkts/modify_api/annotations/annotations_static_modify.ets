/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

@interface Anno1 {}

@interface Anno2 {}

@interface Anno3 {}

@interface Anno4 {
    a: string
}

@interface Anno5 {}

@interface Anno6 {}

@interface Anno7 {}

@interface Anno8 {}

@interface Anno9 {}

@interface Anno10 {
    a: number = 3
    // b: number[] = [13, 9]
    b: number = 4
}

@interface Anno11 {}

@interface Anno12 {}

@interface Anno13 {
    a: string = 'a'
    b: number = 1
}

@interface Anno14 {
    a: number = 14
    b: number = 15
}

@Anno1()
@Anno2()
export class Class1 {
    @Anno3()
    @Anno4({
        a: 'field2'
    })
    @Anno1()
    public field1 = 1;
    foo() {}
}

@Anno5()
@Anno6()
interface Interface1 {
    @Anno7()
    @Anno8()
    field2: int;
}

@Anno9()
function bar1() {}

@Anno10({
    a: 10,
    b: 20
})
function bar2() {}

@Anno11()
function bar3() {}

@Anno13
function bar4() {}

@Anno14
function bar5() {}

function testFunc(n: number): number {
  return n * 2;
}

function bar6(f: (x: number) => number): number {
  return f(5);
}

let result = bar6(testFunc);