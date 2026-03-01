/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

import { xml } from "@ohos.xml"
import { BusinessError } from "@ohos.base"

const TypeErrorCodeId: int = 401;
const MaxBufferLength = 2048;

function main(): int {
    const suite = new arktest.ArkTestsuite('XmlSerializer API tests');

    suite.addTest('Test adding an empty element between start and end elements.',
        testAddEmptyElementBetweenStartAndEnd);
    suite.addTest('Test adding an empty element after the end element.', testAddEmptyElementAfterEnd);
    suite.addTest('Test adding an empty element before the start element.', testAddEmptyElementBeforeStart);
    suite.addTest('Test adding an empty element without start or end elements.', testAddEmptyElementWithoutStartOrEnd);
    suite.addTest('Test adding two empty elements between start and end elements.',
        testAddTwoEmptyElementsBetweenStartAndEnd);
    suite.addTest('Test adding an empty element with an empty name.', testAddEmptyElementWithEmptyName);

    return suite.run();
}

/**
 * Helper function to test XML serialization
 * @param serializerOps Function that performs operations on the XML serializer
 * @param expectedResult Expected XML output as string
 */
function performXmlTest(serializerOps: (serializer: xml.XmlSerializer) => void, expectedResult: string): void {
    let arrayBuffer = new ArrayBuffer(MaxBufferLength);
    let serializer = new xml.XmlSerializer(arrayBuffer);
    
    // Execute the XML operations
    serializerOps(serializer);
    
    // Convert buffer to string for comparison
    let view = new Uint8Array(arrayBuffer);
    let actualResult = '';
    for (let i = 0; i < expectedResult.length; ++i) {
        actualResult = actualResult + String.fromCodePoint(view[i].toInt());
    }
    
    arktest.assertEQ(actualResult, expectedResult);
}

// Test cases ported from ArkTS 1.0: XML_02600
function testAddEmptyElementBetweenStartAndEnd() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.addEmptyElement('a');
        serializer.endElement();
    }, '<note>\r\n  <a/>\r\n</note>');
}

// Test cases ported from ArkTS 1.0: XML_02700
function testAddEmptyElementAfterEnd() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.endElement();
        serializer.addEmptyElement('b');
    }, '<note/>\r\n<b/>');
}

// Test cases ported from ArkTS 1.0: XML_02800
function testAddEmptyElementBeforeStart() {
    performXmlTest((serializer) => {
        serializer.addEmptyElement('c');
        serializer.startElement('note');
        serializer.endElement();
    }, '<c/>\r\n<note/>');
}

// Test cases ported from ArkTS 1.0: XML_02900
function testAddEmptyElementWithoutStartOrEnd() {
    performXmlTest((serializer) => {
        serializer.addEmptyElement('d');
    }, '<d/>');
}

// Test cases ported from ArkTS 1.0: XML_03000
function testAddTwoEmptyElementsBetweenStartAndEnd() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.addEmptyElement('c');
        serializer.addEmptyElement('d');
        serializer.endElement();
    }, '<note>\r\n  <c/>\r\n  <d/>\r\n</note>');
}

function testAddEmptyElementWithEmptyName() {
    let arrayBuffer = new ArrayBuffer(MaxBufferLength);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement('note');
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == TypeErrorCodeId &&
                e.message == 'Parameter error. Parameter cannot be empty';
        }
        return false;
    }
    arktest.expectThrow(() => { thatSer.addEmptyElement('') }, exceptionCheck);
}
