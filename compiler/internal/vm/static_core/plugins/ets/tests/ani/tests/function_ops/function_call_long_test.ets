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
function sum(a0: long, a1: long): long {
    return a0 + a1 as long;
}
}

function moduleFunction(value1: long, value2: long): long {
    return value1 + value2 as long;
}

function moduleFunction(value1: long, value2: long, value3: long): long {
    return value1 + value2 + value3 as long;
}

namespace A {
    function longFunctionA(value1: long, value2: long): long {
        return value1 + value2 as long;
    }

    function longFunctionA(value1: long, value2: long, value3: long): long {
        return value1 + value2 + value3 as long;
    }

    function nestedFunction(value1: long, value2: long): long {
        let lambdaFunction: (p1: long, p2: long) => long =
            (param1: long, param2: long): long => {
                return param1 + param2 as long;
        };
        return lambdaFunction(value1, value2);
    }

    export function recursiveFunction(value: int): long {
        if (value <= 1) {
            return 1;
        } else {
            return value + recursiveFunction(value - 1) as long;
        }
    }

    function calculateSum(value1: long, value2: long, value3: char, value4: double): long {
        let aChar: char = c'A';
        if (value3 == aChar) {
            return value2 - value1 as long;
        }
        if (value4 == 1.0) {
            return value1;
        }
        return value2 + value1 as long;
    }

    namespace B {
        function longFunctionB(value1: long, value2: long): long {
            return value1 + value2 as long;
        }
    }
}
