/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { xml } from "@ohos.xml"

function main(): int {
    const suite = new arktest.ArkTestsuite('XmlDynamicSerializer API tests');

    suite.addTest('Testing add an empty element inside a start and end element.', testAddEmptyElementInsideStartAndEnd);
    suite.addTest('Testing add an empty element after the end element.', testAddEmptyElementAfterEnd);
    suite.addTest('Testing add an empty element before the start element.', testAddEmptyElementBeforeStart);
    suite.addTest('Testing add an empty element without any start or end element.',
        testAddEmptyElementWithoutStartOrEnd);
    suite.addTest('Testing add multiple empty elements inside a start and end element.',
        testAddMultipleEmptyElementsInsideStartAndEnd);
    suite.addTest('Testing Boundary Value', testBoundaryValue);

    return suite.run();
}

/**
 * Common function to perform XML serialization tests
 * @param serializerOperation Function to perform operations on the serializer
 * @param expectedResult Expected XML string result
 */
function performXmlTest(serializerOperation: (serializer: xml.XmlDynamicSerializer) => void, expectedResult: string): void {
    let thatSer = new xml.XmlDynamicSerializer();
    
    serializerOperation(thatSer);
    
    let arrayBuffer = thatSer.getOutput();
    let view = new Uint8Array(arrayBuffer);
    let view1 = '';
    for (let i = 0; i < expectedResult.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i].toInt());
    }
    arktest.assertEQ(view1, expectedResult);
}

// Test cases ported from ArkTS 1.0: XML_01600
function testAddEmptyElementInsideStartAndEnd() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.addEmptyElement('a');
        serializer.endElement();
    }, '<note>\r\n  <a/>\r\n</note>');
}

// Test cases ported from ArkTS 1.0: XML_01700
function testAddEmptyElementAfterEnd() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.endElement();
        serializer.addEmptyElement('b');
    }, '<note/>\r\n<b/>');
}

// Test cases ported from ArkTS 1.0: XML_01800
function testAddEmptyElementBeforeStart() {
    performXmlTest((serializer) => {
        serializer.addEmptyElement('c');
        serializer.startElement('note');
        serializer.endElement();
    }, '<c/>\r\n<note/>');
}

// Test cases ported from ArkTS 1.0: XML_01900
function testAddEmptyElementWithoutStartOrEnd() {
    performXmlTest((serializer) => {
        serializer.addEmptyElement('d');
    }, '<d/>');
}

// Test cases ported from ArkTS 1.0: XML_02000
function testAddMultipleEmptyElementsInsideStartAndEnd() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.addEmptyElement('c');
        serializer.addEmptyElement('d');
        serializer.endElement();
    }, '<note>\r\n  <c/>\r\n  <d/>\r\n</note>');
}

function testBoundaryValue() {
    let thatSer = new xml.XmlDynamicSerializer();
    let arrayBuffer = thatSer.getOutput();
    let view1 = "";
    let result = "";
    let view = new Uint8Array(arrayBuffer);
    for (let i = 0; i < view.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i].toInt());
    }
    arktest.assertEQ(view1, result);
}
