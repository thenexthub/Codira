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

namespace ops {
let count = 0;
function sum(a0: int, a1: int): void {
    count = a0 + a1;
}
function getCount(): int {
    return count as int;
}
}

export let intExport: int = 3;
export let charExport: char = c'a';
function moduleFunction(value1: int, value2: int): void {
    intExport = value1 + value2;
}

function moduleFunction(value: char): void {
    charExport = value;
}

function getIntExport(): int
{
    return intExport;
}

function getCharExport(): char
{
    return charExport;
}

namespace A {
	let intValue: int = 0;
	let doubleValue: double = 0.0;
	let charValue: char;

    function getIntValue(): int
    {
        return intValue;
    }

    function getDoubleValue(): double
    {
        return doubleValue;
    }

    function getCharValue(): char
    {
        return charValue;
    }

    function voidFunctionA(value1: int, value2: int): void {
        intValue = value1 + value2;
    }

    function voidFunctionA(value: char): void {
        charValue = value;
    }

    function nestedFunction(value1: int, value2: int): void {
        let lambdaFunction: (p1: int, p2: int) => int =
            (param1: int, param2: int): int => {
                return param1 + param2 as int;
        };
        intValue = lambdaFunction(value1, value2);
    }

    export function recursiveFunction(value: int): void {
        if (value <= 0) {
            return ;
        }
        intValue += value;
        recursiveFunction(value - 1);
    }

    function calculateSum(value1: int, value2: char, value3: double): void {
        intValue = value1;
        charValue = value2;
        doubleValue = value3;
    }

    namespace B {
        let intValueB: int = 0;
        function getIntValueB(): int
        {
            return intValueB;
        }
        function voidFunctionB(value1: int, value2: int): void {
            intValueB = value1 + value2;
        }
    }
}
