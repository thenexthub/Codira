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

/*---
desc: >-
    There could be cases when the type of an entity (mostly local variables)
    is a union type and in the particular context of the program the compiler
    can narrow (smart cast) the static type to a more precise type (smart type)
    and allow operations which are specific to such narrowed type.
---*/
class A {}
let a: A|undefined | null = new A()

let num: Number = 0;
let str: string = "";
let err: Error = new Error();
let bool: Boolean = false;

function foo(p: Object | undefined | null): number {
    if (p == undefined) {
        a = p
        return 1
    } else {
        let obj: Object = p // narrowing conversion
        if (obj instanceof Number) {
            num = obj * 2 // numeric operation without cast
            return 2
        } else {
            if (obj instanceof Error) {
                err = obj
                str = obj.message // field access without cast
                return 3
            } else {
                if (obj instanceof string) {
                    str = obj.substr(0, 1) // string method without cast
                    return 4
                } else {
                    bool = !obj // boolean operator without cast
                    return 5
                }
            }
        }
    }
}

function main() {
    arktest.assertTrue(foo(undefined) == 1 && a === undefined)
    arktest.assertTrue(foo(42.0) == 2 && num == 84)
    let e: Error = new Error("xyz")
    arktest.assertTrue(foo(e) == 3 && err == e && str == "xyz")
    arktest.assertTrue(foo("abc") == 4 && str == "a")
    arktest.assertTrue(foo(true) == 5 && !bool)
}
