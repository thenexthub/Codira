/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

export namespace ns {

export function main(): int {
    // By providing the upper-bound execution time of the test functions we ensure that
    // enough profiler samples are collected while preventing timeouts in CI
    const repetitions: int = 10000000
    const functionExecTimeMs: int = 3000
    return executeTest(repetitions, functionExecTimeMs)
}

function executeTest(repetitions: int, functionExecTimeMs: int): int {
    let res: double = 0
    let pi: double = 3.14158

    res = callSlowFunction(1, repetitions, functionExecTimeMs)
    if (absolute(res - pi) > 0.001) {
        console.println("Pi calculation went wrong")
        return 1
    }
    res = callSlowFunction(2, repetitions, functionExecTimeMs)
    if (absolute(res - pi) > 0.0001) {
        console.println("Pi calculation went wrong")
        return 1
    }
    res = callSlowFunction(3, repetitions, functionExecTimeMs)
    if (absolute(res - pi) > 0.0001) {
        console.println("Pi calculation went wrong")
        return 1
    }
    res = callSlowFunction(4, repetitions, functionExecTimeMs)
    if (absolute(res - pi) > 0.0001) {
        console.println("Pi calculation went wrong")
        return 1
    }

    return 0
}

function absolute(val: double): double {
    if (val < 0) {
        return val *= -1
    }
    return val
}

function callSlowFunction(foo: int, repetitions: int, functionExecTimeMs: int): double {
    let res: double = 0
    if (foo == 1) {
        res = slowFunction(repetitions, functionExecTimeMs)
    } else if (foo == 2) {
        res = slowFunctionCopy1(repetitions, functionExecTimeMs)
    } else if (foo == 3) {
        res = slowFunctionCopy2(repetitions, functionExecTimeMs)
    } else if (foo == 4) {
        res = slowFunctionCopy3(repetitions, functionExecTimeMs)
    }
    return res
}

// Doing copypaste because of compiler optimisations
function slowFunction(repetitions: int, functionExecTimeMs: int): double {
    let ret: double = 0.0
    let sign: int = 1
    const startTime = Date.now()
    for (let i: int = 0; i < repetitions; i++) {
        ret += sign / (2.0 * i + 1);
        sign *= -1

        if ((Date.now() - startTime) > functionExecTimeMs) {
            break
        }
    }
    return 4 * ret
}

function slowFunctionCopy1(repetitions: int, functionExecTimeMs: int): double {
    let ret: double = 0.0
    let sign: int = 1
    const startTime = Date.now()
    for (let i: int = 0; i < repetitions; i++) {
        ret += sign / (2.0 * i + 1);
        sign *= -1

        if ((Date.now() - startTime) > functionExecTimeMs) {
            break
        }
    }
    return 4 * ret
}

function slowFunctionCopy2(repetitions: int, functionExecTimeMs: int): double {
    let ret: double = 0.0
    let sign: int = 1
    const startTime = Date.now()
    for (let i: int = 0; i < repetitions; i++) {
        ret += sign / (2.0 * i + 1);
        sign *= -1

        if ((Date.now() - startTime) > functionExecTimeMs) {
            break
        }
    }
    return 4 * ret
}

function slowFunctionCopy3(repetitions: int, functionExecTimeMs: int): double {
    let ret: double = 0.0
    let sign: int = 1
    const startTime = Date.now()
    for (let i: int = 0; i < repetitions; i++) {
        ret += sign / (2.0 * i + 1);
        sign *= -1

        if ((Date.now() - startTime) > functionExecTimeMs) {
            break
        }
    }
    return 4 * ret
}

}
