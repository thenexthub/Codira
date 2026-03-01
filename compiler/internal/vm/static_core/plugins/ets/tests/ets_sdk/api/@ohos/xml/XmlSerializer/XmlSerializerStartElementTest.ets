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
    const suite = new arktest.ArkTestsuite("XmlSerializer API tests");

    suite.addTest("Testing invalid encoding for XmlSerializer", testInvalidEncodingForXmlSerializer);
    suite.addTest("Testing single element start tag", testSingleElementStartElement);
    suite.addTest("Testing multiple element start and end tags", testMultipleElementStartAndEndElements);
    suite.addTest("Testing nested element start and end tags", testNestedElementStartAndEndElements);
    suite.addTest("Testing deeply nested element start and end tags", testDeeplyNestedElementStartAndEndElements);
    suite.addTest("Testing sibling elements within a parent element", testSiblingElementsWithinParent);
    suite.addTest("Testing empty name for startElement", testEmptyNameForStartElement);
    suite.addTest("Testing startElement with arrayBuffer overflow", testStartElementWithArrayBufferOverflow);

    return suite.run();
}

/**
 * Common function to perform XML serialization tests
 * @param serializerOperation Function to perform operations on the serializer
 * @param expectedResult Expected XML string result
 */
function performXmlTest(serializerOperation: (serializer: xml.XmlSerializer) => void, expectedResult: string): void {
    let arrayBuffer = new ArrayBuffer(MaxBufferLength);
    let serializer = new xml.XmlSerializer(arrayBuffer);
    
    serializerOperation(serializer);
    
    let view = new Uint8Array(arrayBuffer);
    let result = '';
    for (let i = 0; i < expectedResult.length; ++i) {
        result += String.fromCodePoint(view[i].toInt());
    }
    arktest.assertEQ(result, expectedResult);
}

function testInvalidEncodingForXmlSerializer() {
    let arrayBuffer = new ArrayBuffer(MaxBufferLength);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == TypeErrorCodeId &&
                e.message == "Parameter error.Just support utf-8";
        }
        return false;
    }
    arktest.expectThrow(() => {let thatSer = new xml.XmlSerializer(arrayBuffer, 'utf-16')}, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: XML_00600
function testSingleElementStartElement() {
    performXmlTest((serializer) => {
        serializer.startElement("note1");
        serializer.endElement();
    }, '<note1/>');
}

// Test cases ported from ArkTS 1.0: XML_00700
function testMultipleElementStartAndEndElements() {
    performXmlTest((serializer) => {
        serializer.startElement("note1");
        serializer.endElement();
        serializer.startElement("note2");
        serializer.endElement();
    }, '<note1/>\r\n<note2/>');
}

// Test cases ported from ArkTS 1.0: XML_00800
function testNestedElementStartAndEndElements() {
    performXmlTest((serializer) => {
        serializer.startElement("note1");
        serializer.startElement("note2");
        serializer.endElement();
        serializer.endElement();
    }, '<note1>\r\n  <note2/>\r\n</note1>');
}

// Test cases ported from ArkTS 1.0: XML_00900
function testDeeplyNestedElementStartAndEndElements() {
    performXmlTest((serializer) => {
        serializer.startElement("note1");
        serializer.startElement("note2");
        serializer.startElement("note3");
        serializer.endElement();
        serializer.endElement();
        serializer.endElement();
    }, '<note1>\r\n  <note2>\r\n    <note3/>\r\n  </note2>\r\n</note1>');
}

// Test cases ported from ArkTS 1.0: XML_01000
function testSiblingElementsWithinParent() {
    performXmlTest((serializer) => {
        serializer.startElement("note1");
        serializer.startElement("note2");
        serializer.endElement();
        serializer.startElement("note3");
        serializer.endElement();
        serializer.endElement();
    }, '<note1>\r\n  <note2/>\r\n  <note3/>\r\n</note1>');
}

function testEmptyNameForStartElement() {
    let arrayBuffer = new ArrayBuffer(MaxBufferLength);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == TypeErrorCodeId &&
                e.message == "Parameter error. Parameter cannot be empty";
        }
        return false;
    }
    arktest.expectThrow(() => {thatSer.startElement("")}, exceptionCheck);
}

function testStartElementWithArrayBufferOverflow() {
    let arrayBuffer = new ArrayBuffer(30);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.setNamespace("h", "aaa");
    thatSer.startElement("note");
    thatSer.endElement();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == TypeErrorCodeId &&
                e.message == "XmlSerializer:: StartElement memcpy_s failed";
        }
        return false;
    }
    arktest.expectThrow(() => {thatSer.startElement("note1")}, exceptionCheck);
}
