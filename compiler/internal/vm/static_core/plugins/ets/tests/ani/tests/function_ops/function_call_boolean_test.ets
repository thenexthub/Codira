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
function or(a0: boolean, a1: boolean): boolean {
    return a0 || a1;
}
}

function moduleFunction(value: int): boolean {
    if (value == 2) {
        return true;
    } else {
        return false;
    }
}

function moduleFunction(value1: int, value2: int): boolean {
    if (value1 > value2) {
        return true;
    } else {
        return false;
    }
}

namespace A {
    function booleanFunctionA(value1: int, value2: int): boolean {
        if (value1 === value2) {
            return true;
        } else {
            return false;
        }
    }

    function booleanFunctionA(value1: int, value2: int, value3: int): boolean {
        if ((value1 + value2) > value3) {
            return true;
        } else {
            return false;
        }
    }

    function nestedFunction(value1: int, value2: int): boolean {
        let lambdaFunction: (p1: int, p2: int) => boolean =
            (param1: int, param2: int): boolean => {
                if (param1 > param2) {
                    return true;
                } else {
                    return false;
                }
        };
        return lambdaFunction(value1, value2);
    }

    export function recursiveFunction(value: int): boolean {
        if (value <= 0) {
            return false;
        }
        if (value === 1) {
            return true;
        }
        return recursiveFunction(value - 1);
    }

    function calculateBoolean(value1: int, value2: int, value3: char, value4: double): boolean {
        let aChar: char = c'A';
        if (value3 == aChar) {
            return true;
        }
        if (value4 == 1.0) {
            return false;
        }
        return value1 > value2 ? true : false;
    }

    namespace B {
        function booleanFunctionB(value1: int, value2: int): boolean {
            if (value1 > value2) {
                return true;
            } else {
                return false;
            }
        }
    }
}
