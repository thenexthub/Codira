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

export function fun1() {
    return 1 + fun2();
}
function fun2() {
    return 2;
}
function fun3() {
    return 3;
}
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test1', fun1() + fun3());

function math1(a: number, b: number, c: number = 3) {
    return a + b + c;
}
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test2', math1(1, 2) === 6);
function add1(x: number, y: number): Any {
    return x + y;
}
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test3', add1(2, 3) === 5);

function buildName1(firstName: string, lastName: string = 'Smith') {
    return firstName + ' ' + lastName;
}
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test4', buildName1('Bob') === 'Bob Smith');
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test4', buildName1('Bob', undefined) === 'Bob Smith');
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test4', buildName1('Bob', 'Adams') === 'Bob Adams');

function buildName2(firstName: string, lastName?: string) {
    if (lastName) {
        return firstName + ' ' + lastName;
    } else {
        return firstName;
    }
}
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test5', buildName2('Bob') === 'Bob');
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test5', buildName2('Bob', 'Adams') === 'Bob Adams');

function buildName3(firstName: string, ...restOfName: string[]) {
    return firstName + ' ' + restOfName.join(' ');
}
let employeeName = buildName3('Joseph', 'Samuel', 'Lucas', 'MacKinzie');
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test6', employeeName === 'Joseph Samuel Lucas MacKinzie');

let z = 100;
function addToZ(x: number, y: number) {
    return x + y + z;
}
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test7', addToZ(100, 200) === 400);

let suits: string[] = ['hearts', 'spades', 'clubs', 'diamonds'];
interface Obj1 {
    suit: string;
    card: number;
}
function pickCard(x: Obj1): number {
    return Math.floor(0.5 * x.card);
}
function pickCard(x: number): Obj1 {
    let pickedSuit = Math.floor(x / 13).toInt();
    return {suit: suits[pickedSuit], card: x % 13};
}

let myDeck: Obj1 = {suit: 'diamonds', card: 2};
let pickedCard1 = pickCard(myDeck);
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test8', pickedCard1 === 1);

let pickedCard2 = pickCard(15);
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test8', pickedCard2.card === 2);
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test8', pickedCard2.suit === 'spades');
