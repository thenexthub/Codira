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
function concat(v0: int, v1: int): String {
    return "" + v0 + v1;
}
}

// function checkSum(): boolean {
//     return ops.sum(8, 16) == 24;
// }
//
// function checkConcat(): boolean {
//     return ops.concat("abc", "def") == "abcdef";
// }

function moduleFunction(value1: int, value2: int): string {
    if (value1 === value2) {
        return "Equality";
    } else {
        return "Inequality";
    }
}

function moduleFunction(value1: int, value2: int, value3: int): string {
    if (value1 > value2 && value1 > value3) {
        return "Greater";
    } else {
        return "Less";
    }
}

namespace A {
    function refFunctionA(value1: int, value2: int): string {
        if (value1 === value2) {
            return "Equality";
        } else {
            return "Inequality";
        }
    }

    function refFunctionA(value1: int, value2: int, value3: int): string {
        if (value1 > value2 && value1 > value3) {
            return "Greater";
        } else {
            return "Less";
        }
    }

    function nestedFunction(value1: int, value2: int): string {
        let lambdaFunction: (p1: int, p2: int) => string =
            (param1: int, param2: int): string => {
                if (param1 === param2) {
                    return "Equality";
                } else {
                    return "Inequality";
                }
        };
        return lambdaFunction(value1, value2);
    }
    
    export function recursiveFunction(value: int): string {
        if (value <= 0) {
            return "Less";
        }
        if (value === 1) {
            return "Greater";
        }
        return recursiveFunction(value - 1);
    }

    function calculateSum(value1: int, value2: int, value3: double, value4: int): string {
        if (value3 == 1.0) {
            return "CharEquality";
        }
        if (value4 == 1) {
            return "NumEquality";
        }
        if (value1 === value2) {
            return "Equality";
        } else {
            return "Inequality";
        }
    }

    namespace B {
        function refFunctionB(value1: int, value2: int): string {
            if (value1 > value2) {
                return "Greater";
            } else {
                return "Less";
            }
        }
    }
}