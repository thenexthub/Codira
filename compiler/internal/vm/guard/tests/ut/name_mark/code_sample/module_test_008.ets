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
function foo1(a: int | string) {
}

function foo2(a: byte | string) {
}

function foo3(a: short | string) {
}

function foo4(a: number | string) {
}

function foo5(a: long | string) {
}

function foo6(a: float | string) {
}

function foo7(a: double | string) {
}

function foo8(a: char | string) {
}

function foo9(a: boolean | string) {
}

function foo10(a: bigint | string) {
}

function foo11(a: never | string) {
}

function foo12(a: null | string) {
}

function foo13(a: undefined | string) {
}

function foo14(a: int[] | string) {
}

function foo15(a: undefined | never) {
}

function foo16(a: undefined | never | int) {
}


function fooEx1(): int | string {
    return 'hello';
}

function fooEx2(): byte | string {
    return 'hello';
}

function fooEx3(): short | string {
    return 'hello';
}

function fooEx4(): number | string {
    return 'hello';
}

function fooEx5(): long | string {
    return 'hello';
}

function fooEx6(): float | string {
    return 'hello';
}

function fooEx7(): double | string {
    return 'hello';
}

function fooEx8(): char | string {
    return 'hello';
}

function fooEx9(): boolean | string {
    return 'hello';
}

function fooEx10(): bigint | string {
    return 'hello';
}

function fooEx11(): never | string {
    return 'hello';
}

function fooEx12(): null | string {
    return 'hello';
}

function fooEx13(): undefined | string {
    return 'hello';
}

function fooEx14(): int[] | string {
    return 'hello';
}

function fooEx15(): undefined | never {
    return undefined;
}

function fooEx16(): undefined | never | int {
    return 3;
}