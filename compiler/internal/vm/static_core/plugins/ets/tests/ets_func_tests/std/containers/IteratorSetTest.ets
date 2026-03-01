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
    
    failures += test(testIteratorWithEmptySet(), "Test Iterator With Empty Set");
    failures += test(testIteratorWithNonEmptySet(), "Test Iterator With Non Empty Set");
    failures += test(testIteratorBoundaryConditions(),"Test Iterator Boundary Conditions");

    failures += test(testIteratorWithNonEmptyReadonlySet(),"Test Iterator With Empty ReadonlySet");
    failures += test(testIteratorWithNonEmptyReadonlySet(), "Test Iterator With Non Empty ReadonlySet");
  
    return test(failures, "All tests run");
}

const RESULT_SUCCESS  = 0;
const RESULT_FAILURE = 1;

function testIteratorWithEmptySet(): int {
    const set = new Set<Number>();
    const iterator = set.$_iterator();
    const v = iterator.next();
    if (!v.done) {
        return RESULT_FAILURE;
    }
    return RESULT_SUCCESS ;
}

function testIteratorWithNonEmptySet(): int {
    const set = new Set<Number>();
    set.add(1);
    set.add(2);
    const iterator = set.$_iterator();
    
    let v = iterator.next();
    if (v.done || v.value != 1) {
        return RESULT_FAILURE;
    }
    
    v = iterator.next();
    if (v.done || v.value != 2) {
        return RESULT_FAILURE;
    }
    
    v = iterator.next();
    if (!v.done) {
        return RESULT_FAILURE;
    }
    
    return RESULT_SUCCESS ;
}

function testIteratorBoundaryConditions(): int {
    const set = new Set<Number>();
    set.add(1);
    const iterator = set.$_iterator();
    
    let v = iterator.next();
    if (v.done || v.value != 1) {
        return RESULT_FAILURE;
    }
    
    v = iterator.next();
    if (!v.done) {
        return RESULT_FAILURE;
    }
    
    // Calling next() again after iteration is done
    /* issue number 18321, when the iterator has reached the end, and then called next() on it, v.done returns false. 
    v = iterator.next();
    if (!v.done) {
        return RESULT_FAILURE;
    }*/
    
    return RESULT_SUCCESS ;
}

function testIteratorWithEmptyReadonlySet(): int {
    const emptySet: ReadonlySet<number> = new Set<number>();
    const iterator = emptySet.values();
    const v = iterator.next();
    if (!v.done) {
        return RESULT_FAILURE;
    }
    return RESULT_SUCCESS;
}

function testIteratorWithNonEmptyReadonlySet(): int {
    const tempSet: Set<number> = new Set<number>();
    tempSet.add(1);
    tempSet.add(2);
    tempSet.add(3);

    const set : ReadonlySet<number> = tempSet as ReadonlySet<number>;
    const iterator = set.values();

    let v = iterator.next();
    if (v.done || v.value != 1) {
        return RESULT_FAILURE;
    }

    v = iterator.next();
    if (v.done || v.value != 2) {
        return RESULT_FAILURE;
    }

    v = iterator.next();
    if (v.done || v.value != 3) {
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
      console.println('PASSED: ' + message);
      return 0;
    }
    console.println('FAILED: ' + message);
    return 1;
}
