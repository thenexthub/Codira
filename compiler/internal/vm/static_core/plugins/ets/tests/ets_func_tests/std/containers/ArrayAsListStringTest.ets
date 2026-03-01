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
    failures += check((): int => { return createDefault()}, "Create default empty list");
    
    failures += check((): int => { return createWithGivenZeroSize()}, "Create list with given zero size");
    failures += check((): int => { return createWithGivenSize()}, "Create list with given size");
    failures += check((): int => { return createWithGivenSizeAndValue0()}, "Create list with given size and empty string as init value");
    failures += check((): int => { return createWithGivenSizeAndValue1()}, "Create list with given size and string init value");


    failures += check((): int => { return pushFrontToEmptyList()}, "Create empty list and push item in front");
    failures += check((): int => { return pushBackToEmptyList()}, "Create empty list and push item in back");
    failures += check((): int => { return pushFrontToNonEmptyList()}, "Create non empty list and push item in front");
    failures += check((): int => { return pushBackToNonEmptyList()}, "Create non empty list and push item in back");
    failures += check((): int => { return pushAndPopFrontFromNonEmptyList()}, "Push and Pop from/to start of non empty list");
    failures += check((): int => { return pushAndPopBackFromNonEmptyList()}, "Push and Pop from/to end of non empty list");

    failures += check((): int => { return emptyToArray()}, "Test empty toArray method");
    failures += check((): int => { return toArray()}, "Test toArray method");
    
    failures += check((): int => { return updateCapacityNoChange()}, "Update list capacity with no change");
    failures += check((): int => { return updateCapacityWithIncrease()}, "Update list capacity with increase");
    failures += check((): int => { return updateCapacityWithDecrease()}, "Update list capacity with decrease");

    failures += check((): int => { return createWithNegativeInitialCapacity()}, "Negative array size passed. Expected to throw NegativeArraySizeError error");
    failures += check((): int => { return accessOutOfBoundsValue()}, "Access value out of bounds. Expected to throw ArrayIndexOutOfBoundsError")
    
    return check(failures, "All tests run");
}


function createDefault(): int {
    let list = new containers.ArrayAsListString();
    return list.size();
}

function createWithNegativeInitialCapacity(): int {
    try {
        new containers.ArrayAsListString(-1);
    } catch(nse) {
        return 0;
    }
    return 1;
}

function accessOutOfBoundsValue(): int {
    let list = new containers.ArrayAsListString(10);
    try {
        list.at(0);
        list.at(12);
    } catch(aiobe) {
        return 0;
    }
    return 1;
}


function createWithGivenZeroSize(): int {
    let list = new containers.ArrayAsListString(0);
    let result = list.size() == 0 ? 0 : 1; 
    return result;
}

function createWithGivenSize(): int {
    let list = new containers.ArrayAsListString(10);
    for(let i: int = 0; i< 10; i++) {
        if(list.at(i) != "") return 1;
    }
    let result = list.size() == 10 ? 0 : 1;
    return result;    
}

function createWithGivenSizeAndValue0(): int {
    let value = "";
    let list = new containers.ArrayAsListString(10, value);
    for(let i: int = 0; i< 10; i++) {
        if(list.at(i) != value) return 1;
    }
    let result = list.size() == 10 ? 0 : 1;
    return result; 
}

function createWithGivenSizeAndValue1(): int {
    let value = "10";
    let list = new containers.ArrayAsListString(10, value);
    for(let i: int; i< 10; i++) {
        if(list.at(i) != value) return 1;
    }
    let result = list.size() == 10 ? 0 : 1;
    return result;
}

function pushFrontToEmptyList(): int {
    let value = "-10";
    let list = new containers.ArrayAsListString();
    list.pushFront(value);

    let expected: String = list.at(0);
    let result = (expected == value && list.size() == 1) ? 0 : 1;
    return result;
}

function pushBackToEmptyList(): int {
    let value = "-10";
    let list = new containers.ArrayAsListString();
    list.pushBack(value);

    let expected: String = list.at(0);
    let result = (expected == value && list.size() == 1) ? 0 : 1;
    return result;
}

function pushFrontToNonEmptyList(): int {
    let value = "-10";
    let list = new containers.ArrayAsListString(9, "9");
    list.pushFront(value);

    let expected: String = list.at(0);
    let result = (expected == value && list.size() == 10) ? 0 : 1;
    return result;    
}

function pushBackToNonEmptyList(): int {
    let value = "-10";
    let list = new containers.ArrayAsListString(9, "9");
    list.pushBack(value);

    let expected: String = list.at(9);
    let result = (expected == value && list.size() == 10) ? 0 : 1;
    return result;
}

function pushAndPopFrontFromNonEmptyList(): int {
    let init = "10";
    let list = new containers.ArrayAsListString(10, init);
    let value = "9";
    
    list.pushFront(value);
    let expected = list.popFront();
    let head = list.at(0);

    let result = (expected == value && head == init && list.size() == 10) ? 0 : 1;
    return result;

}

function pushAndPopBackFromNonEmptyList(): int {
    let init = "10";
    let list = new containers.ArrayAsListString(10, init);
    let value = "9";
    
    list.pushBack(value);
    let expected = list.popBack();
    let tail = list.at(9);

    let result = (expected == value && tail == init && list.size() == 10) ? 0 : 1;
    return result;

}

function emptyToArray(): int {
    let list = new containers.ArrayAsListString();
    let target: String[] = list.toArray();
    if(target.length == 0) {
        return 0;
    }
    console.println("Output array length mismatch");
    return 1;
}

function toArray(): int {
    let value: String = "44";
    let list = new containers.ArrayAsListString(10);

    for(let j: int = 0; j< 10; j++) {
        list.set(j, value);    
    }

    let target: String[] = list.toArray();
    if(target.length != 10) {
        console.println("Output array length mismatch");
        return 1;
    }

    for(let i: int = 0; i< target.length; i++) {
        if(target[i] == value) continue;
        console.println("Unexpected value in array");
        return 1;
    }
    return 0;
}

function updateCapacityNoChange(): int {
    let list = new containers.ArrayAsListString(100);
    list.reserve(100);
    let result = list.size() == 100 ? 0: 1;
    return result;
}

function updateCapacityWithIncrease(): int {
    let list = new containers.ArrayAsListString(100);
    list.reserve(200);
    let result = list.size() == 100 ? 0: 1;
    return result;
}

function updateCapacityWithDecrease(): int {
    let list = new containers.ArrayAsListString(100);
    list.reserve(50);

    let result = list.size() == 100 ? 0: 1;
    return result;
}



function check(result: int, message: String): int {
    if(result == 0) {
      return 0;
    }
    console.println("\nFAILED: " + message);
    return 1;
}

function check(fn: () => int, message: String): int {
    let result: int = 0;
    try {
        result = fn();
    } catch(e) {
        console.println("FAILED: " + message + " => Error catched: " + e);
        return 1;
    }

    if(result == 0) {
      return 0;
    }
    console.println("\nFAILED: " + message);
    return 1;
}
