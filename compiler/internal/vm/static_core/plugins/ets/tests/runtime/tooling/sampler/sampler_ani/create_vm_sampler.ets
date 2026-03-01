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

function main(): int {
    let repetitions: int = 10000000;

    return execute_test(repetitions);
}

function execute_test(repetitions: int): int {

    let res: double = 0;
    let pi: double = 3.14158;

    res = CallSlowFunction(1, repetitions);
    if (Absolute(res - pi) > 0.0001) {
        console.println("Pi calculation went wrong")
        return 1;
    }
    res = CallSlowFunction(2, repetitions);
    if (Absolute(res - pi) > 0.0001) {
        console.println("Pi calculation went wrong")
        return 1;
    }
    res = CallSlowFunction(3, repetitions);
    if (Absolute(res - pi) > 0.0001) {
        console.println("Pi calculation went wrong")
        return 1;
    }
    res = CallSlowFunction(4, repetitions);
    if (Absolute(res - pi) > 0.0001) {
        console.println("Pi calculation went wrong")
        return 1;
    }

    return 0;
}

function Absolute(val: double): double {
    if (val < 0) {
        return val *= -1;
    }
    return val;
}

function CallSlowFunction(foo: int, repetitions: int): double {
    let res: double = 0;
    if (foo == 1) {
        res = SlowETSFunction(repetitions);
    } else if (foo == 2) {
        res = SlowETSFunctionCopy1(repetitions);
    } else if (foo == 3) {
        res = SlowETSFunctionCopy2(repetitions);
    } else if (foo == 4) {
        res = SlowETSFunctionCopy3(repetitions);
    }
    return res;
}

// Doing copypaste because of compiler optimisations
function SlowETSFunction(repetitions: int): double {
    let ret: double = 0.0;
    let sign: int = 1;
    for (let i: int = 0; i < repetitions; i++) {
        ret += sign / (2.0 * i + 1);
        sign *= -1;
    }
    return 4 * ret;
}

function SlowETSFunctionCopy1(repetitions: int): double {
    let ret: double = 0.0;
    let sign: int = 1;
    for (let i: int = 0; i < repetitions; i++) {
        ret += sign / (2.0 * i + 1);
        sign *= -1;
    }
    return 4 * ret;
}

function SlowETSFunctionCopy2(repetitions: int): double {
    let ret: double = 0.0;
    let sign: int = 1;
    for (let i: int = 0; i < repetitions; i++) {
        ret += sign / (2.0 * i + 1);
        sign *= -1;
    }
    return 4 * ret;
}

function SlowETSFunctionCopy3(repetitions: int): double {
    let ret: double = 0.0;
    let sign: int = 1;
    for (let i: int = 0; i < repetitions; i++) {
        ret += sign / (2.0 * i + 1);
        sign *= -1;
    }
    return 4 * ret;
}
