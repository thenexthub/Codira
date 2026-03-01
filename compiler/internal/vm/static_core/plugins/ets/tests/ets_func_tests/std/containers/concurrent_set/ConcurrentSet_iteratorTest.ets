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

import { launch } from "std/concurrency"

let flag: int = 0;
let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];

let ConcurrentSetTestOne: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();

function main(): int {
    let ConcurrentSetTestsuite = new arktest.ArkTestsuite("ConcurrentSet$_iteratorTest");
    // Single Thread Test
    ConcurrentSetTestsuite.addTest("$_iteratorTest001", () =>
    {
        let ConcurrentSetTest: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrValues[i]);
        }
        res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 10);
        let iterValues: IterableIterator<string> = ConcurrentSetTest.$_iterator();
        flag = 0;
        for (let i: int = 0; i < 10; i++) {
            if (iterValues.next().value != arrValues[i]) {
                flag++;
            }
        }
        arktest.assertEQ(flag, 0);
        arktest.assertEQ(iterValues.next().value, undefined);
    });
    ConcurrentSetTestsuite.addTest("$_iteratorTest002", () =>
    {
        let ConcurrentSetTestNull: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = ConcurrentSetTestNull.size;
        arktest.assertEQ(res, 0);
        let iterValue: IterableIterator<string> = ConcurrentSetTestNull.$_iterator();
        arktest.assertEQ(iterValue.next().value, undefined);
    });
    // Multiple Threads Test
    ConcurrentSetTestsuite.addTest("$_iterator&AddTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        addFront();
        let p1 = launch<void, () => void>(addBack);
        let p2 = launch<void, () => void>($_iteratorAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(ConcurrentSetTestOne.size, 10);
    });
    ConcurrentSetTestsuite.addTest("$_iterator&DeteleTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch<void, () => void>(deleteAll);
        let p2 = launch<void, () => void>($_iteratorAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
    });
    ConcurrentSetTestsuite.addTest("$_iterator&DeteleTest002", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        deleteAll();
        $_iteratorAll();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
    });
    ConcurrentSetTestsuite.addTest("$_iterator&ClearTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch<void, () => void>($_iteratorAll);
        let p2 = launch<void, () => void>(clearTest);
        p1.Await();
        p2.Await();
        arktest.assertTrue(ConcurrentSetTestOne.size < 11);
    });

    return ConcurrentSetTestsuite.run();
}

function $_iteratorAll() {
    let iterator$ = ConcurrentSetTestOne.$_iterator();
    let i = 0;
    while (iterator$.next().value != undefined) {
        i++;
    }
}

function addFront() {
    for (let i: int = 0; i < 5; i++) {
        ConcurrentSetTestOne.add(arrKeys[i]);
    }
}

function addBack() {
    for (let i: int = 5; i < 10; i++) {
        ConcurrentSetTestOne.add(arrKeys[i]);
    }
}

function clearTest() {
    ConcurrentSetTestOne.clear();
}

function deleteAll() {
    for (let i: int = 0; i < 10; i++) {
        ConcurrentSetTestOne.delete(i);
    }
}

function addAll() {
    for (let i: int = 0; i < 10 ; i++) {
        ConcurrentSetTestOne.add(arrKeys[i]);
    }
}
