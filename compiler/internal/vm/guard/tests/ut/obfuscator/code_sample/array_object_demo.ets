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

let suits: string[] = ['hearts', 'spades', 'clubs', 'diamonds'];
interface Obj1 {
    suit: string;
    card: number;
}
interface Obj2 {
    suit: string;
}
function pickCard(x: Obj1[]): number {
    return Math.floor(0.5 * x.length);
}
function pickCard(x: Obj2[]): string {
    return 'pickCardTest';
}
function pickCard(x: number): Obj1 {
    let pickedSuit = Math.floor(x / 13).toInt();
    return {suit: suits[pickedSuit], card: x % 13};
}

let myDeck: Obj1[] = [{suit: 'diamonds', card: 2}, {suit: 'spades', card: 10}, {suit: 'hearts', card: 4}];
let pickedCard1 = myDeck[pickCard(myDeck).toInt()];
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test8', pickedCard1.card === 10);
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test8', pickedCard1.suit === 'spades');

let arr2: Obj2[] = [{suit: 'asd'}];
// CC-OFFNXT(G.AOD.03) for test purposes
console.log(pickCard(arr2));

let pickedCard2 = pickCard(15);
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test8', pickedCard2.card === 2);
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test8', pickedCard2.suit === 'spades');

class Object1 {}
let myArr: Object1[] = [new Object1()];