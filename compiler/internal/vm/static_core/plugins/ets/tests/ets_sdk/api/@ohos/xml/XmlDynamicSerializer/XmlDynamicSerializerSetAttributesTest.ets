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

    suite.addTest('Testing writing an element with a single attribute.', testWriteElementWithSingleAttribute);
    suite.addTest('Testing writing an element with multiple attributes.', testWriteElementWithMultipleAttributes);
    suite.addTest("Testing setting an attribute and write text inside an element.", testSetAttributeAndWriteText);
    suite.addTest("Testing setting attributes containing '>'.", testSetAttributesWithGreaterThan);
    suite.addTest("Testing setting attributes containing '<'.", testSetAttributesWithLessThan);
    suite.addTest("Testing setting attributes containing '&'.", testSetAttributesWithAmpersand);
    suite.addTest("Testing setting attributes containing single quote.", testSetAttributesWithSingleQuote);
    suite.addTest("Testing setting attributes containing double quote.", testSetAttributesWithDoubleQuote);

    return suite.run();
}

// Helper function to perform XML serialization and validation
function performXmlTest(setupSerializer: (serializer: xml.XmlDynamicSerializer) => void, expectedXml: string): void {
    let thatSer = new xml.XmlDynamicSerializer();
    setupSerializer(thatSer);
    let arrayBuffer = thatSer.getOutput();
    let view = new Uint8Array(arrayBuffer);
    let result = '';
    for (let i = 0; i < expectedXml.length; ++i) {
        result = result + String.fromCodePoint(view[i].toInt());
    }
    arktest.assertEQ(result, expectedXml);
}

// Test cases ported from ArkTS 1.0: XML_01100
function testWriteElementWithSingleAttribute() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setAttributes('importance1', 'high1');
        serializer.endElement();
    }, '<note importance1="high1"/>');
}

// Test cases ported from ArkTS 1.0: XML_01200
function testWriteElementWithMultipleAttributes() {
    performXmlTest((serializer) => {
        serializer.setDeclaration();
        serializer.startElement('note');
        serializer.setAttributes('importance1', 'high1');
        serializer.setAttributes('importance2', 'high2');
        serializer.endElement();
    }, '<?xml version="1.0" encoding="utf-8"?>\r\n<note importance1="high1" importance2="high2"/>');
}

// Test cases ported from ArkTS 1.0: XML_01300
function testSetAttributeAndWriteText() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setAttributes('importance1', 'high1');
        serializer.setText('cat');
        serializer.endElement();
    }, '<note importance1="high1">cat</note>');
}

function testSetAttributesWithGreaterThan() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setAttributes('importance', 'Hello>World');
        serializer.setText('high');
        serializer.endElement();
    }, '<note importance="Hello&gt;World">high</note>');
}

function testSetAttributesWithLessThan() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setAttributes('importance', 'Hello<World');
        serializer.setText('high');
        serializer.endElement();
    }, '<note importance="Hello&lt;World">high</note>');
}

function testSetAttributesWithAmpersand() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setAttributes('importance', 'Hello&World');
        serializer.setText('high');
        serializer.endElement();
    }, '<note importance="Hello&amp;World">high</note>');
}

function testSetAttributesWithSingleQuote() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setAttributes('importance', 'Hello\'World');
        serializer.setText('high');
        serializer.endElement();
    }, '<note importance="Hello&apos;World">high</note>');
}

function testSetAttributesWithDoubleQuote() {
    performXmlTest((serializer) => {
        serializer.startElement('note');
        serializer.setAttributes('importance', 'Hello"World');
        serializer.setText('high');
        serializer.endElement();
    }, '<note importance="Hello&quot;World">high</note>');
}