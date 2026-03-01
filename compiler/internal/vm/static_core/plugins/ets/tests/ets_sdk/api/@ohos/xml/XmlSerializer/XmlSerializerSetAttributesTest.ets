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

    suite.addTest("Testing single attribute in an element", testSingleAttributeInElement);
    suite.addTest("Testing multiple attributes in an element", testMultipleAttributesInElement);
    suite.addTest("Testing three attributes in an element", testThreeAttributesInElement);
    suite.addTest("Testing four attributes in an element", testFourAttributesInElement);
    suite.addTest("Testing non-sequential attributes in an element", testNonSequentialAttributesInElement);
    suite.addTest("Testing illegal position for setting attributes", testIllegalPositionForAttributes);
    suite.addTest("Testing empty attribute name", testEmptyAttributeName);
    suite.addTest("Testing attribute setting with arrayBuffer overflow", testAttributeSettingWithOverflow);
    suite.addTest("Test writing attributes containing '>'.", testWriteAttributesWithGreaterThan);
    suite.addTest("Test writing attributes containing '<'.", testWriteAttributesWithLessThan);
    suite.addTest("Test writing attributes containing '&'.", testWriteAttributesWithAmpersand);
    suite.addTest("Test writing attributes containing single quote.", testWriteAttributesWithSingleQuote);
    suite.addTest("Test writing attributes containing double quote.", testWriteAttributesWithDoubleQuote);

    return suite.run();
}

// Utility function to perform XML serialization tests
function performXmlTest(
    setupCallback: (serializer: xml.XmlSerializer) => void, 
    expectedXml: string, 
    bufferSize: number = MaxBufferLength.toDouble()
): void {
    const arrayBuffer = new ArrayBuffer(bufferSize);
    const serializer = new xml.XmlSerializer(arrayBuffer);
    
    setupCallback(serializer);
    
    const view = new Uint8Array(arrayBuffer);
    let actualXml = "";
    for (let i = 0; i < expectedXml.length; ++i) {
        actualXml += String.fromCodePoint(view[i].toInt());
    }
    
    arktest.assertEQ(actualXml, expectedXml);
}

// Test cases ported from ArkTS 1.0: XML_01100
function testSingleAttributeInElement() {
    performXmlTest(
        (serializer) => {
            serializer.startElement("note");
            serializer.setAttributes("importance1", "high1");
            serializer.endElement();
        },
        '<note importance1="high1"/>'
    );
}

// Test cases ported from ArkTS 1.0: XML_01200
function testMultipleAttributesInElement() {
    performXmlTest(
        (serializer) => {
            serializer.startElement("note");
            serializer.setAttributes("importance1", "high1");
            serializer.setAttributes("importance2", "high2");
            serializer.endElement();
        },
        '<note importance1="high1" importance2="high2"/>'
    );
}

// Test cases ported from ArkTS 1.0: XML_01300
function testThreeAttributesInElement() {
    performXmlTest(
        (serializer) => {
            serializer.startElement("note");
            serializer.setAttributes("importance1", "high1");
            serializer.setAttributes("importance2", "high2");
            serializer.setAttributes("importance3", "high3");
            serializer.endElement();
        },
        '<note importance1="high1" importance2="high2" importance3="high3"/>'
    );
}

// Test cases ported from ArkTS 1.0: XML_01400
function testFourAttributesInElement() {
    performXmlTest(
        (serializer) => {
            serializer.startElement("note");
            serializer.setAttributes("importance1", "high1");
            serializer.setAttributes("importance2", "high2");
            serializer.setAttributes("importance3", "high3");
            serializer.setAttributes("importance4", "high4");
            serializer.endElement();
        },
        '<note importance1="high1" importance2="high2" importance3="high3" importance4="high4"/>'
    );
}

// Test cases ported from ArkTS 1.0: XML_01500
function testNonSequentialAttributesInElement() {
    performXmlTest(
        (serializer) => {
            serializer.startElement("note");
            serializer.setAttributes("importance1", "high1");
            serializer.setAttributes("importance2", "high2");
            serializer.setAttributes("importance3", "high3");
            serializer.setAttributes("importance5", "high5");
            serializer.endElement();
        },
        '<note importance1="high1" importance2="high2" importance3="high3" importance5="high5"/>'
    );
}

function testIllegalPositionForAttributes() {
    let arrayBuffer = new ArrayBuffer(MaxBufferLength);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    thatSer.endElement();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == TypeErrorCodeId &&
                e.message == "Illegal position for attribute";
        }
        return false;
    }
    arktest.expectThrow(() => {thatSer.setAttributes("a","b")}, exceptionCheck);
}

function testEmptyAttributeName() {
    let arrayBuffer = new ArrayBuffer(MaxBufferLength);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == TypeErrorCodeId &&
                e.message == "Parameter error. Parameter cannot be empty";
        }
        return false;
    }
    arktest.expectThrow(() => {thatSer.setAttributes("","b")}, exceptionCheck);
}

function testAttributeSettingWithOverflow() {
    let arrayBuffer = new ArrayBuffer(35);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.setNamespace("h", "aaa");
    thatSer.startElement("note");
    thatSer.endElement();
    thatSer.startElement("note1");
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == TypeErrorCodeId &&
                e.message == "XmlSerializer:: SetAttributes memcpy_s failed";
        }
        return false;
    }
    arktest.expectThrow(() => {thatSer.setAttributes("a", "b")}, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: XML_11400
function testWriteAttributesWithGreaterThan() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.setAttributes('importance', 'Hello>World');
            serializer.setText('high');
            serializer.endElement();
        },
        '<note importance="Hello&gt;World">high</note>'
    );
}

// Test cases ported from ArkTS 1.0: XML_11500
function testWriteAttributesWithLessThan() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.setAttributes('importance', 'Hello<World');
            serializer.setText('high');
            serializer.endElement();
        },
        '<note importance="Hello&lt;World">high</note>'
    );
}

// Test cases ported from ArkTS 1.0: XML_11600
function testWriteAttributesWithAmpersand() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.setAttributes('importance', 'Hello&World');
            serializer.setText('high');
            serializer.endElement();
        },
        '<note importance="Hello&amp;World">high</note>'
    );
}

// Test cases ported from ArkTS 1.0: XML_11700
function testWriteAttributesWithSingleQuote() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.setAttributes('importance', 'Hello\'World');
            serializer.setText('high');
            serializer.endElement();
        },
        '<note importance="Hello&apos;World">high</note>'
    );
}

// Test cases ported from ArkTS 1.0: XML_11800
function testWriteAttributesWithDoubleQuote() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.setAttributes('importance', 'Hello"World');
            serializer.setText('high');
            serializer.endElement();
        },
        '<note importance="Hello&quot;World">high</note>'
    );
}
