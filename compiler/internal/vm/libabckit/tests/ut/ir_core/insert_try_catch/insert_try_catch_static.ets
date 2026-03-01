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
function foo(a: int) {
    try {
        console.log("TRY1")
        if (a == 0) {
            console.log("TRY2")
            throw new Error("aaa")
        }
        console.log("TRY3")
    } catch (e: Error) {
        if (a == 2) {
            console.log("CATCH_IF")
        }
        console.log("CATCH")
    }
}

function print_catch() {
    console.log("CATCH");
}

function bar(a: int) {
    console.log("TRY")
    if (a == 0) {
        console.log("THROW")
        throw new Error("MOCK_ERROR");
    }
}

function bar0(a: int) {
    try {
        console.log("TRY")
        if (a == 0) {
            console.log("THROW")
            throw new Error("MOCK_ERROR");
        }
    } catch(e: Error) {
        print_catch();
    }
}

function main0() {
    try {
        bar(0)
    } catch(e: Error) {
        bar(1);
    }
}

function main() {
    foo(0);
    foo(1);
}

