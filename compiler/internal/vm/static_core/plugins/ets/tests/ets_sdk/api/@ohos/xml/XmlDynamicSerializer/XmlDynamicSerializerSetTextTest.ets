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

    suite.addTest('Test writing simple text content.', testSetSimpleText);
    suite.addTest('Test writing text with attributes.', testSetTextWithAttributes);
    suite.addTest('Test writing text with attributes and comments.', testSetTextWithAttributesAndComments);
    suite.addTest("Test writing text containing '>'.", testSetTextWithGreaterThan);
    suite.addTest("Test writing text containing '<'.", testSetTextWithLessThan);
    suite.addTest("Test writing text containing '&'.", testSetTextWithAmpersand);
    suite.addTest("Test writing text containing single quote.", testSetTextWithSingleQuote);
    suite.addTest("Test writing text containing double quote.", testSetTextWithDoubleQuote);

    return suite.run();
}

// Helper function to perform XML serialization and verification
function performXmlTest(operations: (serializer: xml.XmlDynamicSerializer) => void, expectedResult: string): void {
    let thatSer = new xml.XmlDynamicSerializer();
    operations(thatSer);
    let arrayBuffer = thatSer.getOutput();
    let view = new Uint8Array(arrayBuffer);
    let view1 = '';
    for (let i = 0; i < expectedResult.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i].toInt());
    }
    arktest.assertEQ(view1, expectedResult);
}

// Test cases ported from ArkTS 1.0: XML_04100
function testSetSimpleText() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.setText('Happy1');
            serializer.endElement();
        },
        '<note>Happy1</note>'
    );
}

// Test cases ported from ArkTS 1.0: XML_04200
function testSetTextWithAttributes() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.setAttributes('importance', 'high');
            serializer.setText('Happy2');
            serializer.endElement();
        },
        '<note importance="high">Happy2</note>'
    );
}

// Test cases ported from ArkTS 1.0: XML_04300
function testSetTextWithAttributesAndComments() {
    performXmlTest(
        (serializer) => {
            serializer.setComment('Hello, World!');
            serializer.startElement('note');
            serializer.setAttributes('importance', 'high');
            serializer.setText('Happy3');
            serializer.endElement();
        },
        '<!--Hello, World!-->\r\n<note importance="high">Happy3</note>'
    );
}

function testSetTextWithGreaterThan() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.setAttributes('importance', 'high');
            serializer.setText('Hello>World');
            serializer.endElement();
        },
        '<note importance="high">Hello&gt;World</note>'
    );
}

function testSetTextWithLessThan() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.setAttributes('importance', 'high');
            serializer.setText('Hello<World');
            serializer.endElement();
        },
        '<note importance="high">Hello&lt;World</note>'
    );
}

function testSetTextWithAmpersand() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.setAttributes('importance', 'high');
            serializer.setText('Hello&World');
            serializer.endElement();
        },
        '<note importance="high">Hello&amp;World</note>'
    );
}

function testSetTextWithSingleQuote() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.setAttributes('importance', 'high');
            serializer.setText("Hello'World");
            serializer.endElement();
        },
        '<note importance="high">Hello&apos;World</note>'
    );
}

function testSetTextWithDoubleQuote() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.setAttributes('importance', 'high');
            serializer.setText('Hello"World');
            serializer.endElement();
        },
        '<note importance="high">Hello&quot;World</note>'
    );
}
