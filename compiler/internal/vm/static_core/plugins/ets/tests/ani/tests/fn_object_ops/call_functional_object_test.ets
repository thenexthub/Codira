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

class NotFnObj {
    invoke0(): String {
        return "test";
    }
}

function GetNonFnObj(): NotFnObj {
    return new NotFnObj();
}

function GetFnObj(): Object {
    let x = (a : String) => { return a; }
    return x;
}

function GetFnObjWithClosure(): Object {
    let y = "test";
    let x = (a : String) => { return y + a; }
    return x;
}

function GetFnObjNested(): Object {
    let y = "hello ";
    let x = (a : String) => {
        let z = (b : String): String => {
            return y + b;
        }
        return z(a);
    }

    return x;
}


function recursiveFunction(y: string, a: string): string {
    if (y == a) {
        return a;
    } else {
        return recursiveFunction(y, a + "1");
    }
}

function GetFnObjRecursive(): Object {
    let y = "hello1111111111";
    let x = (a: string): string => {
        return recursiveFunction(y, a);
    };
    return x;
}