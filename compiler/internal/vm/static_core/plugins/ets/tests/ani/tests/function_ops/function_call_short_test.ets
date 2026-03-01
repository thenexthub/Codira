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
function sum(a0: short, a1: short): short {
    return (a0 + a1).toShort();
}
}

function moduleFunction(value1: short, value2: short): short {
    return (value1 + value2).toShort();
}

function moduleFunction(value1: short, value2: short, value3: short): short {
    return (value1 + value2 + value3).toShort();
}

namespace A {
    function shortFunctionA(value1: short, value2: short): short {
        return (value1 + value2).toShort();
    }

    function shortFunctionA(value1: short, value2: short, value3: short): short {
        return (value1 + value2 + value3).toShort();
    }

    function nestedFunction(value1: short, value2: short): short {
        let lambdaFunction: (p1: short, p2: short) => short =
            (param1: short, param2: short): short => {
                return (param1 + param2).toShort();
        };
        return lambdaFunction(value1, value2);
    }

    export function recursiveFunction(value: int): short {
        if (value <= 1) {
            return 1;
        } else {
            return (value + recursiveFunction(value - 1)).toShort();
        }
    }

    function calculateSum(value1: short, value2: short, value3: char, value4: double): short {
        let aChar: char = c'A';
        if (value3 == aChar) {
            return (value2 - value1).toShort();
        }
        if (value4 == 1.0) {
            return value1;
        }
        return (value2 + value1).toShort();
    }

    namespace B {
        function shortFunctionB(value1: short, value2: short): short {
            return (value1 + value2).toShort();
        }
    }
}
