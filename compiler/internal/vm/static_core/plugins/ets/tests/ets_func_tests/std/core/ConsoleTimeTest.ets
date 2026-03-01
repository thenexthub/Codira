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
    const suite = new arktest.ArkTestsuite("Console Time API tests")
    suite.addTest("Test console timeLog with two parameters", TestConsoleTimeLogTwoPara)
    suite.addTest("Test console time with Null Character", TestConsoleTimeNullCharacter)
    suite.addTest("Test console timeEnd with Null Character", TestConsoleTimeEndNullCharacter)
    suite.addTest("Test console timeLog with Null Character", TestConsoleTimeLogNullCharacter)
    suite.addTest("Test console time with Many Null Characters", TestConsoleTimeManyNullCharacters)
    suite.addTest("Test console time with Size Zero String", TestConsoleTimeSizeZeroString)

    return suite.run()
}

function TestConsoleTimeLogTwoPara(): void {
    arktest.expectNoThrow(() => {
        let TestLog = new Array<string>();
        const str = '太好了';
        console.time(str);
        console.timeLog(str, TestLog);
        console.timeEnd(str);
    })
}

function TestConsoleTimeNullCharacter(): void {
    arktest.expectError(() => {
        const str = '\0';
        console.time(str);
    }, 'Parameter error. The input parameters are invalid, Timer or Counter name must be not null.');
}

function TestConsoleTimeEndNullCharacter(): void {
    arktest.expectError(() => {
        const str = '\0';
        console.time(str);
        console.timeEnd(str);
    }, 'Parameter error. The input parameters are invalid, Timer or Counter name must be not null.');
}

function TestConsoleTimeLogNullCharacter(): void {
    arktest.expectError(() => {
        const str = '\0';
        console.time(str);
        console.timeLog(str)
        console.timeEnd(str);
    }, 'Parameter error. The input parameters are invalid, Timer or Counter name must be not null.');
}

function TestConsoleTimeSizeZeroString(): void {
    arktest.expectError(() => {
        let str = "";
        console.time(str);
    }, 'Parameter error. The input parameters are invalid, Timer or Counter name must be not null.')
}

function TestConsoleTimeManyNullCharacters(): void {
    arktest.expectError(() => {
        let str = '\0\0';
        console.time(str);
    }, 'Parameter error. The input parameters are invalid, Timer or Counter name must be not null.')
}