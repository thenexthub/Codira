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

function foo1(a: int, b: string, c?: boolean, d: number = 1): boolean {
    return a === 1 && b === 'str' && c === undefined && d === 1;
}

function foo2(a: int, b: string, c?: boolean, d: number = 1): boolean {
    return a === 1 && b === 'str' && c === true && d === 1;
}

function foo3(a: int, b: string, c?: boolean, d: number = 1): boolean {
    return a === 1 && b === 'str' && c === true && d === 2;
}

function fun1(a: int, b: string, c: boolean = true, d?: number): boolean {
    return a === 1 && b === 'str' && c === true && d === undefined;
}

function fun2(a: int, b: string, c: boolean = true, d?: number): boolean {
    return a === 1 && b === 'str' && c === false && d === undefined;
}

function fun3(a: int, b: string, c: boolean = true, d?: number): boolean {
    return a === 1 && b === 'str' && c === false && d === 2;
}