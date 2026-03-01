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
function sum(a0: double, a1: double): double {
    return a0 + a1 as double;
}
}

function moduleFunction(value1: double, value2: double): double {
    return value1 + value2 as double;
}

function moduleFunction(value1: double, value2: double, value3: double): double {
    return value1 + value2 + value3 as double;
}

namespace A {
    function doubleFunctionA(value1: double, value2: double): double {
        return value1 + value2 as double;
    }

    function doubleFunctionA(value1: double, value2: double, value3: double): double {
        return value1 + value2 + value3 as double;
    }

    function nestedFunction(value1: double, value2: double): double {
        let lambdaFunction: (p1: double, p2: double) => double =
            (param1: double, param2: double): double => {
                return param1 + param2 as double;
        };
        return lambdaFunction(value1, value2);
    }

    export function recursiveFunction(value: int): double {
        if (value <= 1) {
            return 1;
        } else {
            return value + recursiveFunction(value - 1) as double;
        }
    }

    function calculateSum(value1: double, value2: double, value3: char, value4: int): double {
        let aChar: char = c'A';
        if (value3 == aChar) {
            return value2 - value1 as double;
        }
        if (value4 == 1) {
            return value1;
        }
        return value2 + value1 as double;
    }

    namespace B {
        function doubleFunctionB(value1: double, value2: double): double {
            return value1 + value2 as double;
        }
    }
}
