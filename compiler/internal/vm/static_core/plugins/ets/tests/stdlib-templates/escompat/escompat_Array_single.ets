/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc:  {function: Test_escompat_Array }
---*/

function main(): int {
    let failures: int = 0;

    failures += check(testCreateFromString(), "Test Create Array object from String");
    failures += check(testCreateFromString1(), "Test Create Array object from String with mapping func");
    failures += check(testCreateFromString2(), "Test Create Array object from String with mapping func");
    failures += check(testFiltered(), "Test Create Array object from given with filter function");

    return check(failures, "All tests run");
}


function testCreateFromString(): int {
    let src: String = "Hello World";
    let test: Array<String> = Array.from<string>(src);
    if(test.length != src.length) {
        console.println("Array size mismatch");
        return 1;
    }

    for(let i: int = 0; i < src.length; i++) {
        let cc: char = src.charAt(i);
        let ac: char = test[i].charAt(0);
        if(ac == cc) continue;
        console.println("Array data mismatch");
        return 1;
    }
    return 0;
}

function testCreateFromString1(): int {
    let src: String = "HELLO";

    let test = Array.from(src, (item: String, k: int): String => { return item.toLowerCase(); });
    if(test.length != src.length) {
        console.println("Array size mismatch");
        return 1;
    }
    let exp: String = "hello";

    for (let i: int = 0; i < exp.length; i++) {
        let tmp: String = test[i]; // <- verifier fails otherwise
        let ac: char = tmp.charAt(0);
        let cc: char = exp.charAt(i);

        if(ac == cc) continue;
        console.println("Array data mismatch");
        return 1;
    }
    return 0;
}

function testCreateFromString2(): int {
    let src: String = "Hello";
    let test = Array.from(src, (v: String, k: int): Int => { return k });
    if(test.length != src.length) {
        console.println("Array size mismatch");
        return 1;
    }
    for(let i: int = 0; i < src.length; i++) {
        let av: Int = (test[i] as Object) as Int; // <- verifier fails otherwise
        if(av == i) continue;
        console.println("Array data mismatch");
        return 1;
    }
    return 0;
}


function testFiltered(): int {
    let src: String = "HHeelllloo";
    let exp: String = "Hello";
    let test = Array.from(src, (c: string, i: int): string => c).filter((v: String, k: int): boolean => k%2 == 0);

    if(test.length != exp.length) {
        console.println("Array size mismatch");
        return 1;
    }

    for(let i: int = 0; i < exp.length; i++) {
        let ac: char = test[i].charAt(0);
        let cc: char = exp.charAt(i);

        if(ac == cc) continue;
        console.println("Array data mismatch");
        return 1;
    }

    return 0;
}


function check(result: int, message: String): int {
    if(result == 0) {
      return 0;
    }
    console.println("\nFAILED: " + message);
    return 1;
}
