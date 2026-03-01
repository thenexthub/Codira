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
function sum(a0: int, a1: int): int {
    return a0 + a1 as int;
}
}

function moduleFunction(value1: int, value2: int): int {
    return value1 + value2 as int;
}

function moduleFunction(value1: int, value2: int, value3: int): int {
    return value1 + value2 + value3 as int;
}

namespace A {
    function intFunctionA(value1: int, value2: int): int {
        return value1 + value2 as int;
    }

    function intFunctionA(value1: int, value2: int, value3: int): int {
        return value1 + value2 + value3 as int;
    }

    function nestedFunction(value1: int, value2: int): int {
        let lambdaFunction: (p1: int, p2: int) => int =
            (param1: int, param2: int): int => {
                return param1 + param2 as int;
        };
        return lambdaFunction(value1, value2);
    }

    export function recursiveFunction(value: int): int {
        if (value <= 1) {
            return 1;
        } else {
            return value + recursiveFunction(value - 1) as int;
        }
    }

    function calculateSum(value1: int, value2: int, value3: char, value4: double): int {
        let aChar: char = c'A';
        if (value3 == aChar) {
            return value2 - value1 as int;
        }
        if (value4 == 1.0) {
            return value1;
        }
        return value2 + value1 as int;
    }

    namespace B {
        function intFunctionB(value1: int, value2: int): int {
            return value1 + value2 as int;
        }
    }
}
