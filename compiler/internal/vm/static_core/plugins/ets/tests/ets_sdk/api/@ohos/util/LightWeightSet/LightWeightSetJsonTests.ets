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

import {LightWeightSet} from "@ohos.util.LightWeightSet";

class C1 {
    name: string = "";
    age: string = "";
}

function main() {
    const suite = new arktest.ArkTestsuite("LightWeightSet API tests");

    suite.addTest("Test JSON  after adding different elements.", testLightWeightSetJsonAfterAddDifferentElement);
    suite.addTest("Test JSON after adding duplicate elements.", testLightWeightSetJsonAfterAddDuplicateElement);
    suite.addTest("Test JSON after adding object element.", testLightWeightSetJsonAfterAddObjectElement);
    suite.addTest("Test JSON after adding large number element.", testLightWeightSetJsonAfterAddLargeNumberElement);
    suite.addTest("Test JSON for iterator traversal with multiple elements.",
        testLightWeightSetJsonIteratorMultiElement);
    suite.addTest("Test JSON for iterator traversal with empty set.", testLightWeightSetJsonValuesEmptySet);
    suite.addTest("Test JSON for iterator traversal with string concatenation.",
        testLightWeightSetJsonIteratorStringConcat);
    suite.addTest("Test JSON for iterator with empty set.", testLightWeightSetJsonIteratorEmptySet);

    return suite.run();
}

function testLightWeightSetJsonAfterAddDifferentElement() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let result = lightWeightSet.toArray();
    arktest.assertEQ(JSON.stringify(result), '["sparrow","squirrel"]');
}

function testLightWeightSetJsonAfterAddDuplicateElement() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("squirrel");
    let result = lightWeightSet.toArray();
    arktest.assertEQ(JSON.stringify(result), '["squirrel"]');
}

function testLightWeightSetJsonAfterAddObjectElement() {
    let lightWeightSet = new LightWeightSet<object>();
    let a: C1 = {
        name: "Dylon", age: "Dylon"
    };
    lightWeightSet.add(a);
    let result = lightWeightSet.toArray();
    arktest.assertEQ(JSON.stringify(result), '[{"name":"Dylon","age":"Dylon"}]');
}

function testLightWeightSetJsonAfterAddLargeNumberElement() {
    let element = 0;
    let lightWeightSet = new LightWeightSet<number>();
    for (let index = 0; index < 1024; index++) {
        element = element += index;
    }
    lightWeightSet.add(element);
    let result = lightWeightSet.toArray();
    arktest.assertEQ(JSON.stringify(result), '[523776]');
}

function testLightWeightSetJsonIteratorMultiElement() {
    let lightWeightSet = new LightWeightSet<string>();
    lightWeightSet.add("squirrel");
    lightWeightSet.add("sparrow");
    let iter = lightWeightSet.values();
    let expectedValue = ['"sparrow"', '"squirrel"'];
    let index = 0;
    while (index < lightWeightSet.length) {
        arktest.assertEQ(JSON.stringify(iter.next().value), expectedValue[index]);
        index++;
    }
}

function testLightWeightSetJsonValuesEmptySet() {
    let lightWeightSet = new LightWeightSet<string>();
    let iter = lightWeightSet.values();
    arktest.assertEQ(JSON.stringify(iter.next().value), 'undefined');
}

function testLightWeightSetJsonIteratorStringConcat() {
    let element = '';
    let lightWeightSet = new LightWeightSet<string>();
    let expectedValue = new Array<string>();
    for (let index = 0; index < 1; index++) {
        element = element.concat('squirrel');
        lightWeightSet.add(element);
        expectedValue.push(JSON.stringify(element));
    }
    let iter = lightWeightSet.values();
    let index = 0;
    while (index < lightWeightSet.length) {
        let result = JSON.stringify(iter.next().value);
        arktest.assertTrue(expectedValue.includes(result), `Element ${result} not found in expectedValue`);
        index++;
    }
}

function testLightWeightSetJsonIteratorEmptySet() {
    let lightWeightSet = new LightWeightSet<string>();
    let iter = lightWeightSet.$_iterator().next();
    let index = 0;
    while (index < lightWeightSet.length) {
        arktest.assertEQ(JSON.stringify(iter.value), 'iter.next');
        index++;
    }
}
