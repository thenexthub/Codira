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

function main(): int {
    let failures: int = 0;

    failures += test(testIteratorWithEmptyMap(),"Test Iterator With Empty Map");
    failures += test(testIteratorWithNonEmptyMap(), "Test Iterator With Non Empty Map");
    failures += test(testIteratorBoundaryConditions(), "Test Iterator Boundary Conditions");

    failures += test(testIteratorWithEmptyReadonlyMap(),"Test Iterator With Empty ReadonlyMap");
    failures += test(testIteratorWithNonEmptyReadonlyMap(), "Test Iterator With Non Empty ReadonlyMap");

    return test(failures, "All tests run");
}

const RESULT_FAILURE = 1;
const RESULT_SUCCESS = 0;

function testIteratorWithEmptyMap(): int {
    const map = new Map<String, Number>();
    const iterator = map.$_iterator();
    const v = iterator.next();
    if (!v.done) {
        return RESULT_FAILURE ;
    }
    return RESULT_SUCCESS ;
}

function testIteratorWithNonEmptyMap(): int {
    const map = new Map<String, Number>();
    map.set("key1", 1);
    map.set("key2", 2);
    const iterator = map.$_iterator();

    let v = iterator.next();
    if (v.done || (v.value as [string, Number])[0] != "key1" || (v.value as [string, Number])[1] != 1) {
        return RESULT_FAILURE ;
    }

    v = iterator.next();
    if (v.done || (v.value as [string, Number])[0] != "key2" || (v.value as [string, Number])[1] != 2) {
        return RESULT_FAILURE ;
    }

    v = iterator.next();
    if (!v.done) {
        return RESULT_FAILURE ;
    }

    return RESULT_SUCCESS ;
}

function testIteratorBoundaryConditions(): int {
    const map = new Map<String, Number>();
    map.set("key1", 1);
    const iterator = map.$_iterator();

    let v = iterator.next();
    if (v.done || (v.value as [string, Number])[0] != "key1" || (v.value as [string, Number])[1] != 1) {
        return RESULT_FAILURE ;
    }

    v = iterator.next();
    if (!v.done) {
        return RESULT_FAILURE ;
    }

    // Calling next() again after iteration is done
    /* issue number 18321, when the iterator has reached the end, and then called next() on it, v.done returns false.
    v = iterator.next();
    if (!v.done) {
        return RESULT_FAILURE;
    }*/

    return RESULT_SUCCESS ;
}

function testIteratorWithEmptyReadonlyMap(): int {
    const emptyMap: ReadonlyMap<string, number> = new Map<string, number>();
    const iterator = emptyMap.$_iterator();
    const v = iterator.next();
    if (!v.done) {
        return RESULT_FAILURE;
    }
    return RESULT_SUCCESS;
}

function testIteratorWithNonEmptyReadonlyMap(): int {
    const map: ReadonlyMap<string, number> = new Map<string, number>();
    map.set("one", 1);
    map.set("two", 2);
    map.set("three", 3);

    const iterator = map.$_iterator();

    let v = iterator.next();
    if (v.done || (v.value as [string, Number])[0]  != 'one' || (v.value as [string, Number])[1]  != 1) {
        return RESULT_FAILURE;
    }

    v = iterator.next();
    if (v.done || (v.value as [string, Number])[0]  != 'two' || (v.value as [string, Number])[1]  != 2) {
        return RESULT_FAILURE;
    }

    v = iterator.next();
    if (v.done || (v.value as [string, Number])[0]  != 'three' || (v.value as [string, Number])[1]  != 3) {
        return RESULT_FAILURE;
    }

    v = iterator.next();
    if (!v.done) {
        return RESULT_FAILURE;
    }

    return RESULT_SUCCESS;
}

function test(result: int, message: String ): int {
    if(result == 0) {
      console.println("PASSED: " + message);
      return 0;
    }
    console.println("FAILED: " + message);
    return 1;
}
