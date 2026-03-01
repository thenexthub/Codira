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

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];
let strAll: string = "zero,zero,one,one,two,two,three,three,four,four,five,five,six,six,seven,seven,eight,eight,nine,nine";

let ConcurrentSetTestOne: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();

function main(): int {
    let ConcurrentSetTestsuite = new arktest.ArkTestsuite("ConcurrentSetEntriesTest");
    // Single Thread Test
    ConcurrentSetTestsuite.addTest("EntriesTest001", () =>
    {
        let ConcurrentSetTestNull: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = ConcurrentSetTestNull.size;
        arktest.assertEQ(res, 0);
        let iterValue: IterableIterator<[string, string]> = ConcurrentSetTestNull.entries();
        arktest.assertEQ(iterValue.next().value, undefined);
    });
    // Multiple Threads Test
    // Unexpected error: ClassCastError: [Lstd/core/Object; cannot be cast to [Double, Double]|undefined
    /*
    ConcurrentSetTestsuite.addTest("Entries&AddTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        addFront();
        let p1 = launch<void, () => void>(addBack);
        let p2 = launch<void, () => void>(entriesAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(ConcurrentSetTestOne.size, 10);
    });
    ConcurrentSetTestsuite.addTest("Entries&DeteleTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch<void, () => void>(deleteAll);
        let p2 = launch<void, () => void>(entriesAll);
        p1.Await();
        p2.Await();
        arktest.assertTrue(ConcurrentSetTestOne.size < 10);
    });
    ConcurrentSetTestsuite.addTest("Entries&ClearTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        arktest.assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch<void, () => void>(entriesAll);
        let p2 = launch<void, () => void>(clearTest);
        p1.Await();
        p2.Await();
        arktest.assertTrue(ConcurrentSetTestOne.size < 10);
    });
    */

    // Unexpected error: ClassCastError: [Lstd/core/Object; cannot be cast to [string, string]|undefined
    /*
    ConcurrentSetTestsuite.addTest("EntriesTest001", () =>
    {
        let ConcurrentSetTest: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrValues[i]);
        }
        res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 10);
        let iterValues = ConcurrentSetTest.entries();
        let str = "";
        for (let i: int = 0; i < 10; i++) {
            str += iterValues.next().value!.toString();
            if (i < 9){
                str += ",";
            }
        }
        arktest.assertEQ(str, strAll);
        arktest.assertEQ(iterValues.next().value, undefined);
    });
    */

    return ConcurrentSetTestsuite.run();
}

function entriesAll() {
    let iterEntries = ConcurrentSetTestOne.entries();
    let i = 0;
    while (iterEntries.next().value != undefined) {
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
