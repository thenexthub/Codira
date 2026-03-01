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

    suite.addTest("Testing end tag without attributes", testEndElementWithoutAttributes);
    suite.addTest("Testing end tag with single attribute", testEndElementWithSingleAttribute);
    suite.addTest("Testing end tag with multiple attributes", testEndElementWithMultipleAttributes);
    suite.addTest("Testing nested end tags", testNestedEndElements);
    suite.addTest("Testing end tag with attributes in nested elements", testEndElementWithAttributesInNestedElements);
    suite.addTest("Testing end tag with arrayBuffer overflow", testEndElementWithArrayBufferOverflow);

    return suite.run();
}

/**
 * Helper function to create XmlSerializer, perform operations and verify result
 * @param bufferSize Size of the ArrayBuffer to use
 * @param operations A callback function that performs XML operations on the serializer
 * @param expectedResult The expected XML string output
 */
function performXmlTest(bufferSize: number, operations: (serializer: xml.XmlSerializer) => void, expectedResult: string) {
    let arrayBuffer = new ArrayBuffer(bufferSize);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    operations(thatSer);
    
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < expectedResult.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i].toInt());
    }
    arktest.assertEQ(view1, expectedResult);
}

// Test cases ported from ArkTS 1.0: XML_01600
function testEndElementWithoutAttributes() {
    performXmlTest(MaxBufferLength, (serializer) => {
        serializer.startElement("note");
        serializer.endElement();
    }, '<note/>');
}

// Test cases ported from ArkTS 1.0: XML_01700
function testEndElementWithSingleAttribute() {
    performXmlTest(MaxBufferLength, (serializer) => {
        serializer.startElement("note");
        serializer.setAttributes("importance", "high");
        serializer.endElement();
    }, '<note importance="high"/>');
}

// Test cases ported from ArkTS 1.0: XML_01800
function testEndElementWithMultipleAttributes() {
    performXmlTest(MaxBufferLength, (serializer) => {
        serializer.startElement("note");
        serializer.setAttributes("impo", "hi");
        serializer.endElement();
    }, '<note impo="hi"/>');
}

// Test cases ported from ArkTS 1.0: XML_01900
function testNestedEndElements() {
    performXmlTest(MaxBufferLength, (serializer) => {
        serializer.startElement("note1");
        serializer.startElement("note2");
        serializer.endElement();
        serializer.endElement();
    }, '<note1>\r\n  <note2/>\r\n</note1>');
}

// Test cases ported from ArkTS 1.0: XML_02000
function testEndElementWithAttributesInNestedElements() {
    performXmlTest(MaxBufferLength, (serializer) => {
        serializer.startElement("note2");
        serializer.setAttributes("importance", "high");
        serializer.endElement();
    }, '<note2 importance="high"/>');
}

function testEndElementWithArrayBufferOverflow() {
    let arrayBuffer = new ArrayBuffer(20);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.setNamespace("h", "aaa");
    thatSer.startElement("note");
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == TypeErrorCodeId &&
                e.message == 'XmlSerializer:: EndElement memcpy_s failed';
        }
        return false;
    }
    arktest.expectThrow(() => {thatSer.endElement()}, exceptionCheck);
}
